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

#include "k3bfilesplitter.h"
#include "k3bfilesysteminfo.h"

#include <kdebug.h>

#include <tqfile.h>


class K3bFileSplitter::Private
{
public:
  Private( K3bFileSplitter* splitter )
    : m_splitter( splitter ) {
  }

  TQString filename;
  TQFile file;
  int counter;

  // TQIODevice::Offset is too small on most compilations
  TDEIO::filesize_t maxFileSize;

  TDEIO::filesize_t currentOverallPos;
  TDEIO::filesize_t currentFilePos;

  void determineMaxFileSize() {
    if( maxFileSize == 0 ) {
      if( K3bFileSystemInfo( filename ).type() == K3bFileSystemInfo::FS_FAT )
	maxFileSize = 1024ULL*1024ULL*1024ULL; // 1GB
      else
	maxFileSize = 1024ULL*1024ULL*1024ULL*1024ULL*1024ULL;  // incredibly big, 1024 TB
    }
  }

  TQString buildFileName( int counter ) {
    if( counter > 0 )
      return filename + '.' + TQString::number(counter).rightJustify( 3, '0' );
    else
      return filename;
  }

  TQString currentFileName() {
    return buildFileName( counter );
  }

  bool openPrevFile() {
    return openFile( --counter );
  }

  bool openNextFile() {
    return openFile( ++counter );
  }

  bool openFile( int counter ) {
    file.close();
    file.setName( buildFileName( counter ) );
    currentFilePos = 0;
    if( file.open( m_splitter->mode() ) ) {
      m_splitter->setState( IO_Open );
      return true;
    }
    else {
      m_splitter->setState( ~IO_Open );
      return false;
    }
  }

private:
  K3bFileSplitter* m_splitter;
};


K3bFileSplitter::K3bFileSplitter()
{
  d = new Private( this );
}


K3bFileSplitter::K3bFileSplitter( const TQString& filename )
{
  d = new Private( this );
  setName( filename );
}


K3bFileSplitter::~K3bFileSplitter()
{
  delete d;
}


const TQString& K3bFileSplitter::name() const
{
  return d->filename;
}


void K3bFileSplitter::setName( const TQString& filename )
{
  close();
  d->maxFileSize = 0;
  d->filename = filename;
}


bool K3bFileSplitter::open( int mode )
{
  close();

  d->determineMaxFileSize();

  d->counter = 0;
  d->currentFilePos = 0;
  d->currentOverallPos = 0;
  setMode( mode );

  return d->openFile( 0 );
}


void K3bFileSplitter::close()
{
  d->file.close();
  d->counter = 0;
  d->currentFilePos = 0;
  d->currentOverallPos = 0;
}


int K3bFileSplitter::handle() const
{
  // FIXME: use a K3bPipe to simulate this
  return -1;
}



void K3bFileSplitter::flush()
{
  d->file.flush();
}

#ifdef USE_QT4
qint64 K3bFileSplitter::size() const
#else // USE_QT4
TQIODevice::Offset K3bFileSplitter::size() const
#endif // USE_QT4
{
  // not implemented due to Offset size limitations
  return 0;
}


TQIODevice::Offset K3bFileSplitter::at() const
{
  return d->currentOverallPos;
}


bool K3bFileSplitter::at( TQIODevice::Offset pos )
{
  Q_UNUSED( pos );
  // not implemented due to Offset size limitations
  return false;
}


bool K3bFileSplitter::atEnd() const
{
  return d->file.atEnd() && !TQFile::exists( d->buildFileName( d->counter+1 ) );
}


TQ_LONG K3bFileSplitter::readBlock( char *data, TQ_ULONG maxlen )
{
  TQ_LONG r = d->file.readBlock( data, maxlen );
  if( r == 0 ) {
    if( atEnd() ) {
      return r;
    }
    else if( d->openNextFile() ) {
      // recursively call us
      return readBlock( data, maxlen );
    }
  }
  else if( r > 0 ) {
    d->currentOverallPos += r;
    d->currentFilePos += r;
  }

  return r;
}


TQ_LONG K3bFileSplitter::writeBlock( const char *data, TQ_ULONG len )
{
  // We cannot rely on TQFile::at since it uses long on most copmpilations
  TQ_ULONG max = (TQ_ULONG)TQMIN( (TDEIO::filesize_t)len, d->maxFileSize - d->currentFilePos );

  TQ_LONG r = d->file.writeBlock( data, max );

  if( r < 0 )
    return r;

  d->currentOverallPos += r;
  d->currentFilePos += r;

  // recursively call us
  if( (TQ_ULONG)r < len ) {
    if( d->openNextFile() )
      return r + writeBlock( data+r, len-r );
    else
      return -1;
  }
  else
    return r;
}


int K3bFileSplitter::getch()
{
  int r = d->file.getch();
  if( r == -1 ) {
    if( !d->file.atEnd() ) {
      return -1;
    }
    else if( !atEnd() ) {
      if( !d->openNextFile() )
	return -1;
      else
	return getch();
    }
  }

  d->currentOverallPos++;
  d->currentFilePos++;

  return r;
}


int K3bFileSplitter::putch( int c )
{
  if( d->currentFilePos < d->maxFileSize ) {
    d->currentOverallPos++;
    d->currentFilePos++;
    return d->file.putch( c );
  }
  else if( d->openNextFile() ) {
    // recursively call us
    return putch( c );
  }
  else
    return -1;
}


int K3bFileSplitter::ungetch( int c )
{
  if( d->currentFilePos > 0 ) {
    int r = d->file.ungetch( c );
    if( r != -1 ) {
      d->currentOverallPos--;
      d->currentFilePos--;
    }
    return r;
  }
  else if( d->counter > 0 ) {
    // open prev file
    if( d->openPrevFile() ) {
      // seek to the end
      d->file.at( d->file.size() );
      d->currentFilePos = d->file.at();
      return getch();
    }
    else
      return -1;
  }
  else
    return -1;
}


void K3bFileSplitter::remove()
{
  close();
  while( TQFile::exists( d->buildFileName( d->counter ) ) )
    TQFile::remove( d->buildFileName( d->counter++ ) );
}


void K3bFileSplitter::setMaxFileSize( TDEIO::filesize_t size )
{
  d->maxFileSize = size;
}
