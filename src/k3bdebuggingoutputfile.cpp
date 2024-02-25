/* 
 *
 * $Id: k3bdebuggingoutputfile.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bdebuggingoutputfile.h"

#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bcore.h>
#include <k3bversion.h>
#include <k3bdeviceglobals.h>
#include <k3bglobals.h>

#include <kstandarddirs.h>
#include <tdeglobalsettings.h>
#include <tdeapplication.h>

#include <tqtextstream.h>


K3bDebuggingOutputFile::K3bDebuggingOutputFile()
  : TQFile( locateLocal( "appdata", "lastlog.log", true ) )
{
}


bool K3bDebuggingOutputFile::open()
{
  if( !TQFile::open( IO_WriteOnly ) )
    return false;

  addOutput( "System", "K3b Version: " + k3bcore->version() );
  addOutput( "System", "KDE Version: " + TQString(KDE::versionString()) );
  addOutput( "System", "TQt Version:  " + TQString(tqVersion()) );
  addOutput( "System", "Kernel:      " + K3b::kernelVersion() );
  
  // devices in the logfile
  for( TQPtrListIterator<K3bDevice::Device> it( k3bcore->deviceManager()->allDevices() ); *it; ++it ) {
    K3bDevice::Device* dev = *it;
    addOutput( "Devices", 
	       TQString( "%1 (%2, %3) [%5] [%6] [%7]" )
	       .arg( dev->vendor() + " " + dev->description() + " " + dev->version() )
	       .arg( dev->blockDeviceName() )
	       .arg( dev->genericDevice() )
	       .arg( K3bDevice::deviceTypeString( dev->type() ) )
	       .arg( K3bDevice::mediaTypeString( dev->supportedProfiles() ) )
	       .arg( K3bDevice::writingModeString( dev->writingModes() ) ) );
  }

  return true;
}


void K3bDebuggingOutputFile::addOutput( const TQString& app, const TQString& msg )
{
  if( !isOpen() )
    open();

  TQTextStream s( this );
  s << "[" << app << "] " << msg << endl;
  flush();
}

#include "k3bdebuggingoutputfile.moc"
