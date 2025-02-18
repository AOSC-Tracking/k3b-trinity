/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bdirsizejob.h"

#include <k3bthread.h>
#include <k3bthreadjob.h>
#include <k3bsimplejobhandler.h>
#include <k3bglobals.h>

#include <kdebug.h>
#include <tdeglobal.h>

#include <tqfileinfo.h>
#include <tqdir.h>


class K3bDirSizeJob::WorkThread : public K3bThread
{
public:
  WorkThread()
    : K3bThread(),
      followSymlinks(false),
      totalSize(0),
      totalFiles(0),
      totalDirs(0),
      totalSymlinks(0) {
  }

  void init() {
    m_canceled = false;

    totalSize = 0;
    totalFiles = 0;
    totalDirs = 0;
    totalSymlinks = 0;
  }

  void run() {
    emitStarted();

    TQStringList l;
    for( KURL::List::const_iterator it = urls.begin();
	 it != urls.end(); ++it ) {
      const KURL& url = *it;

      if( !url.isLocalFile() ) {
	kdDebug() << "(K3bDirSizeJob) no remote support." << endl;
	emitFinished( false );
	return;
      }

      l.append( url.path() );
    }

    emitFinished( countFiles( l, TQString() ) );
  }
  
  bool countDir( const TQString& dir ) {
    const TQString& dot = TDEGlobal::staticQString( "." );
    const TQString& dotdot = TDEGlobal::staticQString( ".." );
    TQStringList l = TQDir(dir).entryList( TQDir::All|TQDir::Hidden|TQDir::System );
    l.remove( dot );
    l.remove( dotdot );

    return countFiles( l, dir );
  }


  bool countFiles( const TQStringList& l, const TQString& dir ) {
    for( TQStringList::const_iterator it = l.begin();
	 it != l.end(); ++it ) {

      if( m_canceled )
	return false;

      k3b_struct_stat s;
      if( k3b_lstat( TQFile::encodeName( dir + *it ), &s ) )
	return false;	

      if( S_ISLNK( s.st_mode ) ) {
	++totalSymlinks;
	if( followSymlinks ) {
	  if( k3b_stat( TQFile::encodeName( dir + *it ), &s ) )
	    return false;	
	}
      }

      if( S_ISDIR( s.st_mode ) ) {
	++totalDirs;
	if( !countDir( dir + *it + '/' ) )
	  return false;
      }
      else if( !S_ISLNK( s.st_mode ) ) {
	++totalFiles;
	totalSize += (TDEIO::filesize_t)s.st_size;
      }
    }

    return true;
  }

  void cancel() {
    m_canceled = true;
    emitCanceled();
    wait();
  }

  KURL::List urls;
  bool followSymlinks;

  TDEIO::filesize_t totalSize;
  TDEIO::filesize_t totalFiles;
  TDEIO::filesize_t totalDirs;
  TDEIO::filesize_t totalSymlinks;

private:
  bool m_canceled;
};


K3bDirSizeJob::K3bDirSizeJob( TQObject* parent )
  : K3bThreadJob( new K3bSimpleJobHandler(), parent )
{
  d = new WorkThread;
  setThread( d );
}


K3bDirSizeJob::~K3bDirSizeJob()
{
  delete d;
  delete jobHandler();
}


TDEIO::filesize_t K3bDirSizeJob::totalSize() const
{
  return d->totalSize;
}


TDEIO::filesize_t K3bDirSizeJob::totalFiles() const
{
  return d->totalFiles;
}


TDEIO::filesize_t K3bDirSizeJob::totalDirs() const
{
  return d->totalDirs;
}


TDEIO::filesize_t K3bDirSizeJob::totalSymlinks() const
{
  return d->totalSymlinks;
}


void K3bDirSizeJob::setUrls( const KURL::List& urls )
{
  d->urls = urls;
}


void K3bDirSizeJob::setFollowSymlinks( bool b )
{
  d->followSymlinks = b;
}

#include "k3bdirsizejob.moc"
