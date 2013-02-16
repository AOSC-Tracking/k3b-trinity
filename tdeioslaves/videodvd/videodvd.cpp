/* 
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include <tqcstring.h>
#include <tqdatetime.h>
#include <tqbitarray.h>
#include <tqptrlist.h>

#include <kdebug.h>
#include <kinstance.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kurl.h>

#include <stdlib.h>

#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3biso9660.h>
#include <k3biso9660backend.h>
#include <k3b_export.h>

#include "videodvd.h"

using namespace TDEIO;

extern "C"
{
  LIBK3B_EXPORT int kdemain( int argc, char **argv )
  {
    TDEInstance instance( "tdeio_videodvd" );

    kdDebug(7101) << "*** Starting tdeio_videodvd " << endl;

    if (argc != 4)
    {
      kdDebug(7101) << "Usage: tdeio_videodvd  protocol domain-socket1 domain-socket2" << endl;
      exit(-1);
    }

    tdeio_videodvdProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** tdeio_videodvd Done" << endl;
    return 0;
  }
}



// FIXME: Does it really make sense to use a static device manager? Are all instances
// of videodvd started in another process?
K3bDevice::DeviceManager* tdeio_videodvdProtocol::s_deviceManager = 0;
int tdeio_videodvdProtocol::s_instanceCnt = 0;

tdeio_videodvdProtocol::tdeio_videodvdProtocol(const TQCString &pool_socket, const TQCString &app_socket)
    : SlaveBase("tdeio_videodvd", pool_socket, app_socket)
{
  kdDebug() << "tdeio_videodvdProtocol::tdeio_videodvdProtocol()" << endl;
  if( !s_deviceManager )
  {
    s_deviceManager = new K3bDevice::DeviceManager();
    s_deviceManager->setCheckWritingModes( false );
    s_deviceManager->scanBus();
  }
  s_instanceCnt++;
}


tdeio_videodvdProtocol::~tdeio_videodvdProtocol()
{
  kdDebug() << "tdeio_videodvdProtocol::~tdeio_videodvdProtocol()" << endl;
  s_instanceCnt--;
  if( s_instanceCnt == 0 )
  {
    delete s_deviceManager;
    s_deviceManager = 0;
  }
}


TDEIO::UDSEntry tdeio_videodvdProtocol::createUDSEntry( const K3bIso9660Entry* e ) const
{
  TDEIO::UDSEntry uds;
  TDEIO::UDSAtom a;

  a.m_uds = TDEIO::UDS_NAME;
  a.m_str = e->name();
  uds.append( a );

  a.m_uds = TDEIO::UDS_ACCESS;
  a.m_long = e->permissions();
  uds.append( a );

  a.m_uds = TDEIO::UDS_CREATION_TIME;
  a.m_long = e->date();
  uds.append( a );

  a.m_uds = TDEIO::UDS_MODIFICATION_TIME;
  a.m_long = e->date();
  uds.append( a );

  if( e->isDirectory() )
  {
    a.m_uds = TDEIO::UDS_FILE_TYPE;
    a.m_long = S_IFDIR;
    uds.append( a );

    a.m_uds = TDEIO::UDS_MIME_TYPE;
    a.m_str = "inode/directory";
    uds.append( a );
  }
  else
  {
    const K3bIso9660File* file = static_cast<const K3bIso9660File*>( e );

    a.m_uds = TDEIO::UDS_SIZE;
    a.m_long = file->size();
    uds.append( a );

    a.m_uds = TDEIO::UDS_FILE_TYPE;
    a.m_long = S_IFREG;
    uds.append( a );

    a.m_uds = TDEIO::UDS_MIME_TYPE;
    if( e->name().endsWith( "VOB" ) )
      a.m_str = "video/mpeg";
    else
      a.m_str = "unknown";
    uds.append( a );
  }

  return uds;
}


// FIXME: remember the iso instance for quicker something and search for the videodvd
//        in the available devices.
K3bIso9660* tdeio_videodvdProtocol::openIso( const KURL& url, TQString& plainIsoPath )
{
  // get the volume id from the url
  TQString volumeId = url.path().section( '/', 1, 1 );

  kdDebug() << "(tdeio_videodvdProtocol) searching for Video dvd: " << volumeId << endl;

  // now search the devices for this volume id
  // FIXME: use the cache created in listVideoDVDs
  for( TQPtrListIterator<K3bDevice::Device> it( s_deviceManager->dvdReader() ); *it; ++it ) {
    K3bDevice::Device* dev = *it;
    K3bDevice::DiskInfo di = dev->diskInfo();

    // we search for a DVD with a single track.
    // this time let K3bIso9660 decide if we need dvdcss or not
    // FIXME: check for encryption and libdvdcss and report an error
    if( di.isDvdMedia() && di.numTracks() == 1 ) {
      K3bIso9660* iso = new K3bIso9660( dev );
      iso->setPlainIso9660( true );
      if( iso->open() && iso->primaryDescriptor().volumeId == volumeId ) {
	plainIsoPath = url.path().section( "/", 2, -1 ) + "/";
	kdDebug() << "(tdeio_videodvdProtocol) using iso path: " << plainIsoPath << endl;
	return iso;
      }
      delete iso;
    }
  }

  error( ERR_SLAVE_DEFINED, i18n("No VideoDVD found") );
  return 0;
}


void tdeio_videodvdProtocol::get(const KURL& url )
{
  kdDebug() << "tdeio_videodvd::get(const KURL& url)" << endl ;

  TQString isoPath;
  if( K3bIso9660* iso = openIso( url, isoPath ) )
  {
    const K3bIso9660Entry* e = iso->firstIsoDirEntry()->entry( isoPath );
    if( e && e->isFile() )
    {
      const K3bIso9660File* file = static_cast<const K3bIso9660File*>( e );
      totalSize( file->size() );
      TQByteArray buffer( 10*2048 );
      int read = 0;
      int cnt = 0;
      TDEIO::filesize_t totalRead = 0;
      while( (read = file->read( totalRead, buffer.data(), buffer.size() )) > 0 )
      {
        buffer.resize( read );
        data(buffer);
        ++cnt;
        totalRead += read;
        if( cnt == 10 )
        {
          cnt = 0;
          processedSize( totalRead );
        }
      }

      delete iso;

      data(TQByteArray()); // empty array means we're done sending the data

      if( read == 0 )
        finished();
      else
        error( ERR_SLAVE_DEFINED, i18n("Read error.") );
    }
    else
      error( ERR_DOES_NOT_EXIST, url.path() );
  }
}


void tdeio_videodvdProtocol::listDir( const KURL& url )
{
  if( url.path() == "/" ) {
    listVideoDVDs();
  }
  else {
    TQString isoPath;
    K3bIso9660* iso = openIso( url, isoPath );
    if( iso ) {
      const K3bIso9660Directory* mainDir = iso->firstIsoDirEntry();
      const K3bIso9660Entry* e = mainDir->entry( isoPath );
      if( e ) {
	if( e->isDirectory() ) {
	  const K3bIso9660Directory* dir = static_cast<const K3bIso9660Directory*>(e);
	  TQStringList el = dir->entries();
	  el.remove( "." );
	  el.remove( ".." );
	  UDSEntryList udsl;
	  for( TQStringList::const_iterator it = el.begin(); it != el.end(); ++it )
	    udsl.append( createUDSEntry( dir->entry( *it ) ) );
	  listEntries( udsl );
	  finished();
	}
	else {
	  error( ERR_CANNOT_ENTER_DIRECTORY, url.path() );
	}
      }
      else {
	error( ERR_CANNOT_ENTER_DIRECTORY, url.path() );
      }

      // for testing we always do the whole thing
      delete iso;
    }
  }
}


void tdeio_videodvdProtocol::listVideoDVDs()
{
  int cnt = 0;

  for( TQPtrListIterator<K3bDevice::Device> it( s_deviceManager->dvdReader() ); *it; ++it ) {
    K3bDevice::Device* dev = *it;
    K3bDevice::DiskInfo di = dev->diskInfo();

    // we search for a DVD with a single track.
    if( di.isDvdMedia() && di.numTracks() == 1 ) {
      //
      // now do a quick check for VideoDVD.
      // - no dvdcss for speed
      // - only a check for the VIDEO_TS dir
      //
      K3bIso9660 iso( new K3bIso9660DeviceBackend(dev) );
      iso.setPlainIso9660( true );
      if( iso.open() && iso.firstIsoDirEntry()->entry( "VIDEO_TS" ) ) {
	// FIXME: cache the entry for speedup

        UDSEntryList udsl;
	TDEIO::UDSEntry uds;
	TDEIO::UDSAtom a;
	
	a.m_uds = TDEIO::UDS_NAME;
	a.m_str = iso.primaryDescriptor().volumeId;
	uds.append( a );

	a.m_uds = TDEIO::UDS_FILE_TYPE;
	a.m_long = S_IFDIR;
	uds.append( a );
	
	a.m_uds = TDEIO::UDS_MIME_TYPE;
	a.m_str = "inode/directory";
	uds.append( a );

	a.m_uds = TDEIO::UDS_ICON_NAME;
	a.m_str = "dvd_unmount";
	uds.append( a );

	udsl.append( uds );

	listEntries( udsl );

	++cnt;
      }
    }
  }

  if( cnt )
    finished();
  else
    error( ERR_SLAVE_DEFINED, i18n("No VideoDVD found") );
}


void tdeio_videodvdProtocol::stat( const KURL& url )
{
  if( url.path() == "/" ) {
    //
    // stat the root path
    //
    TDEIO::UDSEntry uds;
    TDEIO::UDSAtom a;
    
    a.m_uds = TDEIO::UDS_NAME;
    a.m_str = "/";
    uds.append( a );
    
    a.m_uds = TDEIO::UDS_FILE_TYPE;
    a.m_long = S_IFDIR;
    uds.append( a );

    a.m_uds = TDEIO::UDS_MIME_TYPE;
    a.m_str = "inode/directory";
    uds.append( a );

    statEntry( uds );
    finished();
  }
  else {
    TQString isoPath;
    K3bIso9660* iso = openIso( url, isoPath );
    if( iso ) {
      const K3bIso9660Entry* e = iso->firstIsoDirEntry()->entry( isoPath );
      if( e ) {
	statEntry( createUDSEntry( e ) );
	finished();
      }
      else
	error( ERR_DOES_NOT_EXIST, url.path() );
      
      delete iso;
    }
  }
}


// FIXME: when does this get called? It seems not to be used for the files.
void tdeio_videodvdProtocol::mimetype( const KURL& url )
{
  if( url.path() == "/" ) {
    error( ERR_UNSUPPORTED_ACTION, "mimetype(/)" );
    return;
  }

  TQString isoPath;
  K3bIso9660* iso = openIso( url, isoPath );
  if( iso )
  {
    const K3bIso9660Entry* e = iso->firstIsoDirEntry()->entry( isoPath );
    if( e )
    {
      if( e->isDirectory() )
        mimeType( "inode/directory" );
      else if( e->name().endsWith( ".VOB" ) )
      {
        mimetype( "video/mpeg" );
      }
      else
      {
        // send some data
        const K3bIso9660File* file = static_cast<const K3bIso9660File*>( e );
        TQByteArray buffer( 10*2048 );
        int read = file->read( 0, buffer.data(), buffer.size() );
        if( read > 0 )
        {
          buffer.resize( read );
          data(buffer);
          data(TQByteArray());
          finished();
          // FIXME: do we need to emit finished() after emitting the end of data()?
        }
        else
          error( ERR_SLAVE_DEFINED, i18n("Read error.") );
      }
    }
    delete iso;
  }
}
