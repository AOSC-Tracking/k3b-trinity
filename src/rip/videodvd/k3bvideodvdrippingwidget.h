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

#ifndef _K3B_VIDEODVD_RIPPING_WIDGET_H_
#define _K3B_VIDEODVD_RIPPING_WIDGET_H_

#include "base_k3bvideodvdrippingwidget.h"

#include <k3bvideodvdtitletranscodingjob.h>

#include <tqvaluevector.h>
#include <tqmap.h>

#include <tdeio/global.h>

class TQTimer;

class K3bVideoDVDRippingWidget : public base_K3bVideoDVDRippingWidget
{
  TQ_OBJECT
  

 public:
  K3bVideoDVDRippingWidget( TQWidget* parent );
  ~K3bVideoDVDRippingWidget();

  K3bVideoDVDTitleTranscodingJob::VideoCodec selectedVideoCodec() const;
  K3bVideoDVDTitleTranscodingJob::AudioCodec selectedAudioCodec() const;
  int selectedAudioBitrate() const;
  TQSize selectedPictureSize() const;

  void setSelectedVideoCodec( K3bVideoDVDTitleTranscodingJob::VideoCodec codec );
  void setSelectedAudioCodec( K3bVideoDVDTitleTranscodingJob::AudioCodec codec );
  void setSelectedAudioBitrate( int bitrate );
  void setSelectedPictureSize( const TQSize& );

  void setNeededSize( TDEIO::filesize_t );

 signals:
  void changed();

 private slots:
  void slotUpdateFreeTempSpace();
  void slotSeeSpecialStrings();
  void slotAudioCodecChanged( int codec );
  void slotVideoSizeChanged( int sizeIndex );
  void slotCustomPictureSize();

 private:
  TQTimer* m_freeSpaceUpdateTimer;
  TDEIO::filesize_t m_neededSize;

  TQSize m_customVideoSize;
};

#endif
