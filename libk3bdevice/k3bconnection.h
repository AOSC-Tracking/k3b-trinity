/* 
 *
 * $Id: sourceheader,v 1.3 2005/01/19 13:03:46 trueg Exp $
 * Copyright (C) 2005-2007 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_CONNECTION_H_
#define _K3B_CONNECTION_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "k3bdevice_export.h"

#include <tqobject.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <kdemacros.h>

#ifdef HAVE_TDEHWLIB
class TDEGenericDevice;
#else
#define TDEGenericDevice void
#endif

namespace K3bDevice {

  class Device;

  /**
   * This is a simple wrapper which creates QT signals whenever a new optical
   * drive is plugged into the system or one is unplugged.
   *
   * The Connection class also handles media changes. Whenever a new medium is inserted
   * into a drive or a medium is removed (i.e. ejected) a signal is emitted. This way it
   * is easy to keep track of the inserted media.
   *
   * This class does not deal with K3b devices but with system device names
   * such as /dev/cdrom. These device names can be used in DeviceManager::findDevice().
   */
  class LIBK3BDEVICE_EXPORT Connection : public TQObject
    {
      TQ_OBJECT
  

    public:
      ~Connection();

      /**
       * Creates a new singleton Connection object or returns the already existing one.
       * A newly created Connection will emit newDevice signals for all devices in the 
       * manager. However, since one cannot be sure if this is the first time the Connection
       * is created it is recommended to connect to the signals and query the list of current
       * devices.
       *
       * \return An instance of the singleton Connection object.
       */
      static Connection* instance();

      /**
       * \return true if a connection to the deamon could be established and
       *         communication has been set up.
       */
      bool isConnected() const;

      /**
       * \return a list of optical devices as reported.
       */
      TQStringList devices() const;

      /**
       * Error codes named as the deamon raises them
       */
      enum ErrorCodes {
	Success = 0, //*< The operation was successful.
	CommunicationError,
	NoSuchDevice,
	DeviceAlreadyLocked,
	PermissionDenied,
	Device_Volume_NoSuchDevice,
	Device_Volume_PermissionDenied,
	Device_Volume_AlreadyMounted,
	Device_Volume_InvalidMountOption,
	Device_Volume_UnknownFilesystemType,
	Device_Volume_InvalidMountpoint,
	Device_Volume_MountPointNotAvailable,
	Device_Volume_PermissionDeniedByPolicy,
	Device_Volume_InvalidUnmountOption,
	Device_Volume_InvalidEjectOption
      };

     public slots:
      /**
       * Lock the device
       * 
       * Be aware that once the method returns the deamon has not necessarily 
       * finished the procedure yet.
       *
       * \param dev The device to lock
       * \return An error code
       *
       * \see ErrorCode
       */
      int lock( Device* );

      /**
       * Unlock a previously locked device
       * 
       * Be aware that once the method returns the deamon has not necessarily 
       * finished the procedure yet.
       *
       * \param dev The device to lock
       * \return An error code
       *
       * \see ErrorCode
       */
      int unlock( Device* );

      /**
       * Mounts a device
       * 
       * Be aware that once the method returns the deamon has not necessarily 
       * finished the procedure yet.
       *
       * \param dev The device to lock
       * \return An error code
       *
       * \see ErrorCode
       */
      int mount( Device*, 
		 const TQString& mountPoint = TQString(), 
		 const TQString& fstype = TQString(),
		 const TQStringList& options = TQStringList() );

      /**
       * Unmounts a device
       * 
       * Be aware that once the method returns the deamon has not necessarily 
       * finished the procedure yet.
       *
       * \param dev The device to lock
       * \return An error code
       *
       * \see ErrorCode
       */
      int unmount( Device*,
       const TQStringList& options = TQStringList() );

      /**
       * Unmounts a device
       * 
       * Be aware that once the method returns the deamon has not necessarily 
       * finished the procedure yet.
       *
       * \param dev The device to lock
       * \return An error code
       *
       * \see ErrorCode
       */
      int eject( Device*,
     const TQStringList& options = TQStringList() );

    private slots:
      /**
       * \internal
       */
      void AddDeviceHandler(TDEGenericDevice*);

      /**
       * \internal
       */
      void RemoveDeviceHandler(TDEGenericDevice*);

      /**
       * \internal
       */
      void ModifyDeviceHandler(TDEGenericDevice*);

    signals:
      /**
       * This signal gets emitted whenever it finds a new optical drive.
       *
       * \param dev The block device name of the new drive.
       */
      void deviceAdded( const TQString& dev );

      /**
       * This signal gets emitted whenever it detects that an optical drive
       * has been unplugged.
       *
       * \param dev The block device name of the drive.
       */
      void deviceRemoved( const TQString& dev );

      /**
       * This signal gets emitted whenever a new medium is inserted into a
       * device or an inserted is removed (i.e. ejected)
       *
       * \param dev The block device name of the drive the medium is or was inserted into.
       */
      void mediumChanged( const TQString& dev );

    private:
      /**
       * Connection is a signelton class. Use the instance() method to create it.
       */
      Connection( TQObject* parent = 0, const char* name = 0 );

      /**
       * Tries to open a connection
       */
      bool open();
      void close();

      static Connection* s_instance;

      class Private;
      Private* d;
    };
}

#endif
