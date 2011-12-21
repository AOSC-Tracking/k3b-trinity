/* 
 *
 * $Id: k3baudiotracksplitdialog.cpp 630444 2007-02-05 12:43:19Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudiotracksplitdialog.h"
#include "k3baudiotrack.h"
#include "k3baudioeditorwidget.h"

#include <k3bmsf.h>
#include <k3bmsfedit.h>

#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include <tqlabel.h>
#include <tqframe.h>
#include <tqlayout.h>


K3bAudioTrackSplitDialog::K3bAudioTrackSplitDialog( K3bAudioTrack* track, TQWidget* parent, const char* name )
  : KDialogBase( KDialogBase::Plain, i18n("Split Audio Track"), 
		 KDialogBase::Ok|KDialogBase::Cancel,
		 KDialogBase::Ok, parent, name ),
    m_track(track)
{
  TQFrame* frame = plainPage();
  
  m_editorWidget = new K3bAudioEditorWidget( frame );
  m_msfEditStart = new K3bMsfEdit( frame );
  m_msfEditEnd = new K3bMsfEdit( frame );

  TQGridLayout* layout = new TQGridLayout( frame );
  layout->setMargin( 0 );
  layout->setSpacing( spacingHint() );

  // FIXME: After the string freeze replace the text with a better one explaning how to use this dialog
  layout->addMultiCellWidget( new TQLabel( i18n("Please select the position where the track should be split."),
			      frame ), 0, 0, 0, 3 );
  layout->addMultiCellWidget( m_editorWidget, 1, 1, 0, 3 );
  layout->addWidget( m_msfEditStart, 2, 1 );
  layout->addWidget( new TQLabel( " - ", frame ), 2, 2 );
  layout->addWidget( m_msfEditEnd, 2, 3 );
  layout->addWidget( new TQLabel( i18n("Split track at:"), frame ), 2, 0 );
  layout->setColStretch( 0, 1 );

  m_editorWidget->setAllowOverlappingRanges( false );
  m_editorWidget->enableRangeSelection( true );
  m_editorWidget->installEventFilter( this );

  connect( m_editorWidget, TQT_SIGNAL(rangeChanged(int, const K3b::Msf&, const K3b::Msf&)),
	   this, TQT_SLOT(slotRangeModified(int, const K3b::Msf&, const K3b::Msf&)) );
  connect( m_editorWidget, TQT_SIGNAL(selectedRangeChanged(int)),
	   this, TQT_SLOT(slotRangeSelectionChanged(int)) );
  connect( m_msfEditStart, TQT_SIGNAL(valueChanged(const K3b::Msf&)),
	   this, TQT_SLOT(slotMsfEditChanged(const K3b::Msf&)) );
  connect( m_msfEditEnd, TQT_SIGNAL(valueChanged(const K3b::Msf&)),
	   this, TQT_SLOT(slotMsfEditChanged(const K3b::Msf&)) );

  setupActions();

  // load the track
  m_editorWidget->setLength( m_track->length() );

  // default split
  K3b::Msf mid = m_track->length().lba() / 2;
  m_editorWidget->addRange( 0, mid-1 );
  m_editorWidget->addRange( mid, m_track->length()-1 );

  slotRangeSelectionChanged( 0 );
}


K3bAudioTrackSplitDialog::~K3bAudioTrackSplitDialog()
{
}


void K3bAudioTrackSplitDialog::setupActions()
{
  m_actionCollection = new KActionCollection( this );
  m_popupMenu = new KPopupMenu( this );

  KAction* actionSplitHere = new KAction( i18n("Split Here"), 0,
					  KShortcut(), TQT_TQOBJECT(this), TQT_SLOT(slotSplitHere()),
					  actionCollection(), "range_split" );
  // FIXME: after the message freeze give this action a proper name like "Remove track part"
  KAction* actionRemoveRange = new KAction( i18n("Remove this Range"), 0,
					    KShortcut(), TQT_TQOBJECT(this), TQT_SLOT(slotRemoveRange()),
					    actionCollection(), "range_remove" );

  actionSplitHere->plug( m_popupMenu );
  actionRemoveRange->plug( m_popupMenu );
}


void K3bAudioTrackSplitDialog::slotRangeModified( int id, const K3b::Msf& start, const K3b::Msf& end )
{
  if( id == m_editorWidget->selectedRange() ) {
    m_msfEditStart->blockSignals( true );
    m_msfEditEnd->blockSignals( true );
    
    m_msfEditStart->setMsfValue( start );
    m_msfEditEnd->setMsfValue( end );
    
    m_msfEditStart->blockSignals( false );
    m_msfEditEnd->blockSignals( false );
  }
}


void K3bAudioTrackSplitDialog::slotMsfEditChanged( const K3b::Msf& )
{
  m_editorWidget->modifyRange( m_editorWidget->selectedRange(), m_msfEditStart->msfValue(), m_msfEditEnd->msfValue() );
}


void K3bAudioTrackSplitDialog::slotRangeSelectionChanged( int id )
{
  if( id > 0 ) {
    m_msfEditStart->blockSignals( true );
    m_msfEditEnd->blockSignals( true );

    m_msfEditStart->setMsfValue( m_editorWidget->rangeStart( id ) );
    m_msfEditEnd->setMsfValue( m_editorWidget->rangeEnd( id ) );
    m_msfEditStart->setEnabled( true );
    m_msfEditEnd->setEnabled( true );

    m_msfEditStart->blockSignals( false );
    m_msfEditEnd->blockSignals( false );
  }
  else {
    m_msfEditStart->setEnabled( false );
    m_msfEditEnd->setEnabled( false );
  }
}


void K3bAudioTrackSplitDialog::splitAt( const TQPoint& p )
{
  int id = m_editorWidget->findRange( p.x() );
  if( id ) {
    K3b::Msf msf = m_editorWidget->posToMsf( p.x() );
    m_editorWidget->addRange( msf+1, m_editorWidget->rangeEnd( id ) );
    m_editorWidget->modifyRange( id, m_editorWidget->rangeStart( id ), msf );
    slotRangeSelectionChanged( m_editorWidget->selectedRange() );
  }
}


bool K3bAudioTrackSplitDialog::eventFilter( TQObject* o, TQEvent* e )
{
  if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorWidget) ) {
    if( e->type() == TQEvent::MouseButtonDblClick ) {
      TQMouseEvent* me = TQT_TQMOUSEEVENT( e );
      splitAt( me->pos() );
    }
    else if( e->type() == TQEvent::ContextMenu ) {
      TQContextMenuEvent* ce = TQT_TQCONTEXTMENUEVENT( e );
      ce->consume();
      m_lastClickPosition = ce->pos();
      if( m_editorWidget->findRange( ce->pos().x() ) > 0 )
	m_popupMenu->popup( ce->globalPos() );
    }
  }

  return KDialogBase::eventFilter( o, e );
}


void K3bAudioTrackSplitDialog::slotSplitHere()
{
  splitAt( m_lastClickPosition );
}


void K3bAudioTrackSplitDialog::slotRemoveRange()
{
  m_editorWidget->removeRange( m_editorWidget->findRange( m_lastClickPosition.x() ) );
}


void K3bAudioTrackSplitDialog::splitTrack( K3bAudioTrack* track,
					   TQWidget* parent, 
					   const char* name )
{
  K3bAudioTrackSplitDialog d( track, parent, name );
  if( d.exec() == TQDialog::Accepted ) {
    TQValueList<int> ranges = d.m_editorWidget->allRanges();
    // we split the track at all range ends and just delete those that relate to the gaps in between
    K3b::Msf pos = 0;
    for( TQValueList<int>::const_iterator it = ranges.constBegin();
	 it != ranges.constEnd(); ++it ) {

      // delete the unwanted part
      if( d.m_editorWidget->rangeStart( *it ) > pos ) {
	// split so the range's start is the first frame of the new track
	K3bAudioTrack* nextTrack = track->split( d.m_editorWidget->rangeStart( *it ) - pos );
	delete track;
	track = nextTrack;
      }

      // create a new track part for the range itself
      pos = d.m_editorWidget->rangeStart( *it );
      if( d.m_editorWidget->rangeEnd( *it ) < d.m_editorWidget->length()-1 ) {
	// split so the range's end is the last frame in the old track 
	// and thus, the range's end + 1 the first frame in the new track
	track = track->split( d.m_editorWidget->rangeEnd( *it ) - pos + 1 );
      }
      pos = d.m_editorWidget->rangeEnd( *it )+1;
    }

    // remove the last unwanted part
    if( pos < d.m_editorWidget->length() ) {
      delete track;
    }
  }
}

#include "k3baudiotracksplitdialog.moc"
