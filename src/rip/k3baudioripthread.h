/*
 *
 * $Id: k3baudioripthread.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3B_AUDIO_RIP_THREAD_H
#define K3B_AUDIO_RIP_THREAD_H

#include <k3bthread.h>
#include <tqobject.h>
#include <tqvaluevector.h>
#include <tqpair.h>

#include <k3bcddbquery.h>


class K3bAudioEncoder;
class K3bCdparanoiaLib;
namespace K3bDevice {
  class Device;
}


class K3bAudioRipThread : public TQObject, public K3bThread
{
  TQ_OBJECT
  

 public:
  K3bAudioRipThread();
  ~K3bAudioRipThread();

  TQString jobDescription() const;
  TQString jobDetails() const;

  // paranoia settings
  void setParanoiaMode( int mode );
  void setMaxRetries( int r );
  void setNeverSkip( bool b );

  void setSingleFile( bool b ) { m_singleFile = b; }

  void setUseIndex0( bool b ) { m_useIndex0 = b; }

  void setDevice( K3bDevice::Device* dev ) { m_device = dev; }

  void setCddbEntry( const K3bCddbResultEntry& e ) { m_cddbEntry = e; }

  // if 0 (default) wave files are created
  void setEncoder( K3bAudioEncoder* f );

  /**
   * Used for encoders that support multiple formats
   */
  void setFileType( const TQString& );

  /**
   * 1 is the first track
   */
  void setTracksToRip( const TQValueVector<TQPair<int, TQString> >& t ) { m_tracks = t; }

  void setWritePlaylist( bool b ) { m_writePlaylist = b; }
  void setPlaylistFilename( const TQString& s ) { m_playlistFilename = s; }
  void setUseRelativePathInPlaylist( bool b ) { m_relativePathInPlaylist = b; }
  void setWriteCueFile( bool b ) { m_writeCueFile = b; }

  void cancel();

 private slots:
  void slotCheckIfThreadStillRunning();

 private:
  /** reimplemented from TQThread. Does the work */
  void run();

  bool ripTrack( int track, const TQString& filename );
  void cleanupAfterCancellation();
  bool writePlaylist();
  bool writeCueFile();

  /**
   * Finds a relative path from baseDir to absPath
   */
  TQString findRelativePath( const TQString& absPath, const TQString& baseDir );

  K3bCddbResultEntry m_cddbEntry;
  K3bDevice::Device* m_device;

  bool m_bUsePattern;
  bool m_singleFile;
  bool m_useIndex0;

  bool m_writePlaylist;
  bool m_relativePathInPlaylist;
  TQString m_playlistFilename;

  bool m_writeCueFile;

  TQValueVector<TQPair<int, TQString> > m_tracks;

  friend class K3bAudioRipJob;

  class Private;
  Private* d;
};

#endif
