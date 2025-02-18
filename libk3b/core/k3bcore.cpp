/* 
 *
 * $Id: k3bcore.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <config.h>

#include "k3bcore.h"
#include "k3bjob.h"

#include <k3bdevicemanager.h>
#include <k3bconnection.h>
#include <k3bexternalbinmanager.h>
#include <k3bdefaultexternalprograms.h>
#include <k3bglobals.h>
#include <k3bversion.h>
#include <k3bjob.h>
#include <k3bthreadwidget.h>
#include <k3bglobalsettings.h>
#include <k3bpluginmanager.h>

#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdeaboutdata.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>

#include <tqptrlist.h>
#include <tqthread.h>
#include <tqmutex.h>


static TQt::HANDLE s_guiThreadHandle = TQThread::currentThread();

// We cannot use TQWaitCondition here since the event might be handled faster
// than the thread starts the waiting
class DeviceBlockingEventDoneCondition {
public:
  DeviceBlockingEventDoneCondition()
    : m_done(false) {
  }

  void done() {
    m_doneMutex.lock();
    m_done = true;
    m_doneMutex.unlock();
  }

  void wait() {
    while( true ) {
      m_doneMutex.lock();
      bool done = m_done;
      m_doneMutex.unlock();
      if( done )
	return;
    }
  }

private:
  TQMutex m_doneMutex;
  bool m_done;
};

class DeviceBlockingEvent : public TQCustomEvent
{
public:
  DeviceBlockingEvent( bool block_, K3bDevice::Device* dev, DeviceBlockingEventDoneCondition* cond_, bool* success_ )
    : TQCustomEvent( TQEvent::User + 33 ),
      block(block_),
      device(dev),
      cond(cond_),
      success(success_) {
  }

  bool block;
  K3bDevice::Device* device;
  DeviceBlockingEventDoneCondition* cond;
  bool* success;
};


class K3bCore::Private {
public:
  Private()
    : version( LIBK3B_VERSION ),
      config(0),
      deleteConfig(false),
      deviceManager(0),
      externalBinManager(0),
      pluginManager(0),
      globalSettings(0) {
  }

  K3bVersion version;
  TDEConfig* config;
  bool deleteConfig;
  K3bDevice::DeviceManager* deviceManager;
  K3bExternalBinManager* externalBinManager;
  K3bPluginManager* pluginManager;
  K3bGlobalSettings* globalSettings;

  TQValueList<K3bJob*> runningJobs;
  TQValueList<K3bDevice::Device*> blockedDevices;
};



K3bCore* K3bCore::s_k3bCore = 0;



K3bCore::K3bCore( TQObject* parent, const char* name )
  : TQObject( parent, name )
{
  d = new Private();

  if( s_k3bCore )
    tqFatal("ONLY ONE INSTANCE OF K3BCORE ALLOWED!");
  s_k3bCore = this;

  // create the thread widget instance in the GUI thread
  K3bThreadWidget::instance();
}


K3bCore::~K3bCore()
{
  s_k3bCore = 0;

  delete d->globalSettings;
  delete d;
}


K3bDevice::DeviceManager* K3bCore::deviceManager() const
{
  const_cast<K3bCore*>(this)->initDeviceManager();
  return d->deviceManager;
}


K3bExternalBinManager* K3bCore::externalBinManager() const
{
  const_cast<K3bCore*>(this)->initExternalBinManager();
  return d->externalBinManager;
}


K3bPluginManager* K3bCore::pluginManager() const
{
  const_cast<K3bCore*>(this)->initPluginManager();
  return d->pluginManager;
}


K3bGlobalSettings* K3bCore::globalSettings() const
{
  const_cast<K3bCore*>(this)->initGlobalSettings();
  return d->globalSettings;
}


const K3bVersion& K3bCore::version() const
{
  return d->version;
}


TDEConfig* K3bCore::config() const
{
  if( !d->config ) {
    kdDebug() << "(K3bCore) opening k3b config file." << endl;
    kdDebug() << "(K3bCore) while I am a " << className() << endl;
    d->deleteConfig = true;
    d->config = new TDEConfig( "k3brc" );
  }

  return d->config;
}


void K3bCore::init()
{
  initGlobalSettings();
  initExternalBinManager();
  initDeviceManager();
  initPluginManager();

  // load the plugins before doing anything else
  // they might add external bins
  pluginManager()->loadAll();

  externalBinManager()->search();

  connect( K3bDevice::Connection::instance(), TQ_SIGNAL(deviceAdded(const TQString&)),
	   deviceManager(), TQ_SLOT(addDevice(const TQString&)) );
  connect( K3bDevice::Connection::instance(), TQ_SIGNAL(deviceRemoved(const TQString&)),
	   deviceManager(), TQ_SLOT(removeDevice(const TQString&)) );
  TQStringList devList = K3bDevice::Connection::instance()->devices();
  if( devList.isEmpty() )
    deviceManager()->scanBus();
  else
    for( unsigned int i = 0; i < devList.count(); ++i )
      deviceManager()->addDevice( devList[i] );
}


void K3bCore::initGlobalSettings()
{
  if( !d->globalSettings )
    d->globalSettings = new K3bGlobalSettings();
}


void K3bCore::initExternalBinManager()
{
  if( !d->externalBinManager ) {
    d->externalBinManager = new K3bExternalBinManager( this );
    K3b::addDefaultPrograms( d->externalBinManager );
  }
}


void K3bCore::initDeviceManager()
{
  if( !d->deviceManager )
    d->deviceManager = new K3bDevice::DeviceManager( this );
}


void K3bCore::initPluginManager()
{
  if( !d->pluginManager )
    d->pluginManager = new K3bPluginManager( this );
}


void K3bCore::readSettings( TDEConfig* cnf )
{
  TDEConfig* c = cnf;
  if( !c )
    c = config();

  TQString oldGrp = c->group();

  globalSettings()->readSettings( c );
  deviceManager()->readConfig( c );
  externalBinManager()->readConfig( c );

  c->setGroup( oldGrp );
}


void K3bCore::saveSettings( TDEConfig* cnf )
{
  TDEConfig* c = cnf;
  if( !c )
    c = config();

  TQString oldGrp = c->group();

  c->setGroup( "General Options" );
  c->writeEntry( "config version", version() );

  deviceManager()->saveConfig( c );
  externalBinManager()->saveConfig( c );
  d->globalSettings->saveSettings( c );

  c->setGroup( oldGrp );
}


void K3bCore::registerJob( K3bJob* job )
{
  d->runningJobs.append( job );
  emit jobStarted( job );
  if( K3bBurnJob* bj = dynamic_cast<K3bBurnJob*>( job ) )
    emit burnJobStarted( bj );
}


void K3bCore::unregisterJob( K3bJob* job )
{
  d->runningJobs.remove( job );
  emit jobFinished( job );
  if( K3bBurnJob* bj = dynamic_cast<K3bBurnJob*>( job ) )
    emit burnJobFinished( bj );
}


bool K3bCore::jobsRunning() const
{
  return !d->runningJobs.isEmpty();
}


const TQValueList<K3bJob*>& K3bCore::runningJobs() const
{
  return d->runningJobs;
}


bool K3bCore::blockDevice( K3bDevice::Device* dev )
{
  if( TQThread::currentThread() == s_guiThreadHandle ) {
    return internalBlockDevice( dev );
  }
  else {
    bool success = false;
    DeviceBlockingEventDoneCondition w;
    TQApplication::postEvent( this, new DeviceBlockingEvent( true, dev, &w, &success ) );
    w.wait();
    return success;
  }
}


void K3bCore::unblockDevice( K3bDevice::Device* dev )
{
  if( TQThread::currentThread() == s_guiThreadHandle ) {
    internalUnblockDevice( dev );
  }
  else {
    DeviceBlockingEventDoneCondition w;
    TQApplication::postEvent( this, new DeviceBlockingEvent( false, dev, &w, 0 ) );
    w.wait();
  }
}


bool K3bCore::internalBlockDevice( K3bDevice::Device* dev )
{
  if( !d->blockedDevices.contains( dev ) ) {
    d->blockedDevices.append( dev );
    return true;
  }
  else
    return false;
}


void K3bCore::internalUnblockDevice( K3bDevice::Device* dev )
{
  d->blockedDevices.remove( dev );
}


void K3bCore::customEvent( TQCustomEvent* e )
{
  if( DeviceBlockingEvent* de = dynamic_cast<DeviceBlockingEvent*>(e) ) {
    if( de->block )
      *de->success = internalBlockDevice( de->device );
    else
      internalUnblockDevice( de->device );
    de->cond->done();
  }
}

#include "k3bcore.moc"
