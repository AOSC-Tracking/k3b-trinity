/* 
 *
 * $Id: k3bcddbhttpquery.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BCDDB_HTTP_QUERY_H
#define K3BCDDB_HTTP_QUERY_H

#include "k3bcddbquery.h"
#include "k3bcddbresult.h"

#include <tqvaluelist.h>

namespace TDEIO {
  class Job;
}

class K3bCddbHttpQuery : public K3bCddbQuery
{
  Q_OBJECT
  

 public:
  K3bCddbHttpQuery( TQObject* parent = 0, const char* name = 0 );
  ~K3bCddbHttpQuery();

 public slots:
  void setServer( const TQString& s, int port = 80 ) { m_server = s; m_port = port; }
  void setCgiPath( const TQString& p ) { m_cgiPath = p; }

 protected slots:
  void doQuery();
  void doMatchQuery();
  void slotResult( TDEIO::Job* );
  void slotData( TDEIO::Job*, const TQByteArray& data );

 private:
  void performCommand( const TQString& );

  enum State { QUERY, QUERY_DATA, READ, READ_DATA, FINISHED };

  int m_state;
  TQString m_server;
  int m_port;
  TQString m_cgiPath;

  TQString m_currentlyConnectingServer;

  TQByteArray m_data;
  TQString m_parsingBuffer;
};

#endif

