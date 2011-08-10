/*
 *
 * $Id: k3bdevicemanager.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDEVICEMANAGER_H
#define K3BDEVICEMANAGER_H

#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqmemarray.h>
#include <tqptrlist.h>

#include "k3bdevice_export.h"
#include <kdebug.h>

class KProcess;
class KConfig;
class K3bExternalBin;


namespace K3bDevice {

  class Device;

  /**
   * \brief Manages all devices.
   *
   * Searches the system for devices and maintains lists of them.
   *
   * <b>Basic usage:</b>
   * \code
   *   K3bDevice::DeviceManager* manager = new K3bDevice::DeviceManager( this );
   *   manager->scanBus();
   *   K3bDevice::Device* dev = manager->findDevice( "/dev/cdrom" );
   * \endcode
   */
  class LIBK3BDEVICE_EXPORT DeviceManager : public TQObject
    {
      Q_OBJECT
  TQ_OBJECT

    public:
      /**
       * Creates a new DeviceManager
       */
      DeviceManager( TQObject* parent = 0, const char* name = 0 );
      virtual ~DeviceManager();

      /**
       * By default the DeviceManager makes the Devices check their writing modes.
       * This includes commands to be sent which require writing permissions on the
       * devices and might take some time.
       *
       * So if you don't need the information about the writing modes use this method
       * to speed up the device detection procedure.
       *
       * Be aware that this only refers to CD writing modes. If you only want to handle
       * DVD devices it's always save to set this to false.
       */
      void setCheckWritingModes( bool b );

      /**
       * \deprecated use findDevice( const TQString& )
       */
      Device* deviceByName( const TQString& );

      /**
       * Search an SCSI device by SCSI bus, id, and lun.
       *
       * \note This method does not initialize new devices.
       *       Devices cannot be found until they have been added via addDevice(const TQString&)
       *       or scanBus().
       *
       * \return The corresponding device or 0 if there is no such device.
       */
      Device* findDevice( int bus, int id, int lun );

      /**
       * Search a device by blockdevice name.
       *
       * \note This method does not initialize new devices.
       *       Devices cannot be found until they have been added via addDevice(const TQString&)
       *       or scanBus().
       *
       * \return The corresponding device or 0 if there is no such device.
       */
      Device* findDevice( const TQString& devicename );

      /**
       * Before getting the devices do a @ref scanBus().
       * \return List of all cd writer devices.
       * \deprecated use cdWriter()
       */
      const TQPtrList<Device>& burningDevices() const;

      /**
       * \return List of all reader devices without writer devices.
       * \deprecated use cdReader()
       **/
      const TQPtrList<Device>& readingDevices() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all devices.
       */
      const TQPtrList<Device>& allDevices() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all cd writer devices.
       */
      const TQPtrList<Device>& cdWriter() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all cd reader devices.
       */
      const TQPtrList<Device>& cdReader() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all DVD writer devices.
       */
      const TQPtrList<Device>& dvdWriter() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all DVD reader devices.
       */
      const TQPtrList<Device>& dvdReader() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all Blue Ray reader devices.
       */
      const TQPtrList<Device>& blueRayReader() const;

      /**
       * Before getting the devices do a @ref scanBus() or add 
       * devices via addDevice( const TQString& ).
       *
       * \return List of all Blue Ray writer devices.
       */
      const TQPtrList<Device>& blueRayWriters() const;

      /**
       * Reads the device information from the config file.
       */
      virtual bool readConfig( KConfig* );

      virtual bool saveConfig( KConfig* );


    public slots:
      /**
       * Writes a list of all devices to stderr.
       */
      void printDevices();

      /**
       * Scan the system for devices. Call this to initialize all devices.
       * 
       * If the system uses the HAL device deamon it is possible to use
       * HalConnection instead of calling this method.
       *
       * \return Number of found devices.
       **/
      virtual int scanBus();

      /**
       * Clears the writers and readers list of devices.
       */
      virtual void clear();

      /**
       * Add a new device.
       *
       * \param dev Name of a block device or link to a block device. If the 
       *            corresponding device has already been detected it will simply
       *            be returned. Otherwise if a device is found it will be initialized
       *            and added to the internal lists (meaning it can be accessed through
       *            emthods like cdReader()).
       *
       * Called by scanBus()
       *
       * \return The device if it could be found or 0 otherwise.
       */
      virtual Device* addDevice( const TQString& dev );

      /**
       * Remove a device from the device manager. Basicly this method
       * only makes sense in combination with the HalConnection. Connect
       * it to the deviceRemoved signal.
       */
      virtual void removeDevice( const TQString& dev );

    signals:
      /**
       * Emitted if the device configuration changed, i.e. a device was added or removed.
       */
      void changed( K3bDevice::DeviceManager* );
      void changed();

    private:
      bool testForCdrom( const TQString& );
      bool determineBusIdLun( const TQString &dev, int& bus, int& id, int& lun );
      TQString resolveSymLink( const TQString& path );

      class Private;
      Private* d;

      /**
       * Add a device to the managers device lists and initialize the device.
       */
      Device *addDevice( Device* );

      void BSDDeviceScan();
      void NetBSDDeviceScan();
      void LinuxDeviceScan();
    };
}

#endif
