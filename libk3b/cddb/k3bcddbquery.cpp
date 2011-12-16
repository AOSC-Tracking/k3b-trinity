/*
 *
 * $Id: k3bcddbquery.cpp 619556 2007-01-03 17:38:12Z trueg $
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



#include "k3bcddbquery.h"

#include "k3bcddbresult.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>


#include <tqtextstream.h>
#include <tqstringlist.h>
#include <tqregexp.h>
#include <tqtimer.h>

#include <stdlib.h>




K3bCddbQuery::K3bCddbQuery( TQObject* parent, const char* name )
  : TQObject(parent, name)
{
  m_bQueryFinishedEmited = false;
}


K3bCddbQuery::~K3bCddbQuery()
{
}


void K3bCddbQuery::query( const K3bDevice::Toc& toc )
{
  m_bQueryFinishedEmited = false;
  m_toc = toc;
  m_inexactMatches.clear();

  TQTimer::singleShot( 0, this, TQT_SLOT(doQuery()) );
}


void K3bCddbQuery::queryMatch( const K3bCddbResultHeader& header )
{
  m_header = header;
  m_result = K3bCddbResultEntry();
  m_result.category = header.category;
  m_result.discid = header.discid;

  TQTimer::singleShot( 0, this, TQT_SLOT(doMatchQuery()) );
}


const TQStringList& K3bCddbQuery::categories()
{
  static TQStringList s_cat = TQStringList::split( ",", "rock,blues,misc,classical,"
						 "country,data,folk,jazz,newage,reggae,soundtrack" );
  return s_cat;
}


bool K3bCddbQuery::parseEntry( TQTextStream& stream, K3bCddbResultEntry& entry )
{
  entry.rawData = "";

  stream.setEncoding( TQTextStream::UnicodeUTF8 );

  // parse data
  TQString line;
  while( !(line = stream.readLine()).isNull() ) {
    entry.rawData.append(line + "\n");

    // !all fields may be splitted into several lines!
  
    if( line.startsWith( "DISCID" ) ) {
      // TODO: this could be several discids separated by comma!
    }

    else if( line.startsWith( "DYEAR" ) ) {
      TQString year = line.mid( 6 );
      if( year.length() == 4 )
	entry.year = year.toInt();
    }
    
    else if( line.startsWith( "DGENRE" ) ) {
      entry.genre = line.mid( 7 );
    }

    else if( line.startsWith( "DTITLE" ) ) {
      entry.cdTitle += line.mid( 7 );
    }
    
    else if( line.startsWith( "TTITLE" ) ) {
      int eqSgnPos = line.find( "=" );
      bool ok;
      uint trackNum = (uint)line.mid( 6, eqSgnPos - 6 ).toInt( &ok );
      if( !ok )
	kdDebug() << "(K3bCddbQuery) !!! PARSE ERROR: " << line << endl;
      else {
	//	kdDebug() << "(K3bCddbQuery) Track title for track " << trackNum << endl;
	
	// make sure the list is big enough
	while( entry.titles.count() <= trackNum )
	  entry.titles.append( "" );
	
	entry.titles[trackNum] += line.mid( eqSgnPos+1 );
      }
    }
    
    else if( line.startsWith( "EXTD" ) ) {
      entry.cdExtInfo += line.mid( 5 );
    }
    
    else if( line.startsWith( "EXTT" ) ) {
      int eqSgnPos = line.find( "=" );
      bool ok;
      uint trackNum = (uint)line.mid( 4, eqSgnPos - 4 ).toInt( &ok );
      if( !ok )
	kdDebug() << "(K3bCddbQuery) !!! PARSE ERROR: " << line << endl;
      else {
	//	kdDebug() << "(K3bCddbQuery) Track extr track " << trackNum << endl;

	// make sure the list is big enough
	while( entry.extInfos.count() <= trackNum )
	  entry.extInfos.append( "" );
	
	entry.extInfos[trackNum] += line.mid( eqSgnPos+1 );
      }
    }
    
    else if( line.startsWith( "#" ) ) {
      //      kdDebug() <<  "(K3bCddbQuery) comment: " << line << endl;
    }
    
    else {
      kdDebug() <<  "(K3bCddbQuery) Unknown field: " << line << endl;
    }
  }

  // now split the titles in the last added match 
  // if no " / " delimiter is present title and artist are the same
  // -------------------------------------------------------------------
  TQString fullTitle = entry.cdTitle;
  int splitPos = fullTitle.find( " / " );
  if( splitPos < 0 )
    entry.cdArtist = fullTitle;
  else {
    // split
    entry.cdTitle = fullTitle.mid( splitPos + 3 );
    entry.cdArtist = fullTitle.left( splitPos );
  }


  for( TQStringList::iterator it = entry.titles.begin();
       it != entry.titles.end(); ++it ) {
    TQString fullTitle = *it;
    int splitPos = fullTitle.find( " / " );
    if( splitPos < 0 )
      entry.artists.append( entry.cdArtist );
    else {
      // split
      *it = fullTitle.mid( splitPos + 3 );
      entry.artists.append( fullTitle.left( splitPos ) );
    }
  }


  // replace all "\\n" with "\n"
  for( TQStringList::iterator it = entry.titles.begin();
       it != entry.titles.end(); ++it ) {
    (*it).replace( "\\\\\\\\n", "\\n" );
  }

  for( TQStringList::iterator it = entry.artists.begin();
       it != entry.artists.end(); ++it ) {
    (*it).replace( "\\\\\\\\n", "\\n" );
  }

  for( TQStringList::iterator it = entry.extInfos.begin();
       it != entry.extInfos.end(); ++it ) {
    (*it).replace( "\\\\\\\\n", "\\n" );
  }

  entry.cdTitle.replace( "\\\\\\\\n", "\\n" );
  entry.cdArtist.replace( "\\\\\\\\n", "\\n" );
  entry.cdExtInfo.replace( "\\\\\\\\n", "\\n" );
  entry.genre.replace( "\\\\\\\\n", "\\n" );

  return true;
}


int K3bCddbQuery::getCode( const TQString& line )
{
  bool ok;
  int code = line.left( 3 ).toInt( &ok );
  if( !ok )
    code = -1;
  return code;
}


TQString K3bCddbQuery::handshakeString() const
{
  TQString user( getenv("USER") );
  TQString host( getenv("HOST") );
  if( user.isEmpty() )
    user = "kde-user";
  if( host.isEmpty() )
    host = "kde-host";
  
  return TQString("%1 %2 K3b %3").tqarg(user).tqarg(host).tqarg(kapp->aboutData()->version());
}


TQString K3bCddbQuery::queryString() const
{
  TQString query = "cddb query " 
    + TQString::number( m_toc.discId(), 16 ).rightJustify( 8, '0' ) 
    + " "
    + TQString::number( (unsigned int)m_toc.count() );
  
  for( K3bDevice::Toc::const_iterator it = m_toc.begin(); it != m_toc.end(); ++it ) {
    query.append( TQString( " %1" ).tqarg( (*it).firstSector().lba() ) );
  }
  
  query.append( TQString( " %1" ).tqarg( m_toc.length().lba() / 75 ) );
  
  return query;
}


bool K3bCddbQuery::parseMatchHeader( const TQString& line, K3bCddbResultHeader& header )
{
  // format: category id title
  // where title could be artist and title splitted with a /
  header.category = line.section( ' ', 0, 0 );
  header.discid = line.section( ' ', 1, 1 );
  header.title = line.mid( header.category.length() + header.discid.length() + 2 );
  int slashPos = header.title.find( "/" );
  if( slashPos > 0 ) {
    header.artist = header.title.left(slashPos).stripWhiteSpace();
    header.title = header.title.mid( slashPos+1 ).stripWhiteSpace();
  }
  return true;
}


void K3bCddbQuery::emitQueryFinished()
{
  if( !m_bQueryFinishedEmited ) {
    m_bQueryFinishedEmited = true;
    emit queryFinished( this );
  }
}


#include "k3bcddbquery.moc"
