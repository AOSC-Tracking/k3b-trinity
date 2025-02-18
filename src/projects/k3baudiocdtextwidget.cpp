/*
 *
 * $Id: k3baudiocdtextwidget.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudiocdtextwidget.h"
#include "base_k3baudiocdtextallfieldswidget.h"

#include "k3baudiodoc.h"
#include "k3baudiotrack.h"
#include <k3bcdtextvalidator.h>

#include <tqcheckbox.h>
#include <tqtoolbutton.h>
#include <tqpushbutton.h>
#include <tqptrlist.h>
#include <tqlayout.h>
#include <tqgroupbox.h>

#include <klineedit.h>
#include <tdelocale.h>
#include <kdialogbase.h>
#include <kiconloader.h>


class K3bAudioCdTextWidget::AllFieldsDialog : public KDialogBase
{
public:
  AllFieldsDialog( TQWidget* parent )
    : KDialogBase( parent,
		   "cdtext_allfields_dialog",
		   true,
		   i18n("CD-Text"),
		   Ok|Cancel,
		   Ok,
		   true ) {
    w = new base_K3bAudioCdTextAllFieldsWidget( this );
    setMainWidget( w );
  }

  base_K3bAudioCdTextAllFieldsWidget* w;
};


K3bAudioCdTextWidget::K3bAudioCdTextWidget( TQWidget* parent, const char* name )
  : base_K3bAudioCdTextWidget( parent, name ),
    m_doc(0)
{
  m_allFieldsDlg = new AllFieldsDialog( this );

  m_buttonCopyTitle->setPixmap( SmallIcon( "edit-copy" ) );
  m_buttonCopyPerformer->setPixmap( SmallIcon( "edit-copy" ) );

  m_allFieldsDlg->w->m_buttonCopyTitle->setPixmap( SmallIcon( "edit-copy" ) );
  m_allFieldsDlg->w->m_buttonCopyPerformer->setPixmap( SmallIcon( "edit-copy" ) );
  m_allFieldsDlg->w->m_buttonCopySongwriter->setPixmap( SmallIcon( "edit-copy" ) );
  m_allFieldsDlg->w->m_buttonCopyComposer->setPixmap( SmallIcon( "edit-copy" ) );
  m_allFieldsDlg->w->m_buttonCopyArranger->setPixmap( SmallIcon( "edit-copy" ) );

  TQValidator* cdTextVal = new K3bCdTextValidator( this );
  m_editTitle->setValidator( cdTextVal );
  m_editPerformer->setValidator( cdTextVal );

  m_allFieldsDlg->w->m_editTitle->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editPerformer->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editDisc_id->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editUpc_ean->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editMessage->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editArranger->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editSongwriter->setValidator( cdTextVal );
  m_allFieldsDlg->w->m_editComposer->setValidator( cdTextVal );

  connect( m_allFieldsDlg->w->m_buttonCopyTitle, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotCopyTitle()) );
  connect( m_allFieldsDlg->w->m_buttonCopyPerformer, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotCopyPerformer()) );
  connect( m_allFieldsDlg->w->m_buttonCopyArranger, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotCopyArranger()) );
  connect( m_allFieldsDlg->w->m_buttonCopySongwriter, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotCopySongwriter()) );
  connect( m_allFieldsDlg->w->m_buttonCopyComposer, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotCopyComposer()) );

  connect( m_buttonMoreFields, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotMoreFields()) );
}


K3bAudioCdTextWidget::~K3bAudioCdTextWidget()
{
}

void K3bAudioCdTextWidget::load( K3bAudioDoc* doc )
{
  m_doc = doc;
  m_groupCdText->setChecked( doc->cdText() );

  m_editTitle->setText( doc->title() );
  m_editPerformer->setText( doc->artist() );

  m_allFieldsDlg->w->m_editTitle->setText( doc->title() );
  m_allFieldsDlg->w->m_editPerformer->setText( doc->artist() );
  m_allFieldsDlg->w->m_editDisc_id->setText( doc->disc_id() );
  m_allFieldsDlg->w->m_editUpc_ean->setText( doc->upc_ean() );
  m_allFieldsDlg->w->m_editArranger->setText( doc->arranger() );
  m_allFieldsDlg->w->m_editSongwriter->setText( doc->songwriter() );
  m_allFieldsDlg->w->m_editComposer->setText( doc->composer() );
  m_allFieldsDlg->w->m_editMessage->setText( doc->cdTextMessage() );
}

void K3bAudioCdTextWidget::save( K3bAudioDoc* doc )
{
  m_doc = doc;
  doc->writeCdText( m_groupCdText->isChecked() );

  // we save the title and artist values from the main view since
  // the dialog is only updated before it is shown
  doc->setTitle( m_editTitle->text() );
  doc->setArtist( m_editPerformer->text() );
  doc->setDisc_id( m_allFieldsDlg->w->m_editDisc_id->text() );
  doc->setUpc_ean( m_allFieldsDlg->w->m_editUpc_ean->text() );
  doc->setArranger( m_allFieldsDlg->w->m_editArranger->text() );
  doc->setSongwriter( m_allFieldsDlg->w->m_editSongwriter->text() );
  doc->setComposer( m_allFieldsDlg->w->m_editComposer->text() );
  doc->setCdTextMessage( m_allFieldsDlg->w->m_editMessage->text() );
}


void K3bAudioCdTextWidget::slotMoreFields()
{
  // update dlg to current state
  m_allFieldsDlg->w->m_editTitle->setText( m_editTitle->text() );
  m_allFieldsDlg->w->m_editPerformer->setText( m_editPerformer->text() );

  // save old settings
  TQString title = m_allFieldsDlg->w->m_editTitle->text();
  TQString performer = m_allFieldsDlg->w->m_editPerformer->text();
  TQString disc_id = m_allFieldsDlg->w->m_editDisc_id->text();
  TQString upc_ean = m_allFieldsDlg->w->m_editUpc_ean->text();
  TQString arranger = m_allFieldsDlg->w->m_editArranger->text();
  TQString songwriter = m_allFieldsDlg->w->m_editSongwriter->text();
  TQString composer = m_allFieldsDlg->w->m_editComposer->text();
  TQString message = m_allFieldsDlg->w->m_editMessage->text();

  // exec dlg
  if( m_allFieldsDlg->exec() == TQDialog::Accepted ) {
    // accept new entries
    m_editTitle->setText( m_allFieldsDlg->w->m_editTitle->text() );
    m_editPerformer->setText( m_allFieldsDlg->w->m_editPerformer->text() );
  }
  else {
    // reset
    m_allFieldsDlg->w->m_editTitle->setText( title );
    m_allFieldsDlg->w->m_editPerformer->setText( performer );
    m_allFieldsDlg->w->m_editDisc_id->setText( disc_id );
    m_allFieldsDlg->w->m_editUpc_ean->setText( upc_ean );
    m_allFieldsDlg->w->m_editArranger->setText( arranger );
    m_allFieldsDlg->w->m_editSongwriter->setText( songwriter );
    m_allFieldsDlg->w->m_editComposer->setText( composer );
    m_allFieldsDlg->w->m_editMessage->setText( message );
  }
}


void K3bAudioCdTextWidget::setChecked( bool b )
{
  m_groupCdText->setChecked( b );
}

bool K3bAudioCdTextWidget::isChecked() const
{
  return m_groupCdText->isChecked();
}


void K3bAudioCdTextWidget::slotCopyTitle()
{
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    track->setTitle( m_allFieldsDlg->isVisible() 
		     ? m_allFieldsDlg->w->m_editTitle->text()
		     : m_editTitle->text() );
    track = track->next();
  }
}

void K3bAudioCdTextWidget::slotCopyPerformer()
{
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    track->setPerformer( m_allFieldsDlg->isVisible() 
			 ? m_allFieldsDlg->w->m_editPerformer->text() 
			 : m_editPerformer->text() );
    track = track->next();
  }
}

void K3bAudioCdTextWidget::slotCopyArranger()
{
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    track->setArranger( m_allFieldsDlg->w->m_editArranger->text() );
    track = track->next();
  }
}

void K3bAudioCdTextWidget::slotCopySongwriter()
{
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    track->setSongwriter( m_allFieldsDlg->w->m_editSongwriter->text() );
    track = track->next();
  }
}

void K3bAudioCdTextWidget::slotCopyComposer()
{
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    track->setComposer( m_allFieldsDlg->w->m_editComposer->text() );
    track = track->next();
  }
}


#include "k3baudiocdtextwidget.moc"
