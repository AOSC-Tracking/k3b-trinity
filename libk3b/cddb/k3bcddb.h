/* 
 *
 * $Id: k3bcddb.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BCDDB_H
#define K3BCDDB_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqobject.h>

#include <k3btoc.h>

#include "k3bcddbresult.h"
#include "k3b_export.h"

class TDEConfig;
class K3bCddbQuery;
class K3bCddbHttpQuery;
class K3bCddbpQuery;
class K3bCddbLocalQuery;
class K3bCddbSubmit;
class K3bCddbLocalSubmit;


class LIBK3B_EXPORT K3bCddb : public TQObject 
{
  TQ_OBJECT
  

 public:
  K3bCddb( TQObject* parent = 0, const char* name = 0 );
  ~K3bCddb();

  TQString errorString() const;

  /**
   * Do NOT call this before queryResult has
   * been emitted
   */
  const K3bCddbResultEntry& result() const;

 public slots:  
  /** query a cd and connect to the queryFinished signal */
  void query( const K3bDevice::Toc& );
  void readConfig( TDEConfig* c );
  void saveEntry( const K3bCddbResultEntry& );

 signals:
  void queryFinished( int error );
  void submitFinished( bool success );
  void infoMessage( const TQString& );

 private slots:
  void localQuery();
  void remoteQuery();
  void slotQueryFinished( K3bCddbQuery* );
  void slotSubmitFinished( K3bCddbSubmit* );
  void slotMultibleMatches( K3bCddbQuery* );
  void slotNoEntry();

 private:
  K3bCddbQuery* getQuery( const TQString& );

  K3bCddbHttpQuery* m_httpQuery;
  K3bCddbpQuery* m_cddbpQuery;
  K3bCddbLocalQuery* m_localQuery;
  K3bCddbLocalSubmit* m_localSubmit;

  K3bDevice::Toc m_toc;
  unsigned int m_iCurrentQueriedServer;
  unsigned int m_iCurrentQueriedLocalDir;

  const K3bCddbQuery* m_lastUsedQuery;
  K3bCddbResultEntry m_lastResult;

  // config
  TQStringList m_cddbServer;
  TQString m_proxyServer;
  int m_proxyPort;
  TQString m_cgiPath;
  bool m_bUseProxyServer;
  bool m_bUseKdeSettings;
  TQStringList m_localCddbDirs;
  bool m_bSaveCddbEntriesLocally;
  bool m_bUseManualCgiPath;
  bool m_bRemoteCddbQuery;
  bool m_bLocalCddbQuery;
};
  

#endif
