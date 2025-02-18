/*
 *
 * $Id: k3bstatusbarmanager.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bstatusbarmanager.h"
#include <k3bcore.h>
#include "k3bbusywidget.h"
#include "k3b.h"
#include <k3bversion.h>
#include <k3bglobals.h>
#include "k3bprojectmanager.h"
#include "k3bapplication.h"
#include <k3baudiodoc.h>
#include <k3bdatadoc.h>
#include <k3bmixeddoc.h>
#include <k3bvcddoc.h>
#include <k3bdiritem.h>
#include <k3bview.h>

#include <kiconloader.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdeio/global.h>
#include <kstatusbar.h>
#include <tdeaboutdata.h>
#include <tdeaction.h>

#include <tqlabel.h>
#include <tqhbox.h>
#include <tqfile.h>
#include <tqtimer.h>
#include <tqevent.h>
#include <tqtooltip.h>



K3bStatusBarManager::K3bStatusBarManager( K3bMainWindow* parent )
  : TQObject(parent),
    m_mainWindow(parent)
{
  // setup free temp space box
  TQHBox* boxFreeTemp = new TQHBox( m_mainWindow->statusBar() );
  boxFreeTemp->setSpacing(2);

  m_labelProjectInfo = new TQLabel( m_mainWindow->statusBar() );

  m_pixFreeTemp = new TQLabel( boxFreeTemp );
  (void)new TQLabel( i18n("Temp:"), boxFreeTemp );
  m_pixFreeTemp->setPixmap( SmallIcon("folder_green") );
  m_labelFreeTemp = new TQLabel( boxFreeTemp );
  boxFreeTemp->installEventFilter( this );

  // setup info area
  m_labelInfoMessage = new TQLabel( " ", m_mainWindow->statusBar() );

  // setup version info
  m_versionBox = new TQLabel( TQString("K3b %1").arg(kapp->aboutData()->version()), m_mainWindow->statusBar() );
  m_versionBox->installEventFilter( this );

  // setup the statusbar
  m_mainWindow->statusBar()->addWidget( m_labelInfoMessage, 1 ); // for showing some info
  m_mainWindow->statusBar()->addWidget( m_labelProjectInfo, 0, true );
  // a spacer item
  m_mainWindow->statusBar()->addWidget( new TQLabel( "  ", m_mainWindow->statusBar() ), 0, true );
  m_mainWindow->statusBar()->addWidget( boxFreeTemp, 0, true );
  // a spacer item
  m_mainWindow->statusBar()->addWidget( new TQLabel( "  ", m_mainWindow->statusBar() ), 0, true );
  m_mainWindow->statusBar()->addWidget( m_versionBox, 0, true );

  connect( m_mainWindow, TQ_SIGNAL(configChanged(TDEConfig*)), this, TQ_SLOT(update()) );
  connect( m_mainWindow->actionCollection(), TQ_SIGNAL(actionStatusText(const TQString&)),
	   this, TQ_SLOT(showActionStatusText(const TQString&)) );
  connect( m_mainWindow->actionCollection(), TQ_SIGNAL(clearStatusText()),
	   this, TQ_SLOT(clearActionStatusText()) );
  connect( k3bappcore->projectManager(), TQ_SIGNAL(activeProjectChanged(K3bDoc*)),
	   this, TQ_SLOT(slotActiveProjectChanged(K3bDoc*)) );
  connect( k3bappcore->projectManager(), TQ_SIGNAL(projectChanged(K3bDoc*)),
	   this, TQ_SLOT(slotActiveProjectChanged(K3bDoc*)) );
  connect( (m_updateTimer = new TQTimer( this )), TQ_SIGNAL(timeout()), this, TQ_SLOT(slotUpdateProjectStats()) );

  update();
}


K3bStatusBarManager::~K3bStatusBarManager()
{
}


void K3bStatusBarManager::update()
{
  TQString path = K3b::defaultTempPath();

  if( !TQFile::exists( path ) )
    path.truncate( path.findRev('/') );

  unsigned long size, avail;
  if( K3b::kbFreeOnFs( path, size, avail ) )
    slotFreeTempSpace( path, size, 0, avail );
  else
    m_labelFreeTemp->setText(i18n("No info"));

  if( path != TQToolTip::textFor( m_labelFreeTemp->parentWidget() ) ) {
    TQToolTip::remove( m_labelFreeTemp->parentWidget() );
    TQToolTip::add( m_labelFreeTemp->parentWidget(), path );
  }
}


void K3bStatusBarManager::slotFreeTempSpace(const TQString&,
					    unsigned long kbSize,
					    unsigned long,
					    unsigned long kbAvail)
{
  m_labelFreeTemp->setText(TDEIO::convertSizeFromKB(kbAvail)  + "/" +
	                   TDEIO::convertSizeFromKB(kbSize)  );

  // if we have less than 640 MB that is not good
  if( kbAvail < 655360 )
    m_pixFreeTemp->setPixmap( SmallIcon("folder_red") );
  else
    m_pixFreeTemp->setPixmap( SmallIcon("folder_green") );

  // update the display every second
  TQTimer::singleShot( 2000, this, TQ_SLOT(update()) );
}


void K3bStatusBarManager::showActionStatusText( const TQString& text )
{
  m_mainWindow->statusBar()->message( text );
}


void K3bStatusBarManager::clearActionStatusText()
{
  m_mainWindow->statusBar()->clear();
}


bool K3bStatusBarManager::eventFilter( TQObject* o, TQEvent* e )
{
  if( e->type() == TQEvent::MouseButtonDblClick ) {
    if( o == m_labelFreeTemp->parentWidget() )
      m_mainWindow->showOptionDialog( 0 );  // FIXME: use an enumeration for the option pages
    else if( o == m_versionBox )
      if( TDEAction* a = m_mainWindow->action( "help_about_app" ) )
	a->activate();
  }

  return TQObject::eventFilter( o, e );
}


static TQString dataDocStats( K3bDataDoc* dataDoc )
{
  return i18n("1 file in %1", "%n files in %1", dataDoc->root()->numFiles() )
    .arg( i18n("1 folder", "%n folders", dataDoc->root()->numDirs()+1 ) );
}


void K3bStatusBarManager::slotActiveProjectChanged( K3bDoc* doc )
{
  if( doc && doc == k3bappcore->projectManager()->activeProject() ) {
    // cache updates
    if( !m_updateTimer->isActive() ) {
      slotUpdateProjectStats();
      m_updateTimer->start( 1000, false );
    }
  }
  else if( !doc ) {
    m_labelProjectInfo->setText( TQString() );
  }
}


void K3bStatusBarManager::slotUpdateProjectStats()
{
  K3bDoc* doc = k3bappcore->projectManager()->activeProject();
  if( doc ) {
    switch( doc->type() ) {
    case K3bDoc::AUDIO: {
      K3bAudioDoc* audioDoc = static_cast<K3bAudioDoc*>( doc );
      m_labelProjectInfo->setText( i18n("Audio CD (1 track)", "Audio CD (%n tracks)", audioDoc->numOfTracks() ) );
      break;
    }

    case K3bDoc::DATA: {
      K3bDataDoc* dataDoc = static_cast<K3bDataDoc*>( doc );
      m_labelProjectInfo->setText( i18n("Data CD (%1)").arg(dataDocStats(dataDoc)) );
      break;
    }

    case K3bDoc::MIXED: {
      K3bAudioDoc* audioDoc = static_cast<K3bMixedDoc*>( doc )->audioDoc();
      K3bDataDoc* dataDoc = static_cast<K3bMixedDoc*>( doc )->dataDoc();
      m_labelProjectInfo->setText( i18n("Mixed CD (1 track and %1)", "Mixed CD (%n tracks and %1)", audioDoc->numOfTracks() )
				   .arg( dataDocStats(dataDoc)) );
      break;
    }

    case K3bDoc::VCD: {
      K3bVcdDoc* vcdDoc = static_cast<K3bVcdDoc*>( doc );
      m_labelProjectInfo->setText( i18n("Video CD (1 track)", "Video CD (%n tracks)", vcdDoc->numOfTracks() ) );
      break;
    }

    case K3bDoc::MOVIX: {
      K3bDataDoc* dataDoc = static_cast<K3bDataDoc*>( doc );
      m_labelProjectInfo->setText( i18n("eMovix CD (%1)").arg(dataDocStats(dataDoc)) );
      break;
    }

    case K3bDoc::MOVIX_DVD: {
      K3bDataDoc* dataDoc = static_cast<K3bDataDoc*>( doc );
      m_labelProjectInfo->setText( i18n("eMovix DVD (%1)").arg(dataDocStats(dataDoc)) );
      break;
    }

    case K3bDoc::DVD: {
      K3bDataDoc* dataDoc = static_cast<K3bDataDoc*>( doc );
      m_labelProjectInfo->setText( i18n("Data DVD (%1)").arg(dataDocStats(dataDoc)) );
      break;
    }
      
    case K3bDoc::VIDEODVD: {
      K3bDataDoc* dataDoc = static_cast<K3bDataDoc*>( doc );
      m_labelProjectInfo->setText( i18n("Video DVD (%1)").arg(dataDocStats(dataDoc)) );
      break;
    }
    }
  }
  else {
    m_labelProjectInfo->setText( TQString() );
  }
}

#include "k3bstatusbarmanager.moc"
