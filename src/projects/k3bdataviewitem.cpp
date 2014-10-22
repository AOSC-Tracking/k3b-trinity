/*
 *
 * $Id: k3bdataviewitem.cpp 689533 2007-07-18 14:19:39Z trueg $
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

#include "k3bdataviewitem.h"

#include "k3bfileitem.h"
#include "k3bdiritem.h"
#include "k3bspecialdataitem.h"
#include "k3bsessionimportitem.h"
#include "k3bdatadoc.h"
#include <k3bvalidators.h>

#include <tdeio/global.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kpixmapeffect.h>
#include <kpixmap.h>

#include <tqpainter.h>
#include <tqpalette.h>
#include <tqfileinfo.h>


K3bDataViewItem::K3bDataViewItem( K3bDataItem* item, TQListView* parent )
  : K3bListViewItem( parent ),
    m_dataItem(item)
{
  init();
}

K3bDataViewItem::K3bDataViewItem( K3bDataItem* item, TQListViewItem* parent )
  : K3bListViewItem( parent ),
    m_dataItem(item)
{
  init();
}

K3bDataViewItem::~K3bDataViewItem()
{}


void K3bDataViewItem::init()
{
  setEditor( 0, LINE );
  static TQValidator* s_validator = K3bValidators::iso9660Validator();
  setValidator( 0, s_validator );
}

void K3bDataViewItem::paintCell( TQPainter* p, const TQColorGroup& cg, int column, int width, int align )
{
  TQColorGroup _cg = cg;

  if( !dataItem()->isRemoveable() && dataItem()->doc()->root() != dataItem() ) {
    _cg.setColor( TQColorGroup::Text, listView()->palette().disabled().foreground() );
  }

  if( column == 0 ) {
    TQFontMetrics fm = p->fontMetrics();

    if( dataItem()->hideOnRockRidge() ) {
      int tw = TQMAX( fm.width( "rr" ) + 2*listView()->itemMargin(), height() );
      p->fillRect( width-tw, 0, tw, height(), isSelected() ? _cg.highlight() : _cg.brush(TQColorGroup::Base) );
      p->setPen( isSelected() ? _cg.highlightedText() : red );
      p->drawEllipse( width-tw, 0, tw, height() );
      p->drawText( width-tw, 0, tw, height(), TQt::AlignCenter, "rr" );
      width -= tw;
    }

    if( dataItem()->hideOnJoliet() ) {
      int tw = TQMAX( fm.width( "j" ) + 2*listView()->itemMargin(), height() );
      p->fillRect( width-tw, 0, tw, height(), isSelected() ? _cg.highlight() : _cg.brush(TQColorGroup::Base) );
      p->setPen( isSelected() ? _cg.highlightedText() : blue );
      p->drawEllipse( width-tw, 0, tw, height() );
      p->drawText( width-tw, 0, tw, height(), TQt::AlignCenter, "j" );
      width -= tw;
    }
  }
  else if( column == 4 ) {
    if( dataItem()->isSymLink() ) {
      if( !dataItem()->doc()->isoOptions().followSymbolicLinks() &&
	  dataItem()->doc()->isoOptions().createRockRidge() &&
	  !dataItem()->isValid() ) {
	// paint the link in red
	_cg.setColor( TQColorGroup::Text, TQt::red );
      }
    }
  }

  K3bListViewItem::paintCell( p, _cg, column, width, align );
}


void K3bDataViewItem::setText( int col, const TQString& text )
{
  if( col == 0 && dataItem()->isRenameable() ) {
    dataItem()->setK3bName( text );
  }
  K3bListViewItem::setText( col, text );
}


TQString K3bDataViewItem::key( int col, bool a ) const
{
  if( col == 2 ) {
    // to have correct sorting we need to justify the size in bytes
    // 100 TB should be enough for the next year... ;-)

    if( a )
      return ( dataItem()->isDir() ? TQString("0") : TQString("1") )
	+ TQString::number( dataItem()->size() ).rightJustify( 16, '0' );
    else
      return ( dataItem()->isDir() ? TQString("1") : TQString("0") )
	+ TQString::number( dataItem()->size() ).rightJustify( 16, '0' );
  }

  if( a )
    return ( dataItem()->isDir() ? TQString("0") : TQString("1") ) + text(col);
  else
    return ( dataItem()->isDir() ? TQString("1") : TQString("0") ) + text(col);
}


K3bDataDirViewItem::K3bDataDirViewItem( K3bDirItem* dir, TQListView* parent )
  : K3bDataViewItem( dir, parent )
{
  m_dirItem = dir;
  setPixmap( 0, dir->depth() > 7 ? SmallIcon( "folder_red" ) : SmallIcon( "folder" ) );
}


K3bDataDirViewItem::K3bDataDirViewItem( K3bDirItem* dir, TQListViewItem* parent )
  : K3bDataViewItem( dir, parent )
{
  m_dirItem = dir;
  setPixmap( 0, dir->depth() > 7 ? SmallIcon( "folder_red" ) : SmallIcon( "folder" ) );
}


K3bDataDirViewItem::~K3bDataDirViewItem()
{
}


void K3bDataDirViewItem::dragEntered()
{
  setOpen( true );
}


TQString K3bDataDirViewItem::text( int index ) const
{
  switch( index ) {
  case 0:
    return m_dirItem->k3bName();
  case 1:
    return i18n("Directory");
  case 2:
    return TDEIO::convertSize( m_dirItem->size() );
  default:
    return "";
  }
}


void K3bDataDirViewItem::highlightIcon( bool b )
{
  if( m_pixmap.isNull() )
    m_pixmap = *pixmap(0);

  if( b )
    setPixmap( 0, KPixmapEffect::selectedPixmap( m_pixmap, listView()->colorGroup().highlight() ) );
  else
    setPixmap( 0, m_pixmap );
}



K3bDataFileViewItem::K3bDataFileViewItem( K3bFileItem* file, TQListView* parent )
  : K3bDataViewItem( file, parent )
{
  init( file );
}


K3bDataFileViewItem::K3bDataFileViewItem( K3bFileItem* file, TQListViewItem* parent )
  : K3bDataViewItem( file, parent )
{
  init( file );
}


void K3bDataFileViewItem::init( K3bFileItem* file )
{
  m_fileItem = file;

  // determine the mimetype
  m_pMimeType = KMimeType::findByURL( KURL::fromPathOrURL(file->localPath()) );
  if( !m_pMimeType )
    setPixmap( 0, DesktopIcon( "unknown", 16, TDEIcon::DefaultState ) );
  else
    setPixmap( 0, m_pMimeType->pixmap( KURL::fromPathOrURL(file->localPath()), TDEIcon::Desktop, 16, TDEIcon::DefaultState ) );
}


TQString K3bDataFileViewItem::text( int index ) const
{
  switch( index ) {
  case 0:
    return m_fileItem->k3bName();
  case 1:
    {
      TQString comment = m_pMimeType->comment( KURL::fromPathOrURL(m_fileItem->localPath()), true );
      if( comment.isEmpty() )
	comment = m_pMimeType->name();

      if( m_fileItem->isSymLink() )
	return i18n("Link to %1").arg(comment);
      else
	return comment;
    }
  case 2:
    return TDEIO::convertSize( m_fileItem->size() );
  case 3:
      return m_fileItem->localPath();
  case 4: {
      if( !m_fileItem->isSymLink() ) {
          return TQString();
      }

      TQString s;
      if ( m_fileItem->doc()->isoOptions().followSymbolicLinks() ) {
          s = K3b::resolveLink( m_fileItem->localPath() );
      }
      else {
          s = TQFileInfo( m_fileItem->localPath() ).readLink();
      }

      if( !m_fileItem->isValid() ) {
          s += " (" + i18n("outside of project") + ")";
      }

      return s;
  }
  default:
    return "";
  }
}



K3bDataRootViewItem::K3bDataRootViewItem( K3bDataDoc* doc, TQListView* parent )
  : K3bDataDirViewItem( doc->root(), parent )
{
  m_doc = doc;
  setPixmap( 0, SmallIcon( "cdrom_unmount" ) );
  setValidator( 0, new K3bLatin1Validator() );
}


K3bDataRootViewItem::~K3bDataRootViewItem()
{
  delete validator(0);
}


TQString K3bDataRootViewItem::text( int index ) const
{
  switch( index ) {
  case 0:
    return ( m_doc->isoOptions().volumeID().isEmpty() ? i18n("root") : m_doc->isoOptions().volumeID() );
  default:
    return "";
  }
}


void K3bDataRootViewItem::setText( int col, const TQString& text )
{
  if( col == 0 )
    m_doc->setVolumeID( text );

  K3bDataViewItem::setText( col, text );
}


K3bSpecialDataViewItem::K3bSpecialDataViewItem( K3bSpecialDataItem* item, TQListView* parent )
  : K3bDataViewItem( item, parent )
{
  setPixmap( 0, SmallIcon("unknown") );
}

TQString K3bSpecialDataViewItem::text( int col ) const
{
  switch( col ) {
  case 0:
    return dataItem()->k3bName();
  case 1:
    return ((K3bSpecialDataItem*)dataItem())->mimeType();
  case 2:
    return TDEIO::convertSize( dataItem()->size() );
  default:
    return "";
  }
}



K3bSessionImportViewItem::K3bSessionImportViewItem( K3bSessionImportItem* item, TQListView* parent )
  : K3bDataViewItem( item, parent )
{
  setPixmap( 0, SmallIcon("unknown") );
}

TQString K3bSessionImportViewItem::text( int col ) const
{
  switch( col ) {
  case 0:
    return dataItem()->k3bName();
  case 1:
    return i18n("From previous session");
  case 2:
    return TDEIO::convertSize( dataItem()->size() );
  default:
    return "";
  }
}
