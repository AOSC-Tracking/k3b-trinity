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

#ifndef _K3B_APP_DEVICE_MANAGER_H_
#define _K3B_APP_DEVICE_MANAGER_H_

class TDEActionCollection;
class TDEAction;
class K3bMediaCache;

namespace K3bDevice {
  class Device;
  class DiskInfo;
  class DiskInfoDetector;
}

namespace TDEIO {
  class Job;
}

#include <k3bdevicemanager.h>


/**
 * Enhanced device manager which can do some additional actions
 * and maintains a current device
 */
class K3bAppDeviceManager : public K3bDevice::DeviceManager
{
  TQ_OBJECT
  

 public:
  K3bAppDeviceManager( TQObject* parent = 0, const char* name = 0 );
  ~K3bAppDeviceManager();

  K3bDevice::Device* currentDevice() const;
  TDEActionCollection* actionCollection() const { return m_actionCollection; }
  void setMediaCache( K3bMediaCache* c );

 signals:
  void currentDeviceChanged( K3bDevice::Device* );

  /**
   * Emitted when starting to detect the diskinfo. This may be used to show some info
   * to the user since deteting the diskinfo might take some time.
   */
  void detectingDiskInfo( K3bDevice::Device* );
  void diskInfoReady( K3bDevice::DiskInfoDetector* );

  void mountFinished( const TQString& mountPoint );
  void unmountFinished( bool success );

 public slots:
  /**
   * \reimplemeted for internal reasons. The API is unaffected.
   */
  void clear();

  /**
   * \reimplemeted for internal reasons. The API is unaffected.
   */
  void removeDevice( const TQString& );

  /**
   * \reimplemeted for internal reasons. The API is unaffected.
   */
  int scanBus();

  void setCurrentDevice( K3bDevice::Device* );

  void diskInfo();
  void unlockDevice();
  void lockDevice();
  void mountDisk();
  void unmountDisk();
  void ejectDisk();
  void loadDisk();
  void setReadSpeed();

  void diskInfo( K3bDevice::Device* );
  void unlockDevice( K3bDevice::Device* );
  void lockDevice( K3bDevice::Device* );
  void mountDisk( K3bDevice::Device* );
  void unmountDisk( K3bDevice::Device* );
  void ejectDisk( K3bDevice::Device* );
  void loadDisk( K3bDevice::Device* );
  void setReadSpeed( K3bDevice::Device* );

 private slots:
  void slotMediumChanged( K3bDevice::Device* dev );

 private:
  TDEAction* m_actionDiskInfo;
  TDEAction* m_actionUnmount;
  TDEAction* m_actionMount;
  TDEAction* m_actionEject;
  TDEAction* m_actionLoad;
  TDEAction* m_actionSetReadSpeed;

  mutable K3bDevice::Device* m_currentDevice;
  TDEActionCollection* m_actionCollection;
  K3bDevice::DiskInfoDetector* m_diskInfoDetector;

  bool m_ejectRequested;
};

#endif
