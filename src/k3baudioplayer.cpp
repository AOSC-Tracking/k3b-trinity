/*
 *
 * $Id: k3baudioplayer.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3baudioplayer.h"
#include <k3bmsf.h>
#include "kcutlabel.h"

#include <tqlabel.h>
#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqdatetime.h>
#include <tqfont.h>
#include <tqslider.h>
#include <tqlistview.h>
#include <tqfile.h>
#include <tqpalette.h>
#include <tqheader.h>
#include <tqevent.h>
#include <tqdragobject.h>
#include <tqptrlist.h>
#include <kurldrag.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kurl.h>
#include <kaction.h>

#include <string.h>

#ifdef WITH_ARTS
#include <arts/artsflow.h>
#endif

#include <kdebug.h>

using namespace std;

K3bPlayListViewItem::K3bPlayListViewItem( const TQString& filename, TQListView* parent )
  : KListViewItem( parent ), m_filename( filename )
{
  m_length = 0;
  m_bActive = false;
}


K3bPlayListViewItem::K3bPlayListViewItem( const TQString& filename, TQListView* parent, TQListViewItem* after )
  : KListViewItem( parent, after ), m_filename( filename )
{
  m_length = 0;
  m_bActive = false;
}


K3bPlayListViewItem::~K3bPlayListViewItem()
{
}


TQString K3bPlayListViewItem::text( int c ) const
{
  switch( c ) {
  case 0:
    {
      int pos = m_filename.findRev("/");
      if( pos >= 0 )
	return m_filename.mid(pos+1);
      return m_filename;
    }

  case 1:
    if( m_length > 0 )
      return K3b::Msf(m_length).toString(false);

  default:
    return "";
  }
}


void K3bPlayListViewItem::paintCell( TQPainter* p, const TQColorGroup& cg, int c, int w, int a )
{
  if( m_bActive ) {
    // change the color of the text:
    // change roles: Text, HighlightedText, HighLight
    TQColorGroup newCg( cg );

    // we assume the user has not configured a very dark color as base color
    newCg.setColor( TQColorGroup::Text, red );
    newCg.setColor( TQColorGroup::Highlight, red );
    newCg.setColor( TQColorGroup::HighlightedText, white );

    KListViewItem::paintCell( p, newCg, c, w, a );
  }
  else
    KListViewItem::paintCell( p, cg, c, w, a );
}


K3bPlayListView::K3bPlayListView( TQWidget* parent, const char* name )
  : KListView( parent, name )
{
  addColumn( i18n("Filename") );
  addColumn( i18n("Length") );
  setAllColumnsShowFocus( true );
  setAcceptDrops( true );
  setDropVisualizer( true );
  setDragEnabled(true);
  setItemsMovable( true );
  header()->setClickEnabled( false );
  setSorting( -1 );
}


K3bPlayListView::~K3bPlayListView()
{
}


bool K3bPlayListView::acceptDrag( TQDropEvent* e ) const
{
  // we accept textdrag (urls) and moved items (supported by KListView)
  return KURLDrag::canDecode(e) || KListView::acceptDrag(e);
}


TQDragObject* K3bPlayListView::dragObject()
{
  TQPtrList<TQListViewItem> list = selectedItems();

  if( list.isEmpty() )
    return 0;

  TQPtrListIterator<TQListViewItem> it(list);
  KURL::List urls;

  for( ; it.current(); ++it )
    urls.append( KURL( ((K3bPlayListViewItem*)it.current())->filename() ) );

  return KURLDrag::newDrag( urls, viewport() );
}


K3bAudioPlayer::K3bAudioPlayer( TQWidget* parent, const char* name )
  : TQWidget( parent, name )
#ifdef WITH_ARTS
, m_playObject( Arts::PlayObject::null() )
#endif
{
    m_currentItem = 0L;
  // initialize
  // ------------------------------------------------------------------------
  m_labelFilename    = new KCutLabel( i18n("no file"), this );
  m_labelOverallTime = new TQLabel( "00:00", this );
  m_labelCurrentTime = new TQLabel( "00:00", this );

  m_viewPlayList = new K3bPlayListView( this );

  m_labelOverallTime->setAlignment( AlignHCenter | AlignVCenter );
  m_labelCurrentTime->setAlignment( AlignHCenter | AlignVCenter );
  m_labelOverallTime->setFrameStyle( TQFrame::StyledPanel | TQFrame::Plain );
  m_labelCurrentTime->setFrameStyle( TQFrame::StyledPanel | TQFrame::Plain );
  m_labelFilename->setFrameStyle( TQFrame::StyledPanel | TQFrame::Plain );
  m_labelOverallTime->setPalette( TQPalette( TQColor(238, 238, 205) ) );
  m_labelCurrentTime->setPalette( TQPalette( TQColor(238, 238, 205) ) );
  m_labelFilename->setPalette( TQPalette( TQColor(238, 238, 205) ) );

  m_buttonPlay = new TQToolButton( this );
  m_buttonPause = new TQToolButton( this );
  m_buttonStop = new TQToolButton( this );
  m_buttonPlay->setIconSet( SmallIconSet("player_play") );
  m_buttonPause->setIconSet( SmallIconSet("player_pause") );
  m_buttonStop->setIconSet( SmallIconSet("player_stop") );
  m_buttonForward = new TQToolButton( this );
  m_buttonBack = new TQToolButton( this );
  m_buttonForward->setIconSet( SmallIconSet("player_end") );
  m_buttonBack->setIconSet( SmallIconSet("player_start") );

  m_seekSlider = new TQSlider( TQSlider::Horizontal, this );

  m_updateTimer = new TQTimer( this );
  // ------------------------------------------------------------------------

  // tqlayout
  // ------------------------------------------------------------------------
  TQGridLayout* grid = new TQGridLayout( this );
  grid->setSpacing( 2 );
  grid->setMargin( 0 );

  grid->addWidget( m_buttonPlay, 1, 0 );
  grid->addWidget( m_buttonPause, 1, 1 );
  grid->addWidget( m_buttonStop, 1, 2 );
  grid->addColSpacing( 3, 5 );
  grid->addWidget( m_buttonBack, 1, 4 );
  grid->addWidget( m_buttonForward, 1, 5 );

  grid->addMultiCellWidget( m_labelFilename, 0, 0, 0, 6 );

  grid->addMultiCellWidget( m_seekSlider, 1, 1, 6, 8 );

  grid->addWidget( m_labelCurrentTime, 0, 7 );
  grid->addWidget( m_labelOverallTime, 0, 8 );

  grid->addMultiCellWidget( m_viewPlayList, 2, 2, 0, 8 );
  grid->setRowStretch( 2, 1 );
  grid->setColStretch( 6, 1 );
  // ------------------------------------------------------------------------


  // actions
  // ------------------------------------------------------------------------
  m_actionRemove = new KAction( i18n( "Remove" ), "editdelete",
				Key_Delete, this, TQT_SLOT(slotRemoveSelected()),
				this, "audioplayer_remove" );
  m_actionClear = new KAction( i18n( "Clear List" ), "editclear",
			       0, this, TQT_SLOT(clear()),
			       this, "audioplayer_clear" );

  m_contextMenu = new KActionMenu( this, "audio_player_menu" );
  m_contextMenu->insert(m_actionRemove);
  m_contextMenu->insert(m_actionClear);
  // ------------------------------------------------------------------------


  // connections
  // ------------------------------------------------------------------------
  connect( m_viewPlayList, TQT_SIGNAL(contextMenu(KListView*, TQListViewItem*, const TQPoint&)),
	   this, TQT_SLOT(slotShowContextMenu(KListView*, TQListViewItem*, const TQPoint&)) );

  connect( m_buttonPlay, TQT_SIGNAL(clicked()), this, TQT_SLOT(play()) );
  connect( m_buttonStop, TQT_SIGNAL(clicked()), this, TQT_SLOT(stop()) );
  connect( m_buttonPause, TQT_SIGNAL(clicked()), this, TQT_SLOT(pause()) );

  connect( m_buttonForward, TQT_SIGNAL(clicked()), this, TQT_SLOT(forward()) );
  connect( m_buttonBack, TQT_SIGNAL(clicked()), this, TQT_SLOT(back()) );

  connect( m_seekSlider, TQT_SIGNAL(sliderMoved(int)), this, TQT_SLOT(seek(int)) );
  connect( m_seekSlider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(slotUpdateCurrentTime(int)) );

  connect( m_updateTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotUpdateDisplay()) );
  connect( m_updateTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotCheckEnd()) );

  connect( m_viewPlayList, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
	   this, TQT_SLOT(slotPlayItem(TQListViewItem*)) );
  connect( m_viewPlayList, TQT_SIGNAL(dropped(TQDropEvent*,TQListViewItem*)),
	   this, TQT_SLOT(slotDropped(TQDropEvent*,TQListViewItem*)) );
  // ------------------------------------------------------------------------


  m_bLengthReady = false;
}


K3bAudioPlayer::~K3bAudioPlayer()
{
  // we remove the reference to the play object
  // if we don't do this it won't be removed and K3b will crash (not sure why)
#ifdef WITH_ARTS
  m_playObject = Arts::PlayObject::null();
#endif
}


int K3bAudioPlayer::state()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    switch( m_playObject.state() ) {
    case Arts::posIdle:
      return STOPPED;
    case Arts::posPlaying:
      return PLAYING;
    case Arts::posPaused:
      return PAUSED;
    }
  }
  else if( m_currentItem )
    return STOPPED;
#endif

  return EMPTY;
}


void K3bAudioPlayer::playFile( const TQString& filename )
{
  clear();
  if( TQFile::exists( filename ) ) {
    K3bPlayListViewItem* item = new K3bPlayListViewItem( filename, m_viewPlayList );
    setCurrentItem( item );
    play();
    emit started( filename );
  }
}


void K3bAudioPlayer::playFiles( const TQStringList& files )
{
  clear();
  TQStringList::ConstIterator it = files.begin();
  playFile( *it );
  ++it;

  for( ; it != files.end(); ++it )
    enqueueFile( *it );
}


void K3bAudioPlayer::enqueueFile( const TQString& filename )
{
  if( TQFile::exists( filename ) )
    (void)new K3bPlayListViewItem( filename, m_viewPlayList, m_viewPlayList->lastChild() );
}


void K3bAudioPlayer::enqueueFiles( const TQStringList& files )
{
  for( TQStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    enqueueFile( *it );
}


void K3bAudioPlayer::play()
{
#ifdef WITH_ARTS
  if( !m_currentItem ) {
    setCurrentItem( m_viewPlayList->firstChild() );
  }

  if( m_currentItem ) {
    if( m_playObject.isNull() ) {
      Arts::PlayObjectFactory factory = Arts::Reference("global:Arts_PlayObjectFactory");
      if( factory.isNull() ) {
	kdDebug() << "(K3bAudioPlayer) could not create PlayObjectFactory. Possibly no artsd running." << endl;
	m_labelFilename->setText( i18n("No running aRtsd found") );
	return;
      }

      m_playObject = factory.createPlayObject( string(TQFile::encodeName(m_currentItem->filename()) ) );
      if( m_playObject.isNull() ) {
	kdDebug() << "(K3bAudioPlayer) no aRts module available for: " << m_currentItem->filename() << endl;
	m_labelFilename->setText( i18n("Unknown file format") );

	// play the next if there is any
	if( m_currentItem->itemBelow() ) {
	  setCurrentItem( m_currentItem->itemBelow() );
	  play();
	}
	return;
      }
    }
    if( m_playObject.state() != Arts::posPlaying ) {
      m_playObject.play();
      emit started();
      m_updateTimer->start( 1000 );
    }

    slotUpdateFilename();
  }
#endif
}


void K3bAudioPlayer::slotPlayItem( TQListViewItem* item )
{
  setCurrentItem( item );
  play();
}


void K3bAudioPlayer::stop()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    m_updateTimer->stop();
    m_playObject.halt();
    m_playObject = Arts::PlayObject::null();
    m_bLengthReady = false;
 
    emit stopped();
  }
#endif

  m_seekSlider->setValue(0);
  slotUpdateFilename();
  slotUpdateCurrentTime(0);
}


void K3bAudioPlayer::pause()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    if( m_playObject.state() == Arts::posPlaying ) {
      m_updateTimer->stop();
      m_playObject.pause();
      emit paused();
    }

    slotUpdateFilename();
  }
#endif
}


void K3bAudioPlayer::seek( long pos )
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    if( m_playObject.state() != Arts::posIdle ) {
      if( pos < 0 ) {
	m_playObject.seek( Arts::poTime() );
      }
      else if( m_playObject.overallTime().seconds < pos ) {
	m_playObject.seek( m_playObject.overallTime() );
      }
      else if( pos != m_playObject.currentTime().seconds ) {
	m_playObject.seek( Arts::poTime( pos, 0, -1, "" ) );
      }
    }
  }
  else {
    m_seekSlider->setValue(0);
    slotUpdateCurrentTime(0);
  }
#endif
}



void K3bAudioPlayer::seek( int pos )
{
  seek( (long)pos );
}


void K3bAudioPlayer::forward()
{
  if( m_currentItem ) {
    if( m_currentItem->itemBelow() ) {
      bool bPlay = false;
      if( state() == PLAYING )
	bPlay = true;

      setCurrentItem( m_currentItem->itemBelow() );

      if( bPlay )
	play();
    }
  }
}


void K3bAudioPlayer::back()
{
  if( m_currentItem ) {
    if( m_currentItem->itemAbove() ) {
      bool bPlay = false;
      if( state() == PLAYING )
	bPlay = true;

      setCurrentItem( m_currentItem->itemAbove() );

      if( bPlay )
	play();
    }
  }
}


void K3bAudioPlayer::clear()
{
  setCurrentItem( 0 );
  m_viewPlayList->clear();
}


long K3bAudioPlayer::length()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    return m_playObject.overallTime().seconds;
  }
#endif
  return 0;
}


long K3bAudioPlayer::position()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    return m_playObject.currentTime().seconds;
  }
#endif
  return 0;
}


// FIXME: let my do some useful stuff!
bool K3bAudioPlayer::supportsMimetype( const TQString& mimetype )
{
  if( mimetype.contains("audio") || mimetype.contains("ogg") )
    return true;
  else
    return false;
}


void K3bAudioPlayer::slotCheckEnd()
{
#ifdef WITH_ARTS
  if( !m_playObject.isNull() ) {
    if( m_playObject.state() == Arts::posIdle ) {
      if( m_currentItem->nextSibling() ) {
	setCurrentItem( m_currentItem->nextSibling() );
	play();
      }
      else {
	stop();
      }
      emit ended();
    }
  }
#endif
}


void K3bAudioPlayer::setCurrentItem( TQListViewItem* item )
{
  if( item == 0 ) {
    stop();
    m_labelOverallTime->setText("00:00");
    m_labelFilename->setText( i18n("no file") );
    m_currentItem = 0;
  }
  else if( K3bPlayListViewItem* playItem = dynamic_cast<K3bPlayListViewItem*>(item) ) {
    if( m_currentItem ) {
      // reset m_currentItem
      m_currentItem->setActive( false );
      stop();
    }
    m_currentItem = playItem;
    m_currentItem->setActive( true );

    // paint the activity changes
    m_viewPlayList->viewport()->update();

    slotUpdateFilename();
  }
}


void K3bAudioPlayer::slotUpdateCurrentTime( int time )
{
  m_labelCurrentTime->setText( K3b::Msf( time*75 ).toString(false) );
}


void K3bAudioPlayer::slotUpdateLength( long time )
{
  m_labelOverallTime->setText( K3b::Msf( time*75 ).toString(false) );
}


void K3bAudioPlayer::slotUpdateFilename()
{
  if( m_currentItem ) {
    TQString display = m_currentItem->filename();
    int pos = display.findRev("/");
    if( pos >= 0 )
      display = display.mid(pos+1);

    switch( state() ) {
    case PLAYING:
      display.prepend( TQString("(%1) ").arg(i18n("playing")) );
      break;
    case PAUSED:
      display.prepend( TQString("(%1) ").arg(i18n("paused")) );
      break;
    case STOPPED:
      display.prepend( TQString("(%1) ").arg(i18n("stopped")) );
      break;
    default:
      break;
    }

    m_labelFilename->setText( display );
  }
}


void K3bAudioPlayer::slotUpdateDisplay()
{
  if( m_currentItem ) {
    // we need to set the length here because sometimes it is not ready in the beginning (?)
    if( !m_bLengthReady && length() > 0 ) {
      slotUpdateLength( length() );
      m_seekSlider->setMaxValue( length() );
      m_currentItem->setLength( 75 * length() );
      m_bLengthReady = true;

      m_viewPlayList->viewport()->update();
    }

    m_seekSlider->setValue( position() );
  }
}


void K3bAudioPlayer::slotDropped( TQDropEvent* e, TQListViewItem* after )
{
  if( !after )
    after = m_viewPlayList->lastChild();

  KURL::List urls;
  KURLDrag::decode( e, urls );

  for( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
    if( TQFile::exists( (*it).path() ) ) {
      TQListViewItem* newItem = new K3bPlayListViewItem( (*it).path(), m_viewPlayList, after );
      after = newItem;
    }
  }
}


void K3bAudioPlayer::slotRemoveSelected()
{
  TQPtrList<TQListViewItem> selected = m_viewPlayList->selectedItems();
  for( TQListViewItem* item = selected.first(); item; item = selected.next() ) {
    if( item == m_currentItem )
      setCurrentItem(0);
    delete item;
  }
}


void K3bAudioPlayer::slotShowContextMenu( KListView*, TQListViewItem* item, const TQPoint& p )
{
  if( item )
    m_actionRemove->setEnabled( true );
  else
    m_actionRemove->setEnabled( false );

  m_contextMenu->popup(p);
}


#include "k3baudioplayer.moc"
