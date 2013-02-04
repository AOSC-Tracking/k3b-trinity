/* 
 *
 * $Id: k3baudiotracktrmlookupdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <config.h>

#ifdef HAVE_MUSICBRAINZ

#include "k3baudiotracktrmlookupdialog.h"
#include "k3bmusicbrainzjob.h"

#include <k3bbusywidget.h>
#include <k3baudiotrack.h>
#include <k3baudiofile.h>
#include <k3bpassivepopup.h>

#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kdebug.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqframe.h>
#include <tqeventloop.h>
#include <tqpushbutton.h>
#include <tqapplication.h>


K3bAudioTrackTRMLookupDialog::K3bAudioTrackTRMLookupDialog( TQWidget* parent, const char* name )
  : KDialogBase( KDialogBase::Plain, 
		 i18n("MusicBrainz Query"), 
		 KDialogBase::Cancel,
		 KDialogBase::Cancel,
		 parent, 
		 name,
		 true,
		 true )
{
  TQGridLayout* grid = new TQGridLayout( plainPage() );
  grid->setMargin( marginHint() );
  grid->setSpacing( spacingHint() );

  m_infoLabel = new TQLabel( plainPage() );
  TQLabel* pixLabel = new TQLabel( plainPage() );
  pixLabel->setPixmap( TDEGlobal::iconLoader()->loadIcon( "musicbrainz", TDEIcon::NoGroup, 64 ) );
  pixLabel->setScaledContents( false );

  m_busyWidget = new K3bBusyWidget( plainPage() );

  grid->addMultiCellWidget( pixLabel, 0, 1, 0, 0 );
  grid->addWidget( m_infoLabel, 0, 1 );
  grid->addWidget( m_busyWidget, 1, 1 );

  m_inLoop = false;
  m_mbJob = new K3bMusicBrainzJob( this );
  connect( m_mbJob, TQT_SIGNAL(infoMessage(const TQString&, int)), 
	   this, TQT_SLOT(slotMbJobInfoMessage(const TQString&, int)) );
  connect( m_mbJob, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotMbJobFinished(bool)) );
  connect( m_mbJob, TQT_SIGNAL(trackFinished(K3bAudioTrack*, bool)), 
	   this, TQT_SLOT(slotTrackFinished(K3bAudioTrack*, bool)) );
}


K3bAudioTrackTRMLookupDialog::~K3bAudioTrackTRMLookupDialog()
{
}


int K3bAudioTrackTRMLookupDialog::lookup( const TQPtrList<K3bAudioTrack>& tracks )
{
  m_mbJob->setTracks( tracks );
  m_mbJob->start();

  m_busyWidget->showBusy(true);
  setModal( true );
  show();
  m_inLoop = true;
  TQApplication::eventLoop()->enterLoop();

  return 0;
}


void K3bAudioTrackTRMLookupDialog::slotMbJobInfoMessage( const TQString& message, int )
{
  m_infoLabel->setText( message );
}


void K3bAudioTrackTRMLookupDialog::slotMbJobFinished( bool )
{
  m_busyWidget->showBusy(false);
  hide();
  if( m_inLoop )
    TQApplication::eventLoop()->exitLoop();
}


void K3bAudioTrackTRMLookupDialog::slotCancel()
{
  actionButton( Cancel )->setEnabled( false );
  m_mbJob->cancel();
}


void K3bAudioTrackTRMLookupDialog::slotTrackFinished( K3bAudioTrack* track, bool success )
{
  if( !success )
    K3bPassivePopup::showPopup( i18n("Track %1 was not found in the MusicBrainz database.")
				.arg( track->trackNumber()),
				i18n("Audio Project") );
}

#include "k3baudiotracktrmlookupdialog.moc"

#endif
