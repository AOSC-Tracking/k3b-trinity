/* 
 *
 * $Id: k3baudiocdtrackdrag.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3baudiocdtrackdrag.h"

#include <k3bdevice.h>
#include <k3bdevicemanager.h>
#include <k3btoc.h>
#include <k3btrack.h>
#include <k3bcore.h>

#include <tqdatastream.h>
#include <tqcstring.h>


// FIXME: multiple tracks
K3bAudioCdTrackDrag::K3bAudioCdTrackDrag( const K3bDevice::Toc& toc, const TQValueList<int>& cdTrackNumbers, 
					  const K3bCddbResultEntry& cddb,
					  K3bDevice::Device* lastDev, TQWidget* dragSource, const char* name )
    : TQStoredDrag( "k3b/audio_track_drag", dragSource, name ),
    m_toc(toc),
    m_cdTrackNumbers(cdTrackNumbers),
    m_cddb(cddb),
    m_device(lastDev)
{
  TQByteArray data;
  TQDataStream s( data, IO_WriteOnly );
  s << (unsigned int)toc.count();
  for( K3bDevice::Toc::const_iterator it = toc.begin(); it != toc.end(); ++it ) {
    const K3bDevice::Track& track = *it;
    s << track.firstSector().lba() << track.lastSector().lba();
  }
  TQTextStream t( s.device() );
  t << cddb.cdArtist << endl
    << cddb.cdTitle << endl;
  for( unsigned int i = 0; i < toc.count(); ++i ) {
    t << cddb.artists[i] << endl
      << cddb.titles[i] << endl;
  }

  s << (unsigned int)cdTrackNumbers.count();

  for( TQValueList<int>::const_iterator it = cdTrackNumbers.begin();
       it != cdTrackNumbers.end(); ++it )
    s << *it;

  if( lastDev )
    t << lastDev->blockDeviceName() << endl;
  else
    t << endl;

  // TODO: the rest
  setEncodedData( data );
}


bool K3bAudioCdTrackDrag::decode( const TQMimeSource* e, 
				  K3bDevice::Toc& toc, TQValueList<int>& trackNumbers, 
				  K3bCddbResultEntry& cddb, K3bDevice::Device** dev )
{
  TQByteArray data = e->tqencodedData( "k3b/audio_track_drag" );

  TQDataStream s( data, IO_ReadOnly );

  unsigned int trackCnt;
  s >> trackCnt;
  for( unsigned int i = 0; i < trackCnt; ++i ) {
    int fs, ls;
    s >> fs;
    s >> ls;
    toc.append( K3bDevice::Track( fs, ls, K3bDevice::Track::AUDIO ) );
  }

  TQTextStream t( s.device() );
  cddb.artists.clear();
  cddb.titles.clear();
  cddb.cdArtist = t.readLine();
  cddb.cdTitle = t.readLine();
  for( unsigned int i = 0; i < trackCnt; ++i ) {
    cddb.artists.append( t.readLine() );
    cddb.titles.append( t.readLine() );
  }

  s >> trackCnt;
  trackNumbers.clear();
  for( unsigned int i = 0; i < trackCnt; ++i ) {
    int trackNumber = 0;
    s >> trackNumber;
    trackNumbers.append( trackNumber );
  }

  TQString devName = t.readLine();
  if( dev && !devName.isEmpty() )
    *dev = k3bcore->deviceManager()->findDevice( devName );

  return true;
}
