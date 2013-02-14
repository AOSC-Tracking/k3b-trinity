/*
 *
 * $Id: k3bglobals.cpp 659634 2007-04-30 14:51:32Z trueg $
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

#include <config.h>


#include "k3bglobals.h"
#include <k3bversion.h>
#include <k3bdevice.h>
#include <k3bdevicemanager.h>
#include <k3bdeviceglobals.h>
#include <k3bexternalbinmanager.h>
#include <k3bcore.h>
#include <k3bhalconnection.h>

#include <tdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeio/job.h>
#include <tdeio/netaccess.h>
#include <kurl.h>
#include <dcopref.h>
#include <kprocess.h>

#include <tqdatastream.h>
#include <tqdir.h>
#include <tqfile.h>

#include <cmath>
#include <sys/utsname.h>
#include <sys/stat.h>

#if defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/param.h>
#  include <sys/mount.h>
#  include <sys/endian.h>
#  define bswap_16(x) bswap16(x)
#  define bswap_32(x) bswap32(x)
#  define bswap_64(x) bswap64(x)
#else
#  include <byteswap.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#  include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#  include <sys/vfs.h>
#endif


/*
struct Sample {
  unsigned char msbLeft;
  unsigned char lsbLeft;
  unsigned char msbRight;
  unsigned char lsbRight;

  short left() const {
    return ( msbLeft << 8 ) | lsbLeft;
  }
  short right() const {
    return ( msbRight << 8 ) | lsbRight;
  }
  void left( short d ) {
    msbLeft = d >> 8;
    lsbLeft = d;
  }
  void right( short d ) {
    msbRight = d >> 8;
    lsbRight = d;
  }
};
*/

TQString K3b::framesToString( int h, bool showFrames )
{
  int m = h / 4500;
  int s = (h % 4500) / 75;
  int f = h % 75;

  TQString str;

  if( showFrames ) {
    // cdrdao needs the MSF format where 1 second has 75 frames!
    str.sprintf( "%.2i:%.2i:%.2i", m, s, f );
  }
  else
    str.sprintf( "%.2i:%.2i", m, s );

  return str;
}

/*TQString K3b::sizeToTime(long size)
{
  int h = size / sizeof(Sample) / 588;
  return framesToString(h, false);
}*/


TQ_INT16 K3b::swapByteOrder( const TQ_INT16& i )
{
  return bswap_16( i );
  //((i << 8) & 0xff00) | ((i >> 8 ) & 0xff);
}


TQ_INT32 K3b::swapByteOrder( const TQ_INT32& i )
{
  //return ((i << 24) & 0xff000000) | ((i << 8) & 0xff0000) | ((i >> 8) & 0xff00) | ((i >> 24) & 0xff );
  return bswap_32( i );
}


TQ_INT64 K3b::swapByteOrder( const TQ_INT64& i )
{
  return bswap_64( i );
}


int K3b::round( double d )
{
  return (int)( floor(d) + 0.5 <= d ? ceil(d) : floor(d) );
}


TQString K3b::findUniqueFilePrefix( const TQString& _prefix, const TQString& path )
{
  TQString url;
  if( path.isEmpty() || !TQFile::exists(path) )
    url = defaultTempPath();
  else
    url = prepareDir( path );

  TQString prefix = _prefix;
  if( prefix.isEmpty() )
    prefix = "k3b_";

  // now create the unique prefix
  TQDir dir( url );
  TQStringList entries = dir.entryList( TQDir::DefaultFilter, TQDir::Name );
  int i = 0;
  for( TQStringList::iterator it = entries.begin();
       it != entries.end(); ++it ) {
    if( (*it).startsWith( prefix + TQString::number(i) ) ) {
      i++;
      it = entries.begin();
    }
  }

  return url + prefix + TQString::number(i);
}


TQString K3b::findTempFile( const TQString& ending, const TQString& d )
{
  return findUniqueFilePrefix( "k3b_", d ) + ( ending.isEmpty() ? TQString() : (TQString::fromLatin1(".") + ending) );
}


TQString K3b::defaultTempPath()
{
  TQString oldGroup = kapp->config()->group();
  kapp->config()->setGroup( "General Options" );
  TQString url = kapp->config()->readPathEntry( "Temp Dir", TDEGlobal::dirs()->resourceDirs( "tmp" ).first() );
  kapp->config()->setGroup( oldGroup );
  return prepareDir(url);
}


TQString K3b::prepareDir( const TQString& dir )
{
  return (dir + (dir[dir.length()-1] != '/' ? "/" : ""));
}


TQString K3b::parentDir( const TQString& path )
{
  TQString parent = path;
  if( path[path.length()-1] == '/' )
    parent.truncate( parent.length()-1 );

  int pos = parent.findRev( '/' );
  if( pos >= 0 )
    parent.truncate( pos+1 );
  else // relative path, do anything...
    parent = "/";

  return parent;
}


TQString K3b::fixupPath( const TQString& path )
{
  TQString s;
  bool lastWasSlash = false;
  for( unsigned int i = 0; i < path.length(); ++i ) {
    if( path[i] == '/' ) {
      if( !lastWasSlash ) {
	lastWasSlash = true;
	s.append( "/" );
      }
    }
    else {
      lastWasSlash = false;
      s.append( path[i] );
    }
  }

  return s;
}


K3bVersion K3b::kernelVersion()
{
  // initialize kernel version
  K3bVersion v;
  utsname unameinfo;
  if( ::uname(&unameinfo) == 0 ) {
    v = TQString::fromLocal8Bit( unameinfo.release );
    kdDebug() << "kernel version: " << v << endl;
  }
  else
    kdError() << "could not determine kernel version." << endl;
  return v;
}


K3bVersion K3b::simpleKernelVersion()
{
  return kernelVersion().simplify();
}


TQString K3b::systemName()
{
  TQString v;
  utsname unameinfo;
  if( ::uname(&unameinfo) == 0 ) {
    v = TQString::fromLocal8Bit( unameinfo.sysname );
  }
  else
    kdError() << "could not determine system name." << endl;
  return v;
}


bool K3b::kbFreeOnFs( const TQString& path, unsigned long& size, unsigned long& avail )
{
  struct statvfs fs;
  if( ::statvfs( TQFile::encodeName(path), &fs ) == 0 ) {
    unsigned long kBfak = fs.f_frsize/1024;

    size = fs.f_blocks*kBfak;
    avail = fs.f_bavail*kBfak;

    return true;
  }
  else
    return false;
}


TDEIO::filesize_t K3b::filesize( const KURL& url )
{
    if( url.isLocalFile() ) {
        k3b_struct_stat buf;
        if ( !k3b_stat( TQFile::encodeName( url.path() ), &buf ) ) {
            return (TDEIO::filesize_t)buf.st_size;
        }
    }

    TDEIO::UDSEntry uds;
    TDEIO::NetAccess::stat( url, uds, 0 );
    for( TDEIO::UDSEntry::const_iterator it = uds.begin(); it != uds.end(); ++it ) {
        if( (*it).m_uds == TDEIO::UDS_SIZE ) {
            return (*it).m_long;
        }
    }

    return ( TDEIO::filesize_t )0;
}


TDEIO::filesize_t K3b::imageFilesize( const KURL& url )
{
  TDEIO::filesize_t size = K3b::filesize( url );
  int cnt = 0;
  while( TDEIO::NetAccess::exists( KURL::fromPathOrURL( url.url() + '.' + TQString::number(cnt).rightJustify( 3, '0' ) ), true ) )
    size += K3b::filesize( KURL::fromPathOrURL( url.url() + '.' + TQString::number(cnt++).rightJustify( 3, '0' ) ) );
  return size;
}


TQString K3b::cutFilename( const TQString& name, unsigned int len )
{
  if( name.length() > len ) {
    TQString ret = name;

    // determine extension (we think of an extension to be at most 5 chars in length)
    int pos = name.find( '.', -6 );
    if( pos > 0 )
      len -= (name.length() - pos);

    ret.truncate( len );

    if( pos > 0 )
      ret.append( name.mid( pos ) );

    return ret;
  }
  else
    return name;
}


TQString K3b::removeFilenameExtension( const TQString& name )
{
  TQString v = name;
  int dotpos = v.findRev( '.' );
  if( dotpos > 0 )
    v.truncate( dotpos );
  return v;
}


TQString K3b::appendNumberToFilename( const TQString& name, int num, unsigned int maxlen )
{
  // determine extension (we think of an extension to be at most 5 chars in length)
  TQString result = name;
  TQString ext;
  int pos = name.find( '.', -6 );
  if( pos > 0 ) {
    ext = name.mid(pos);
    result.truncate( pos );
  }

  ext.prepend( TQString::number(num) );
  result.truncate( maxlen - ext.length() );

  return result + ext;
}


bool K3b::plainAtapiSupport()
{
  // FIXME: what about BSD?
  return ( K3b::simpleKernelVersion() >= K3bVersion( 2, 5, 40 ) );
}


bool K3b::hackedAtapiSupport()
{
  // IMPROVEME!!!
  // FIXME: since when does the kernel support this?
  return ( K3b::simpleKernelVersion() >= K3bVersion( 2, 4, 0 ) );
}


TQString K3b::externalBinDeviceParameter( K3bDevice::Device* dev, const K3bExternalBin* bin )
{
#ifdef Q_OS_LINUX
  //
  // experimental: always use block devices on 2.6 kernels
  //
  if( simpleKernelVersion() >= K3bVersion( 2, 6, 0 ) )
    return dev->blockDeviceName();
  else
#endif
  if( dev->interfaceType() == K3bDevice::SCSI )
    return dev->busTargetLun();
  else if( (plainAtapiSupport() && bin->hasFeature("plain-atapi") ) )
    return dev->blockDeviceName();
  else
    return TQString("ATAPI:%1").arg(dev->blockDeviceName());
}


int K3b::writingAppFromString( const TQString& s )
{
  if( s.lower() == "cdrdao" )
    return K3b::CDRDAO;
  else if( s.lower() == "cdrecord" )
    return K3b::CDRECORD;
  else if( s.lower() == "dvdrecord" )
    return K3b::DVDRECORD;
  else if( s.lower() == "growisofs" )
    return K3b::GROWISOFS;
  else if( s.lower() == "dvd+rw-format" )
    return K3b::DVD_RW_FORMAT;
  else
    return K3b::DEFAULT;
}


TQString K3b::writingModeString( int mode )
{
  if( mode == WRITING_MODE_AUTO )
    return i18n("Auto");
  else
    return K3bDevice::writingModeString( mode );
}


TQString K3b::resolveLink( const TQString& file )
{
  TQFileInfo f( file );
  TQStringList steps( f.absFilePath() );
  while( f.isSymLink() ) {
    TQString p = f.readLink();
    if( !p.startsWith( "/" ) )
      p.prepend( f.dirPath(true) + "/" );
    f.setFile( p );
    if( steps.contains( f.absFilePath() ) ) {
      kdDebug() << "(K3b) symlink loop detected." << endl;
      break;
    }
    else
      steps.append( f.absFilePath() );
  }
  return f.absFilePath();
}


K3bDevice::Device* K3b::urlToDevice( const KURL& deviceUrl )
{
  if( deviceUrl.protocol() == "media" || deviceUrl.protocol() == "system" ) {
    kdDebug() << "(K3b) Asking mediamanager for " << deviceUrl.fileName() << endl;
    DCOPRef mediamanager("kded", "mediamanager");
    DCOPReply reply = mediamanager.call("properties(TQString)", deviceUrl.fileName());
    TQStringList properties = reply;
    if( !reply.isValid() || properties.count() < 6 ) {
      kdError() << "(K3b) Invalid reply from mediamanager" << endl;
      return 0;
    }
    else {
      kdDebug() << "(K3b) Reply from mediamanager " << properties[5] << endl;
      return k3bcore->deviceManager()->findDevice( properties[5] );
    }
  }

  return k3bcore->deviceManager()->findDevice( deviceUrl.path() );
}


KURL K3b::convertToLocalUrl( const KURL& url )
{
  if( !url.isLocalFile() ) {
#if KDE_IS_VERSION(3,4,91)
    return TDEIO::NetAccess::mostLocalURL( url, 0 );
#else
#ifndef UDS_LOCAL_PATH
#define UDS_LOCAL_PATH (72 | TDEIO::UDS_STRING)
#else
    using namespace TDEIO;
#endif
    TDEIO::UDSEntry e;
    if( TDEIO::NetAccess::stat( url, e, 0 ) ) {
      const TDEIO::UDSEntry::ConstIterator end = e.end();
      for( TDEIO::UDSEntry::ConstIterator it = e.begin(); it != end; ++it ) {
	if( (*it).m_uds == UDS_LOCAL_PATH && !(*it).m_str.isEmpty() )
	  return KURL::fromPathOrURL( (*it).m_str );
      }
    }
#endif
  }

  return url;
}


KURL::List K3b::convertToLocalUrls( const KURL::List& urls )
{
  KURL::List r;
  for( KURL::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it )
    r.append( convertToLocalUrl( *it ) );
  return r;
}


TQ_INT16 K3b::fromLe16( char* data )
{
#ifdef WORDS_BIGENDIAN // __BYTE_ORDER == __BIG_ENDIAN
  return swapByteOrder( *((TQ_INT16*)data) );
#else
  return *((TQ_INT16*)data);
#endif
}


TQ_INT32 K3b::fromLe32( char* data )
{
#ifdef WORDS_BIGENDIAN // __BYTE_ORDER == __BIG_ENDIAN
  return swapByteOrder( *((TQ_INT32*)data) );
#else
  return *((TQ_INT32*)data);
#endif
}


TQ_INT64 K3b::fromLe64( char* data )
{
#ifdef WORDS_BIGENDIAN // __BYTE_ORDER == __BIG_ENDIAN
  return swapByteOrder( *((TQ_INT64*)data) );
#else
  return *((TQ_INT64*)data);
#endif
}


TQString K3b::findExe( const TQString& name )
{
  // first we search the path
  TQString bin = TDEStandardDirs::findExe( name );

  // then go on with our own little list
  if( bin.isEmpty() )
    bin = TDEStandardDirs::findExe( name, k3bcore->externalBinManager()->searchPath().join(":") );

  return bin;
}


bool K3b::isMounted( K3bDevice::Device* dev )
{
  if( !dev )
    return false;

  return !TDEIO::findDeviceMountPoint( dev->blockDeviceName() ).isEmpty();
}


bool K3b::unmount( K3bDevice::Device* dev )
{
  if( !dev )
    return false;

  TQString mntDev = dev->blockDeviceName();

#if KDE_IS_VERSION(3,4,0)
  // first try to unmount it the standard way
  if( TDEIO::NetAccess::synchronousRun( TDEIO::unmount( mntDev, false ), 0 ) )
    return true;
#endif

  TQString umountBin = K3b::findExe( "umount" );
  if( !umountBin.isEmpty() ) {
    TDEProcess p;
    p << umountBin;
    p << "-l"; // lazy unmount
    p << dev->blockDeviceName();
    p.start( TDEProcess::Block );
    if( !p.exitStatus() )
      return true;
  }

  // now try pmount
  TQString pumountBin = K3b::findExe( "pumount" );
  if( !pumountBin.isEmpty() ) {
    TDEProcess p;
    p << pumountBin;
    p << "-l"; // lazy unmount
    p << dev->blockDeviceName();
    p.start( TDEProcess::Block );
    return !p.exitStatus();
  }
  else {
#ifdef HAVE_HAL
    return !K3bDevice::HalConnection::instance()->unmount( dev );
#else
    return false;
#endif
  }
}


bool K3b::mount( K3bDevice::Device* dev )
{
  if( !dev )
    return false;

  TQString mntDev = dev->blockDeviceName();

#if KDE_IS_VERSION(3,4,0)
  // first try to mount it the standard way
  if( TDEIO::NetAccess::synchronousRun( TDEIO::mount( true, 0, mntDev, false ), 0 ) )
    return true;
#endif

#ifdef HAVE_HAL
  if( !K3bDevice::HalConnection::instance()->mount( dev ) )
    return true;
#endif

  // now try pmount
  TQString pmountBin = K3b::findExe( "pmount" );
  if( !pmountBin.isEmpty() ) {
    TDEProcess p;
    p << pmountBin;
    p << mntDev;
    p.start( TDEProcess::Block );
    return !p.exitStatus();
  }
  return false;
}


bool K3b::eject( K3bDevice::Device* dev )
{
#ifdef HAVE_HAL
  if( !K3bDevice::HalConnection::instance()->eject( dev ) )
    return true;
#endif

  if( K3b::isMounted( dev ) )
    K3b::unmount( dev );

  return dev->eject();
}
