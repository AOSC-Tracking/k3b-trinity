/*
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bappdevicemanager.h"

#include <k3bdevice.h>
#include <k3bdevicehandler.h>
#include <k3bglobals.h>
#include <k3bapplication.h>
#include <k3bmediacache.h>

#include <tdeaction.h>
#include <kinputdialog.h>
#include <tdemessagebox.h>
#include <kinputdialog.h>
#include <tdeio/job.h>
#include <tdelocale.h>
#include <tdeio/global.h>
#include <tdepopupmenu.h>


K3bAppDeviceManager::K3bAppDeviceManager( TQObject* parent, const char* name )
  : K3bDevice::DeviceManager( parent, name ),
    m_currentDevice(0),
    m_ejectRequested(false)
{
  // FIXME: should we pass the mainwindow as watch window here?
  //        Is there a proper way to insert this actioncollection into the mainwindow's?
  m_actionCollection = new TDEActionCollection( this );

  // setup actions
  TDEActionMenu* devicePopupMenu = new TDEActionMenu( m_actionCollection, "device_popup" );
  m_actionDiskInfo = new TDEAction( i18n("Media &Info"), "application-vnd.tde.info", 0, this, TQ_SLOT(diskInfo()),
				  m_actionCollection, "device_diskinfo");
  m_actionUnmount = new TDEAction( i18n("&Unmount"), "media-optical-cdrom-unmounted", 0, this, TQ_SLOT(unmountDisk()),
				 m_actionCollection, "device_unmount");
  m_actionMount = new TDEAction( i18n("&Mount"), "media-optical-cdrom-mounted", 0, this, TQ_SLOT(mountDisk()),
			       m_actionCollection, "device_mount");
  m_actionEject = new TDEAction( i18n("&Eject"), "", 0, this, TQ_SLOT(ejectDisk()),
			       m_actionCollection, "device_eject");
  m_actionLoad = new TDEAction( i18n("L&oad"), "", 0, this, TQ_SLOT(loadDisk()),
			      m_actionCollection, "device_load");
//   TDEAction* actionUnlock = new TDEAction( i18n("Un&lock"), "", 0, this, TQ_SLOT(unlockDevice()),
// 				       m_actionCollection, "device_unlock" );
//   TDEAction* actionlock = new TDEAction( i18n("Loc&k"), "", 0, this, TQ_SLOT(lockDevice()),
// 				     m_actionCollection, "device_lock" );
  m_actionSetReadSpeed = new TDEAction( i18n("Set Read Speed..."), "", 0, this, TQ_SLOT(setReadSpeed()),
				      m_actionCollection, "device_set_read_speed" );

  m_actionDiskInfo->setToolTip( i18n("Display generic medium information") );
  m_actionUnmount->setToolTip( i18n("Unmount the medium") );
  m_actionMount->setToolTip( i18n("Mount the medium") );
  m_actionEject->setToolTip( i18n("Eject the medium") );
  m_actionLoad->setToolTip( i18n("(Re)Load the medium") );
  m_actionSetReadSpeed->setToolTip( i18n("Force the drive's read speed") );

  devicePopupMenu->insert( m_actionDiskInfo );
  devicePopupMenu->insert( new TDEActionSeparator( this ) );
  devicePopupMenu->insert( m_actionUnmount );
  devicePopupMenu->insert( m_actionMount );
  devicePopupMenu->insert( new TDEActionSeparator( this ) );
  devicePopupMenu->insert( m_actionEject );
  devicePopupMenu->insert( m_actionLoad );
//  devicePopupMenu->insert( new TDEActionSeparator( this ) );
//  devicePopupMenu->insert( actionUnlock );
//  devicePopupMenu->insert( actionlock );
  devicePopupMenu->insert( new TDEActionSeparator( this ) );
  devicePopupMenu->insert( m_actionSetReadSpeed );

  setCurrentDevice( 0 );
}


void K3bAppDeviceManager::setMediaCache( K3bMediaCache* c )
{
  connect( c, TQ_SIGNAL(mediumChanged(K3bDevice::Device*)),
	   this, TQ_SLOT(slotMediumChanged(K3bDevice::Device*)) );
}


int K3bAppDeviceManager::scanBus()
{
  return K3bDevice::DeviceManager::scanBus();
}


K3bDevice::Device* K3bAppDeviceManager::currentDevice() const
{
  return m_currentDevice;
}


void K3bAppDeviceManager::clear()
{
  // make sure we do not use a deleted device
  m_currentDevice = 0;
  K3bDevice::DeviceManager::clear();
}


void K3bAppDeviceManager::removeDevice( const TQString& dev )
{
  if( m_currentDevice == findDevice(dev) )
    m_currentDevice = 0;

  K3bDevice::DeviceManager::removeDevice( dev );

  if( !m_currentDevice )
    setCurrentDevice( allDevices().getFirst() );
}


K3bAppDeviceManager::~K3bAppDeviceManager()
{
}


void K3bAppDeviceManager::slotMediumChanged( K3bDevice::Device* dev )
{
  m_actionDiskInfo->setEnabled( dev != 0 );
  m_actionUnmount->setEnabled( dev != 0 );
  m_actionMount->setEnabled( dev != 0 );
  m_actionEject->setEnabled( dev != 0 );
  m_actionLoad->setEnabled( dev != 0 );
  m_actionSetReadSpeed->setEnabled( dev != 0 );

  if( dev && dev == currentDevice() ) {
    bool mounted = K3b::isMounted( dev );
    bool mediumMountable = k3bappcore->mediaCache()->medium( dev ).content() & K3bMedium::CONTENT_DATA;
    m_actionMount->setEnabled( !mounted && mediumMountable );
    m_actionUnmount->setEnabled( mounted );
  }
}


void K3bAppDeviceManager::setCurrentDevice( K3bDevice::Device* dev )
{
  if( dev && dev != m_currentDevice ) {
    m_currentDevice = dev;
    emit currentDeviceChanged( dev );
  }

  slotMediumChanged( dev );
}


void K3bAppDeviceManager::diskInfo()
{
  if( currentDevice() ) {
    emit detectingDiskInfo( currentDevice() );
  }
}


void K3bAppDeviceManager::unlockDevice()
{
  if( currentDevice() )
    K3bDevice::unblock( currentDevice() );
}


void K3bAppDeviceManager::lockDevice()
{
  if( currentDevice() )
    K3bDevice::block( currentDevice() );
}


void K3bAppDeviceManager::mountDisk()
{
    if ( currentDevice() ) {
        // FIXME: make this non-blocking
        if( !K3b::isMounted( currentDevice() ) )
            K3b::mount( currentDevice() );

        emit mountFinished( TDEIO::findDeviceMountPoint( currentDevice()->blockDeviceName() ) );
    }
}


void K3bAppDeviceManager::unmountDisk()
{
    if ( currentDevice() ) {
        // FIXME: make this non-blocking
        if( K3b::isMounted( currentDevice() ) )
            emit unmountFinished( K3b::unmount( currentDevice() ) );
        else
            emit unmountFinished( true );
    }
}


void K3bAppDeviceManager::ejectDisk()
{
  // FIXME: make this non-blocking
    if ( currentDevice() )
        K3b::eject( currentDevice() ); // just ignore errors here
}


void K3bAppDeviceManager::loadDisk()
{
  if( currentDevice() )
    K3bDevice::reload( currentDevice() );
}


void K3bAppDeviceManager::setReadSpeed()
{
  if( currentDevice() ) {
    bool ok = false;
    int s = KInputDialog::getInteger( i18n("CD Read Speed"),
				      i18n("<p>Please enter the preferred read speed for <b>%1</b>. "
					   "This speed will be used for the currently mounted "
					   "medium."
					   "<p>This is especially useful to slow down the drive when "
					   "watching movies which are read directly from the drive "
					   "and the spinning noise is intrusive."
					   "<p>Be aware that this has no influence on K3b since it will "
					   "change the reading speed again when copying CDs or DVDs.")
				      .arg(currentDevice()->vendor() + " " + currentDevice()->description()),
				      12,
				      1,
				      currentDevice()->maxReadSpeed(),
				      1,
				      10,
				      &ok,
				      0 );
    if( ok ) {
      if( !currentDevice()->setSpeed( s*175, 0xFFFF ) )
	KMessageBox::error( 0, i18n("Setting the read speed failed.") );
    }
  }
}


void K3bAppDeviceManager::diskInfo( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  diskInfo();
}


void K3bAppDeviceManager::unlockDevice( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  unlockDevice();
}


void K3bAppDeviceManager::lockDevice( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  lockDevice();
}


void K3bAppDeviceManager::mountDisk( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  mountDisk();
}


void K3bAppDeviceManager::unmountDisk( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  unmountDisk();
}


void K3bAppDeviceManager::ejectDisk( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  ejectDisk();
}


void K3bAppDeviceManager::loadDisk( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  loadDisk();
}


void K3bAppDeviceManager::setReadSpeed( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  setReadSpeed();
}

#include "k3bappdevicemanager.moc"
