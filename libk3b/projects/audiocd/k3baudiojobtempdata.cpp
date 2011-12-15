/* 
 *
 * $Id: k3baudiojobtempdata.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3baudiojobtempdata.h"
#include "k3baudiodoc.h"
#include "k3baudiotrack.h"
#include <k3bglobals.h>
#include <k3bversion.h>
#include <k3bmsf.h>
#include <k3bcore.h>

#include <tqfile.h>
#include <textstream.h>
#include <tqvaluevector.h>

#include <kdebug.h>


class K3bAudioJobTempData::Private
{
public:
  Private( K3bAudioDoc* _doc ) 
    : doc(_doc) {
  }

  TQValueVector<TQString> bufferFiles;
  TQValueVector<TQString> infFiles;
  TQString tocFile;

  K3bAudioDoc* doc;
};


K3bAudioJobTempData::K3bAudioJobTempData( K3bAudioDoc* doc, TQObject* parent, const char* name )
  : TQObject( parent, name )
{
  d = new Private( doc );
}


K3bAudioJobTempData::~K3bAudioJobTempData()
{
  delete d;
}


const TQString& K3bAudioJobTempData::bufferFileName( int track )
{
  if( (int)d->bufferFiles.count() < track )
    prepareTempFileNames();
  return d->bufferFiles.at(track-1);
}

const TQString& K3bAudioJobTempData::bufferFileName( K3bAudioTrack* track )
{
  return bufferFileName( track->trackNumber() );
}


const TQString& K3bAudioJobTempData::tocFileName()
{
  if( d->tocFile.isEmpty() )
    prepareTempFileNames();
  return d->tocFile;
}


const TQString& K3bAudioJobTempData::infFileName( int track )
{
  if( (int)d->infFiles.count() < track )
    prepareTempFileNames();
  return d->infFiles.at( track - 1 );
}

const TQString& K3bAudioJobTempData::infFileName( K3bAudioTrack* track )
{
  return infFileName( track->trackNumber() );
}


K3bAudioDoc* K3bAudioJobTempData::doc() const
{
  return d->doc;
}


void K3bAudioJobTempData::prepareTempFileNames( const TQString& path ) 
{
  d->bufferFiles.clear();
  d->infFiles.clear();

  TQString prefix = K3b::findUniqueFilePrefix( "k3b_audio_", path ) + "_";

  for( int i = 0; i < d->doc->numOfTracks(); i++ ) {
    d->bufferFiles.append( prefix + TQString::number( i+1 ).rightJustify( 2, '0' ) + ".wav" );
    d->infFiles.append( prefix + TQString::number( i+1 ).rightJustify( 2, '0' ) + ".inf" );
  }

  d->tocFile = prefix + ".toc";
}


void K3bAudioJobTempData::cleanup()
{
  for( uint i = 0; i < d->infFiles.count(); ++i ) {
    if( TQFile::exists( d->infFiles[i] ) )
      TQFile::remove(  d->infFiles[i] );
  }

  for( uint i = 0; i < d->bufferFiles.count(); ++i ) {
    if( TQFile::exists( d->bufferFiles[i] ) )
      TQFile::remove(  d->bufferFiles[i] );
  }

  if( TQFile::exists( d->tocFile ) )
    TQFile::remove(  d->tocFile );
}


#include "k3baudiojobtempdata.moc"
