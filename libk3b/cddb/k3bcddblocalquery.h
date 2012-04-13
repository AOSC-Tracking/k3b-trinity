/* 
 *
 * $Id: k3bcddblocalquery.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef K3BCDDB_LOCAL_QUERY_H
#define K3BCDDB_LOCAL_QUERY_H

#include "k3bcddbquery.h"
#include "k3bcddbresult.h"

#include <tqstring.h>


class K3bCddbLocalQuery : public K3bCddbQuery
{
  Q_OBJECT
  

 public:
  K3bCddbLocalQuery( TQObject* parent = 0, const char* name = 0 );
  ~K3bCddbLocalQuery();

 public slots:
  void setCddbDir( const TQString& dir ) { m_cddbDir = dir; }

 protected:
  void doQuery();
  void doMatchQuery();

 private:
  TQString preparePath( const TQString& p );

  TQString m_cddbDir;
};

#endif
