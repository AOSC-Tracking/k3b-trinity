/* 
 *
 * $Id: k3bspecialdataitem.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BSPECIALDATAITEM_H
#define K3BSPECIALDATAITEM_H

#include "k3bdataitem.h"
#include "k3bdiritem.h"

#include <kio/global.h>

/**
 * This can be used to create fake items like the boot catalog
 * It's mainly a K3bDataItem where everything has to be set manually
 */
class K3bSpecialDataItem : public K3bDataItem
{
 public:
  K3bSpecialDataItem( K3bDataDoc* doc, KIO::filesize_t size, K3bDirItem* tqparent = 0, const TQString& k3bName = TQString() )
    : K3bDataItem( doc, tqparent ),
    m_size( size )
    {
      setK3bName( k3bName );

      // add automagically like a qlistviewitem
      if( tqparent )
	tqparent->addDataItem( this );
    }

    K3bSpecialDataItem( const K3bSpecialDataItem& item )
      : K3bDataItem( item ),
      m_mimeType( item.m_mimeType ),
      m_size( item.m_size ) {
    }

  ~K3bSpecialDataItem() {
    // remove this from tqparentdir
    if( tqparent() )
      tqparent()->takeDataItem( this );
  }

  K3bDataItem* copy() const {
    return new K3bSpecialDataItem( *this );
  }

  void setMimeType( const TQString& s ) { m_mimeType = s; }
  const TQString& mimeType() const { return m_mimeType; }

  bool isSpecialFile() const { return true; }

 protected:
  /**
   * Normally one does not use this method but K3bDataItem::size()
   */
  KIO::filesize_t itemSize( bool ) const { return m_size; }

 private:
  TQString m_mimeType;
  KIO::filesize_t m_size;
};

#endif

