/* 
 *
 * $Id: k3btocfilewriter.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_TOC_FILE_WRITER_H_
#define _K3B_TOC_FILE_WRITER_H_

#include <tqtextstream.h>
#include <tqstringlist.h>

#include <k3btoc.h>
#include <k3bcdtext.h>

namespace K3bDevice {
  class TrackCdText;
}

class K3bTocFileWriter
{
 public:
  K3bTocFileWriter();

  bool save( TQTextStream& );
  bool save( const TQString& filename );

  void setData( const K3bDevice::Toc& toc ) { m_toc = toc; }
  void setCdText( const K3bDevice::CdText& text ) { m_cdText = text; }
  void setFilenames( const TQStringList& names ) { m_filenames = names; }
  void setHideFirstTrack( bool b ) { m_hideFirstTrack = b; }

  /**
   * The default is 1.
   */
  void setSession( int s ) { m_sessionToWrite = s; }

 private:
  void writeHeader( TQTextStream& t );
  void writeGlobalCdText( TQTextStream& t );
  void writeTrackCdText( const K3bDevice::TrackCdText& track, TQTextStream& t );
  void writeTrack( unsigned int index, const K3b::Msf& offset, TQTextStream& t );
  void writeDataSource( unsigned int trackNumber, TQTextStream& t );
  bool readFromStdin() const;

  K3bDevice::Toc m_toc;
  K3bDevice::CdText m_cdText;
  TQStringList m_filenames;
  bool m_hideFirstTrack;
  int m_sessionToWrite;
};

#endif
