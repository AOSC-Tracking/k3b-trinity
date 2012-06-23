/* 
 *
 * $Id: k3bcddbpquery.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BCDDBP_QUERY_H
#define K3BCDDBP_QUERY_H

#include "k3bcddbquery.h"
#include "k3bcddbresult.h"

#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqtextstream.h>

class TQSocket;

class K3bCddbpQuery : public K3bCddbQuery
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bCddbpQuery( TQObject* parent = 0, const char* name = 0 );
  ~K3bCddbpQuery();

 public slots:
  void setServer( const TQString& s, int port = 8080 ) { m_server = s; m_port = port; }

 protected slots:
  void slotHostFound();
  void slotConnected();
  void slotConnectionClosed();
  void slotReadyRead();
  void slotError( int e );
  void doQuery();
  void doMatchQuery();

 private:
  void cddbpQuit();
  enum State { GREETING, HANDSHAKE, PROTO, QUERY, QUERY_DATA, READ, READ_DATA, QUIT };

  int m_state;
  TQString m_server;
  int m_port;

  TQSocket* m_socket;
  TQTextStream m_stream;

  TQString m_parsingBuffer;
};

#endif
