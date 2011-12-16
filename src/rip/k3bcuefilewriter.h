/* 
 *
 * $Id: k3bcuefilewriter.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_CUE_FILE_WRITER_H_
#define _K3B_CUE_FILE_WRITER_H_

#include <tqtextstream.h>
#include <tqstringlist.h>

#include <k3btoc.h>
#include <k3bcdtext.h>

namespace K3bDevice {
  class TrackCdText;
}

/**
 * Write a CDRWIN cue file.
 * For now this writer only supports audio CDs
 * for usage in the K3b audio CD ripper.
 */

class K3bCueFileWriter
{
 public:
  K3bCueFileWriter();

  bool save( TQTextStream& );
  bool save( const TQString& filename );

  void setData( const K3bDevice::Toc& toc ) { m_toc = toc; }
  void setCdText( const K3bDevice::CdText& text ) { m_cdText = text; }
  void setImage( const TQString& name, const TQString& type ) { m_image = name; m_dataType = type; }

 private:
  K3bDevice::Toc m_toc;
  K3bDevice::CdText m_cdText;
  TQString m_image;
  TQString m_dataType;
};

#endif
