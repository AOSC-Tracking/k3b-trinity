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

#ifndef _K3B_VIDEODVD_RIPPING_PREVIEW_H_
#define _K3B_VIDEODVD_RIPPING_PREVIEW_H_

#include <tqobject.h>
#include <tqimage.h>

#include <k3bvideodvd.h>


class KTempDir;
class TDEProcess;

class K3bVideoDVDRippingPreview : public TQObject
{
  TQ_OBJECT
  

 public:
  K3bVideoDVDRippingPreview( TQObject* parent = 0 );
  ~K3bVideoDVDRippingPreview();

  const TQImage& preview() const { return m_preview; }

 public slots:
  /**
   * \param dvd The Video DVD object
   * \param title The Video DVD title to generate the preview for
   * \param chapter The Chapter number to use for the preview. 
   *                If 0 the middle of the title is used.
   */
  void generatePreview( const K3bVideoDVD::VideoDVD& dvd, int title, int chapter = 0 );

  void cancel();

 signals:
  void previewDone( bool );

 private slots:
  void slotTranscodeFinished( TDEProcess* );

 private:
  TQImage m_preview;
  KTempDir* m_tempDir;
  TDEProcess* m_process;
  int m_title;
  int m_chapter;
  K3bVideoDVD::VideoDVD m_dvd;

  bool m_canceled;
};

#endif
