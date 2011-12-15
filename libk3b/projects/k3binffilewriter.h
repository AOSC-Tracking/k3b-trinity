/* 
 *
 * $Id: k3binffilewriter.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_INF_FILE_WRITER_H_
#define _K3B_INF_FILE_WRITER_H_

#include <tqvaluevector.h>
#include <textstream.h>

#include <k3bmsf.h>


namespace K3bDevice {
  class Track;
  class TrackCdText;
  class CdText;
}


class K3bInfFileWriter
{
 public:
  K3bInfFileWriter();

  bool save( TQTextStream& );
  bool save( const TQString& filename );

  /**
   * Use this to set:
   * @li trackStart
   * @li trackLength
   * @li index0
   * @li all indices
   * @li preemphasis
   * @li copyPermitted
   * @li ISRC
   *
   * Endianess is set to big.
   *
   * Tracknumber needs to be set manually.
   */
  void setTrack( const K3bDevice::Track& );

  void clearIndices() { m_indices.clear(); }

  /**
   * This is relative to the track start
   */
  void setIndex0( int i ) { m_index0 = i; }
  void addIndex( long i );

  void setTrackNumber( int i ) { m_trackNumber = i; }

  void setTrackStart( const K3b::Msf& i ) { m_trackStart = i; }
  void setTrackLength( const K3b::Msf& i ) { m_trackLength = i; }

  void setPreEmphasis( bool b ) { m_preEmphasis = b; }
  void setCopyPermitted( bool b ) { m_copyPermitted = b; }

  /**
   * Cdrecord seems to ignore this anyway and always expect big endian
   * data on stdin and wavs are little endian anyway.
   */
  void setBigEndian( bool b ) { m_bigEndian = b; }

  void setTrackCdText( const K3bDevice::TrackCdText& );
  void setTrackTitle( const TQString& s ) { m_trackTitle = s; }
  void setTrackPerformer( const TQString& s ) { m_trackPerformer = s; }
  void setTrackSongwriter( const TQString& s ) { m_trackSongwriter = s; }
  void setTrackComposer( const TQString& s ) { m_trackComposer = s; }
  void setTrackArranger( const TQString& s ) { m_trackArranger = s; }
  void setTrackMessage( const TQString& s ) { m_trackMessage = s; }

  void setCdText( const K3bDevice::CdText& );
  void setAlbumTitle( const TQString& s ) { m_albumTitle = s; }
  void setAlbumPerformer( const TQString& s ) { m_albumPerformer = s; }

  void setIsrc( const TQCString& s ) { m_isrc = s; }
  void setMcn( const TQCString& s ) { m_mcn = s; }

 private:
  long m_index0;

  TQValueVector<long> m_indices;

  int m_trackNumber;
  K3b::Msf m_trackStart;
  K3b::Msf m_trackLength;
  bool m_preEmphasis;
  bool m_copyPermitted;
  bool m_bigEndian;

  TQString m_trackTitle;
  TQString m_trackPerformer;
  TQString m_trackSongwriter;
  TQString m_trackComposer;
  TQString m_trackArranger;
  TQString m_trackMessage;

  TQString m_albumTitle;
  TQString m_albumPerformer;

  TQCString m_isrc;
  TQCString m_mcn;
};

#endif
