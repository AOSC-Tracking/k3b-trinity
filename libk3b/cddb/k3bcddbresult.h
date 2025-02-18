/* 
 *
 * $Id: k3bcddbresult.h 768492 2008-01-30 08:39:42Z trueg $
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



#ifndef K3B_CDDB_RESULT_H
#define K3B_CDDB_RESULT_H


#include <tqstringlist.h>
#include "k3b_export.h"


class LIBK3B_EXPORT K3bCddbResultHeader
{
 public:
  TQString category;
  TQString title;
  TQString artist;
  TQString discid;
};


class LIBK3B_EXPORT K3bCddbResultEntry
{
 public:
  // just to set a default
  K3bCddbResultEntry()
    : category("misc"),
    year(0) {
  }

  TQStringList titles;
  TQStringList artists;
  TQStringList extInfos;

  TQString cdTitle;
  TQString cdArtist;
  TQString cdExtInfo;

  TQString genre;
  TQString category;
  int year;
  TQString discid;

  TQString rawData;
};


class LIBK3B_EXPORT K3bCddbResult
{
 public:
  K3bCddbResult();
  //  K3bCddbQuery( const K3bCddbQuery& );

  void clear();
  void addEntry( const K3bCddbResultEntry& = K3bCddbResultEntry() );
  const K3bCddbResultEntry& entry( unsigned int number = 0 ) const;
  int foundEntries() const;

 private:
  TQValueList<K3bCddbResultEntry> m_entries;

  K3bCddbResultEntry m_emptyEntry;
};

#endif
