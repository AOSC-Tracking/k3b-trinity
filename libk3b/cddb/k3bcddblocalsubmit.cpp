/* 
 *
 * $Id: k3bcddblocalsubmit.cpp 619556 2007-01-03 17:38:12Z trueg $
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



#include "k3bcddblocalsubmit.h"

#include <tqdir.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <kdebug.h>
#include <klocale.h>


K3bCddbLocalSubmit::K3bCddbLocalSubmit( TQObject* tqparent, const char* name )
  : K3bCddbSubmit( tqparent, name )
{
}


K3bCddbLocalSubmit::~K3bCddbLocalSubmit()
{
}


void K3bCddbLocalSubmit::doSubmit()
{
  TQString path = m_cddbDir;
  if( path.startsWith( "~" ) )
    path.replace( 0, 1, TQDir::homeDirPath() + "/" );
  else if( !path.startsWith( "/" ) )
    path.prepend( TQDir::homeDirPath() + "/" );
  if( path[path.length()-1] != '/' )
    path.append( "/" );

  if( !TQFile::exists( path ) && !TQDir().mkdir( path ) ) {
    kdDebug() << "(K3bCddbLocalSubmit) could not create directory: " << path << endl;
    setError( IO_ERROR );
    emit submitFinished( this );
    return;
  }

  if( TQFile::exists( path ) ) {
    // if the category dir does not exists
    // create it

    path += resultEntry().category;

    if( !TQFile::exists( path ) ) {
      if( !TQDir().mkdir( path ) ) {
	kdDebug() << "(K3bCddbLocalSubmit) could not create directory: " << path << endl;
	setError( IO_ERROR );
	emit submitFinished( this );
	return;
      }
    }

    // we always overwrite existing entries
    path += "/" + resultEntry().discid;
    TQFile entryFile( path );
    if( entryFile.exists() ) {
      kdDebug() << "(K3bCddbLocalSubmit) file already exists: " << path << endl;
    }
    
    if( !entryFile.open( IO_WriteOnly ) ) {
      kdDebug() << "(K3bCddbLocalSubmit) could not create file: " << path << endl;
      setError( IO_ERROR );
      emit submitFinished( this );
    }
    else {
      kdDebug() << "(K3bCddbLocalSubmit) creating file: " << path << endl;
      TQTextStream entryStream( &entryFile );
      entryStream.setEncoding( TQTextStream::UnicodeUTF8 );
      entryStream << resultEntry().rawData;
      entryFile.close();

      setError( SUCCESS );
      emit submitFinished( this );
    }
  }
  else {
    kdDebug() << "(K3bCddbLocalSubmit) could not find directory: " << path << endl;
    setError( IO_ERROR );
    emit infoMessage( i18n("Could not find directory: %1").tqarg(path) );
    emit submitFinished( this );
  }
}

#include "k3bcddblocalsubmit.moc"
