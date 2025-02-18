/* 
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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


#ifndef _videodvd_H_
#define _videodvd_H_

#include <tqstring.h>
#include <tqcstring.h>

#include <kurl.h>
#include <tdeio/global.h>
#include <tdeio/slavebase.h>

class TQCString;
class K3bIso9660Entry;
class K3bIso9660;
namespace K3bDevice
{
  class DeviceManager;
}

class tdeio_videodvdProtocol : public TDEIO::SlaveBase
{
public:
  tdeio_videodvdProtocol(const TQCString &pool_socket, const TQCString &app_socket);
  ~tdeio_videodvdProtocol();

  void mimetype( const KURL& url );
  void stat( const KURL& url );
  void get( const KURL& url );
  void listDir( const KURL& url );

private:
  K3bIso9660* openIso( const KURL&, TQString& plainIsoPath );
  TDEIO::UDSEntry createUDSEntry( const K3bIso9660Entry* e ) const;
  void listVideoDVDs();

  static K3bDevice::DeviceManager* s_deviceManager;
  static int s_instanceCnt;
};

#endif
