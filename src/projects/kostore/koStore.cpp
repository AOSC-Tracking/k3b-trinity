/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>, Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "koStore.h"
//#include "koTarStore.h"
#include "koZipStore.h"
//#include "koDirectoryStore.h"

#include <tqfileinfo.h>
#include <tqfile.h>
#include <tqdir.h>

#include <kurl.h>
#include <kdebug.h>
#include <tdeversion.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdeio/netaccess.h>

//#define DefaultFormat KoStore::Tar
#define DefaultFormat KoStore::Zip

const int KoStore::s_area = 30002;

KoStore::Backend KoStore::determineBackend( TQIODevice* dev )
{
    unsigned char buf[5];
    if ( dev->readBlock( (char *)buf, 4 ) < 4 )
      return DefaultFormat; // will create a "bad" store (bad()==true)
    if ( buf[0] == 0037 && buf[1] == 0213 ) // gzip -> tar.gz
      return Tar;
    if ( buf[0] == 'P' && buf[1] == 'K' && buf[2] == 3 && buf[3] == 4 )
      return Zip;
    return DefaultFormat; // fallback
}

KoStore* KoStore::createStore( const TQString& fileName, Mode mode, const TQCString & appIdentification, Backend backend )
{
  if ( backend == Auto ) {
    if ( mode == KoStore::Write )
      backend = DefaultFormat;
    else
    {
      TQFileInfo inf( fileName );
      if ( inf.isDir() )
        backend = Directory;
      else
      {
        TQFile file( fileName );
        if ( file.open( IO_ReadOnly ) )
          backend = determineBackend( TQT_TQIODEVICE(&file) );
        else
          backend = DefaultFormat; // will create a "bad" store (bad()==true)
      }
    }
  }
  switch ( backend )
  {
//   case Tar:
//     return new KoTarStore( fileName, mode, appIdentification );
  case Zip:
    return new KoZipStore( fileName, mode, appIdentification );
//   case Directory:
//     return new KoDirectoryStore( fileName /* should be a dir name.... */, mode );
  default:
    kdWarning(s_area) << "Unsupported backend requested for KoStore : " << backend << endl;
    return 0L;
  }
}

KoStore* KoStore::createStore( TQIODevice *device, Mode mode, const TQCString & appIdentification, Backend backend )
{
  if ( backend == Auto )
  {
    if ( mode == KoStore::Write )
      backend = DefaultFormat;
    else {
      if ( device->open( IO_ReadOnly ) ) {
        backend = determineBackend( device );
        device->close();
      }
    }
  }
  switch ( backend )
  {
//   case Tar:
//     return new KoTarStore( device, mode, appIdentification );
//   case Directory:
//     kdError(s_area) << "Can't create a Directory store for a memory buffer!" << endl;
    // fallback
  case Zip:
    return new KoZipStore( device, mode, appIdentification );
  default:
    kdWarning(s_area) << "Unsupported backend requested for KoStore : " << backend << endl;
    return 0L;
  }
}

KoStore* KoStore::createStore( TQWidget* window, const KURL& url, Mode mode, const TQCString & appIdentification, Backend backend )
{
  if ( url.isLocalFile() )
    return createStore(url.path(), mode,  appIdentification, backend );

  TQString tmpFile;
  if ( mode == KoStore::Write )
  {
    if ( backend == Auto )
      backend = DefaultFormat;
  }
  else
  {
    const bool downloaded =
        TDEIO::NetAccess::download( url, tmpFile, window );

    if (!downloaded)
    {
      kdError(s_area) << "Could not download file!" << endl;
      backend = DefaultFormat; // will create a "bad" store (bad()==true)
    }
    else if ( backend == Auto )
    {
      TQFile file( tmpFile );
      if ( file.open( IO_ReadOnly ) )
      {
        backend = determineBackend( TQT_TQIODEVICE(&file) );
        file.close();
      }
    }
  }
  switch ( backend )
  {
//   case Tar:
//     return new KoTarStore( window, url, tmpFile, mode, appIdentification );
  case Zip:
    return new KoZipStore( window, url, tmpFile, mode, appIdentification );
  default:
    kdWarning(s_area) << "Unsupported backend requested for KoStore (KURL) : " << backend << endl;
    KMessageBox::sorry( window,
        i18n("The directory mode is not supported for remote locations."),
        i18n("KOffice Storage"));
    return 0L;
  }
}

namespace {
  const char* const ROOTPART = "root";
  const char* const MAINNAME = "maindoc.xml";
}

bool KoStore::init( Mode _mode )
{
  d = 0;
  m_bIsOpen = false;
  m_mode = _mode;
  m_stream = 0;

  // Assume new style names.
  m_namingVersion = NAMING_VERSION_2_2;
  return true;
}

KoStore::~KoStore()
{
  delete m_stream;
}

bool KoStore::open( const TQString & _name )
{
  // This also converts from relative to absolute, i.e. merges the currentPath()
  m_sName = toExternalNaming( _name );

  if ( m_bIsOpen )
  {
    kdWarning(s_area) << "KoStore: File is already opened" << endl;
    //return TDEIO::ERR_INTERNAL;
    return false;
  }

  if ( m_sName.length() > 512 )
  {
      kdError(s_area) << "KoStore: Filename " << m_sName << " is too long" << endl;
      //return TDEIO::ERR_MALFORMED_URL;
      return false;
  }

  if ( m_mode == Write )
  {
    kdDebug(s_area) << "KoStore: opening for writing '" << m_sName << "'" << endl;
    if ( m_strFiles.findIndex( m_sName ) != -1 ) // just check if it's there
    {
      kdWarning(s_area) << "KoStore: Duplicate filename " << m_sName << endl;
      //return TDEIO::ERR_FILE_ALREADY_EXIST;
      return false;
    }

    m_strFiles.append( m_sName );

    m_iSize = 0;
    if ( !openWrite( m_sName ) )
      return false;
  }
  else if ( m_mode == Read )
  {
    kdDebug(s_area) << "Opening for reading '" << m_sName << "'" << endl;
    if ( !openRead( m_sName ) )
      return false;
  }
  else
    //return TDEIO::ERR_UNSUPPORTED_ACTION;
    return false;

  m_bIsOpen = true;
  return true;
}

bool KoStore::isOpen() const
{
  return m_bIsOpen;
}

bool KoStore::close()
{
  kdDebug(s_area) << "KoStore: Closing" << endl;

  if ( !m_bIsOpen )
  {
    kdWarning(s_area) << "KoStore: You must open before closing" << endl;
    //return TDEIO::ERR_INTERNAL;
    return false;
  }

  bool ret = m_mode == Write ? closeWrite() : closeRead();

  delete m_stream;
  m_stream = 0L;
  m_bIsOpen = false;
  return ret;
}

TQIODevice* KoStore::device() const
{
  if ( !m_bIsOpen )
    kdWarning(s_area) << "KoStore: You must open before asking for a device" << endl;
  if ( m_mode != Read )
    kdWarning(s_area) << "KoStore: Can not get device from store that is opened for writing" << endl;
  return m_stream;
}

TQByteArray KoStore::read( unsigned long int max )
{
  TQByteArray data; // Data is a TQArray<char>

  if ( !m_bIsOpen )
  {
    kdWarning(s_area) << "KoStore: You must open before reading" << endl;
    data.resize( 0 );
    return data;
  }
  if ( m_mode != Read )
  {
    kdError(s_area) << "KoStore: Can not read from store that is opened for writing" << endl;
    data.resize( 0 );
    return data;
  }

  if ( m_stream->atEnd() )
  {
    data.resize( 0 );
    return data;
  }

  if ( max > m_iSize - m_stream->at() )
    max = m_iSize - m_stream->at();
  if ( max == 0 )
  {
    data.resize( 0 );
    return data;
  }

  char *p = new char[ max ];
  m_stream->readBlock( p, max );

  data.setRawData( p, max );
  return data;
}

TQ_LONG KoStore::write( const TQByteArray& data )
{
  return write( data.data(), data.size() ); // see below
}

TQ_LONG KoStore::read( char *_buffer, TQ_ULONG _len )
{
  if ( !m_bIsOpen )
  {
    kdError(s_area) << "KoStore: You must open before reading" << endl;
    return -1;
  }
  if ( m_mode != Read )
  {
    kdError(s_area) << "KoStore: Can not read from store that is opened for writing" << endl;
    return -1;
  }

  if ( m_stream->atEnd() )
    return 0;

  if ( _len > m_iSize - m_stream->at() )
    _len = m_iSize - m_stream->at();
  if ( _len == 0 )
    return 0;

  return m_stream->readBlock( _buffer, _len );
}

TQ_LONG KoStore::write( const char* _data, TQ_ULONG _len )
{
  if ( _len == 0L ) return 0;

  if ( !m_bIsOpen )
  {
    kdError(s_area) << "KoStore: You must open before writing" << endl;
    return 0L;
  }
  if ( m_mode != Write  )
  {
    kdError(s_area) << "KoStore: Can not write to store that is opened for reading" << endl;
    return 0L;
  }

  int nwritten = m_stream->writeBlock( _data, _len );
  Q_ASSERT( nwritten == (int)_len );
  m_iSize += nwritten;

  return nwritten;
}

TQIODevice::Offset KoStore::size() const
{
  if ( !m_bIsOpen )
  {
    kdWarning(s_area) << "KoStore: You must open before asking for a size" << endl;
    return static_cast<TQIODevice::Offset>(-1);
  }
  if ( m_mode != Read )
  {
    kdWarning(s_area) << "KoStore: Can not get size from store that is opened for writing" << endl;
    return static_cast<TQIODevice::Offset>(-1);
  }
  return m_iSize;
}

bool KoStore::enterDirectory( const TQString& directory )
{
  //kdDebug(s_area) << "KoStore::enterDirectory " << directory << endl;
  int pos;
  bool success = true;
  TQString tmp( directory );

  while ( ( pos = tmp.find( '/' ) ) != -1 &&
          ( success = enterDirectoryInternal( tmp.left( pos ) ) ) )
          tmp = tmp.mid( pos + 1 );

  if ( success && !tmp.isEmpty() )
    return enterDirectoryInternal( tmp );
  return success;
}

bool KoStore::leaveDirectory()
{
  if ( m_currentPath.isEmpty() )
    return false;

  m_currentPath.pop_back();

  return enterAbsoluteDirectory( expandEncodedDirectory( currentPath() ) );
}

TQString KoStore::currentDirectory() const
{
  return expandEncodedDirectory( currentPath() );
}

TQString KoStore::currentPath() const
{
  TQString path;
  TQStringList::ConstIterator it = m_currentPath.begin();
  TQStringList::ConstIterator end = m_currentPath.end();
  for ( ; it != end; ++it ) {
    path += *it;
    path += '/';
  }
  return path;
}

void KoStore::pushDirectory()
{
  m_directoryStack.push( currentPath() );
}

void KoStore::popDirectory()
{
  m_currentPath.clear();
  enterAbsoluteDirectory( TQString() );
  enterDirectory( m_directoryStack.pop() );
}

bool KoStore::addLocalFile( const TQString &fileName, const TQString &destName )
{
  TQFileInfo fi( fileName );
  uint size = fi.size();
  TQFile file( fileName );
  if ( !file.open( IO_ReadOnly ))
  {
    return false;
  }

  if ( !open ( destName ) )
  {
    return false;
  }

  TQByteArray data ( 8 * 1024 );

  uint total = 0;
  for ( int block = 0; ( block = file.readBlock ( data.data(), data.size() ) ) > 0; total += block )
  {
    data.resize(block);
    if ( write( data ) != block )
      return false;
    data.resize(8*1024);
  }
  Q_ASSERT( total == size );

  close();
  file.close();

  return true;
}

bool KoStore::extractFile ( const TQString &srcName, const TQString &fileName )
{
  if ( !open ( srcName ) )
    return false;

  TQFile file( fileName );

  if( !file.open ( IO_WriteOnly ) )
  {
    close();
    return false;
  }

  TQByteArray data ( 8 * 1024 );
  uint total = 0;
  for( int block = 0; ( block = read ( data.data(), data.size() ) ) > 0; total += block )
  {
    file.writeBlock ( data.data(), block );
  }

  if( size() != static_cast<TQIODevice::Offset>(-1) )
  	Q_ASSERT( total == size() );

  file.close();
  close();

  return true;
}

TQStringList KoStore::addLocalDirectory( const TQString &dirPath, const TQString &destName )
{
  TQString dot = ".";
  TQString dotdot = "..";
  TQStringList content;

  TQDir dir(dirPath);
  if ( !dir.exists() )
    return 0;

  TQStringList files = dir.entryList();
  for ( TQStringList::Iterator it = files.begin(); it != files.end(); ++it )
  {
     if ( *it != dot && *it != dotdot )
     {
        TQString currentFile = dirPath + "/" + *it;
        TQString dest = destName.isEmpty() ? *it : (destName + "/" + *it);

        TQFileInfo fi ( currentFile );
        if ( fi.isFile() )
        {
          addLocalFile ( currentFile, dest );
          content.append(dest);
        }
        else if ( fi.isDir() )
        {
          content += addLocalDirectory ( currentFile, dest );
        }
     }
  }

  return content;
}


bool KoStore::at( TQIODevice::Offset pos )
{
  return m_stream->at( pos );
}

TQIODevice::Offset KoStore::at() const
{
  return m_stream->at();
}

bool KoStore::atEnd() const
{
  return m_stream->atEnd();
}

// See the specification for details of what this function does.
TQString KoStore::toExternalNaming( const TQString & _internalNaming ) const
{
  if ( _internalNaming == ROOTPART )
    return expandEncodedDirectory( currentPath() ) + MAINNAME;

  TQString intern;
  if ( _internalNaming.startsWith( "tar:/" ) ) // absolute reference
    intern = _internalNaming.mid( 5 ); // remove protocol
  else
    intern = currentPath() + _internalNaming;

  return expandEncodedPath( intern );
}

TQString KoStore::expandEncodedPath( TQString intern ) const
{
  if ( m_namingVersion == NAMING_VERSION_RAW )
    return intern;

  TQString result;
  int pos;

  if ( ( pos = intern.findRev( '/', -1 ) ) != -1 ) {
    result = expandEncodedDirectory( intern.left( pos ) ) + '/';
    intern = intern.mid( pos + 1 );
  }

  // Now process the filename. If the first character is numeric, we have
  // a main document.
  if ( TQChar(intern.at(0)).isDigit() )
  {
    // If this is the first part name, check if we have a store with
    // old-style names.
    if ( ( m_namingVersion == NAMING_VERSION_2_2 ) &&
         ( m_mode == Read ) &&
         ( fileExists( result + "part" + intern + ".xml" ) ) )
      m_namingVersion = NAMING_VERSION_2_1;

    if ( m_namingVersion == NAMING_VERSION_2_1 )
      result = result + "part" + intern + ".xml";
    else
      result = result + "part" + intern + "/" + MAINNAME;
  }
  else
    result += intern;
  return result;
}

TQString KoStore::expandEncodedDirectory( TQString intern ) const
{
  if ( m_namingVersion == NAMING_VERSION_RAW )
    return intern;

  TQString result;
  int pos;
  while ( ( pos = intern.find( '/' ) ) != -1 ) {
    if ( TQChar(intern.at(0)).isDigit() )
      result += "part";
    result += intern.left( pos + 1 ); // copy numbers (or "pictures") + "/"
    intern = intern.mid( pos + 1 ); // remove the dir we just processed
  }

  if ( TQChar(intern.at(0)).isDigit() )
    result += "part";
  result += intern;
  return result;
}

bool KoStore::enterDirectoryInternal( const TQString& directory )
{
    if ( enterRelativeDirectory( expandEncodedDirectory( directory ) ) )
    {
      m_currentPath.append( directory );
      return true;
    }
    return false;
}

void KoStore::disallowNameExpansion( void )
{
    m_namingVersion = NAMING_VERSION_RAW;
}

bool KoStore::hasFile( const TQString& fileName ) const
{
  return fileExists( toExternalNaming( currentPath() + fileName ) );
}
