/* This file is part of the KDE project
   Copyright (C) 2000-2002 David Faure <faure@kde.org>

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

#include "koZipStore.h"

#include <tqbuffer.h>

#include <kzip.h>
#include <kdebug.h>
#include <tdeversion.h>
#include <kurl.h>
#include <tdeio/netaccess.h>
#if ! KDE_IS_VERSION( 3, 4, 1 )
#include <tqdir.h>
#include <tqfileinfo.h>
#endif

KoZipStore::KoZipStore( const TQString & _filename, Mode _mode, const TQCString & appIdentification )
{
    kdDebug(s_area) << "KoZipStore Constructor filename = " << _filename
                    << " mode = " << int(_mode)
                    << " mimetype = " << appIdentification << endl;

    m_pZip = new KZip( _filename );

#if ! KDE_IS_VERSION( 3, 4, 1 )
    // Workaround for KZip KSaveFile double deletion in tdelibs-3.4,
    // when trying to write to a non-writable directory.
    TQDir dir( TQFileInfo( _filename ).dir() );
    if (_mode == Write && !TQFileInfo( dir.path() ).isWritable()  )
    {
        kdWarning(s_area) << dir.path() << " isn't writable" << endl;
        m_bGood = false;
        m_currentDir = 0;
        KoStore::init( _mode );
    }
    else
#endif
    {
        m_bGood = init( _mode, appIdentification ); // open the zip file and init some vars
    }
}

KoZipStore::KoZipStore( TQIODevice *dev, Mode mode, const TQCString & appIdentification )
{
    m_pZip = new KZip( dev );
    m_bGood = init( mode, appIdentification );
}

KoZipStore::KoZipStore( TQWidget* window, const KURL & _url, const TQString & _filename, Mode _mode, const TQCString & appIdentification )
{
    kdDebug(s_area) << "KoZipStore Constructor url" << _url.prettyURL()
                    << " filename = " << _filename
                    << " mode = " << int(_mode)
                    << " mimetype = " << appIdentification << endl;

    m_url = _url;
    m_window = window;

    if ( _mode == KoStore::Read )
    {
        m_fileMode = KoStoreBase::RemoteRead;
        m_localFileName = _filename;

    }
    else
    {
        m_fileMode = KoStoreBase::RemoteWrite;
        m_localFileName = "/tmp/kozip"; // ### FIXME with KTempFile
    }

    m_pZip = new KZip( m_localFileName );
    m_bGood = init( _mode, appIdentification ); // open the zip file and init some vars
}

KoZipStore::~KoZipStore()
{
    kdDebug(s_area) << "KoZipStore::~KoZipStore" << endl;
    m_pZip->close();
    delete m_pZip;

    // Now we have still some job to do for remote files.
    if ( m_fileMode == KoStoreBase::RemoteRead )
    {
        TDEIO::NetAccess::removeTempFile( m_localFileName );
    }
    else if ( m_fileMode == KoStoreBase::RemoteWrite )
    {
        TDEIO::NetAccess::upload( m_localFileName, m_url, m_window );
        // ### FIXME: delete temp file
    }
}

bool KoZipStore::init( Mode _mode, const TQCString& appIdentification )
{
    KoStore::init( _mode );
    m_currentDir = 0;
    bool good = m_pZip->open( _mode == Write ? IO_WriteOnly : IO_ReadOnly );

    if ( good && _mode == Read )
        good = m_pZip->directory() != 0;
    else if ( good && _mode == Write )
    {
        //kdDebug(s_area) << "KoZipStore::init writing mimetype " << appIdentification << endl;

        m_pZip->setCompression( KZip::NoCompression );
        m_pZip->setExtraField( KZip::NoExtraField );
        // Write identification
        (void)m_pZip->writeFile( "mimetype", "", "", appIdentification.length(), appIdentification.data() );
        m_pZip->setCompression( KZip::DeflateCompression );
        // We don't need the extra field in KOffice - so we leave it as "no extra field".
    }
    return good;
}

bool KoZipStore::openWrite( const TQString& name )
{
#if 0
    // Prepare memory buffer for writing
    m_byteArray.resize( 0 );
    m_stream = new TQBuffer( m_byteArray );
    m_stream->open( IO_WriteOnly );
    return true;
#endif
    m_stream = 0L; // Don't use!
    return m_pZip->prepareWriting( name, "", "" /*m_pZip->rootDir()->user(), m_pZip->rootDir()->group()*/, 0 );
}

bool KoZipStore::openRead( const TQString& name )
{
    const KArchiveEntry * entry = m_pZip->directory()->entry( name );
    if ( entry == 0L )
    {
        //kdWarning(s_area) << "Unknown filename " << name << endl;
        //return TDEIO::ERR_DOES_NOT_EXIST;
        return false;
    }
    if ( entry->isDirectory() )
    {
        kdWarning(s_area) << name << " is a directory !" << endl;
        //return TDEIO::ERR_IS_DIRECTORY;
        return false;
    }
    // Must cast to KZipFileEntry, not only KArchiveFile, because device() isn't virtual!
    const KZipFileEntry * f = static_cast<const KZipFileEntry *>(entry);
    delete m_stream;
    m_stream = f->device();
    m_iSize = f->size();
    return true;
}

TQ_LONG KoZipStore::write( const char* _data, TQ_ULONG _len )
{
  if ( _len == 0L ) return 0;
  //kdDebug(s_area) << "KoZipStore::write " << _len << endl;

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

  m_iSize += _len;
  if ( m_pZip->writeData( _data, _len ) ) // writeData returns a bool!
      return _len;
  return 0L;
}

bool KoZipStore::closeWrite()
{
    kdDebug(s_area) << "Wrote file " << m_sName << " into ZIP archive. size "
                    << m_iSize << endl;
    return m_pZip->doneWriting( m_iSize );
#if 0
    if ( !m_pZip->writeFile( m_sName , "user", "group", m_iSize, m_byteArray.data() ) )
        kdWarning( s_area ) << "Failed to write " << m_sName << endl;
    m_byteArray.resize( 0 ); // save memory
    return true;
#endif
}

bool KoZipStore::enterRelativeDirectory( const TQString& dirName )
{
    if ( m_mode == Read ) {
        if ( !m_currentDir ) {
            m_currentDir = m_pZip->directory(); // initialize
            Q_ASSERT( m_currentPath.isEmpty() );
        }
        const KArchiveEntry *entry = m_currentDir->entry( dirName );
        if ( entry && entry->isDirectory() ) {
            m_currentDir = dynamic_cast<const KArchiveDirectory*>( entry );
            return m_currentDir != 0;
        }
        return false;
    }
    else  // Write, no checking here
        return true;
}

bool KoZipStore::enterAbsoluteDirectory( const TQString& path )
{
    if ( path.isEmpty() )
    {
        m_currentDir = 0;
        return true;
    }
    m_currentDir = dynamic_cast<const KArchiveDirectory*>( m_pZip->directory()->entry( path ) );
    Q_ASSERT( m_currentDir );
    return m_currentDir != 0;
}

bool KoZipStore::fileExists( const TQString& absPath ) const
{
    const KArchiveEntry *entry = m_pZip->directory()->entry( absPath );
    return entry && entry->isFile();
}
