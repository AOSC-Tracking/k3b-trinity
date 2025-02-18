/* 
 *
 * $Id: k3bdataprojectinterface.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bdataprojectinterface.h"

#include <k3bdatadoc.h>
#include <k3bdiritem.h>
#include <k3bisooptions.h>


K3bDataProjectInterface::K3bDataProjectInterface( K3bDataDoc* doc, const char* name )
  : K3bProjectInterface( doc, name ),
    m_dataDoc(doc)
{
}


K3bDataProjectInterface::~K3bDataProjectInterface()
{
}


bool K3bDataProjectInterface::createFolder( const TQString& name )
{
  return createFolder( name, "/" );
}


bool K3bDataProjectInterface::createFolder( const TQString& name, const TQString& parent )
{
  K3bDataItem* p = m_dataDoc->root()->findByPath( parent );
  if( p && p->isDir() && !static_cast<K3bDirItem*>(p)->find( name ) ) {
    m_dataDoc->addEmptyDir( name, static_cast<K3bDirItem*>(p) );
    return true;
  }
  return false;
}


void K3bDataProjectInterface::addUrl( const TQString& url, const TQString& parent )
{
  addUrls( TQStringList(url), parent );
}


void K3bDataProjectInterface::addUrls( const TQStringList& urls, const TQString& parent )
{
  K3bDataItem* p = m_dataDoc->root()->findByPath( parent );
  if( p && p->isDir() )
    m_dataDoc->addUrls( KURL::List(urls), static_cast<K3bDirItem*>(p) );
}


bool K3bDataProjectInterface::removeItem( const TQString& path )
{
  K3bDataItem* p = m_dataDoc->root()->findByPath( path );
  if( p && p->isRemoveable() ) {
    m_dataDoc->removeItem( p );
    return true;
  }
  else
    return false;
}


bool K3bDataProjectInterface::renameItem( const TQString& path, const TQString& newName )
{
  K3bDataItem* p = m_dataDoc->root()->findByPath( path );
  if( p && p->isRenameable() && !newName.isEmpty() ) {
    p->setK3bName( newName );
    return true;
  }
  else
    return false;
}


void K3bDataProjectInterface::setVolumeID( const TQString& id )
{
  m_dataDoc->setVolumeID( id );
}

bool K3bDataProjectInterface::isFolder( const TQString& path ) const
{
  K3bDataItem* p =  m_dataDoc->root()->findByPath( path );
  if( p )
    return p->isDir();
  else
    return false;
}


TQStringList K3bDataProjectInterface::children( const TQString& path ) const
{
  TQStringList l;
  K3bDataItem* item =  m_dataDoc->root()->findByPath( path );
  if( item && item->isDir() ) {
    const TQPtrList<K3bDataItem>& cl = static_cast<K3bDirItem*>(item)->children();
    for( TQPtrListIterator<K3bDataItem> it( cl ); *it; ++it )
      l.append( it.current()->k3bName() );
  }

  return l;
}


bool K3bDataProjectInterface::setSortWeight( const TQString& path, long weight ) const
{
  K3bDataItem* item =  m_dataDoc->root()->findByPath( path );
  if( item ) {
    item->setSortWeight( weight );
    return true;
  }
  else
    return false;
}
