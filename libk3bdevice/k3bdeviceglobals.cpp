/* 
 *
 * $Id: k3bdeviceglobals.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bdeviceglobals.h"
#include "k3bdiskinfo.h"
#include "k3bdevice.h"

#include <tdelocale.h>
#include <k3bdebug.h>

#include <tqstringlist.h>


TQString K3bDevice::deviceTypeString( int t )
{
  TQStringList s;
  if( t & K3bDevice::DEVICE_CD_R )
    s += i18n("CD-R");
  if( t & K3bDevice::DEVICE_CD_RW )
    s += i18n("CD-RW");
  if( t & K3bDevice::DEVICE_CD_ROM )
    s += i18n("CD-ROM");
  if( t & K3bDevice::DEVICE_DVD_ROM )
    s += i18n("DVD-ROM");
  if( t & K3bDevice::DEVICE_DVD_RAM )
    s += i18n("DVD-RAM");
  if( t & K3bDevice::DEVICE_DVD_R )
    s += i18n("DVD-R");
  if( t & K3bDevice::DEVICE_DVD_RW )
    s += i18n("DVD-RW");
  if( t & K3bDevice::DEVICE_DVD_R_DL )
    s += i18n("DVD-R DL");
  if( t & DEVICE_HD_DVD_ROM )
    s += i18n("HD DVD-ROM");
  if( t & DEVICE_HD_DVD_R )
    s += i18n("HD DVD-R");
  if( t & DEVICE_HD_DVD_RAM )
    s += i18n("HD DVD-RAM");
  if( t & DEVICE_BD_ROM )
    s += i18n("BD-ROM");
  if( t & DEVICE_BD_R )
    s += i18n("BD-R");
  if( t & DEVICE_BD_RE )
    s += i18n("BD-RE");
  if( t & K3bDevice::DEVICE_DVD_PLUS_R )
    s += i18n("DVD+R");
  if( t & K3bDevice::DEVICE_DVD_PLUS_RW )
    s += i18n("DVD+RW");
  if( t & K3bDevice::DEVICE_DVD_PLUS_R_DL )
    s += i18n("DVD+R DL");

  if( s.isEmpty() )
    return i18n("Error");
  else
    return s.join( ", " );
}


TQString K3bDevice::writingModeString( int m )
{
  TQStringList s;
  if( m & K3bDevice::WRITINGMODE_SAO )
    s += i18n("SAO");
  if( m & K3bDevice::WRITINGMODE_TAO )
    s += i18n("TAO");
  if( m & K3bDevice::WRITINGMODE_RAW )
    s += i18n("RAW");
  if( m & K3bDevice::WRITINGMODE_SAO_R96P )
    s += i18n("SAO/R96P");
  if( m & K3bDevice::WRITINGMODE_SAO_R96R )
    s += i18n("SAO/R96R");
  if( m & K3bDevice::WRITINGMODE_RAW_R16 )
    s += i18n("RAW/R16");
  if( m & K3bDevice::WRITINGMODE_RAW_R96P )
    s += i18n("RAW/R96P");
  if( m & K3bDevice::WRITINGMODE_RAW_R96R )
    s += i18n("RAW/R96R");
  if( m & K3bDevice::WRITINGMODE_INCR_SEQ )
    s += i18n("Incremental Sequential");
  if( m & K3bDevice::WRITINGMODE_RES_OVWR )
    s += i18n("Restricted Overwrite");
  if( m & K3bDevice::WRITINGMODE_LAYER_JUMP )
    s += i18n("Layer Jump");

  if( m & K3bDevice::WRITINGMODE_RRM )
    s += i18n("Random Recording");
  if( m & K3bDevice::WRITINGMODE_SRM )
    s += i18n("Sequential Recording");
  if( m & K3bDevice::WRITINGMODE_SRM_POW )
    s += i18n("Sequential Recording + POW");

  if( s.isEmpty() )
    return i18n("None");
  else
    return s.join( ", " );
}


TQString K3bDevice::mediaTypeString( int m, bool simple )
{
  if( m == K3bDevice::MEDIA_UNKNOWN )
    return i18n("Unknown");

  TQStringList s;
  if( m & MEDIA_NONE )
    s += i18n("No media");
  if( m & MEDIA_DVD_ROM )
    s += i18n("DVD-ROM");
  if( m & MEDIA_DVD_R || 
      (simple && (m & MEDIA_DVD_R_SEQ)) )
    s += i18n("DVD-R");
  if( m & MEDIA_DVD_R_SEQ && !simple )
    s += i18n("DVD-R Sequential");
  if( m & MEDIA_DVD_R_DL ||
      (simple && (m & (MEDIA_DVD_R_DL_SEQ|MEDIA_DVD_R_DL_JUMP))) )
    s += i18n("DVD-R Dual Layer");
  if( m & MEDIA_DVD_R_DL_SEQ && !simple )
    s += i18n("DVD-R Dual Layer Sequential");
  if( m & MEDIA_DVD_R_DL_JUMP && !simple )
    s += i18n("DVD-R Dual Layer Jump");
  if( m & MEDIA_DVD_RAM )
    s += i18n("DVD-RAM");
  if( m & MEDIA_DVD_RW ||
      (simple && (m & (MEDIA_DVD_RW_OVWR|MEDIA_DVD_RW_SEQ))) )
    s += i18n("DVD-RW");
  if( m & MEDIA_DVD_RW_OVWR && !simple )
    s += i18n("DVD-RW Restricted Overwrite");
  if( m & MEDIA_DVD_RW_SEQ && !simple )
    s += i18n("DVD-RW Sequential");
  if( m & MEDIA_DVD_PLUS_RW )
    s += i18n("DVD+RW");
  if( m & MEDIA_DVD_PLUS_R )
    s += i18n("DVD+R");
  if( m & MEDIA_DVD_PLUS_RW_DL )
    s += i18n("DVD+RW Dual Layer");
  if( m & MEDIA_DVD_PLUS_R_DL )
    s += i18n("DVD+R Dual Layer");
  if( m & MEDIA_CD_ROM )
    s += i18n("CD-ROM");
  if( m & MEDIA_CD_R )
    s += i18n("CD-R");
  if( m & MEDIA_CD_RW )
    s += i18n("CD-RW");
  if( m & MEDIA_HD_DVD_ROM )
    s += i18n("HD DVD-ROM");
  if( m & MEDIA_HD_DVD_R )
    s += i18n("HD DVD-R");
  if( m & MEDIA_HD_DVD_RAM )
    s += i18n("HD DVD-RAM");
  if( m & MEDIA_BD_ROM )
    s += i18n("BD-ROM");
  if( m & MEDIA_BD_R ||
      (simple && (m & (MEDIA_BD_R_SRM|MEDIA_BD_R_RRM))) )
    s += i18n("BD-R");
  if( m & MEDIA_BD_R_SRM && !simple )
    s += i18n("BD-R Sequential (SRM)");
  if( m & MEDIA_BD_R_SRM_POW && !simple )
    s += i18n("BD-R Sequential Pseudo Overwrite (SRM+POW)");
  if( m & MEDIA_BD_R_RRM && !simple )
    s += i18n("BD-R Random (RRM)");
  if( m & MEDIA_BD_RE )
    s += i18n("BD-RE");

  if( s.isEmpty() )
    return i18n("Error");
  else
    return s.join( ", " );
}


void K3bDevice::debugBitfield( unsigned char* data, long len )
{
  for( int i = 0; i < len; ++i ) {
    TQString index, bitString;
    index.sprintf( "%4i", i );
    for( int bp = 7; bp >= 0; --bp )
      bitString[7-bp] = ( data[i] & (1<<bp) ? '1' : '0' );
    k3bDebug() << index << " - " << bitString << " - " << (int)data[i] << endl;
  }
}


K3bDevice::uint16 K3bDevice::from2Byte( unsigned char* d )
{
  return ( d[0] << 8 & 0xFF00 |
	   d[1]      & 0xFF );
}


K3bDevice::uint32 K3bDevice::from4Byte( unsigned char* d )
{
  return ( d[0] << 24 & 0xFF000000 |
	   d[1] << 16 & 0xFF0000 |
	   d[2] << 8  & 0xFF00 |
	   d[3]       & 0xFF );
}


char K3bDevice::fromBcd( const char& i )
{
  return (i & 0x0f) + 10 * ( (i >> 4) & 0x0f );
}


char K3bDevice::toBcd( const char& i )
{
  return ( i % 10 ) | ( ( (( i / 10 ) % 10) << 4 ) & 0xf0 );
}


bool K3bDevice::isValidBcd( const char& i )
{
  return ( i & 0x0f ) <= 0x09 && ( i & 0xf0 ) <= 0x90;
}


int K3bDevice::determineMaxReadingBufferSize( K3bDevice::Device* dev, const K3b::Msf& firstSector )
{
  //
  // As long as we do not know how to determine the max read buffer properly we simply determine it
  // by trying. :)
  //

  int bufferSizeSectors = 128;
  unsigned char buffer[2048*128];
  while( !dev->read10( buffer, 2048*bufferSizeSectors, firstSector.lba(), bufferSizeSectors ) ) {
    k3bDebug() << "(K3bDataTrackReader) determine max read sectors: "
	      << bufferSizeSectors << " too high." << endl;
    bufferSizeSectors--;
  }
  k3bDebug() << "(K3bDataTrackReader) determine max read sectors: " 
	    << bufferSizeSectors << " is max." << endl;

  return bufferSizeSectors;
}
