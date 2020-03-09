/* 
 *
 * $Id: k3baudioencoder.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3baudioencoder.h"

#include <tqfile.h>

#include <kdebug.h>


class K3bAudioEncoder::Private
{
public:
  Private()
    : outputFile(0) {
  }

  TQFile* outputFile;
  TQString outputFilename;

  TQString lastErrorString;
};


K3bAudioEncoder::K3bAudioEncoder( TQObject* parent, const char* name )
  : K3bPlugin( parent, name )
{
  d = new Private();
}


K3bAudioEncoder::~K3bAudioEncoder()
{
  closeFile();
  delete d;
}


bool K3bAudioEncoder::openFile( const TQString& ext, const TQString& filename, const K3b::Msf& length )
{
  closeFile();

  d->outputFile = new TQFile( filename );
  if( d->outputFile->open( IO_WriteOnly ) ) {
    return initEncoder( ext, length );
  }
  else {
    kdDebug() << "(K3bAudioEncoder) unable to open file " << filename << endl;
    closeFile();
    return false;
  }
}


bool K3bAudioEncoder::isOpen() const
{
  if( d->outputFile )
    return d->outputFile->isOpen();
  else
    return false;
}


void K3bAudioEncoder::closeFile()
{
  if( d->outputFile ) {
    finishEncoder();
    if( d->outputFile->isOpen() )
      d->outputFile->close();
    delete d->outputFile;
    d->outputFile = 0;
    d->outputFilename = TQString();
  }
}


const TQString& K3bAudioEncoder::filename() const
{
  if( d->outputFile )
    return d->outputFilename;
  else
    return TQString::null;
}



void K3bAudioEncoder::setMetaData( K3bAudioEncoder::MetaDataField f, const TQString& data )
{
  if( !data.isEmpty() )
    return setMetaDataInternal( f, data );
}


long K3bAudioEncoder::encode( const char* data, TQ_ULONG len )
{
  return encodeInternal( data, len );
}


bool K3bAudioEncoder::initEncoder( const TQString& ext, const K3b::Msf& length )
{
  if( !isOpen() ) {
    kdDebug() << "(K3bAudioEncoder) call to initEncoder without openFile!" << endl;
    return false;
  }

  return initEncoderInternal( ext, length );
}


TQ_LONG K3bAudioEncoder::writeData( const char* data, TQ_ULONG len )
{
  if( d->outputFile ) {
    return d->outputFile->writeBlock( data, len );
  }
  else {
    kdDebug() << "(K3bAudioEncoder) call to writeData without opening a file first." << endl;
    return -1;
  }
}


bool K3bAudioEncoder::initEncoderInternal( const TQString&, const K3b::Msf& )
{
  // do nothing
  return true;
}


void K3bAudioEncoder::setMetaDataInternal( K3bAudioEncoder::MetaDataField, const TQString& )
{
  // do nothing
}


void K3bAudioEncoder::finishEncoder()
{
  if( isOpen() )
    finishEncoderInternal();
}


void K3bAudioEncoder::finishEncoderInternal()
{
  // do nothing
}


void K3bAudioEncoder::setLastError( const TQString& e )
{
  d->lastErrorString = e;
}


TQString K3bAudioEncoder::lastErrorString() const
{
  if( d->lastErrorString.isEmpty() )
    return i18n("An unknown error occurred.");
  else
    return d->lastErrorString;
}

#include "k3baudioencoder.moc"
