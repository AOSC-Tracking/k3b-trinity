/* 
 *
 * $Id: k3bcrc.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_CRC_H_
#define _K3B_CRC_H_

#include <tqglobal.h>

namespace K3bDevice
{
  //  static K3bCrc* x25();

  // bool check( unsigned char* message, unsigned int len, unsigned char* crc, unsigned int crcLen );

  TQ_UINT16 calcX25( unsigned char* message, unsigned int len, TQ_UINT16 start = 0x0000 );

  /**
   * subdata is 12 bytes in long.
   */
  bool checkTQCrc( unsigned char* subdata );
};

#endif
