/* 
 *
 * $Id: k3baudiojobtempdata.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_AUDIO_JOB_TEMPDATA_H_
#define _K3B_AUDIO_JOB_TEMPDATA_H_

#include <tqobject.h>
#include <k3bmsf.h>

class K3bAudioTrack;
class K3bAudioDoc;
class TQTextStream;


class K3bAudioJobTempData : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bAudioJobTempData( K3bAudioDoc* doc, TQObject* tqparent = 0, const char* name = 0 );
  ~K3bAudioJobTempData();

  const TQString& bufferFileName( int track );
  const TQString& bufferFileName( K3bAudioTrack* track );
  
  const TQString& infFileName( int track );
  const TQString& infFileName( K3bAudioTrack* track );
  
  const TQString& tocFileName();

  K3bAudioDoc* doc() const;

  /**
   * use this if you want
   * a specific directory
   * it defaults to the default K3b temp dir
   */
  void prepareTempFileNames( const TQString& path = TQString() );

  /**
   * remove all temp files (this does not include the audio buffer files
   * since these are not created and thus not handled by the K3bAudioJobTempData)
   */
  void cleanup();

 private:
  class Private;
  Private* d;
};

#endif
