/*
 *
 * $Id: k3bdevice.cpp 732002 2007-11-02 14:13:14Z trueg $
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

#include <config.h>


#include "k3bdevice.h"
#include "k3bdeviceglobals.h"
#include "k3btrack.h"
#include "k3btoc.h"
#include "k3bdiskinfo.h"
#include "k3bmmc.h"
#include "k3bscsicommand.h"
#include "k3bcrc.h"

#include <tqstringlist.h>
#include <tqfile.h>
#include <tqglobal.h>
#include <tqvaluevector.h>
#include <tqmutex.h>

#include <k3bdebug.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>
#include <stdarg.h>


#ifdef Q_OS_LINUX

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,70)
typedef unsigned char u8;
#endif

#undef __STRICT_ANSI__
#include <linux/cdrom.h>
#define __STRICT_ANSI__

#endif // Q_OS_LINUX

#ifdef Q_OS_FREEBSD
#include <stdio.h>
#include <camlib.h>
#define CD_FRAMESIZE_RAW 2352
#endif

#ifdef Q_OS_NETBSD
#include <sys/cdio.h>
#endif

#ifdef HAVE_RESMGR
extern "C" {
#include <resmgr.h>
}
#endif


//
// Very evil hacking: force the speed values to be acurate
// as long as "they" do not introduce other "broken" DVD
// speeds like 2.4 this works fine
//
static int fixupDvdWritingSpeed( int speed )
{
  //
  // Some writers report their speeds in 1000 bytes per second instead of 1024.
  //
  if( speed % 1385 == 0 )
    return speed;

  else if( speed % 1352 == 0 )
    return speed*1385/1352;

  // has to be 2.4x speed
  else
    return 3324;
}


const char* K3bDevice::Device::cdrdao_drivers[] =
  { "auto", "plextor", "plextor-scan", "cdd2600", "generic-mmc",
    "generic-mmc-raw", "ricoh-mp6200", "sony-cdu920",
    "sony-cdu948", "taiyo-yuden", "teac-cdr55", "toshiba",
    "yamaha-cdr10x", 0
  };


#if defined(Q_OS_LINUX) || defined(Q_OS_NETBSD)
int K3bDevice::openDevice( const char* name, bool write )
{
  int fd = -1;
  int flags = O_NONBLOCK;
  if( write )
    flags |= O_RDWR;
  else
    flags |= O_RDONLY;

#ifdef HAVE_RESMGR
  // first try resmgr
  fd = ::rsm_open_device( name, flags );
  //  k3bDebug() << "(K3bDevice::Device) resmgr open: " << fd << endl;
#endif

  if( fd < 0 )
    fd = ::open( name, flags );

  if( fd < 0 ) {
    k3bDebug() << "(K3bDevice::Device) could not open device "
	      << name << ( write ? " for writing" : " for reading" ) << endl;
    k3bDebug() << "                    (" << strerror(errno) << ")" << endl;
    fd = -1;

    // at least open it read-only (which is sufficient for kernels < 2.6.8 anyway)
    if( write )
      return openDevice( name, false );
  }

  return fd;
}
#endif


class K3bDevice::Device::Private
{
public:
  Private()
    : supportedProfiles(0),
#ifdef Q_OS_LINUX
      deviceFd(-1),
#endif
#ifdef Q_OS_NETBSD
      deviceFd(-1),
#endif
#ifdef Q_OS_FREEBSD
      cam(0),
#endif
      openedReadWrite(false),
      burnfree(false) {
  }

  int readCapabilities;
  int writeCapabilities;
  int supportedProfiles;
  TQStringList allNodes;
#ifdef Q_OS_LINUX
  int deviceFd;
#endif
#ifdef Q_OS_NETBSD
  int deviceFd;
#endif
#ifdef Q_OS_FREEBSD
  struct cam_device *cam;
#endif
  bool openedReadWrite;
  bool burnfree;

  TQMutex mutex;
    TQMutex openCloseMutex;
};


K3bDevice::Device::Device( const TQString& devname )
  : m_bus(-1),
    m_target(-1),
    m_lun(-1),
    m_writeModes(0)
{
  d = new Private;

  m_blockDevice = devname;
  d->allNodes.append(devname);

  m_cdrdaoDriver = "auto";
  m_cdTextCapable = 0;
  m_maxWriteSpeed = 0;
  m_maxReadSpeed = 0;
  d->burnfree = false;
  m_dvdMinusTestwrite = true;
  m_bufferSize = 0;
}


K3bDevice::Device::~Device()
{
  close();
  delete d;
}


bool K3bDevice::Device::init( bool bCheckWritingModes )
{
  k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": init()" << endl;

  //
  // they all should read CD-ROM.
  //
  d->readCapabilities = MEDIA_CD_ROM;
  d->writeCapabilities = 0;
  d->supportedProfiles = 0;

  if( !open() )
    return false;

  //
  // inquiry
  // use a 36 bytes buffer since not all devices return the full inquiry struct
  //
  ScsiCommand cmd( this );
  unsigned char buf[36];
  cmd.clear();
  ::memset( buf, 0, sizeof(buf) );
  struct inquiry* inq = (struct inquiry*)buf;
  cmd[0] = MMC_INQUIRY;
  cmd[4] = sizeof(buf);
  cmd[5] = 0;
  if( cmd.transport( TR_DIR_READ, buf, sizeof(buf) ) ) {
    kdError() << "(K3bDevice::Device) Unable to do inquiry." << endl;
    close();
    return false;
  }
  else {
    m_vendor = TQString::fromLatin1( (char*)(inq->vendor), 8 ).stripWhiteSpace();
    m_description = TQString::fromLatin1( (char*)(inq->product), 16 ).stripWhiteSpace();
    m_version = TQString::fromLatin1( (char*)(inq->revision), 4 ).stripWhiteSpace();
  }

  if( m_vendor.isEmpty() )
    m_vendor = "UNKNOWN";
  if( m_description.isEmpty() )
    m_description = "UNKNOWN";

  //
  // We probe all features of the device. Since not all devices support the GET CONFIGURATION command
  // we also query the mode page 2A and use the cdrom.h stuff to get as much information as possible
  //
  checkFeatures();

  //
  // Check the supported write modes (WRITINGMODE_TAO, WRITINGMODE_SAO, WRITINGMODE_RAW) by trying to set them
  // We do this before checking mode page 2A in case some readers allow changin
  // the write parameter page
  //
  if( bCheckWritingModes )
    checkWritingModes();

  //
  // Most current drives support the 2A mode page
  // Here we can get some more information (cdrecord -prcap does exactly this)
  //
  checkFor2AFeatures();

  m_maxWriteSpeed = determineMaximalWriteSpeed();

  //
  // Check Just-Link via Ricoh mode page 0x30
  //
  if( !d->burnfree )
    checkForJustLink();

  //
  // Support for some very old drives
  //
  checkForAncientWriters();

  //
  // If it can be written it can also be read
  //
  d->readCapabilities |= d->writeCapabilities;

  close();

  return furtherInit();
}


bool K3bDevice::Device::furtherInit()
{
#ifdef Q_OS_LINUX

  //
  // Since all CDR drives at least support WRITINGMODE_TAO, all CDRW drives should support
  // mode page 2a and all DVD writer should support mode page 2a or the GET CONFIGURATION
  // command this is redundant and may be removed for BSD ports or even completely
  //
  // We just keep it here because of the "should" in the sentence above. If someone can tell me
  // that the linux driver does nothing more we can remove it completely.
  //
  open();
  int drivetype = ::ioctl( handle(), CDROM_GET_CAPABILITY, CDSL_CURRENT );
  if( drivetype < 0 ) {
    k3bDebug() << "Error while retrieving capabilities." << endl;
    close();
    return false;
  }

  d->readCapabilities |= DEVICE_CD_ROM;

  if( drivetype & CDC_CD_R )
    d->writeCapabilities |= MEDIA_CD_R;
  if( drivetype & CDC_CD_RW )
    d->writeCapabilities |= MEDIA_CD_RW;
  if( drivetype & CDC_DVD_R )
    d->writeCapabilities |= MEDIA_DVD_R;
  if( drivetype & CDC_DVD )
    d->readCapabilities |= MEDIA_DVD_ROM;

  close();

#endif // Q_OS_LINUX
  return true;
}


void K3bDevice::Device::checkForAncientWriters()
{
  // TODO: add a boolean which determines if this device is non-MMC so we may warn the user at K3b startup about it

  //
  // There are a lot writers out there which behave like the TEAC R5XS
  //
  if( ( vendor().startsWith("TEAC") && ( description().startsWith("CD-R50S") ||
					 description().startsWith("CD-R55S") ) )
      ||
      ( vendor().startsWith("SAF") && ( description().startsWith("CD-R2006PLUS") ||
					description().startsWith("CD-RW226") ||
					description().startsWith("CD-R4012") ) )
      ||
      ( vendor().startsWith("JVC") && ( description().startsWith("XR-W2001") ||
					description().startsWith("XR-W2010") ||
					description().startsWith("R2626") ) )
      ||
      ( vendor().startsWith("PINNACLE") && ( description().startsWith("RCD-1000") ||
					description().startsWith("RCD5020") ||
					description().startsWith("RCD5040") ||
					description().startsWith("RCD 4X4") ) )
      ||
      ( vendor().startsWith("Traxdata") && description().startsWith("CDR4120") ) ) {
    m_writeModes = WRITINGMODE_TAO;
    d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
    d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
    m_maxWriteSpeed = 4;
    m_maxReadSpeed = 12;
    m_bufferSize = 1024;
    d->burnfree = false;
  }
  else if( vendor().startsWith("TEAC") ) {
    if( description().startsWith("CD-R56S") ) {
      m_writeModes |= TAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 6;
      m_maxReadSpeed = 24;
      m_bufferSize = 1302;
      d->burnfree = false;
    }
    if( description().startsWith("CD-R58S") ) {
      m_writeModes |= TAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 8;
      m_maxReadSpeed = 24;
      m_bufferSize = 4096;
      d->burnfree = false;
    }
  }
  else if( vendor().startsWith("MATSHITA") ) {
    if( description().startsWith("CD-R   CW-7501") ) {
      m_writeModes = WRITINGMODE_TAO|WRITINGMODE_SAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 2;
      m_maxReadSpeed = 4;
      m_bufferSize = 1024;
      d->burnfree = false;
    }
    if( description().startsWith("CD-R   CW-7502") ) {
      m_writeModes = WRITINGMODE_TAO|WRITINGMODE_SAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 4;
      m_maxReadSpeed = 8;
      m_bufferSize = 1024;
      d->burnfree = false;
    }
    else if( description().startsWith("CD-R56S") ) {
      m_writeModes |= WRITINGMODE_TAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 6;
      m_maxReadSpeed = 24;
      m_bufferSize = 1302;
      d->burnfree = false;
    }
  }
  else if( vendor().startsWith("HP") ) {
    if( description().startsWith("CD-Writer 6020") ) {
      m_writeModes = WRITINGMODE_TAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 2;
      m_maxReadSpeed = 6;
      m_bufferSize = 1024;
      d->burnfree = false;
    }
  }
  else if( vendor().startsWith( "PHILIPS" ) ) {
    if( description().startsWith( "CDD2600" ) ) {
      m_writeModes = WRITINGMODE_TAO|WRITINGMODE_SAO;
      d->readCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      d->writeCapabilities = MEDIA_CD_ROM|MEDIA_CD_R;
      m_maxWriteSpeed = 2;
      m_maxReadSpeed = 6;
      m_bufferSize = 1024;
      d->burnfree = false;
    }
  }
}


K3bDevice::Interface K3bDevice::Device::interfaceType() const
{
  if( m_bus != -1 && m_target != -1 && m_lun != -1 )
    return SCSI;
  else
    return IDE;
}


bool K3bDevice::Device::dao() const
{
  return m_writeModes & WRITINGMODE_SAO;
}


bool K3bDevice::Device::supportsRawWriting() const
{
  return( writingModes() & (WRITINGMODE_RAW|WRITINGMODE_RAW_R16|WRITINGMODE_RAW_R96P|WRITINGMODE_RAW_R96R) );
}


bool K3bDevice::Device::writesCd() const
{
  return ( d->writeCapabilities & MEDIA_CD_R ) && ( m_writeModes & WRITINGMODE_TAO );
}


bool K3bDevice::Device::burner() const
{
  return ( writesCd() || writesDvd() );
}


bool K3bDevice::Device::writesCdrw() const
{
  return d->writeCapabilities & MEDIA_CD_RW;
}


bool K3bDevice::Device::writesDvd() const
{
  return ( writesDvdPlus() || writesDvdMinus() );
}


bool K3bDevice::Device::writesDvdPlus() const
{
  return d->writeCapabilities & (MEDIA_DVD_PLUS_R|MEDIA_DVD_PLUS_RW);
}


bool K3bDevice::Device::writesDvdMinus() const
{
  return d->writeCapabilities & (MEDIA_DVD_R|MEDIA_DVD_RW);
}


bool K3bDevice::Device::readsDvd() const
{
  return d->readCapabilities & MEDIA_DVD_ROM;
}


int K3bDevice::Device::type() const
{
  int r = 0;
  if( readCapabilities() & MEDIA_CD_ROM )
    r |= DEVICE_CD_ROM;
  if( writeCapabilities() & MEDIA_CD_R )
    r |= DEVICE_CD_R;
  if( writeCapabilities() & MEDIA_CD_RW )
    r |= DEVICE_CD_RW;
  if( readCapabilities() & MEDIA_DVD_ROM )
    r |= DEVICE_DVD_ROM;
  if( writeCapabilities() & MEDIA_DVD_RAM )
    r |= DEVICE_DVD_RAM;
  if( writeCapabilities() & MEDIA_DVD_R )
    r |= DEVICE_DVD_R;
  if( writeCapabilities() & MEDIA_DVD_RW )
    r |= DEVICE_DVD_RW;
  if( writeCapabilities() & MEDIA_DVD_R_DL )
    r |= DEVICE_DVD_R_DL;
  if( writeCapabilities() & MEDIA_DVD_PLUS_R )
    r |= DEVICE_DVD_PLUS_R;
  if( writeCapabilities() & MEDIA_DVD_PLUS_RW )
    r |= DEVICE_DVD_PLUS_RW;
  if( writeCapabilities() & MEDIA_DVD_PLUS_R_DL )
    r |= DEVICE_DVD_PLUS_R_DL;
  if( readCapabilities() & MEDIA_HD_DVD_ROM )
    r |= DEVICE_HD_DVD_ROM;
  if( writeCapabilities() & MEDIA_HD_DVD_R )
    r |= DEVICE_HD_DVD_R;
  if( writeCapabilities() & MEDIA_HD_DVD_RAM )
    r |= DEVICE_HD_DVD_RAM;
  if( readCapabilities() & MEDIA_BD_ROM )
    r |= DEVICE_BD_ROM;
  if( writeCapabilities() & MEDIA_BD_R )
    r |= DEVICE_BD_R;
  if( writeCapabilities() & MEDIA_BD_RE )
    r |= DEVICE_BD_RE;

  return r;
}


int K3bDevice::Device::readCapabilities() const
{
  return d->readCapabilities;
}


int K3bDevice::Device::writeCapabilities() const
{
  return d->writeCapabilities;
}


const TQString& K3bDevice::Device::devicename() const
{
  return blockDeviceName();
}


TQString K3bDevice::Device::busTargetLun() const
{
  return TQString("%1,%2,%3").arg(m_bus).arg(m_target).arg(m_lun);
}


int K3bDevice::Device::cdTextCapable() const
{
  if( cdrdaoDriver() == "auto" )
    return 0;
  else
    return m_cdTextCapable;
}


void K3bDevice::Device::setCdTextCapability( bool b )
{
  m_cdTextCapable = ( b ? 1 : 2 );
}


bool K3bDevice::Device::burnproof() const
{
  return burnfree();
}


bool K3bDevice::Device::burnfree() const
{
  return d->burnfree;
}


bool K3bDevice::Device::isDVD() const
{
  if( readsDvd() )
    return( mediaType() & MEDIA_DVD_ALL );
  else
    return false;
}


int K3bDevice::Device::isEmpty() const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  int ret = STATE_UNKNOWN;
  if( !open() )
    return STATE_UNKNOWN;

  if( !testUnitReady() )
    return STATE_NO_MEDIA;

  unsigned char* data = 0;
  unsigned int dataLen = 0;

  if( readDiscInformation( &data, dataLen ) ) {
    disc_info_t* inf = (disc_info_t*)data;
    switch( inf->status ) {
    case 0:
      ret = STATE_EMPTY;
      break;
    case 1:
      ret = STATE_INCOMPLETE;
      break;
    case 2:
      ret = STATE_COMPLETE;
      break;
    default:
      ret = STATE_UNKNOWN;
      break;
    }

    delete [] data;
  }

  if( needToClose )
    close();

  return ret;
}


int K3bDevice::Device::numSessions() const
{
  //
  // Session Info
  // ============
  // Byte 0-1: Data Length
  // Byte   2: First Complete Session Number (Hex) - always 1
  // Byte   3: Last Complete Session Number (Hex)
  //

  int ret = -1;

  unsigned char* data = 0;
  unsigned int len = 0;

  if( mediaType() & MEDIA_CD_ALL ) {
    //
    // Althought disk_info should get the real value without ide-scsi
    // I keep getting wrong values (the value is too high. I think the leadout
    // gets counted as session sometimes :()
    //
    if( readTocPmaAtip( &data, len, 1, 0, 0 ) ) {
      ret = data[3];

      delete [] data;
    }
    else {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": could not get session info !" << endl;
    }
  }
  else {
    if( readDiscInformation( &data, len ) ) {
      ret = (int)( data[9]<<8 | data[4] );

      // do only count complete sessions
      if( (data[2]>>2) != 3 )
	ret--;

      delete [] data;
    }
  }

  return ret;
}


int K3bDevice::Device::getDataMode( const K3b::Msf& sector ) const
{
  bool needToClose = !isOpen();

  int ret = Track::UNKNOWN;

  if( !open() )
    return ret;

  // we use readCdMsf here since it's defined mandatory in MMC1 and
  // we only use this method for CDs anyway
  unsigned char data[2352];
  bool readSuccess = readCdMsf( data, 2352,
				0,      // all sector types
				false,  // no dap
				sector,
				sector+1,
				true, // SYNC
				true, // HEADER
				true, // SUBHEADER
				true, // USER DATA
				true, // EDC/ECC
				0,    // no c2 info
				0 );

  if( readSuccess ) {
    if ( data[15] == 0x1 )
      ret = Track::MODE1;
    else if ( data[15] == 0x2 )
      ret = Track::MODE2;
    if ( ret == Track::MODE2 ) {
      if ( data[16] == data[20] &&
           data[17] == data[21] &&
           data[18] == data[22] &&
           data[19] == data[23] ) {
	if ( data[18] & 0x20 )
	  ret = Track::XA_FORM2;
	else
	  ret = Track::XA_FORM1;
      }
    }
  }

  if( needToClose )
    close();

  return ret;
}



int K3bDevice::Device::getTrackDataMode( const K3bDevice::Track& track ) const
{
  return getDataMode( track.firstSector() );
}


K3bDevice::Toc K3bDevice::Device::readToc() const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  Toc toc;

  if( !open() )
    return toc;

  int mt = mediaType();

  //
  // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
  // if supported at all
  //
  if( currentProfile() == MEDIA_DVD_ROM )
    mt = MEDIA_DVD_ROM;

  if( mt & (MEDIA_DVD_MINUS_ALL|MEDIA_DVD_PLUS_RW|MEDIA_DVD_ROM) ) {
    if( !readFormattedToc( toc, mt ) ) {
      K3b::Msf size;
      if( readCapacity( size ) ) {
	Track track;
	track.m_firstSector = 0;
	track.m_lastSector = size.lba();
	track.m_session = 1;
	track.m_type = Track::DATA;
	track.m_mode = Track::DVD;
	track.m_copyPermitted = ( mt != MEDIA_DVD_ROM );
	track.m_preEmphasis = ( mt != MEDIA_DVD_ROM );

	toc.append( track );
      }
      else
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		  << "READ CAPACITY for toc failed." << endl;
    }
  }

  else if( mt & (MEDIA_DVD_PLUS_R|MEDIA_DVD_PLUS_R_DL) ) {
    //
    // a DVD+R disk may have multiple sessions
    // every session may contain up to 16 fragments
    // if the disk is open there is one open session
    // every closed session is viewed as a track whereas
    // every fragment of the open session is viewed as a track
    //
    // We may use
    // READ DISK INFORMATION
    // READ TRACK INFORMATION: track number FFh, however, does not refer to the invisible track
    // READ TOC/PMA/ATIP: form 0 refers to all closed sessions
    //                    form 1 refers to the last closed session
    //
    readFormattedToc( toc, mt );
  }

  else if( mt & MEDIA_BD_ALL ) {
    readFormattedToc( toc, mt );
  }

  else if( mt == MEDIA_DVD_RAM ) {
    k3bDebug() << "(K3bDevice::readDvdToc) no dvdram support" << endl;
  }


  else if( mt & MEDIA_CD_ALL ) {
    bool success = readRawToc( toc );
    if( !success ) {
      success = readFormattedToc( toc, mt );

#ifdef Q_OS_LINUX
      if( !success ) {
	k3bDebug() << "(K3bDevice::Device) MMC READ TOC failed. falling back to cdrom.h." << endl;
	readTocLinux(toc);
      }
#endif

      if( success )
	fixupToc( toc );
    }
  }

  if( needToClose )
    close();

  return toc;
}


void K3bDevice::Device::readIsrcMcn( K3bDevice::Toc& toc ) const
{
  // read MCN and ISRC of all tracks
  TQCString mcn;
  if( readMcn( mcn ) ) {
    toc.setMcn( mcn );
    k3bDebug() << "(K3bDevice::Device) found MCN: " << mcn << endl;
  }
  else
    k3bDebug() << "(K3bDevice::Device) no MCN found." << endl;

  for( unsigned int i = 1; i <= toc.count(); ++i ) {
    TQCString isrc;
    if( toc[i-1].type() == Track::AUDIO ) {
      if( readIsrc( i, isrc ) ) {
	k3bDebug() << "(K3bDevice::Device) found ISRC for track " << i << ": " << isrc << endl;
	toc[i-1].setIsrc( isrc );
      }
      else
	k3bDebug() << "(K3bDevice::Device) no ISRC found for track " << i << endl;
    }
  }
}


bool K3bDevice::Device::readFormattedToc( K3bDevice::Toc& toc, int mt ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  bool success = false;

  toc.clear();

  unsigned int lastTrack = 0;

  unsigned char* data = 0;
  unsigned int dataLen = 0;
  if( !(mt & MEDIA_CD_ALL) ) {
    //
    // on DVD-R(W) multisession disks only two sessions are represented as tracks in the readTocPmaAtip
    // response (fabricated TOC). Thus, we use readDiscInformation for DVD media to get the proper number of tracks
    //
    if( readDiscInformation( &data, dataLen ) ) {
      lastTrack = (int)( data[11]<<8 | data[6] );

      delete [] data;

      if( readTrackInformation( &data, dataLen, 1, lastTrack ) ) {
	track_info_t* trackInfo = (track_info_t*)data;

	if( trackInfo->blank ) {
	  lastTrack--;
	}

	delete [] data;

	success = true;
      }
      else
	return false;
    }
    else
      return false;
  }
  else {
    if( readTocPmaAtip( &data, dataLen, 0, 0, 1 ) ) {

      if( dataLen < 4 ) {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": formatted toc data too small." << endl;
      }
      else if( dataLen != ( (unsigned int)sizeof(toc_track_descriptor) * ((unsigned int)data[3]+1) ) + 4 ) {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": invalid formatted toc data length: "
		  << (dataLen-2) << endl;
      }
      else {
	lastTrack = data[3];
	toc_track_descriptor* td = (toc_track_descriptor*)&data[4];
	for( unsigned int i = 0; i < lastTrack; ++i ) {

	  Track track;
	  unsigned int control = 0;

	  //
	  // In case READ TRACK INFORMATION fails:
	  // no session number info
	  // no track length and thus possibly incorrect last sector for
	  // multisession disks
	  //
	  track.m_firstSector = from4Byte( td[i].start_adr );
	  track.m_lastSector = from4Byte( td[i+1].start_adr ) - 1;
	  control = td[i].control;

	  track.m_type = (control & 0x4) ? Track::DATA : Track::AUDIO;
	  track.m_mode = getTrackDataMode( track );
	  track.m_copyPermitted = ( control & 0x2 );
	  track.m_preEmphasis = ( control & 0x1 );

	  toc.append( track );
	}

	success = true;
      }

      delete [] data;
    }
  }


  //
  // Try to get information for all the tracks
  //
  for( unsigned int i = 0; i < lastTrack; ++i ) {
    if( toc.count() < i+1 )
      toc.append( Track() );

    unsigned char* trackData = 0;
    unsigned int trackDataLen = 0;
    if( readTrackInformation( &trackData, trackDataLen, 1, i+1 ) ) {
      track_info_t* trackInfo = (track_info_t*)trackData;

      toc[i].m_firstSector = from4Byte( trackInfo->track_start );

      if( i > 0 && toc[i-1].m_lastSector == 0 )
	toc[i-1].m_lastSector = toc[i].m_firstSector - 1;

      // There are drives that return 0 track length here!
      // Some drives even return an invalid length here. :(
      if( from4Byte( trackInfo->track_size ) > 0 )
	toc[i].m_lastSector = toc[i].m_firstSector + from4Byte( trackInfo->track_size ) - 1;

      if( trackInfo->nwa_v ) {
	toc[i].m_nextWritableAddress = from4Byte( trackInfo->next_writable );
	toc[i].m_freeBlocks = from4Byte( trackInfo->free_blocks );
      }

      toc[i].m_session = (int)(trackInfo->session_number_m<<8 & 0xf0 |
			       trackInfo->session_number_l & 0x0f);  //FIXME: is this BCD?

      int control = trackInfo->track_mode;

      if( mt & MEDIA_CD_ALL ) {
	toc[i].m_type = (control & 0x4) ? Track::DATA : Track::AUDIO;
	toc[i].m_mode = getTrackDataMode( toc[i] );
      }
      else {
	toc[i].m_type = Track::DATA;
	toc[i].m_mode = Track::DVD;
      }
      toc[i].m_copyPermitted = ( control & 0x2 );
      toc[i].m_preEmphasis = ( control & 0x1 );

      delete [] trackData;
    }
    else if( !(mt & MEDIA_CD_ALL) ) {
      success = false;
    }
  }

  // this can only happen with DVD media
  if( !toc.isEmpty() && toc.last().lastSector() == 0 ) {
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " no track length for the last non-empty track." << endl;
    unsigned char* trackData = 0;
    unsigned int trackDataLen = 0;
    if( readTrackInformation( &trackData, trackDataLen, 1, lastTrack+1 ) ) {
      track_info_t* trackInfo = (track_info_t*)trackData;

      toc.last().m_lastSector = from4Byte( trackInfo->track_start ) - 1;

      delete [] trackData;
    }
  }


  if( needToClose )
    close();

  return success;
}


bool K3bDevice::Device::readRawToc( K3bDevice::Toc& toc ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  bool success = false;

  toc.clear();

  if( open() ) {
    //
    // Read Raw TOC (format: 0010b)
    //
    // For POINT from 01h-63h we get all the tracks
    // POINT a1h gices us the last track number in the session in PMIN
    // POINT a2h gives the start of the session lead-out in PMIN,PSEC,PFRAME
    //

    unsigned char* data = 0;
    unsigned int dataLen = 0;

    if( readTocPmaAtip( &data, dataLen, 2, false, 1 ) ) {
      if( dataLen > 4 ) {
	success = true;

	toc_raw_track_descriptor* tr = (toc_raw_track_descriptor*)&data[4];

	//
	// debug the raw toc data
	//
	k3bDebug() << "Session |  ADR   | CONTROL|  TNO   | POINT  |  Min   |  Sec   | Frame  |  Zero  |  PMIN  |  PSEC  | PFRAME |" << endl;
	for( unsigned int i = 0; i < (dataLen-4)/(int)sizeof(toc_raw_track_descriptor); ++i ) {
	  TQString s;
	  s += TQString( " %1 |" ).arg( (int)tr[i].session_number, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].adr, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].control, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].tno, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].point, 6, 16 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].min, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].sec, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].frame, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].zero, 6, 16 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].p_min, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].p_sec, 6 );
	  s += TQString( " %1 |" ).arg( (int)tr[i].p_frame, 6 );
	  k3bDebug() << s << endl;
	}

	//
	// First we try to determine if the raw toc data uses BCD values
	//
	int isBcd = rawTocDataWithBcdValues( data, dataLen );
	if( isBcd == -1 ) {
	  delete [] data;
	  return false;
	}

	K3b::Msf sessionLeadOut;

	for( unsigned int i = 0; i < (dataLen-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
	  if( tr[i].adr == 1 && tr[i].point <= 0x63 ) {
	    // track
	    K3bTrack track;
	    track.m_session = tr[i].session_number;

	    // :( We use 00:00:00 == 0 lba)
	    if( isBcd )
	      track.m_firstSector = K3b::Msf( K3bDevice::fromBcd(tr[i].p_min),
					      K3bDevice::fromBcd(tr[i].p_sec),
					      K3bDevice::fromBcd(tr[i].p_frame) ) - 150;
	    else
	      track.m_firstSector = K3b::Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) - 150;

	    track.m_type = ( tr[i].control & 0x4 ? Track::DATA : Track::AUDIO );
	    track.m_mode = ( track.type() == Track::DATA ? getTrackDataMode(track) : Track::UNKNOWN );
	    track.m_copyPermitted = ( tr[i].control & 0x2 );
	    track.m_preEmphasis = ( tr[i].control & 0x1 );

	    //
	    // only do this within a session because otherwise we already set the last sector with the session leadout
	    //
	    if( !toc.isEmpty() )
	      if( toc[toc.count()-1].session() == track.session() )
		toc[toc.count()-1].m_lastSector = track.firstSector() - 1;

	    toc.append(track);
	  }
	  else if( tr[i].point == 0xa2 ) {
	    //
	    // since the session is always reported before the tracks this is where we do this:
	    // set the previous session's last tracks's last sector to the first sector of the
	    // session leadout (which was reported before the tracks)
	    //
	    // This only happens on multisession CDs
	    //
	    if( !toc.isEmpty() )
	      toc[toc.count()-1].m_lastSector = sessionLeadOut - 1;

	    // this is save since the descriptors are reported in ascending order of the session number
	    // :( We use 00:00:00 == 0 lba)
	    if( isBcd )
	      sessionLeadOut = K3b::Msf( K3bDevice::fromBcd(tr[i].p_min),
					 K3bDevice::fromBcd(tr[i].p_sec),
					 K3bDevice::fromBcd(tr[i].p_frame) ) - 150;
	    else
	      sessionLeadOut = K3b::Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) - 150;
	  }
	}

	k3bDebug() << blockDeviceName() << ": setting last sector of last track to " << (sessionLeadOut-1).lba() << endl;

	// set the last track's last sector
	if( !toc.isEmpty() )
	  toc[toc.count()-1].m_lastSector = sessionLeadOut - 1;
      }
      else
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " empty raw toc." << endl;

      delete [] data;
    }
  }

  if( needToClose )
    close();

  return success;
}


int K3bDevice::Device::rawTocDataWithBcdValues( unsigned char* data, unsigned int dataLen ) const
{
  toc_raw_track_descriptor* tr = (toc_raw_track_descriptor*)&data[4];

  bool notBcd = false;
  bool notHex = false;

  //
  // in most cases this will already tell us if a drive does not provide bcd numbers
  // (which should be all newer MMC drives)
  //
  for( unsigned int i = 0; i < (dataLen-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
    if( tr[i].adr == 1 && tr[i].point <= 0xa2) {
      if( !K3bDevice::isValidBcd(tr[i].p_min) ||
	  !K3bDevice::isValidBcd(tr[i].p_sec) ||
	  !K3bDevice::isValidBcd(tr[i].p_frame) ) {
	notBcd = true;
	break;
      }

      // we only need to check sec and frame since min needs to be <= 99
      // and bcd values are never bigger than 99.
      else if( (int)K3bDevice::fromBcd(tr[i].p_sec) >= 60 ||
	       (int)K3bDevice::fromBcd(tr[i].p_frame) >= 75 ) {
	notBcd = true;
	break;
      }
    }
  }


  //
  // all values are valid bcd values but we still don't know for sure if they are really
  // used as bcd. So we also check the HEX values.
  //
  for( unsigned int i = 0; i < (dataLen-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
    if( tr[i].adr == 1 && tr[i].point <= 0xa2 ) {
      if( (int)tr[i].p_min > 99 ||
	  (int)tr[i].p_sec >= 60 ||
	  (int)tr[i].p_frame >= 75 ) {
	notHex = true;
	break;
      }
    }
  }


  //
  // If all values are valid bcd and valid hex we check the start sectors of the tracks.
  //
  if( !notHex || !notBcd ) {
    K3b::Msf sessionLeadOutHex, sessionLeadOutBcd;
    K3b::Msf lastTrackHex, lastTrackBcd;

    for( unsigned int i = 0; i < (dataLen-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {

      if( tr[i].adr == 1 ) {
	if( tr[i].point < 0x64 ) {

	  // check hex values
	  if( K3b::Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) <
	      lastTrackHex )
	    notHex = true;

	  // check bcd values
	  if( K3b::Msf( K3bDevice::fromBcd(tr[i].p_min), K3bDevice::fromBcd(tr[i].p_sec), K3bDevice::fromBcd(tr[i].p_frame) ) <
	      lastTrackBcd )
	    notBcd = true;

	  lastTrackBcd = K3b::Msf( K3bDevice::fromBcd(tr[i].p_min), K3bDevice::fromBcd(tr[i].p_sec), K3bDevice::fromBcd(tr[i].p_frame) );
	  lastTrackHex = K3b::Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame );
	}
	else if( tr[i].point == 0xa2 ) {
	  if( sessionLeadOutHex < lastTrackHex )
	    notHex = true;
	  if( sessionLeadOutBcd < lastTrackBcd )
	    notBcd = true;

	  sessionLeadOutHex = K3b::Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame );
	  sessionLeadOutBcd = K3b::Msf( K3bDevice::fromBcd(tr[i].p_min), K3bDevice::fromBcd(tr[i].p_sec), K3bDevice::fromBcd(tr[i].p_frame) );
	}
      }
    }

    // check the last track
    if( sessionLeadOutHex < lastTrackHex )
      notHex = true;
    if( sessionLeadOutBcd < lastTrackBcd )
      notBcd = true;
  }


  if( !notBcd && !notHex ) {
    k3bDebug() << "(K3bDevice::Device) need to compare raw toc to formatted toc. :(" << endl;
    //
    // All values are valid bcd and valid HEX values so we compare with the formatted toc.
    // This slows us down a lot but in most cases this should not be reached anyway.
    //
    // TODO: also check the bcd values
    //
    K3bDevice::Toc formattedToc;
    if( readFormattedToc( formattedToc, MEDIA_CD_ROM ) ) {
      for( unsigned int i = 0; i < (dataLen-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
	if( tr[i].adr == 1 && tr[i].point < 0x64 ) {
	  unsigned int track = (int)tr[i].point;

	  // FIXME: do bcd drive also encode the track number in bcd? If so test it, too.

	  if( track > formattedToc.count() ) {
	    notHex = true;
	    break;
	  }

	  K3b::Msf posHex( tr[i].p_min,
			   tr[i].p_sec,
			   tr[i].p_frame );
	  K3b::Msf posBcd( K3bDevice::fromBcd(tr[i].p_min),
			   K3bDevice::fromBcd(tr[i].p_sec),
			   K3bDevice::fromBcd(tr[i].p_frame) );
	  posHex -= 150;
	  posBcd -= 150;
	  if( posHex != formattedToc[track-1].firstSector() )
	    notHex = true;
	  if( posBcd != formattedToc[track-1].firstSector() )
	    notBcd = true;
	}
      }
    }
  }

  if( notBcd )
    k3bDebug() << "(K3bDevice::Device) found invalid bcd values. No bcd toc." << endl;
  if( notHex )
    k3bDebug() << "(K3bDevice::Device) found invalid hex values. No hex toc." << endl;

  if( notBcd == notHex ) {
    k3bDebug() << "(K3bDevice::Device) unable to determine if hex (" << notHex << ") or bcd (" << notBcd << ")." << endl;
    if( !notHex ) {
      k3bDebug() << "Assuming hex encoding in favor of newer drives and the more reliable raw toc." << endl;
      return 0;
    }
    return -1;
  }
  else if( notBcd )
    return 0;
  else
    return 1;
}


K3bDevice::CdText K3bDevice::Device::readCdText() const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  K3bDevice::CdText textData;

  if( open() ) {
    unsigned char* data = 0;
    unsigned int dataLen = 0;

    if( readTocPmaAtip( &data, dataLen, 5, false, 0 ) ) {
      textData.setRawPackData( data, dataLen );

      delete [] data;
    }

    if( needToClose )
      close();
  }

  return textData;
}


#ifdef Q_OS_LINUX
// fallback
bool K3bDevice::Device::readTocLinux( K3bDevice::Toc& toc ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  bool success = true;

  toc.clear();

  struct cdrom_tochdr tochdr;
  struct cdrom_tocentry tocentry;

  usageLock();
  if( open() ) {
    //
    // CDROMREADTOCHDR ioctl returns:
    // cdth_trk0: First Track Number
    // cdth_trk1: Last Track Number
    //
    if( ::ioctl( d->deviceFd, CDROMREADTOCHDR, &tochdr ) ) {
      k3bDebug() << "(K3bDevice::Device) could not get toc header !" << endl;
      success = false;
    }
    else {
      Track lastTrack;
      for (int i = tochdr.cdth_trk0; i <= tochdr.cdth_trk1 + 1; i++) {
        ::memset(&tocentry,0,sizeof (struct cdrom_tocentry));
        // get Lead-Out Information too
        tocentry.cdte_track = (i<=tochdr.cdth_trk1) ? i : CDROM_LEADOUT;
        tocentry.cdte_format = CDROM_LBA;
        //
        // CDROMREADTOCENTRY ioctl returns:
        // cdte_addr.lba: Start Sector Number (LBA Format requested)
        // cdte_ctrl:     4 ctrl bits
        //                   00x0b: 2 audio Channels(no pre-emphasis)
        //                   00x1b: 2 audio Channels(pre-emphasis)
        //                   10x0b: audio Channels(no pre-emphasis),reserved in cd-rw
        //                   10x1b: audio Channels(pre-emphasis),reserved in cd-rw
        //                   01x0b: data track, recorded uninterrupted
        //                   01x1b: data track, recorded incremental
        //                   11xxb: reserved
        //                   xx0xb: digital copy prohibited
        //                   xx1xb: digital copy permitted
        // cdte_addr:     4 addr bits (type of Q-Subchannel data)
        //                   0000b: no Information
        //                   0001b: current position data
        //                   0010b: MCN
        //                   0011b: ISRC
        //                   0100b-1111b:  reserved
        // cdte_datamode:  0: Data Mode1
        //                 1: CD-I
        //                 2: CD-XA Mode2
        //

        if( ::ioctl( d->deviceFd, CDROMREADTOCENTRY, &tocentry ) ) {
          k3bDebug() << "(K3bDevice::Device) error reading tocentry " << i << endl;
	  success = false;
	  break;
	}

        int startSec = tocentry.cdte_addr.lba;
        int control  = tocentry.cdte_ctrl & 0x0f;
        int mode     = tocentry.cdte_datamode;
        if( i > tochdr.cdth_trk0 ) {
	  Track track( lastTrack.firstSector(), startSec-1, lastTrack.type(), lastTrack.mode() );
	  track.m_preEmphasis = control & 0x1;
	  track.m_copyPermitted = control & 0x2;
          toc.append( track );
        }
        int trackType = 0;
        int trackMode = Track::UNKNOWN;
        if( (control & 0x04 ) && (tocentry.cdte_track != CDROM_LEADOUT) ) {
          trackType = Track::DATA;
          if( mode == 1 )
            trackMode = Track::MODE1;
          else if( mode == 2 )
            trackMode = Track::MODE2;

          mode = getDataMode(startSec);
          if( mode != Track::UNKNOWN )
            trackMode = mode;
        }
	else
          trackType = Track::AUDIO;

        lastTrack = Track( startSec, startSec, trackType, trackMode );
      }
    }

    if( needToClose )
      close();
  }
  else
    success = false;

  usageUnlock();

  return success;
}
#endif // Q_OS_LINUX


bool K3bDevice::Device::fixupToc( K3bDevice::Toc& toc ) const
{
  bool success = false;

  //
  // This is a very lame method of fixing the TOC of an Advanced Audio CD
  // (a CD with two sessions: one with audio tracks and one with the data track)
  // If a drive does not support reading raw toc or reading track info we only
  // get every track's first sector. But between sessions there is a gap which is used
  // for ms stuff. In this case it's 11400 sectors in size. When ripping ausio we would
  // include these 11400 sectors which would result in a strange ending audio file.
  //
  if( numSessions() > 1 || toc.contentType() == MIXED ) {
    k3bDebug() << "(K3bDevice::Device) fixup multisession toc..." << endl;

    //
    // we need to update the last sector of every last track in every session
    // for now we only update the track before the last session...
    // This is the most often case: Advanced Audio CD
    //

    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( readTocPmaAtip( &data, dataLen, 1, false, 0 ) ) {

      //
      // data[6]    - first track number in last complete session
      // data[8-11] - start address of first track in last session
      //

      toc[(unsigned int)data[6]-2].m_lastSector = from4Byte( &data[8] ) - 11400 - 1;

      delete [] data;
      success = true;
    }
    else
      k3bDebug() << "(K3bDevice::Device) FIXUP TOC failed." << endl;
  }

  return success;
}


bool K3bDevice::Device::block( bool b ) const
{
  //
  // For some reason the Scsi Command does not work here.
  // So we use the ioctl on Linux systems
  //
#if defined(Q_OS_LINUX)
    bool success = false;
    bool needToClose = !isOpen();
    usageLock();
    if( open() ) {
        success = ( ::ioctl( d->deviceFd, CDROM_LOCKDOOR, b ? 1 : 0 ) == 0 );
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#elif defined(Q_OS_NETBSD)
    bool success = false;
    bool needToClose = !isOpen();
    int arg = b ? 1 : 0;
    usageLock();
    if( open() ) {
        success = ( ::ioctl( d->deviceFd, DIOCLOCK, &arg ) == 0 );
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#endif

    ScsiCommand cmd( this );
    cmd[0] = MMC_PREVENT_ALLOW_MEDIUM_REMOVAL;
    cmd[5] = 0; // Necessary to set the proper command length
    if( b )
        cmd[4] = 0x01;
    int r = cmd.transport( TR_DIR_WRITE );

    if( r )
        k3bDebug() << "(K3bDevice::Device) MMC ALLOW MEDIA REMOVAL failed." << endl;

    return ( r == 0 );
}

bool K3bDevice::Device::rewritable() const
{
  unsigned char* data = 0;
  unsigned int dataLen = 0;

  if( readDiscInformation( &data, dataLen ) ) {
    disc_info_t* inf = (disc_info_t*)data;
    bool e = inf->erasable;

    delete [] data;

    return e;
  }
  else
    return false;
}


bool K3bDevice::Device::eject() const
{
#ifdef Q_OS_NETBSD
    bool success = false;
    bool needToClose = !isOpen();
    int arg = 0;

    usageLock();
    if( open() ) {
        if ( ::ioctl( d->deviceFd, DIOCEJECT, &arg ) >= 0)
            success = true;
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#elif defined(Q_OS_LINUX)
    bool success = false;
    bool needToClose = !isOpen();

    usageLock();
    if( open() ) {
        if( ::ioctl( d->deviceFd, CDROMEJECT ) >= 0 )
            success = true;
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#endif

    ScsiCommand cmd( this );
    cmd[0] = MMC_PREVENT_ALLOW_MEDIUM_REMOVAL;
    cmd[5] = 0; // Necessary to set the proper command length
    cmd.transport();

    cmd[0] = MMC_START_STOP_UNIT;
    cmd[5] = 0; // Necessary to set the proper command length
    cmd[4] = 0x1;      // Start unit
    cmd.transport();

    cmd[4] = 0x2;    // LoEj = 1, Start = 0

    return !cmd.transport();
}


bool K3bDevice::Device::load() const
{
#ifdef Q_OS_NETBSD
    bool success = false;
    bool needToClose = !isOpen();
    int arg = 0;

    usageLock();
    if( open() ) {
        if ( ::ioctl( d->deviceFd, CDIOCCLOSE, &arg ) >= 0)
            success = true;
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#elif defined(Q_OS_LINUX)
    bool success = false;
    bool needToClose = !isOpen();

    usageLock();
    if( open() ) {
        if( ::ioctl( d->deviceFd, CDROMCLOSETRAY ) >= 0 )
            success = true;
        if( needToClose )
            close();
    }
    usageUnlock();
    if ( success )
        return success;
#endif

    ScsiCommand cmd( this );
    cmd[0] = MMC_START_STOP_UNIT;
    cmd[4] = 0x3;    // LoEj = 1, Start = 1
    cmd[5] = 0;      // Necessary to set the proper command length
    return !cmd.transport();
}


bool K3bDevice::Device::setAutoEjectEnabled( bool enabled ) const
{
  bool success = false;
#ifdef Q_OS_LINUX

  bool needToClose = !isOpen();
  usageLock();
  if ( open() ) {
      success = ( ::ioctl( d->deviceFd, CDROMEJECT_SW, enabled ? 1 : 0 ) == 0 );
      if ( needToClose ) {
          close();
      }
  }
  usageUnlock();
#endif
  return success;
}


void K3bDevice::Device::addDeviceNode( const TQString& n )
{
  if( !d->allNodes.contains( n ) )
    d->allNodes.append( n );
}


const TQStringList& K3bDevice::Device::deviceNodes() const
{
  return d->allNodes;
}


K3bDevice::Device::Handle K3bDevice::Device::handle() const
{
#ifdef Q_OS_FREEBSD
  return d->cam;
#else
  return d->deviceFd;
#endif
}


bool K3bDevice::Device::open( bool write ) const
{
  if( d->openedReadWrite != write )
    close();

  TQMutexLocker ml( &d->openCloseMutex );

  d->openedReadWrite = write;

#ifdef Q_OS_FREEBSD
  if( !d->cam ) {
    d->cam = cam_open_pass (m_passDevice.latin1(), O_RDWR,0 /* NULL */);
    k3bDebug() << "(K3bDevice::openDevice) open device " << m_passDevice
	      << ((d->cam)?" succeeded.":" failed.") << endl;
  }

  return (d->cam != 0);
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_NETBSD)
  if( d->deviceFd == -1 )
    d->deviceFd = openDevice( TQFile::encodeName(devicename()), write );

  return ( d->deviceFd != -1 );
#endif
}


void K3bDevice::Device::close() const
{
  TQMutexLocker ml( &d->openCloseMutex );

#ifdef Q_OS_FREEBSD
  if( d->cam ) {
    cam_close_device(d->cam);
    d->cam = 0;
  }
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_NETBSD)
  if( d->deviceFd != -1 ) {
    ::close( d->deviceFd );
    d->deviceFd = -1;
  }
#endif
}


bool K3bDevice::Device::isOpen() const
{
#ifdef Q_OS_FREEBSD
  return d->cam;
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_NETBSD)
  return ( d->deviceFd != -1 );
#endif
}


int K3bDevice::Device::supportedProfiles() const
{
  return d->supportedProfiles;
}


int K3bDevice::Device::currentProfile() const
{
  unsigned char profileBuf[8];
  ::memset( profileBuf, 0, 8 );

  ScsiCommand cmd( this );
  cmd[0] = MMC_GET_CONFIGURATION;
  cmd[1] = 1;
  cmd[8] = 8;
  cmd[9] = 0;      // Necessary to set the proper command length

  if( cmd.transport( TR_DIR_READ, profileBuf, 8 ) ) {
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
	       << " GET_CONFIGURATION failed." << endl;
    return MEDIA_UNKNOWN;
  }
  else {
    short profile = from2Byte( &profileBuf[6] );

    //
    // Plextor drives might not set a current profile
    // In that case we get the list of all current profiles
    // and simply use the first one in that list.
    //
    if( profile == 0x00 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		 << " current profile 0. Checking current profile list instead." << endl;
      unsigned char* data;
      unsigned int len = 0;
      if( getFeature( &data, len, FEATURE_PROFILE_LIST ) ) {
	int featureLen( data[11] );
	for( int j = 0; j < featureLen; j+=4 ) {
	  // use the first current profile we encounter
	  if( data[12+j+2] & 0x1 ) {
	    profile = from2Byte( &data[12+j] );
	    break;
	  }
	}

	delete[] data;
      }
    }

    switch (profile) {
    case 0x00: return MEDIA_NONE;
    case 0x08: return MEDIA_CD_ROM;
    case 0x09: return MEDIA_CD_R;
    case 0x0A: return MEDIA_CD_RW;
    case 0x10: return MEDIA_DVD_ROM;
    case 0x11: return MEDIA_DVD_R_SEQ;
    case 0x12: return MEDIA_DVD_RAM;
    case 0x13: return MEDIA_DVD_RW_OVWR;
    case 0x14: return MEDIA_DVD_RW_SEQ;
    case 0x15: return MEDIA_DVD_R_DL_SEQ;
    case 0x16: return MEDIA_DVD_R_DL_JUMP;
    case 0x1A: return MEDIA_DVD_PLUS_RW;
    case 0x1B: return MEDIA_DVD_PLUS_R;
    case 0x2B: return MEDIA_DVD_PLUS_R_DL;
    case 0x40: return MEDIA_BD_ROM;
    case 0x41: {
      if( featureCurrent( FEATURE_BD_PSEUDO_OVERWRITE ) == 1 )
	return MEDIA_BD_R_SRM_POW;
      else
	return MEDIA_BD_R_SRM;
    }
    case 0x42: return MEDIA_BD_R_RRM;
    case 0x43: return MEDIA_BD_RE;
    case 0x50: return MEDIA_HD_DVD_ROM;
    case 0x51: return MEDIA_HD_DVD_R;
    case 0x52: return MEDIA_HD_DVD_RAM;
    default: return MEDIA_UNKNOWN;
    }
  }
}


K3bDevice::DiskInfo K3bDevice::Device::diskInfo() const
{
  DiskInfo inf;

  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  if( open() ) {

    unsigned char* data = 0;
    unsigned int dataLen = 0;

    //
    // The first thing to do should be: checking if a media is loaded
    // We cannot rely on the profile here since at least some Plextor
    // drives return the NO MEDIUM profile for CD media
    //
    if( !testUnitReady() ) {
      // no disk or tray open
      inf.m_diskState = STATE_NO_MEDIA;
      inf.m_mediaType = MEDIA_NONE;
      inf.m_currentProfile = MEDIA_NONE;
    }
    else
      inf.m_currentProfile = currentProfile();

    if( inf.diskState() != STATE_NO_MEDIA ) {

      if( readDiscInformation( &data, dataLen ) ) {
	disc_info_t* dInf = (disc_info_t*)data;
	//
	// Copy the needed values from the disk_info struct
	//
	switch( dInf->status ) {
	case 0:
	  inf.m_diskState = STATE_EMPTY;
	  break;
	case 1:
	  inf.m_diskState = STATE_INCOMPLETE;
	  break;
	case 2:
	  inf.m_diskState = STATE_COMPLETE;
	  break;
	default:
	  inf.m_diskState = STATE_UNKNOWN;
	  break;
	}

	switch( dInf->border ) {
	case 0:
	  inf.m_lastSessionState = STATE_EMPTY;
	  break;
	case 1:
	  inf.m_lastSessionState = STATE_INCOMPLETE;
	  break;
	case 2:
	  inf.m_lastSessionState = STATE_COMPLETE;
	  break;
	default:
	  inf.m_lastSessionState = STATE_UNKNOWN;
	  break;
	}

        switch( dInf->bg_f_status&0x3 ) {
        case 0x0:
            inf.m_bgFormatState = BG_FORMAT_NONE;
            break;
        case 0x1:
            inf.m_bgFormatState = BG_FORMAT_INCOMPLETE;
            break;
        case 0x2:
            inf.m_bgFormatState = BG_FORMAT_IN_PROGRESS;
            break;
        case 0x3:
            inf.m_bgFormatState = BG_FORMAT_COMPLETE;
            break;
        }

	inf.m_numTracks = (dInf->last_track_l & 0xff) | (dInf->last_track_m<<8 & 0xff00);
	if( inf.diskState() == STATE_EMPTY )
	  inf.m_numTracks = 0;

	// FIXME: I am not sure if this is accurate. Better test the last track's RT field
	else if( inf.diskState() == STATE_INCOMPLETE )
	  inf.m_numTracks--;  // do not count the invisible track

	inf.m_rewritable = dInf->erasable;

	//
	// This is the Last Possible Lead-Out Start Address in HMSF format
	// This is only valid for CD-R(W) and DVD+R media.
	// For complete media this shall be filled with 0xff
	//
	if( dInf->lead_out_m != 0xff &&
	    dInf->lead_out_r != 0xff &&
	    dInf->lead_out_s != 0xff &&
	    dInf->lead_out_f != 0xff )
	  inf.m_capacity = K3b::Msf( dInf->lead_out_m + dInf->lead_out_r*60,
				     dInf->lead_out_s,
				     dInf->lead_out_f ) - 150;

	//
	// This is the position where the next Session shall be recorded in HMSF format
	// This is only valid for CD-R(W) and DVD+R media.
	// For complete media this shall be filled with 0xff
	//
	if( dInf->lead_in_m != 0xff &&
	    dInf->lead_in_r != 0xff &&
	    dInf->lead_in_s != 0xff &&
	    dInf->lead_in_f != 0xff )
	  inf.m_usedCapacity = K3b::Msf( dInf->lead_in_m + dInf->lead_in_r*60,
					 dInf->lead_in_s,
					 dInf->lead_in_f ) - 4500;

	delete [] data;
      }
      else {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		   << " fabricating disk information for a stupid device." << endl;
	Toc toc = readToc();
	if( !toc.isEmpty() ) {
	  inf.m_diskState = STATE_COMPLETE;
	  inf.m_lastSessionState = STATE_COMPLETE;
	  inf.m_numTracks = toc.count();
	  inf.m_capacity = inf.m_usedCapacity = toc.length();
	}
      }


      //
      // The mediatype needs to be set
      //
      inf.m_mediaType = mediaType();

      // At least some Plextor drives return profile NONE for CD media
      // or CD_ROM for writable media
      if( inf.m_mediaType & (MEDIA_UNKNOWN|MEDIA_NONE|MEDIA_CD_ROM) ) {
	// probably it is a CD
	if( inf.rewritable() )
	  inf.m_mediaType = MEDIA_CD_RW;
	else if( inf.empty() || inf.appendable() )
	  inf.m_mediaType = MEDIA_CD_R;
	else
	  inf.m_mediaType = MEDIA_CD_ROM;
      }

      if( inf.m_mediaType & MEDIA_DVD_ALL ) {
	if( readDvdStructure( &data, dataLen ) ) {
	  // some debugging stuff
	  K3b::Msf sda, eda, ea0;
	  sda = ( data[4+5]<<16 | data[4+6] << 8 | data[4+7] );
	  eda = ( data[4+9]<<16 | data[4+10] << 8 | data[4+11] );
	  ea0 = ( data[4+13]<<16 | data[4+14] << 8 | data[4+15] );

	  k3bDebug() << "First sec data area: " << sda.toString()
		    << " (LBA " << TQString::number(sda.lba())
		    << ") (" << TQString::number(sda.mode1Bytes()) << endl;
	  k3bDebug() << "Last sec data area: " << eda.toString()
		    << " (LBA " << TQString::number(eda.lba())
		    << ") (" << TQString::number(eda.mode1Bytes()) << " Bytes)" << endl;
	  k3bDebug() << "Last sec layer 1: " << ea0.toString()
		    << " (LBA " << TQString::number(ea0.lba())
		    << ") (" << TQString::number(ea0.mode1Bytes()) << " Bytes)" << endl;


	  K3b::Msf da0 = ea0 - sda + 1;
	  K3b::Msf da1 = eda - ea0;
	  k3bDebug() << "Layer 1 length: " << da0.toString()
		    << " (LBA " << TQString::number(da0.lba())
		    << ") (" << TQString::number(da0.mode1Bytes()) << " Bytes)" << endl;
	  k3bDebug() << "Layer 2 length: " << da1.toString()
		    << " (LBA " << TQString::number(da1.lba())
		    << ") (" << TQString::number(da1.mode1Bytes()) << " Bytes)" << endl;

	  inf.m_numLayers = ((data[6]&0x60) == 0 ? 1 : 2);

	  bool otp = (data[4+2] & 0xF);

	  // ea0 is 0 if the medium does not use Opposite track path
	  if( otp && ea0 > 0 )
	    inf.m_firstLayerSize = da0;
	  else
	    inf.m_firstLayerSize = 0;

	  delete [] data;
	}
	else {
	  k3bDebug() << "(K3bDevice::Device) Unable to read DVD structure for num of layers." << endl;
	  inf.m_numLayers = ( (inf.m_mediaType & MEDIA_WRITABLE_DVD_DL) ? 2 : 1 );
	}
      }


      //
      // Number of sessions for non-empty disks
      //
      if( inf.diskState() != STATE_EMPTY ) {
	int sessions = numSessions();
	if( sessions >= 0 )
	  inf.m_numSessions = sessions;
	else
	  k3bDebug() << "(K3bDevice::Device) could not get session info via READ TOC/PMA/ATIP." << endl;
      }
      else
	inf.m_numSessions = 0;

      inf.m_mediaId = mediaId( inf.mediaType() );

      //
      // Now we determine the size:

      // for all empty and appendable media READ FORMAT CAPACITIES should return the proper unformatted size
      // for complete disks we may use the READ_CAPACITY command or the start sector from the leadout
      //
      int media = inf.mediaType();
      //
      // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
      // if supported at all
      //
      if( inf.currentProfile() == MEDIA_DVD_ROM )
	media = MEDIA_DVD_ROM;

      switch( media ) {
      case MEDIA_CD_R:
      case MEDIA_CD_RW:
	if( inf.m_capacity == 0 ) {
	  if( readTocPmaAtip( &data, dataLen, 0x4, true, 0 ) ) {

	    struct atip_descriptor* atip = (struct atip_descriptor*)data;

	    if( dataLen >= 11 ) {
	      inf.m_capacity = K3b::Msf( atip->lead_out_m, atip->lead_out_s, atip->lead_out_f ) - 150;
	      debugBitfield( &atip->lead_out_m, 3 );
	      k3bDebug() << blockDeviceName() << ": ATIP capacity: " << inf.m_capacity.toString() << endl;
	    }

	    delete [] data;
	  }
	}

	//
	// for empty and appendable media capacity and usedCapacity should be filled in from
	// diskinfo above. If not they are both still 0
	//
	if( inf.m_capacity != 0 &&
	    ( inf.diskState() == STATE_EMPTY || inf.m_usedCapacity != 0 ) ) {
	  // done.
	  break;
	}

      default:
      case MEDIA_CD_ROM:
	if( inf.m_capacity > 0 && inf.m_usedCapacity == 0 )
	  inf.m_usedCapacity = inf.m_capacity;

	if( inf.m_usedCapacity == 0 ) {
	  K3b::Msf readCap;
	  if( readCapacity( readCap ) ) {
	    k3bDebug() << "(K3bDevice::Device) READ CAPACITY: " << readCap.toString()
		      << " other capacity: " << inf.m_capacity.toString() << endl;
	    //
	    // READ CAPACITY returns the last written sector
	    // that means the size is actually readCap + 1
	    //
	    inf.m_usedCapacity = readCap + 1;
	  }
	  else {
	    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		      << " Falling back to readToc for capacity." << endl;
	    inf.m_usedCapacity = readToc().length();
	  }
	}

      case MEDIA_DVD_ROM: {
	K3b::Msf readCap;
	if( readCapacity( readCap ) ) {
	  k3bDebug() << "(K3bDevice::Device) READ CAPACITY: " << readCap.toString()
		    << " other capacity: " << inf.m_capacity.toString() << endl;
	  //
	  // READ CAPACITY returns the last written sector
	  // that means the size is actually readCap + 1
	  //
	  inf.m_usedCapacity = readCap + 1;
	}
	else {
	  //
	  // Only one track, use it's size
	  //
	  if( readTrackInformation( &data, dataLen, 0x1, 0x1 ) ) {
	    track_info_t* trackInfo = (track_info_t*)data;
	    inf.m_usedCapacity = from4Byte( trackInfo->track_size );
	    delete [] data;
	  }
	  else
	    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		      << "READ TRACK INFORMATION for DVD-ROM failed." << endl;
	}

	break;
      }

      case MEDIA_DVD_PLUS_R:
      case MEDIA_DVD_PLUS_R_DL:
	if( inf.appendable() || inf.empty() ) {
	  //
	  // get remaining space via the invisible track
	  //
	  if( readTrackInformation( &data, dataLen, 0x1, /*0xff*/ inf.numTracks()+1 ) ) {
	    track_info_t* trackInfo = (track_info_t*)data;
	    inf.m_usedCapacity = from4Byte( trackInfo->track_start );
	    inf.m_capacity = from4Byte( trackInfo->track_start ) + from4Byte( trackInfo->track_size );
	    delete [] data;
	  }
	}
	else {
	  if( readTrackInformation( &data, dataLen, 0x1, inf.numTracks() ) ) {
	    track_info_t* trackInfo = (track_info_t*)data;
	    inf.m_capacity = inf.m_usedCapacity
	      = from4Byte( trackInfo->track_start ) + from4Byte( trackInfo->track_size );
	    delete [] data;
	  }
	}
	break;

      case MEDIA_DVD_R:
      case MEDIA_DVD_R_SEQ:
      case MEDIA_DVD_R_DL:
      case MEDIA_DVD_R_DL_JUMP:
      case MEDIA_DVD_R_DL_SEQ:
	//
	// get data from the incomplete track (which is NOT the invisible track 0xff)
	// This will fail in case the media is complete!
	//
	if( readTrackInformation( &data, dataLen, 0x1, inf.numTracks()+1 ) ) {
	  track_info_t* trackInfo = (track_info_t*)data;
	  inf.m_usedCapacity = from4Byte( trackInfo->track_start );
	  inf.m_capacity = from4Byte( trackInfo->free_blocks ) + from4Byte( trackInfo->track_start );
	  delete [] data;
	}

	//
	// Get the "really" used space without border-out
	//
	if( !inf.empty() ) {
	  K3b::Msf readCap;
	  if( readCapacity( readCap ) ) {
	    //
	    // READ CAPACITY returns the last written sector
	    // that means the size is actually readCap + 1
	    //
	    inf.m_usedCapacity = readCap + 1;
	  }
	  else
	    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		      << " READ CAPACITY for DVD-R failed." << endl;
	}

	break;

      case MEDIA_DVD_RW_OVWR:
	inf.m_numSessions = 1;
      case MEDIA_DVD_RW:
      case MEDIA_DVD_RW_SEQ:
	// only one track on a DVD-RW media
	if( readTrackInformation( &data, dataLen, 0x1, 0x1 ) ) {
	  track_info_t* trackInfo = (track_info_t*)data;
	  inf.m_capacity = from4Byte( trackInfo->track_size );
	  if( !inf.empty() ) {
	    if( readFormatCapacity( 0x10, inf.m_capacity ) )
	      k3bDebug() << blockDeviceName() << ": Format capacity 0x10: " << inf.m_capacity.toString() << endl;

	    inf.m_usedCapacity = from4Byte( trackInfo->track_size );
	  }

	  delete [] data;
	}
	break;

      case MEDIA_DVD_PLUS_RW: {
	K3b::Msf currentMax;
	int currentMaxFormat = 0;
	if( readFormatCapacity( 0x26, inf.m_capacity, &currentMax, &currentMaxFormat ) ) {
            if( currentMaxFormat == 0x1 ) { // unformatted or blank media
                inf.m_usedCapacity = 0;
                inf.m_capacity = currentMax;
            }
            else {
                inf.m_usedCapacity = currentMax;
                // Plextor drives tend to screw things up and report invalid values
                // for the max format capacity of 1.4 GB DVD media
                if ( inf.bgFormatState() == BG_FORMAT_COMPLETE ) {
                    inf.m_capacity = currentMax;
                }
            }
       	}
	else
	  k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		    << " READ FORMAT CAPACITIES for DVD+RW failed." << endl;

	break;
      }

      case MEDIA_BD_R:
      case MEDIA_BD_R_SRM:
      case MEDIA_BD_R_SRM_POW:
      case MEDIA_BD_R_RRM:
      case MEDIA_BD_RE:
	//
	// get the invisible track's first sector
	// or the next writable address of the last open track
	//
	if( readDiscInformation( &data, dataLen ) ) {
	  int lastTrack = (int)( data[11]<<8 | data[6] );
	  delete [] data;

	  if( readTrackInformation( &data, dataLen, 1, lastTrack ) ) {

	    // capacity: last track's start address + last track's size
	    inf.m_capacity = from4Byte( data+8 ) + from4Byte( data+24 );

	    if( data[6] & 0x80 )
	      inf.m_usedCapacity = from4Byte( data+8 );
	    else if( data[7] & 0x1 )
	      inf.m_usedCapacity = from4Byte( data+12 );
	    delete [] data;
	  }
	}
	break;

      case MEDIA_BD_ROM: {
	K3b::Msf readCap;
	if( readCapacity( readCap ) ) {
	  //
	  // READ CAPACITY returns the last written sector
	  // that means the size is actually readCap + 1
	  //
	  inf.m_usedCapacity = readCap + 1;
	}

	break;
      }
      }
    }

    if( needToClose )
      close();
  }

  return inf;
}


int K3bDevice::Device::mediaType() const
{
  int m = MEDIA_UNKNOWN;

  if( testUnitReady() ) {

    m = currentProfile();

    if( m & (MEDIA_UNKNOWN|MEDIA_DVD_ROM|MEDIA_HD_DVD_ROM) ) {
      //
      // We prefere the mediatype as reported by the media since this way
      // even ROM drives may report the correct type of writable media.
      //

      // 4 bytes header + 2048 bytes layer descriptor
      unsigned char* data = 0;
      unsigned int dataLen = 0;
      if( readDvdStructure( &data, dataLen ) ) {
	switch( data[4]&0xF0 ) {
	case 0x00: m = MEDIA_DVD_ROM; break;
	case 0x10: m = MEDIA_DVD_RAM; break;
	case 0x20: m = MEDIA_DVD_R; break; // there seems to be no value for DVD-R DL, it reports DVD-R
	case 0x30: m = MEDIA_DVD_RW; break;
	case 0x40: m = MEDIA_HD_DVD_ROM; break;
	case 0x50: m = MEDIA_HD_DVD_R; break;
	case 0x60: m = MEDIA_HD_DVD_RAM; break;
	case 0x90: m = MEDIA_DVD_PLUS_RW; break;
	case 0xA0: m = MEDIA_DVD_PLUS_R; break;
	case 0xE0: m = MEDIA_DVD_PLUS_R_DL; break;
	default:
	  k3bDebug() << "(K3bDevice::Device) unknown dvd media type: " << TQString::number(data[4]&0xF0, 8) << endl;
	  break; // unknown
	}

	delete [] data;
      }
    }

    if( m & (MEDIA_UNKNOWN|MEDIA_BD_ROM) ) {
      //
      // We prefere the mediatype as reported by the media since this way
      // even ROM drives may report the correct type of writable media.
      //

      unsigned char* data = 0;
      unsigned int dataLen = 0;
      if( readDiscStructure( &data, dataLen, 1, 0 ) ) {
	if( dataLen > 4+12 &&
	    data[4+8] == 'B' &&  data[4+9] == 'D' ) {
	  switch( data[4+10] ) {
	  case 'O': m = MEDIA_BD_ROM; break;
	  case 'W': m = MEDIA_BD_RE; break;
	  case 'R': m = MEDIA_BD_R; break;
	  }
	}

	delete [] data;
      }
    }

    //
    // Only old CD or DVD devices do not report a current profile
    // or report CD-ROM profile for all CD types
    //
    if( m & (MEDIA_UNKNOWN|MEDIA_CD_ROM) ) {
      unsigned char* data = 0;
      unsigned int dataLen = 0;
      if( readTocPmaAtip( &data, dataLen, 4, false, 0 ) ) {
	if( (data[6]>>6)&1 )
	  m = MEDIA_CD_RW;
	else
	  m = MEDIA_CD_R;

	delete [] data;
      }
      else
	m = MEDIA_CD_ROM;
    }
  }

  return m;
}


bool K3bDevice::Device::readSectorsRaw( unsigned char *buf, int start, int count ) const
{
  return readCd( buf, count*2352,
		 0,      // all sector types
		 false,  // no dap
		 start,
		 count,
		 true, // SYNC
		 true, // HEADER
		 true, // SUBHEADER
		 true, // USER DATA
		 true, // EDC/ECC
		 0,    // no c2 info
		 0 );
}



void K3bDevice::Device::checkForJustLink()
{
  unsigned char* ricoh = 0;
  unsigned int ricohLen = 0;
  if( modeSense( &ricoh, ricohLen, 0x30 ) ) {

    //
    // 8 byte mode header + 6 byte page data
    //

    if( ricohLen >= 14 ) {
      ricoh_mode_page_30* rp = (ricoh_mode_page_30*)(ricoh+8);
      d->burnfree = rp->BUEFS;
    }

    delete [] ricoh;
  }
}


void K3bDevice::Device::checkFeatures()
{
  unsigned char header[1024];
  ::memset( header, 0, 1024 );

  ScsiCommand cmd( this );
  cmd[0] = MMC_GET_CONFIGURATION;
  cmd[1] = 2;
  cmd[9] = 0;      // Necessary to set the proper command length


  //
  // CD writing features
  //
  cmd[2] = FEATURE_CD_MASTERING>>8;
  cmd[3] = FEATURE_CD_MASTERING;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "CD Mastering" << endl;
#ifdef WORDS_BIGENDIAN
      struct cd_mastering_feature {
	unsigned char reserved1 : 1;
	unsigned char BUF       : 1;  // Burnfree
	unsigned char SAO       : 1;  // Session At Once writing
	unsigned char raw_ms    : 1;  // Writing Multisession in Raw Writing Mode
	unsigned char raw       : 1;  // Writing in WRITINGMODE_RAW mode
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char cd_rw     : 1;  // CD-RW support
	unsigned char rw_sub    : 1;  // Write R-W sub channels with user data
	unsigned char max_cue_length[3];
      };
#else
      struct cd_mastering_feature {
	unsigned char rw_sub    : 1;  // Write R-W sub channels with user data
	unsigned char cd_rw     : 1;  // CD-RW support
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char raw       : 1;  // Writing in WRITINGMODE_RAW mode
	unsigned char raw_ms    : 1;  // Writing Multisession in Raw Writing Mode
	unsigned char SAO       : 1;  // Session At Once writing
	unsigned char BUF       : 1;  // Burnfree
	unsigned char reserved1 : 1;
	unsigned char max_cue_length[3];
      };
#endif

      struct cd_mastering_feature* p = (struct cd_mastering_feature*)&header[12];
      if( p->BUF ) d->burnfree = true;
      d->writeCapabilities |= MEDIA_CD_R;
      if( p->cd_rw )
	d->writeCapabilities |= MEDIA_CD_RW;
//       if( p->WRITINGMODE_SAO ) m_writeModes |= WRITINGMODE_SAO;
//       if( p->raw || p->raw_ms ) m_writeModes |= WRITINGMODE_RAW;  // WRITINGMODE_RAW16 always supported when raw is supported?
    }
  }

  cmd[2] = FEATURE_CD_TRACK_AT_ONCE>>8;
  cmd[3] = FEATURE_CD_TRACK_AT_ONCE;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "CD Track At Once" << endl;
#ifdef WORDS_BIGENDIAN
      struct cd_track_at_once_feature {
	unsigned char reserved1 : 1;
	unsigned char BUF       : 1;  // Burnfree
	unsigned char reserved2 : 1;
	unsigned char rw_raw    : 1;  // Writing R-W subcode in Raw mode
	unsigned char rw_pack   : 1;  // Writing R-W subcode in Packet mode
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char cd_rw     : 1;  // CD-RW support
	unsigned char rw_sub    : 1;  // Write R-W sub channels with user data
	unsigned char reserved3;
	unsigned char data_type[2];
      };
#else
      struct cd_track_at_once_feature {
	unsigned char rw_sub    : 1;  // Write R-W sub channels with user data
	unsigned char cd_rw     : 1;  // CD-RW support
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char rw_pack   : 1;  // Writing R-W subcode in Packet mode
	unsigned char rw_raw    : 1;  // Writing R-W subcode in Raw mode
	unsigned char reserved2 : 1;
	unsigned char BUF       : 1;  // Burnfree
	unsigned char reserved1 : 1;
	unsigned char reserved3;
	unsigned char data_type[2];
      };
#endif

      struct cd_track_at_once_feature* p = (struct cd_track_at_once_feature*)&header[12];
      m_writeModes |= WRITINGMODE_TAO;
      if( p->BUF ) d->burnfree = true;
      d->writeCapabilities |= MEDIA_CD_R;
      if( p->cd_rw )
	d->writeCapabilities |= MEDIA_CD_RW;

      // is the following correct? What exactly does rw_sub tell us?
//       if( m_writeModes & WRITINGMODE_RAW ) {
// 	if( p->rw_raw ) m_writeModes |= WRITINGMODE_RAW_R96R;
// 	if( p->rw_pack ) m_writeModes |= WRITINGMODE_RAW_R96P;
//       }

//       // check the data types for 1, 2, and 3 (raw16, raw96p, and raw96r)
//        debugBitfield( p->data_type, 2 );
//       if( m_writeModes & WRITINGMODE_RAW ) {
// 	if( p->data_type[1] & 0x20 ) m_writeModes |= WRITINGMODE_RAW_R16;
// 	if( p->data_type[1] & 0x40 ) m_writeModes |= WRITINGMODE_RAW_R96P;
// 	if( p->data_type[1] & 0x80 ) m_writeModes |= WRITINGMODE_RAW_R96R;
//       }
    }
  }

  cmd[2] = FEATURE_CD_RW_MEDIA_WRITE_SUPPORT>>8;
  cmd[3] = FEATURE_CD_RW_MEDIA_WRITE_SUPPORT;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "CD-RW Media Write Support" << endl;
      d->writeCapabilities |= (MEDIA_CD_R|MEDIA_CD_RW);
    }
  }


  //
  // DVD-ROM
  //
  // FIXME: since MMC5 the feature descr. is 8 bytes in length including a dvd dl read bit at byte 6
  cmd[2] = FEATURE_DVD_READ>>8;
  cmd[3] = FEATURE_DVD_READ;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD Read (MMC5)" << endl;
      d->readCapabilities |= MEDIA_DVD_ROM;
      if( header[8+6] & 0x1 )
	d->readCapabilities |= MEDIA_WRITABLE_DVD_DL;
    }
  }
  else {
    // retry with pre-MMC5 length
    cmd[8] = 8+4;
    if( !cmd.transport( TR_DIR_READ, header, 12 ) ) {
      unsigned int len = from4Byte( header );
      if( len >= 8 ) {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD Read (pre-MMC5)" << endl;
	d->readCapabilities |= MEDIA_DVD_ROM;
      }
    }
  }

  //
  // DVD+R(W) writing features
  //
  cmd[2] = FEATURE_DVD_PLUS_R>>8;
  cmd[3] = FEATURE_DVD_PLUS_R;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD+R" << endl;
      d->readCapabilities |= MEDIA_DVD_PLUS_R;
      if( header[12] & 0x1 )
	d->writeCapabilities |= MEDIA_DVD_PLUS_R;
    }
  }

  cmd[2] = FEATURE_DVD_PLUS_RW>>8;
  cmd[3] = FEATURE_DVD_PLUS_RW;
  cmd[8] = 8+8;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD+RW" << endl;
#ifdef WORDS_BIGENDIAN
      struct dvd_plus_rw_feature {
	unsigned char reserved1   : 7;
	unsigned char write       : 1;
	unsigned char reserved2   : 6;
	unsigned char quick_start : 1;
	unsigned char close_only  : 1;
	// and some stuff we do not use here...
      };
#else
      struct dvd_plus_rw_feature {
	unsigned char write       : 1;
	unsigned char reserved1   : 7;
	unsigned char close_only  : 1;
	unsigned char quick_start : 1;
	unsigned char reserved2   : 6;
	// and some stuff we do not use here...
      };
#endif

      struct dvd_plus_rw_feature* p = (struct dvd_plus_rw_feature*)&header[12];
      d->readCapabilities |= MEDIA_DVD_PLUS_RW;
      if( p->write )
	d->writeCapabilities |= MEDIA_DVD_PLUS_RW;
    }
  }


  // some older DVD-ROM drives claim to support DVD+R DL
  if( d->writeCapabilities & MEDIA_DVD_PLUS_R ) {
    cmd[2] = FEATURE_DVD_PLUS_RW_DUAL_LAYER>>8;
    cmd[3] = FEATURE_DVD_PLUS_RW_DUAL_LAYER;
    cmd[8] = 8+8;
    if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
      unsigned int len = from4Byte( header );
      if( len >= 12 ) {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD+RW Double Layer" << endl;
	d->readCapabilities |= MEDIA_DVD_PLUS_RW_DL;
	if( header[12] & 0x1 )
	  d->writeCapabilities |= MEDIA_DVD_PLUS_RW_DL;
      }
    }

    cmd[2] = FEATURE_DVD_PLUS_R_DUAL_LAYER>>8;
    cmd[3] = FEATURE_DVD_PLUS_R_DUAL_LAYER;
    cmd[8] = 8+8;
    if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
      unsigned int len = from4Byte( header );
      if( len >= 12 ) {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD+R Double Layer" << endl;
	d->readCapabilities |= MEDIA_DVD_PLUS_R_DL;
	if( header[12] & 0x1 )
	  d->writeCapabilities |= MEDIA_DVD_PLUS_R_DL;
      }
    }
  }


  //
  // Blue Ray
  //
  // We do not care for the different BD classes and versions here
  //
  cmd[2] = FEATURE_BD_READ>>8;
  cmd[3] = FEATURE_BD_READ;
  cmd[8] = 8+32;
  if( !cmd.transport( TR_DIR_READ, header, 40 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 36 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "BD Read" << endl;
      if( header[8+8] || header[8+9] || header[8+10] || header[8+11] || header[8+12] || header[8+13] || header[8+14] || header[8+15] )
	d->readCapabilities |= MEDIA_BD_RE;
      if( header[8+16] || header[8+17] || header[8+18] || header[8+19] || header[8+20] || header[8+21] || header[8+22] || header[8+23] )
	d->readCapabilities |= MEDIA_BD_R;
      if( header[8+24] || header[8+25] || header[8+26] || header[8+27] || header[8+28] || header[8+29] || header[8+30] || header[8+31] )
	d->readCapabilities |= MEDIA_BD_ROM;
    }
  }

  cmd[2] = FEATURE_BD_WRITE>>8;
  cmd[3] = FEATURE_BD_WRITE;
  cmd[8] = 8+24;
  if( !cmd.transport( TR_DIR_READ, header, 32 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 28 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "BD Write" << endl;
      if( header[8+8] || header[8+9] || header[8+10] || header[8+11] || header[8+12] || header[8+13] || header[8+14] || header[8+15] )
	d->writeCapabilities |= MEDIA_BD_RE;
      if( header[8+16] || header[8+17] || header[8+18] || header[8+19] || header[8+20] || header[8+21] || header[8+22] || header[8+23] ) {
	d->writeCapabilities |= MEDIA_BD_R;
	m_writeModes |= WRITINGMODE_SRM;

	cmd[2] = FEATURE_BD_PSEUDO_OVERWRITE>>8;
	cmd[3] = FEATURE_BD_PSEUDO_OVERWRITE;
	cmd[8] = 8+8;
	if( !cmd.transport( TR_DIR_READ, header, 8+8 ) ) {
	  unsigned int len = from4Byte( header );
	  if( len >= 4+8 ) {
	    m_writeModes |= WRITINGMODE_SRM_POW;
	  }
	}

	cmd[2] = FEATURE_RANDOM_WRITABLE>>8;
	cmd[3] = FEATURE_RANDOM_WRITABLE;
	cmd[8] = 8+16;
	if( !cmd.transport( TR_DIR_READ, header, 8+16 ) ) {
	  unsigned int len = from4Byte( header );
	  if( len >= 4+16 ) {
	   m_writeModes |= WRITINGMODE_RRM;
	  }
	}
      }
    }
  }



  //
  // DVD-R(W)
  //
  cmd[2] = FEATURE_DVD_R_RW_WRITE>>8;
  cmd[3] = FEATURE_DVD_R_RW_WRITE;
  cmd[8] = 16;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "DVD-R/-RW Write" << endl;
#ifdef WORDS_BIGENDIAN
      struct dvd_r_rw_write_feature {
	unsigned char reserved1 : 1;
	unsigned char BUF       : 1;  // Burnfree
	unsigned char reserved2 : 2;
	unsigned char RDL       : 1;
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char dvd_rw    : 1;  // DVD-RW Writing
	unsigned char reserved3 : 1;
	unsigned char reserved4[3];
      };
#else
      struct dvd_r_rw_write_feature {
	unsigned char reserved3 : 1;
	unsigned char dvd_rw    : 1;  // DVD-RW Writing
	unsigned char testwrite : 1;  // Simulation write support
	unsigned char RDL       : 1;
	unsigned char reserved2 : 2;
	unsigned char BUF       : 1;  // Burnfree
	unsigned char reserved1 : 1;
	unsigned char reserved4[3];
      };
#endif

      struct dvd_r_rw_write_feature* p = (struct dvd_r_rw_write_feature*)&header[12];
      if( p->BUF ) d->burnfree = true;
      d->writeCapabilities |= (MEDIA_DVD_R|MEDIA_DVD_R_SEQ);
      if( p->dvd_rw )
	d->writeCapabilities |= (MEDIA_DVD_RW|MEDIA_DVD_RW_SEQ);
      if( p->RDL )
	d->writeCapabilities |= (MEDIA_DVD_R_DL|MEDIA_DVD_R_DL_SEQ);

      m_dvdMinusTestwrite = p->testwrite;
    }
  }


  //
  // DVD-RW restricted overwrite check
  //
  cmd[2] = FEATURE_RIGID_RESTRICTED_OVERWRITE>>8;
  cmd[3] = FEATURE_RIGID_RESTRICTED_OVERWRITE;
  cmd[8] = 16;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "Rigid Restricted Overwrite" << endl;
      m_writeModes |= WRITINGMODE_RES_OVWR;
      d->writeCapabilities |= (MEDIA_DVD_RW|MEDIA_DVD_RW_OVWR);
    }
  }


  //
  // DVD-R Dual Layer Layer
  //
  cmd[2] = FEATURE_LAYER_JUMP_RECORDING>>8;
  cmd[3] = FEATURE_LAYER_JUMP_RECORDING;
  cmd[8] = 12;
  if( !cmd.transport( TR_DIR_READ, header, 12 ) ) {
    // Now the jump feature is longer than 4 bytes but we don't need the link sizes.
    unsigned int len = from4Byte( header );
    if( len >= 8 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "Layer Jump Recording" << endl;
      d->writeCapabilities |= (MEDIA_DVD_R_DL|MEDIA_DVD_R_DL_JUMP);
      m_writeModes |= WRITINGMODE_LAYER_JUMP;
    }
  }


  //
  // HD-DVD-ROM
  //
  cmd[2] = FEATURE_HD_DVD_READ>>8;
  cmd[3] = FEATURE_HD_DVD_READ;
  cmd[8] = 16;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "HD-DVD Read" << endl;
      d->readCapabilities |= MEDIA_HD_DVD_ROM;
      if( header[8+4] & 0x1 )
	d->readCapabilities |= MEDIA_HD_DVD_R;
      if( header[8+6] & 0x1 )
	d->readCapabilities |= MEDIA_HD_DVD_RAM;
    }
  }


  //
  // HD-DVD-R(AM)
  //
  cmd[2] = FEATURE_HD_DVD_WRITE>>8;
  cmd[3] = FEATURE_HD_DVD_WRITE;
  cmd[8] = 16;
  if( !cmd.transport( TR_DIR_READ, header, 16 ) ) {
    unsigned int len = from4Byte( header );
    if( len >= 12 ) {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " feature: " << "HD-DVD Write" << endl;
      if( header[8+4] & 0x1 )
	d->writeCapabilities |= MEDIA_HD_DVD_R;
      if( header[8+6] & 0x1 )
	d->writeCapabilities |= MEDIA_HD_DVD_RAM;
    }
  }



  //
  // Get the profiles
  //
  // the max len of the returned data is 8 (header) + 4 (feature) + 255 (additional length)
  //
  cmd[2] = FEATURE_PROFILE_LIST>>8;
  cmd[3] = FEATURE_PROFILE_LIST;
  cmd[8] = 12; // get the number of returned profiles first
  if( !cmd.transport( TR_DIR_READ, header, 12 ) ) {
    unsigned int len = from4Byte( header ) + 4;
    if( len >= 12 ) {
      cmd[7] = len>>8;
      cmd[8] = len;
      if( !cmd.transport( TR_DIR_READ, header, len ) ) {
	int featureLen( header[11] );
	for( int j = 0; j < featureLen; j+=4 ) {
	  short profile = from2Byte( &header[12+j] );

	  switch (profile) {
	  case 0x08:
	    d->supportedProfiles |= MEDIA_CD_ROM;
	    break;
	  case 0x09:
	    d->supportedProfiles |= MEDIA_CD_R;
	    break;
	  case 0x0A:
	    d->supportedProfiles |= MEDIA_CD_RW;
	    break;
	  case 0x10:
	    d->supportedProfiles |= MEDIA_DVD_ROM;
	    // 	    d->readCapabilities |= MEDIA_DVD_ROM;
	    break;
	  case 0x11:
	    d->supportedProfiles |= MEDIA_DVD_R_SEQ;
	    // 	    d->writeCapabilities |= (MEDIA_DVD_R|MEDIA_DVD_R_SEQ);
	    break;
	  case 0x12:
	    d->supportedProfiles |= MEDIA_DVD_RAM;
//  	    d->readCapabilities |= (MEDIA_DVD_RAM|MEDIA_DVD_ROM);
//  	    d->writeCapabilities |= MEDIA_DVD_RAM;
	    break;
	  case 0x13:
	    d->supportedProfiles |= MEDIA_DVD_RW_OVWR;
	    // 	    d->writeCapabilities |= (MEDIA_DVD_RW|MEDIA_DVD_RW_OVWR);
	    break;
	  case 0x14:
	    d->supportedProfiles |= MEDIA_DVD_RW_SEQ;
	    // 	    d->writeCapabilities |= (MEDIA_DVD_RW|MEDIA_DVD_R|MEDIA_DVD_RW_SEQ|MEDIA_DVD_R_SEQ);
	    break;
	  case 0x15:
	    d->supportedProfiles |= MEDIA_DVD_R_DL_SEQ;
	    // 	    d->writeCapabilities |= (MEDIA_DVD_R|MEDIA_DVD_R_DL|MEDIA_DVD_R_SEQ|MEDIA_DVD_R_DL_SEQ);
	    break;
	  case 0x16:
	    d->supportedProfiles |= MEDIA_DVD_R_DL_JUMP;
	    // 	    d->writeCapabilities |= (MEDIA_DVD_R|MEDIA_DVD_R_DL||MEDIA_DVD_R_DL_JUMP);
	    break;
	  case 0x1A:
	    d->supportedProfiles |= MEDIA_DVD_PLUS_RW;
	    // 	    d->writeCapabilities |= MEDIA_DVD_PLUS_RW;
	    break;
	  case 0x1B:
	    d->supportedProfiles |= MEDIA_DVD_PLUS_R;
	    // 	    d->writeCapabilities |= MEDIA_DVD_PLUS_R;
	    break;
	  case 0x2A:
	    d->supportedProfiles |= MEDIA_DVD_PLUS_RW_DL;
	    // 	    d->writeCapabilities |= MEDIA_DVD_PLUS_RW_DL;
	    break;
	  case 0x2B:
	    d->supportedProfiles |= MEDIA_DVD_PLUS_R_DL;
	    //	    d->writeCapabilities |= MEDIA_DVD_PLUS_R_DL;
	    break;
	  case 0x40:
	    d->supportedProfiles |= MEDIA_BD_ROM;
	    break;
	  case 0x41:
	    d->supportedProfiles |= MEDIA_BD_R_SRM;
	    break;
	  case 0x42:
	    d->supportedProfiles |= MEDIA_BD_R_RRM;
	    break;
	  case 0x43:
	    d->supportedProfiles |= MEDIA_BD_RE;
	    break;
	  case 0x50:
	    d->supportedProfiles |= MEDIA_HD_DVD_ROM;
	    break;
	  case 0x51:
	    d->supportedProfiles |= MEDIA_HD_DVD_R;
	    break;
	  case 0x52:
	    d->supportedProfiles |= MEDIA_HD_DVD_RAM;
	    break;
	  default:
	    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << " unknown profile: "
		      << profile << endl;
	  }
	}

	// some older DVD-ROM drives claim to support DVD+R DL
	if( !(d->supportedProfiles & MEDIA_DVD_PLUS_R) ) {
	  // remove DVD+R DL capability
	  //	  d->writeCapabilities &= ~MEDIA_DVD_PLUS_R_DL;
	  d->supportedProfiles &= ~MEDIA_DVD_PLUS_R_DL;
	}
      }
    }
  }
}


void K3bDevice::Device::checkFor2AFeatures()
{
  unsigned char* mm_cap_buffer = 0;
  unsigned int mm_cap_len = 0;

  if( modeSense( &mm_cap_buffer, mm_cap_len, 0x2A ) ) {
    mm_cap_page_2A* mm_p = (mm_cap_page_2A*)(mm_cap_buffer+8);
    if( mm_p->BUF )
      d->burnfree = true;

    if( mm_p->cd_r_write )
      d->writeCapabilities |= MEDIA_CD_R;
    else
      d->writeCapabilities &= ~MEDIA_CD_R;

    if( mm_p->cd_rw_write )
      d->writeCapabilities |= MEDIA_CD_RW;
    else
      d->writeCapabilities &= ~MEDIA_CD_RW;

    if( mm_p->dvd_r_write )
      d->writeCapabilities |= MEDIA_DVD_R;
    else
      d->writeCapabilities &= ~MEDIA_DVD_R;

    if( mm_p->dvd_rom_read || mm_p->dvd_r_read )
      d->readCapabilities |= MEDIA_DVD_ROM;

    m_maxReadSpeed = from2Byte(mm_p->max_read_speed);
    m_bufferSize = from2Byte( mm_p->buffer_size );

    delete [] mm_cap_buffer;
  }
  else {
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": read mode page 2A failed!" << endl;
  }
}


void K3bDevice::Device::checkWritingModes()
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  if( !open() )
    return;

  // header size is 8
  unsigned char* buffer = 0;
  unsigned int dataLen = 0;

  if( !modeSense( &buffer, dataLen, 0x05 ) ) {
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": modeSense 0x05 failed!" << endl
	      << "(K3bDevice::Device) " << blockDeviceName() << ": Cannot check write modes." << endl;
  }
  else if( dataLen < 18 ) { // 8 bytes header + 10 bytes used modepage
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": Missing modepage 0x05 data." << endl
	      << "(K3bDevice::Device) " << blockDeviceName() << ": Cannot check write modes." << endl;
  }
  else {
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": dataLen: " << dataLen << endl;

    wr_param_page_05* mp = (struct wr_param_page_05*)(buffer+8);

    // reset some stuff to be on the safe side
    mp->PS = 0;
    mp->BUFE = 0;
    mp->multi_session = 0;
    mp->test_write = 0;
    mp->LS_V = 0;
    mp->copy = 0;
    mp->fp = 0;
    mp->host_appl_code= 0;
    mp->session_format = 0;
    mp->audio_pause_len[0] = 0;
    mp->audio_pause_len[1] = 150;

    // WRITINGMODE_TAO
    mp->write_type = 0x01;  // Track-at-once
    mp->track_mode = 4;     // MMC-4 says: 5, cdrecord uses 4 ?
    mp->dbtype = 8;         // Mode 1

    //    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": modeselect WRITINGMODE_TAO data: " << endl;
    //    debugBitfield( buffer, dataLen );


    //
    // if a writer does not support WRITINGMODE_TAO it surely does not support WRITINGMODE_SAO or WRITINGMODE_RAW writing since WRITINGMODE_TAO is the minimal
    // requirement
    //

    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for TAO" << endl;
    if( modeSelect( buffer, dataLen, 1, 0 ) ) {
      m_writeModes |= WRITINGMODE_TAO;
      d->writeCapabilities |= MEDIA_CD_R;

      // WRITINGMODE_SAO
      mp->write_type = 0x02; // Session-at-once

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for SAO" << endl;
      if( modeSelect( buffer, dataLen, 1, 0 ) )
	m_writeModes |= WRITINGMODE_SAO;

//       mp->dbtype = 1;        // Raw data with P and Q Sub-channel (2368 bytes)
//       if( modeSelect( buffer, dataLen, 1, 0 ) ) {
// 	m_writeModes |= WRITINGMODE_RAW_R16;
//       }

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for SAO_R96P" << endl;
      mp->dbtype = 2;        // Raw data with P-W Sub-channel (2448 bytes)
      if( modeSelect( buffer, dataLen, 1, 0 ) ) {
	m_writeModes |= WRITINGMODE_SAO_R96P;
      }

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for SAO_R96R" << endl;
      mp->dbtype = 3;        // Raw data with P-W raw Sub-channel (2448 bytes)
      if( modeSelect( buffer, dataLen, 1, 0 ) ) {
	m_writeModes |= WRITINGMODE_SAO_R96R;
      }

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for RAW_R16" << endl;
      // WRITINGMODE_RAW
      mp->write_type = 0x03; // WRITINGMODE_RAW
      mp->dbtype = 1;        // Raw data with P and Q Sub-channel (2368 bytes)
      if( modeSelect( buffer, dataLen, 1, 0 ) ) {
	m_writeModes |= WRITINGMODE_RAW;
	m_writeModes |= WRITINGMODE_RAW_R16;
      }

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for RAW_R96P" << endl;
      mp->dbtype = 2;        // Raw data with P-W Sub-channel (2448 bytes)
      if( modeSelect( buffer, dataLen, 1, 0 ) ) {
	m_writeModes |= WRITINGMODE_RAW;
	m_writeModes |= WRITINGMODE_RAW_R96P;
      }

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": checking for RAW_R96R" << endl;
      mp->dbtype = 3;        // Raw data with P-W raw Sub-channel (2448 bytes)
      if( modeSelect( buffer, dataLen, 1, 0 ) ) {
	m_writeModes |= WRITINGMODE_RAW;
	m_writeModes |= WRITINGMODE_RAW_R96R;
      }
    }
    else {
      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName() << ": modeSelect with WRITINGMODE_TAO failed. No writer" << endl;
    }


    delete [] buffer;
  }

  if( needToClose )
    close();
}


int K3bDevice::Device::determineMaximalWriteSpeed() const
{
  int ret = 0;
  unsigned char* data = 0;
  unsigned int dataLen = 0;

  if( mediaType() & MEDIA_CD_ALL ) {
    if( modeSense( &data, dataLen, 0x2A ) ) {
      mm_cap_page_2A* mm = (mm_cap_page_2A*)&data[8];

      // MMC1 used byte 18 and 19 for the max write speed
      if( dataLen > 19 )
	ret = from2Byte( mm->max_write_speed );

      delete [] data;

      if( ret > 0 )
	return ret;
    }
  }

  TQValueList<int> list = determineSupportedWriteSpeeds();
  if( !list.isEmpty() ) {
    for( TQValueList<int>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
      ret = TQMAX( ret, *it );
  }

  if( ret > 0 )
    return ret;
  else
    return m_maxWriteSpeed;
}


TQValueList<int> K3bDevice::Device::determineSupportedWriteSpeeds() const
{
  TQValueList<int> ret;

  if( burner() ) {
    //
    // Tests with all my drives resulted in 2A for CD and GET PERFORMANCE for DVD media
    // as the valid method of speed detection.
    //
    if( mediaType() & MEDIA_CD_ALL ) {
      if( !getSupportedWriteSpeedsVia2A( ret, false ) )
	getSupportedWriteSpeedsViaGP( ret, false );

      // restrict to max speed, although deprecated in MMC3 is still used everywhere and
      // cdrecord also uses it as the max writing speed.
      int max = 0;
      unsigned char* data = 0;
      unsigned int dataLen = 0;
      if( modeSense( &data, dataLen, 0x2A ) ) {
	mm_cap_page_2A* mm = (mm_cap_page_2A*)&data[8];

	// MMC1 used byte 18 and 19 for the max write speed
	if( dataLen > 19 )
	  max = from2Byte( mm->max_write_speed );

	delete [] data;

	if( max > 0 ) {
	  while( !ret.isEmpty() && ret.last() > max ) {
	    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		       << " writing speed " << ret.last() << " higher than max " << max << endl;
	    ret.pop_back();
	  }
	}
      }
    }
    else {
      if( !getSupportedWriteSpeedsViaGP( ret, true ) )
	getSupportedWriteSpeedsVia2A( ret, true );
    }
  }

  return ret;
}


bool K3bDevice::Device::getSupportedWriteSpeedsVia2A( TQValueList<int>& list, bool dvd ) const
{
  unsigned char* data = 0;
  unsigned int dataLen = 0;
  if( modeSense( &data, dataLen, 0x2A ) ) {
    mm_cap_page_2A* mm = (mm_cap_page_2A*)&data[8];

    if( dataLen > 32 ) {
      // we have descriptors
      unsigned int numDesc = from2Byte( mm->num_wr_speed_des );

      // Some CDs writer returns the number of bytes that contain
      // the descriptors rather than the number of descriptors
      // Ensure number of descriptors claimed actually fits in the data
      // returned by the mode sense command.
      if( numDesc > ((dataLen - 32 - 8) / 4) )
	numDesc = (dataLen - 32 - 8) / 4;

      cd_wr_speed_performance* wr = (cd_wr_speed_performance*)mm->wr_speed_des;

      k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		<< ":  Number of supported write speeds via 2A: "
		<< numDesc << endl;


      for( unsigned int i = 0; i < numDesc; ++i ) {
	int s = (int)from2Byte( wr[i].wr_speed_supp );
	//
	// some DVD writers report CD writing speeds here
	// If that is the case we cannot rely on the reported speeds
	// and need to use the values gained from GET PERFORMANCE.
	//
	if( dvd && s < 1352 ) {
	  k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		    << " Invalid DVD speed: " << s << " KB/s" << endl;
	  list.clear();
	  break;
	}
	else {
	  k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		    << " : " << s << " KB/s" << endl;

	  if( dvd )
	    s = fixupDvdWritingSpeed( s );

	  // sort the list
	  TQValueList<int>::iterator it = list.begin();
	  while( it != list.end() && *it < s )
	    ++it;
	  list.insert( it, s );
	}
      }
    }

    delete [] data;
  }

  return !list.isEmpty();
}


bool K3bDevice::Device::getSupportedWriteSpeedsViaGP( TQValueList<int>& list, bool dvd ) const
{
  unsigned char* data = 0;
  unsigned int dataLen = 0;
  if( getPerformance( &data, dataLen, 0x3, 0x0 ) ) {
    int numDesc = (dataLen - 8)/16;
    k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
	      << ":  Number of supported write speeds via GET PERFORMANCE: "
	      << numDesc << endl;

    for( int i = 0; i < numDesc; ++i ) {
      int s = from4Byte( &data[20+i*16] );

      // Looks as if the code below does not make sense with most drives
//       if( !( data[4+i*16] & 0x2 ) ) {
// 	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
// 		   << " No write speed: " << s << " KB/s" << endl;
// 	continue;
//       }

      if( dvd && s < 1352 ) {
	//
	// Does this ever happen?
	//
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		  << " Invalid DVD speed: " << s << " KB/s" << endl;
      }
      else {
	k3bDebug() << "(K3bDevice::Device) " << blockDeviceName()
		  << " : " << s << " KB/s" << endl;

	if( dvd )
	  s = fixupDvdWritingSpeed( s );

	TQValueList<int>::iterator it = list.begin();
	while( it != list.end() && *it < s )
	  ++it;
	// the speed might already have been found in the 2a modepage
	if( it == list.end() || *it != s )
	  list.insert( it, s );
      }
    }

    delete [] data;
  }

  return !list.isEmpty();
}


int K3bDevice::Device::getIndex( unsigned long lba ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  if( !open() )
    return -1;

  int ret = -1;

  //
  // first try readCd
  //
  unsigned char readData[16];
  ::memset( readData, 0, 16 );

  //
  // The index is found in the Mode-1 Q which occupies at least 9 out of 10 successive CD frames
  // It can be indentified by ADR == 1
  //
  // So if the current sector does not provide Mode-1 Q subchannel we try the previous.
  //

  if( readCd( readData,
	      16,
	      1, // CD-DA
	      0, // no DAP
	      lba,
	      1,
	      false,
	      false,
	      false,
	      false,
	      false,
	      0,
	      2 // Q-Subchannel
	      ) ) {
    // byte 0: 4 bits CONTROL (MSB) + 4 bits ADR (LSB)
    if( (readData[0]&0x0f) == 0x1 )
      ret = readData[2];

    // search previous sector for Mode1 Q Subchannel
    else if( readCd( readData,
	      16,
	      1, // CD-DA
	      0, // no DAP
	      lba-1,
	      1,
	      false,
	      false,
	      false,
	      false,
	      false,
	      0,
	      2 // Q-Subchannel
	      ) ) {
      if( (readData[0]&0x0f) == 0x1 )
	ret = readData[2];
      else
	ret = -2;
    }
  }

  else {
    k3bDebug() << "(K3bDevice::Device::getIndex) readCd failed. Trying seek." << endl;

    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( seek( lba ) && readSubChannel( &data, dataLen, 1, 0 ) ) {
      // byte 5: 4 bits ADR (MSB) + 4 bits CONTROL (LSB)
      if( dataLen > 7 && (data[5]>>4 & 0x0F) == 0x1 ) {
	ret = data[7];
      }
      else if( seek( lba-1 ) && readSubChannel( &data, dataLen, 1, 0 ) ) {
	if( dataLen > 7 && (data[5]>>4 & 0x0F) == 0x1 )
	  ret = data[7];
	else
	  ret = -2;
      }

      delete [] data;
    }
    else
      k3bDebug() << "(K3bDevice::Device::getIndex) seek or readSubChannel failed." << endl;
  }

  if( needToClose )
    close();

  return ret;
}


bool K3bDevice::Device::searchIndex0( unsigned long startSec,
				      unsigned long endSec,
				      long& pregapStart ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  if( !open() )
    return false;

  bool ret = false;

  int lastIndex = getIndex( endSec );
  if( lastIndex == 0 ) {
    // there is a pregap
    // let's find the position where the index turns to 0
    // we jump in 1 sec steps backwards until we find an index > 0
    unsigned long sector = endSec;
    while( lastIndex == 0 && sector > startSec ) {
      sector -= 75;
      if( sector < startSec )
	sector = startSec;
      lastIndex = getIndex(sector);
    }

    if( lastIndex == 0 ) {
      k3bDebug() << "(K3bDevice::Device) warning: no index != 0 found." << endl;
    }
    else {
      // search forward to the first index = 0
      while( getIndex( sector ) != 0 && sector < endSec )
	sector++;

      pregapStart = sector;
      ret = true;
    }
  }
  else if( lastIndex > 0 ) {
    // no pregap
    pregapStart = -1;
    ret = true;
  }

  if( needToClose )
    close();

  return ret;
}


bool K3bDevice::Device::indexScan( K3bDevice::Toc& toc ) const
{
  // if the device is already opened we do not close it
  // to allow fast multiple method calls in a row
  bool needToClose = !isOpen();

  if( !open() )
    return false;

  bool ret = true;

  for( Toc::iterator it = toc.begin(); it != toc.end(); ++it ) {
    Track& track = *it;
    if( track.type() == Track::AUDIO ) {
      track.m_indices.clear();
      long index0 = -1;
      if( searchIndex0( track.firstSector().lba(), track.lastSector().lba(), index0 ) ) {
	k3bDebug() << "(K3bDevice::Device) found index 0: " << index0 << endl;
      }
      if( index0 > 0 )
	track.m_index0 = K3b::Msf( index0 - track.firstSector().lba() );
      else
	track.m_index0 = 0;

      if( index0 > 0 )
	searchIndexTransitions( track.firstSector().lba(), index0-1, track );
      else
	searchIndexTransitions( track.firstSector().lba(), track.lastSector().lba(), track );
    }
  }

  if( needToClose )
    close();

  return ret;
}


void K3bDevice::Device::searchIndexTransitions( long start, long end, K3bDevice::Track& track ) const
{
    k3bDebug() << "(K3bDevice::Device) searching for index transitions between "
               << start << " and " << end << endl;
    int startIndex = getIndex( start );
    int endIndex = getIndex( end );

    if( startIndex < 0 || endIndex < 0 ) {
        k3bDebug() << "(K3bDevice::Device) could not retrieve index values." << endl;
    }
    else {
        k3bDebug() << "(K3bDevice::Device) indices: " << start << " - " << startIndex
                   << " and " << end << " - " << endIndex << endl;

        if( startIndex != endIndex ) {
            if( start+1 == end ) {
                k3bDebug() << "(K3bDevice::Device) found index transition: " << endIndex << " " << end << endl;
                track.m_indices.resize( endIndex );
                // we save the index relative to the first sector
                track.m_indices[endIndex-1] = K3b::Msf( end ) - track.firstSector();
            }
            else {
                searchIndexTransitions( start, start+(end-start)/2, track );
                searchIndexTransitions( start+(end-start)/2, end, track );
            }
        }
    }
}


int K3bDevice::Device::copyrightProtectionSystemType() const
{
  unsigned char* dvdheader = 0;
  unsigned int dataLen = 0;
  if( readDvdStructure( &dvdheader, dataLen, 0x1 ) ) {
    int ret = -1;
    if( dataLen >= 6 )
      ret = dvdheader[4];
    delete [] dvdheader;
    return ret;
  }
  else
    return -1;
}


bool K3bDevice::Device::getNextWritableAdress( unsigned int& lastSessionStart, unsigned int& nextWritableAdress ) const
{
  bool success = false;

  // FIXME: add CD media handling
  int m = mediaType();
  if( m & MEDIA_DVD_ALL ) {
    // DVD+RW always returns complete
    if( m & (K3bDevice::MEDIA_DVD_PLUS_RW|K3bDevice::MEDIA_DVD_RW_OVWR) )
      return false;

    unsigned char* data = 0;
    unsigned int dataLen = 0;

    if( readDiscInformation( &data, dataLen ) ) {
      disc_info_t* inf = (disc_info_t*)data;

      //
      // The state of the last session has to be "empty" (0x0) or "incomplete" (0x1)
      // The procedure here is taken from the dvd+rw-tools
      //
      if( !(inf->border & 0x2) ) {
	// the incomplete track number is the first track in the last session (the empty session)
	int nextTrack = inf->first_track_l|inf->first_track_m<<8;

	unsigned char* trackData = 0;
	unsigned int trackDataLen = 0;

	// Read start address of the incomplete track
	if( readTrackInformation( &trackData, trackDataLen, 0x1, nextTrack ) ) {
	  nextWritableAdress = from4Byte( &trackData[8] );
	  delete [] trackData;

	  // Read start address of the first track in the last session
	  if( readTocPmaAtip( &trackData, trackDataLen, 0x1, false, 0x0  ) ) {
	    lastSessionStart = from4Byte( &trackData[8] );
	    delete [] trackData;
	    success = true;
	  }
	}
      }
    }

    delete [] data;
  }

  return success;
}


int K3bDevice::Device::nextWritableAddress() const
{
    unsigned char* data = 0;
    unsigned int dataLen = 0;
    int nwa = -1;

    if( readDiscInformation( &data, dataLen ) ) {
        disc_info_t* inf = (disc_info_t*)data;

        //
        // The state of the last session has to be "empty" (0x0) or "incomplete" (0x1)
        // The procedure here is taken from the dvd+rw-tools and wodim
        //
        if( !(inf->border & 0x2) ) {
            // the incomplete track number is the first track in the last session (the empty session)
            int nextTrack = inf->first_track_l|inf->first_track_m<<8;

            unsigned char* trackData = 0;
            unsigned int trackDataLen = 0;

            // Read start address of the incomplete track
            if( readTrackInformation( &trackData, trackDataLen, 0x1, nextTrack ) ) {
                nwa = from4Byte( &trackData[8] );
                delete [] trackData;
            }

            // Read start address of the invisible track
            else if ( readTrackInformation( &trackData, trackDataLen, 0x1, 0xff ) ) {
                nwa = from4Byte( &trackData[8] );
                delete [] trackData;
            }
        }

        delete [] data;
    }

    return nwa;
}


TQCString K3bDevice::Device::mediaId( int mediaType ) const
{
  TQCString id;

  if( mediaType & MEDIA_CD_ALL ) {
    // FIXME:
  }

  else if( mediaType & MEDIA_DVD_MINUS_ALL ) {
    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( readDvdStructure( &data, dataLen, 0x0E ) ) {
      if( data[4+16] == 3 && data[4+24] == 4 ) {
	id.sprintf( "%6.6s%-6.6s", data+4+17, data+4+25 );
      }
      delete [] data;
    }
  }

  else if( mediaType & MEDIA_DVD_PLUS_ALL ) {
    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( readDvdStructure( &data, dataLen, 0x11 ) ||
	readDvdStructure( &data, dataLen, 0x0 ) ) {
      id.sprintf( "%8.8s/%3.3s", data+23, data+31 );
      delete [] data;
    }
  }

  else if( mediaType & MEDIA_BD_ALL ) {
    unsigned char* data = 0;
    unsigned int dataLen = 0;
    if( readDiscStructure( &data, dataLen, 1, 0 ) ) {
      if( data[4+0] == 'D' && data[4+1] == 'I' )
	id.sprintf ("%6.6s/%-3.3s", data+4+100, data+4+106 );
      delete [] data;
    }
  }

  return id;
}


// int K3bDevice::Device::ioctl( int request, ... ) const
// {
//     int r = -1;
// #if defined(Q_OS_LINUX) || defined(Q_OS_NETBSD)
//     d->mutex.lock();

//     va_list ap;
//     va_start( ap, request );
//     r = ::ioctl( d->deviceFd, request, ap );
//     va_end( ap );

//     d->mutex.unlock();
// #endif
//     return r;
// }


void K3bDevice::Device::usageLock() const
{
    d->mutex.lock();
}


void K3bDevice::Device::usageUnlock() const
{
    d->mutex.unlock();
}
