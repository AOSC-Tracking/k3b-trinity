/* 
 *
 * $Id: k3bcddblocalquery.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bcddblocalquery.h"

#include <tqdir.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <tdeapplication.h>
#include <klocale.h>
#include <kdebug.h>


K3bCddbLocalQuery::K3bCddbLocalQuery( TQObject* parent , const char* name )
  : K3bCddbQuery( parent, name )
{
}


K3bCddbLocalQuery::~K3bCddbLocalQuery()
{
}


void K3bCddbLocalQuery::doQuery()
{
  emit infoMessage( i18n("Searching entry in %1").arg( m_cddbDir ) );
  kapp->processEvents(); //BAD!

  TQString path = preparePath( m_cddbDir );

  kdDebug() << "(K3bCddbLocalQuery) searching in dir " << path << " for " 
	    << TQString::number( toc().discId(), 16 ).rightJustify( 8, '0' ) << endl;

  for( TQStringList::const_iterator it = categories().begin();
       it != categories().end(); ++it ) {

    TQString file = path + *it + "/" +  TQString::number( toc().discId(), 16 ).rightJustify( 8, '0' );

    if( TQFile::exists( file ) ) {
      // found file
      
      TQFile f( file );
      if( !f.open( IO_ReadOnly ) ) {
	kdDebug() << "(K3bCddbLocalQuery) Could not open file" << endl;
      }
      else {
	TQTextStream t( &f );

	K3bCddbResultEntry entry;
	parseEntry( t, entry );
	K3bCddbResultHeader header;
	header.discid = TQString::number( toc().discId(), 16 ).rightJustify( 8, '0' );
	header.category = *it;
	header.title = entry.cdTitle;
	header.artist = entry.cdArtist;
	m_inexactMatches.append(header);
      }
    }
    else {
      kdDebug() << "(K3bCddbLocalQuery) Could not find local entry in category " << *it << endl;
    }
  }

  if( m_inexactMatches.count() > 0 ) {
    setError( SUCCESS );
    if( m_inexactMatches.count() == 1 ) {
      queryMatch( m_inexactMatches.first() );
    }
    else {
      emit inexactMatches( this );
    }
  }
  else {
    setError( NO_ENTRY_FOUND );
    emit queryFinished( this );
  }
}


void K3bCddbLocalQuery::doMatchQuery()
{
  TQString path = preparePath( m_cddbDir ) + header().category + "/" + header().discid;

  TQFile f( path );
  if( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "(K3bCddbLocalQuery) Could not open file" << endl;
    setError( READ_ERROR );
  }
  else {
    TQTextStream t( &f );
    
    parseEntry( t, result() );
    result().discid = header().discid;
    result().category = header().category;
    setError( SUCCESS );
  }
  emit queryFinished( this );
}


TQString K3bCddbLocalQuery::preparePath( const TQString& p ) 
{
  TQString path = p;
  if( path.startsWith( "~" ) )
    path.replace( 0, 1, TQDir::homeDirPath() );
  else if( !path.startsWith( "/" ) )
    path.prepend( TQDir::homeDirPath() );
  if( path[path.length()-1] != '/' )
    path.append( "/" );

  return path;
}

#include "k3bcddblocalquery.moc"
