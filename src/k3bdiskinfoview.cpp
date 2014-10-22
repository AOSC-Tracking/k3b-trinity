/*
 *
 * $Id: k3bdiskinfoview.cpp 643214 2007-03-16 15:25:24Z trueg $
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

#include <config.h>


#include "k3bdiskinfoview.h"

#include <k3bdiskinfo.h>
#include <k3bcdtext.h>
#include <k3bdeviceglobals.h>
#include <k3bglobals.h>
#include <k3bstdguiitems.h>
#include <k3blistview.h>
#include <k3biso9660.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqfont.h>
#include <tqcolor.h>
#include <tqheader.h>
#include <tqstring.h>
#include <tqpainter.h>
#include <tqpalette.h>
#include <tqpixmap.h>
#include <tqregion.h>
#include <tqframe.h>

#include <kdialog.h>
#include <tdelocale.h>
#include <tdelistview.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tdeio/global.h>


// FIXME: use K3bListViewItem instead
class K3bDiskInfoView::HeaderViewItem : public TDEListViewItem
{
public:
  HeaderViewItem( TQListView* parent )
      : TDEListViewItem( parent ) {}
  HeaderViewItem( TQListViewItem* parent )
      : TDEListViewItem( parent ) {}
  HeaderViewItem( TQListView* parent, TQListViewItem* after )
      : TDEListViewItem( parent, after ) {}
  HeaderViewItem( TQListViewItem* parent, TQListViewItem* after )
      : TDEListViewItem( parent, after ) {}
  HeaderViewItem( TQListView* parent, const TQString& t1 )
      : TDEListViewItem( parent, t1 ) {}
  HeaderViewItem( TQListViewItem* parent, const TQString& t1 )
      : TDEListViewItem( parent, t1 ) {}
  HeaderViewItem( TQListView* parent, TQListViewItem* after, const TQString& t1 )
      : TDEListViewItem( parent, after, t1 ) {}
  HeaderViewItem( TQListViewItem* parent, TQListViewItem* after, const TQString& t1 )
      : TDEListViewItem( parent, after, t1 ) {}

  void paintCell( TQPainter* p, const TQColorGroup & cg, int column, int width, int align )
  {
    TQFont f ( p->font() );
    f.setBold( true );
    p->setFont( f );
    TDEListViewItem::paintCell( p, cg, column, width, align );
  }
};


class K3bDiskInfoView::TwoColumnViewItem : public TDEListViewItem
{
public:
  TwoColumnViewItem( TQListView* parent )
      : TDEListViewItem( parent ) {}
  TwoColumnViewItem( TQListViewItem* parent )
      : TDEListViewItem( parent ) {}
  TwoColumnViewItem( TQListView* parent, TQListViewItem* after )
      : TDEListViewItem( parent, after ) {}
  TwoColumnViewItem( TQListViewItem* parent, TQListViewItem* after )
      : TDEListViewItem( parent, after ) {}
  TwoColumnViewItem( TQListView* parent, const TQString& t1 )
      : TDEListViewItem( parent, t1 ) {}
  TwoColumnViewItem( TQListViewItem* parent, const TQString& t1 )
      : TDEListViewItem( parent, t1 ) {}
  TwoColumnViewItem( TQListView* parent, TQListViewItem* after, const TQString& t1 )
      : TDEListViewItem( parent, after, t1 ) {}
  TwoColumnViewItem( TQListViewItem* parent, TQListViewItem* after, const TQString& t1 )
      : TDEListViewItem( parent, after, t1 ) {}

  void paintCell( TQPainter* p, const TQColorGroup & cg, int column, int width, int align )
  {

    if( column == 1 ) {
      int newWidth = width;

      int i = 2;
      for( ; i < listView()->columns(); ++i ) {
        newWidth += listView()->columnWidth( i );
      }

      // TODO: find a way to get the TRUE new width after resizing

      //       TQRect r = p->clipRegion().boundingRect();
      //       r.setWidth( newWidth );
      //       p->setClipRect( r );
      p->setClipping( false );

      TDEListViewItem::paintCell( p, cg, column, newWidth, align );
    } else if( column == 0 )
      TDEListViewItem::paintCell( p, cg, column, width, align );
  }
};



K3bDiskInfoView::K3bDiskInfoView( TQWidget* parent, const char* name )
  : K3bMediaContentsView( true,
			  K3bMedium::CONTENT_ALL,
			  K3bDevice::MEDIA_ALL|K3bDevice::MEDIA_UNKNOWN,
			  K3bDevice::STATE_EMPTY|K3bDevice::STATE_INCOMPLETE|K3bDevice::STATE_COMPLETE|K3bDevice::STATE_UNKNOWN,
			  parent, name )
{
  m_infoView = new TDEListView( this );
  setMainWidget( m_infoView );

  m_infoView->setSorting( -1 );
  m_infoView->setAllColumnsShowFocus( true );
  m_infoView->setSelectionMode( TQListView::NoSelection );
  m_infoView->setResizeMode( TDEListView::AllColumns );
  m_infoView->setAlternateBackground( TQColor() );

  m_infoView->addColumn( "1" );
  m_infoView->addColumn( "2" );
  m_infoView->addColumn( "3" );
  m_infoView->addColumn( "4" );
#ifdef K3B_DEBUG
  m_infoView->addColumn( "index0" );
#endif

  m_infoView->header()->hide();

  // do not automatically reload the disk info
  //  setAutoReload( false );
}


K3bDiskInfoView::~K3bDiskInfoView()
{}


void K3bDiskInfoView::reloadMedium()
{
  m_infoView->clear();

  setTitle( medium().shortString( true ) );

  if( medium().diskInfo().diskState() == K3bDevice::STATE_NO_MEDIA ) {
    (void)new TQListViewItem( m_infoView, i18n("No medium present") );
    setRightPixmap( K3bTheme::MEDIA_NONE );
  }
  else {
    if( medium().diskInfo().empty() ) {
      setRightPixmap( K3bTheme::MEDIA_EMPTY );
    }
    else {
      switch( medium().toc().contentType() ) {
      case K3bDevice::AUDIO:
	setRightPixmap( K3bTheme::MEDIA_AUDIO );
        break;
      case K3bDevice::DATA: {
	if( medium().content() & K3bMedium::CONTENT_VIDEO_DVD ) {
	  setRightPixmap( K3bTheme::MEDIA_VIDEO );
	}
	else {
	  setRightPixmap( K3bTheme::MEDIA_DATA );
	}
        break;
      }
      case K3bDevice::MIXED:
	setRightPixmap( K3bTheme::MEDIA_MIXED );
        break;
      default:
	setTitle( i18n("Unknown Disk Type") );
	setRightPixmap( K3bTheme::MEDIA_NONE );
      }
    }

    createMediaInfoItems( medium() );


    // iso9660 info
    // /////////////////////////////////////////////////////////////////////////////////////
    if( medium().content() & K3bMedium::CONTENT_DATA ) {
      (void)new TDEListViewItem( m_infoView, m_infoView->lastChild() ); // empty spacer item
      createIso9660InfoItems( medium().iso9660Descriptor() );
    }

    // tracks
    // /////////////////////////////////////////////////////////////////////////////////////
    if( !medium().toc().isEmpty() ) {

      if( m_infoView->childCount() )
	(void)new TDEListViewItem( m_infoView, m_infoView->lastChild() ); // empty spacer item

      TDEListViewItem* trackHeaderItem = new HeaderViewItem( m_infoView, m_infoView->lastChild(), i18n("Tracks") );

      // create header item
      TDEListViewItem* item = new TDEListViewItem( trackHeaderItem,
					       i18n("Type"),
					       i18n("Attributes"),
					       i18n("First-Last Sector"),
					       i18n("Length") );

#ifdef K3B_DEBUG
      item->setText( 4, "Index0" );
#endif

      int lastSession = 0;

      // if we have multiple sessions we create a header item for every session
      TDEListViewItem* trackItem = 0;
      if( medium().diskInfo().numSessions() > 1 && medium().toc()[0].session() > 0 ) {
	trackItem = new HeaderViewItem( trackHeaderItem, item, i18n("Session %1").arg(1) );
	lastSession = 1;
      }
      else
	trackItem = trackHeaderItem;

      // create items for the tracks
      K3bDevice::Toc::const_iterator it;
      int index = 1;
      for( it = medium().toc().begin(); it != medium().toc().end(); ++it ) {
        const K3bTrack& track = *it;

	if( medium().diskInfo().numSessions() > 1 && track.session() != lastSession ) {
	  lastSession = track.session();
	  trackItem->setOpen(true);
	  trackItem = new HeaderViewItem( trackHeaderItem,
					  m_infoView->lastItem()->parent(),
					  i18n("Session %1").arg(lastSession) );
	}

        item = new TDEListViewItem( trackItem, item );
        TQString text;
        if( track.type() == K3bTrack::AUDIO ) {
          item->setPixmap( 0, SmallIcon( "audio-x-generic" ) );
          text = i18n("Audio");
        } else {
          item->setPixmap( 0, SmallIcon( "application-x-tar" ) );
          if( track.mode() == K3bTrack::MODE1 )
            text = i18n("Data/Mode1");
          else if( track.mode() == K3bTrack::MODE2 )
            text = i18n("Data/Mode2");
          else if( track.mode() == K3bTrack::XA_FORM1 )
            text = i18n("Data/Mode2 XA Form1");
          else if( track.mode() == K3bTrack::XA_FORM2 )
            text = i18n("Data/Mode2 XA Form2");
	  else
	    text = i18n("Data");
        }
        item->setText( 0, i18n("%1 (%2)").arg( TQString::number(index).rightJustify( 2, ' ' )).arg(text) );
	item->setText( 1, TQString( "%1/%2" )
		       .arg( track.copyPermitted() ? i18n("copy") : i18n("no copy") )
		       .arg( track.type() == K3bTrack::AUDIO
			    ? ( track.preEmphasis() ?  i18n("preemp") : i18n("no preemp") )
			    : ( track.recordedIncremental() ?  i18n("incremental") : i18n("uninterrupted") ) ) );
        item->setText( 2,
		       TQString("%1 - %2")
		       .arg(track.firstSector().lba())
		       .arg(track.lastSector().lba()) );
        item->setText( 3, TQString::number( track.length().lba() ) + " (" + track.length().toString() + ")" );

#ifdef K3B_DEBUG
        if( track.type() == K3bTrack::AUDIO )
	  item->setText( 4, TQString( "%1 (%2)" ).arg(track.index0().toString()).arg(track.index0().lba()) );
#endif
        ++index;
      }

      trackItem->setOpen(true);
      trackHeaderItem->setOpen( true );
    }


    // CD-TEXT
    // /////////////////////////////////////////////////////////////////////////////////////
    if( !medium().cdText().isEmpty() ) {
      medium().cdText().debug();
      if( m_infoView->childCount() )
	(void)new TDEListViewItem( m_infoView, m_infoView->lastChild() ); // empty spacer item

      TDEListViewItem* cdTextHeaderItem = new HeaderViewItem( m_infoView,
							    m_infoView->lastChild(),
							    i18n("CD-TEXT (excerpt)") );

      // create header item
      TDEListViewItem* item = new TDEListViewItem( cdTextHeaderItem,
					       i18n("Performer"),
					       i18n("Title"),
					       i18n("Songwriter"),
					       i18n("Composer") );
      item = new TDEListViewItem( cdTextHeaderItem, item );
      item->setText( 0, i18n("CD:") + " " +
		     medium().cdText().performer() );
      item->setText( 1, medium().cdText().title() );
      item->setText( 2, medium().cdText().songwriter() );
      item->setText( 3, medium().cdText().composer() );

      int index = 1;
      for( unsigned int i = 0; i < medium().cdText().count(); ++i ) {
        item = new TDEListViewItem( cdTextHeaderItem, item );
	item->setText( 0, TQString::number(index).rightJustify( 2, ' ' ) + " " +
		       medium().cdText().at(i).performer() );
	item->setText( 1, medium().cdText().at(i).title() );
	item->setText( 2, medium().cdText().at(i).songwriter() );
	item->setText( 3, medium().cdText().at(i).composer() );
	++index;
      }

      cdTextHeaderItem->setOpen( true );
    }
  }
}


void K3bDiskInfoView::createMediaInfoItems( const K3bMedium& medium )
{
  const K3bDevice::DiskInfo& info = medium.diskInfo();

  TDEListViewItem* atipItem = new HeaderViewItem( m_infoView, m_infoView->lastItem(), i18n("Medium") );
  TQString typeStr;
  if( info.mediaType() != K3bDevice::MEDIA_UNKNOWN )
    typeStr = K3bDevice::mediaTypeString( info.mediaType() );
  else
    typeStr = i18n("Unknown (probably CD-ROM)");

  TDEListViewItem* atipChild = new TDEListViewItem( atipItem, i18n("Type:"), typeStr );

  if( info.isDvdMedia() )
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Media ID:"),
				   !info.mediaId().isEmpty() ? TQString::fromLatin1( info.mediaId() ) : i18n("unknown") );


  atipChild = new TDEListViewItem( atipItem, atipChild,
				 i18n("Capacity:"),
				 i18n("%1 min").arg(info.capacity().toString()),
				 TDEIO::convertSize(info.capacity().mode1Bytes()) );

  if( !info.empty() )
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Used Capacity:"),
				   i18n("%1 min").arg(info.size().toString()),
				   TDEIO::convertSize(info.size().mode1Bytes()) );

  if( info.appendable() )
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Remaining:"),
				   i18n("%1 min").arg( info.remainingSize().toString() ),
				   TDEIO::convertSize(info.remainingSize().mode1Bytes()) );

  atipChild = new TDEListViewItem( atipItem, atipChild,
				 i18n("Rewritable:"),
				 info.rewritable() ? i18n("yes") : i18n("no") );

  atipChild = new TDEListViewItem( atipItem, atipChild,
				 i18n("Appendable:"),
				 info.appendable() ? i18n("yes") : i18n("no") );

  atipChild = new TDEListViewItem( atipItem, atipChild,
				 i18n("Empty:"),
				 info.empty() ? i18n("yes") : i18n("no") );

  if( info.isDvdMedia() )
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Layers:"),
				   TQString::number( info.numLayers() ) );

  if( info.mediaType() == K3bDevice::MEDIA_DVD_PLUS_RW ) {
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Background Format:") );
    switch( info.bgFormatState() ) {
    case K3bDevice::BG_FORMAT_NONE:
      atipChild->setText( 1, i18n("not formatted") );
      break;
    case K3bDevice::BG_FORMAT_INCOMPLETE:
      atipChild->setText( 1, i18n("incomplete") );
      break;
    case K3bDevice::BG_FORMAT_IN_PROGRESS:
      atipChild->setText( 1, i18n("in progress") );
      break;
    case K3bDevice::BG_FORMAT_COMPLETE:
      atipChild->setText( 1, i18n("complete") );
      break;
    }
  }

  atipChild = new TDEListViewItem( atipItem, atipChild,
				 i18n("Sessions:"),
				 TQString::number( info.numSessions() ) );

  if( info.mediaType() & K3bDevice::MEDIA_WRITABLE ) {
    atipChild = new TDEListViewItem( atipItem, atipChild,
				   i18n("Supported writing speeds:") );
    TQString s;
    if( medium.writingSpeeds().isEmpty() )
      s = "-";
    else
      for( TQValueList<int>::const_iterator it = medium.writingSpeeds().begin();
	   it != medium.writingSpeeds().end(); ++it ) {
	if( !s.isEmpty() ) {
	  s.append( "\n" );
	  atipChild->setMultiLinesEnabled( true );
	}

	if( info.isDvdMedia() )
	  s.append( TQString().sprintf( "%.1fx (%d KB/s)", (double)*it / 1385.0, *it ) );
	else
	  s.append( TQString( "%1x (%2 KB/s)" ).arg( *it/175 ).arg( *it ) );
      }

    atipChild->setText( 1, s );
  }

  atipItem->setOpen( true );
}


void K3bDiskInfoView::createIso9660InfoItems( const K3bIso9660SimplePrimaryDescriptor& iso )
{
  TDEListViewItem* iso9660Item = new HeaderViewItem( m_infoView, m_infoView->lastChild(),
						   i18n("ISO9660 Filesystem Info") );
  TDEListViewItem* iso9660Child = 0;

  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("System Id:"),
				    iso.systemId.isEmpty()
				    ? TQString("-")
				    : iso.systemId );
  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("Volume Id:"),
				    iso.volumeId.isEmpty()
				    ? TQString("-")
				    : iso.volumeId );
  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("Volume Set Id:"),
				    iso.volumeSetId.isEmpty()
				    ? TQString("-")
				    : iso.volumeSetId );
  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("Publisher Id:"),
				    iso.publisherId.isEmpty()
				    ? TQString("-")
				    : iso.publisherId );
  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("Preparer Id:"),
				    iso.preparerId.isEmpty()
				    ? TQString("-")
				    : iso.preparerId );
  iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
				    i18n("Application Id:"),
				    iso.applicationId.isEmpty()
				    ? TQString("-")
				    : iso.applicationId );
//   iso9660Child = new TDEListViewItem( iso9660Item, iso9660Child,
// 				    i18n("Volume Size:"),
// 				    TQString( "%1 (%2*%3)" )
// 				    .arg(iso.logicalBlockSize
// 					 *iso.volumeSpaceSize)
// 				    .arg(iso.logicalBlockSize)
// 				    .arg(iso.volumeSpaceSize),
// 				    TDEIO::convertSize(iso.logicalBlockSize
// 						     *iso.volumeSpaceSize)  );

  iso9660Item->setOpen( true );
}


void K3bDiskInfoView::enableInteraction( bool enable )
{
  TQListViewItemIterator it( m_infoView );
  while( it.current() ) {
    it.current()->setEnabled( enable );
    ++it;
  }
}

#include "k3bdiskinfoview.moc"

