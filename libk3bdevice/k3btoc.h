/* 
 *
 * $Id: k3btoc.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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


#ifndef K3BTOC_H
#define K3BTOC_H

#include <tqvaluelist.h>
#include <tqcstring.h>

#include <k3bmsf.h>

#include "k3btrack.h"
#include "k3bdevice_export.h"
class TQString;

namespace K3bDevice
{

  enum ContentsType {
    DATA,
    AUDIO,
    MIXED,
    NONE // no tracks
  };

  /**
   * A list of K3bTracks that represents the contents
   * of a cd.
   * The Toc deletes all its tracks when it is deleted and
   * deletes removed tracks.
   */
  class LIBK3BDEVICE_EXPORT Toc : public TQValueList<K3bTrack>
  {
  public:
    Toc();
    /** deep copy */
    Toc( const Toc& );
    /** deletes all tracks */
    ~Toc();
    /** deep copy */
    Toc& operator=( const Toc& );

    /**
     * CDDB disc Id
     */
    unsigned int discId() const;

    const TQCString& mcn() const { return m_mcn; }

    /**
     * determine the contents type based on the tracks' types.
     * Audio, Data, or Mixed
     */
    int contentType() const;

    /**
     * \return the number of sessions in this TOC.
     */
    int sessions() const;

    /**
     * The first track's first sector could differ from the disc's
     * first sector if there is a pregap before index 1
     */
    const K3b::Msf& firstSector() const;
    K3b::Msf lastSector() const;
    K3b::Msf length() const;

    void setFirstSector( int i ) { m_firstSector = i; }

    void setMcn( const TQCString& mcn ) { m_mcn = mcn; }

    void clear();

    void debug() const;

    bool operator==( const Toc& ) const;
    bool operator!=( const Toc& ) const;

  private:
    unsigned int m_discId;
    K3b::Msf m_firstSector;

    TQCString m_mcn;
  };
}

#endif
