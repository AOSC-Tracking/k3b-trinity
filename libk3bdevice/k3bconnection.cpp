/*
 *
 * $Id: sourceheader,v 1.3 2005/01/19 13:03:46 trueg Exp $
 * Copyright (C) 2005-2007 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C) 2013 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C) 2013 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bconnection.h"
#include "k3bdevice.h"

#include <k3bdebug.h>
#include <tdelocale.h>

#include <tqtimer.h>
#include <tqvariant.h>

#ifdef HAVE_TDEHWLIB
#include <tdehardwaredevices.h>
#else
#define TDEHardwareDevices void
#endif

K3bDevice::Connection* K3bDevice::Connection::s_instance = 0;


class K3bDevice::Connection::Private
{
	public:
		Private()
		: bOpen(false),
		  m_hwdevices(NULL) {
			//
		}
		
		bool bOpen;
		TDEHardwareDevices *m_hwdevices;

		TQMap<TQString, TQString> udiDeviceMap;
		TQMap<TQString, TQString> deviceUdiMap;

		TQMap<TQString, bool> deviceMediumUdiMap;
};


K3bDevice::Connection* K3bDevice::Connection::instance()
{
	if (s_instance == 0) {
		s_instance = new Connection(0);
	}

	if ((!s_instance->isConnected()) && (!s_instance->open())) {
		k3bDebug() << "(K3bDevice::Connection) failed to initialize the TDE hardware backend." << endl;
	}

	return s_instance;
}


K3bDevice::Connection::Connection( TQObject* parent, const char* name )
  : TQObject( parent, name )
{
	d = new Private();
}


K3bDevice::Connection::~Connection()
{
	s_instance = 0;
	close();
	delete d;
}


bool K3bDevice::Connection::isConnected() const
{
	return d->bOpen;
}


bool K3bDevice::Connection::open()
{
#ifdef HAVE_TDEHWLIB
	// Initialize the TDE device manager
	d->m_hwdevices = TDEGlobal::hardwareDevices();

	// Connect device monitoring signals/slots
	connect(d->m_hwdevices, TQT_SIGNAL(hardwareAdded(TDEGenericDevice*)), this, TQT_SLOT(AddDeviceHandler(TDEGenericDevice*)));
	connect(d->m_hwdevices, TQT_SIGNAL(hardwareRemoved(TDEGenericDevice*)), this, TQT_SLOT(RemoveDeviceHandler(TDEGenericDevice*)));
	connect(d->m_hwdevices, TQT_SIGNAL(hardwareUpdated(TDEGenericDevice*)), this, TQT_SLOT(ModifyDeviceHandler(TDEGenericDevice*)));

	d->bOpen = true;
	
	//
	// Report all devices
	//
	TDEGenericHardwareList hwlist = d->m_hwdevices->listAllPhysicalDevices();
	TDEGenericDevice *hwdevice;
	for (hwdevice = hwlist.first(); hwdevice; hwdevice = hwlist.next()) {
		AddDeviceHandler(hwdevice);
	}
	
	return true;
#else
	return false;
#endif
}


void K3bDevice::Connection::close()
{
	d->bOpen = false;
}


TQStringList K3bDevice::Connection::devices() const
{
	return TQStringList(d->udiDeviceMap.values());
}

void K3bDevice::Connection::AddDeviceHandler(TDEGenericDevice* hwdevice)
{
#ifdef HAVE_TDEHWLIB
	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return;
	}
	TQString udi = hwdevice->uniqueID();
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	if (sdevice->diskType() & TDEDiskDeviceType::Optical) {
		TQString blockDevice = sdevice->deviceNode();
		if (!blockDevice.isEmpty()) {
			k3bDebug() << "Mapping udi " << udi << " to device " << blockDevice << endl;
			d->udiDeviceMap[udi] = blockDevice;
			d->deviceUdiMap[blockDevice] = udi;
			emit deviceAdded(blockDevice);
			// Check for medium
			if (sdevice->mediaInserted()) {
				d->deviceMediumUdiMap[blockDevice] = true;
				emit mediumChanged(blockDevice);
			}
		}
	}
#endif
}

void K3bDevice::Connection::RemoveDeviceHandler(TDEGenericDevice* hwdevice)
{
#ifdef HAVE_TDEHWLIB
	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return;
	}
	TQString udi = hwdevice->uniqueID();
	TQString blockDevice = hwdevice->deviceNode();

	TQMapIterator<TQString, TQString> it = d->udiDeviceMap.find(udi);
	if (it != d->udiDeviceMap.end()) {
		k3bDebug() << "Unmapping udi " << udi << " from device " << it.data() << endl;
		emit deviceRemoved(it.data());
		d->udiDeviceMap.erase(it);
		d->deviceUdiMap.erase(it.data());

		if (d->deviceMediumUdiMap[blockDevice]) {
			d->deviceMediumUdiMap[blockDevice] = false;
			emit mediumChanged(blockDevice);
		}
	}
#endif
}

void K3bDevice::Connection::ModifyDeviceHandler(TDEGenericDevice* hwdevice)
{
#ifdef HAVE_TDEHWLIB
	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return;
	}
	TQString udi = hwdevice->uniqueID();
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);
	TQString blockDevice = hwdevice->deviceNode();

	if (d->deviceMediumUdiMap[blockDevice] != sdevice->mediaInserted()) {
		d->deviceMediumUdiMap[blockDevice] = sdevice->mediaInserted();
		emit mediumChanged(blockDevice);
	}
#endif
}

int K3bDevice::Connection::lock(Device* dev)
{
#ifdef HAVE_TDEHWLIB
	if (!d->deviceUdiMap.contains(dev->blockDeviceName())) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TQString udi = d->deviceUdiMap[dev->blockDeviceName()];
	TDEGenericDevice *hwdevice = d->m_hwdevices->findByUniqueID(udi);
	if (!hwdevice) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}

	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	if (sdevice->lockDriveMedia(true)) {
		return ErrorCodes::Success;
	}
	else {
		return ErrorCodes::Device_Volume_InvalidEjectOption;
	}
#else
		return ErrorCodes::Device_Volume_NoSuchDevice;
#endif
}


int K3bDevice::Connection::unlock(Device* dev)
{
#ifdef HAVE_TDEHWLIB
	if (!d->deviceUdiMap.contains(dev->blockDeviceName())) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TQString udi = d->deviceUdiMap[dev->blockDeviceName()];
	TDEGenericDevice *hwdevice = d->m_hwdevices->findByUniqueID(udi);
	if (!hwdevice) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}

	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	if (sdevice->lockDriveMedia(false)) {
		return ErrorCodes::Success;
	}
	else {
		return ErrorCodes::Device_Volume_InvalidEjectOption;
	}
#else
		return ErrorCodes::Device_Volume_NoSuchDevice;
#endif
}


int K3bDevice::Connection::mount( K3bDevice::Device* dev,
				     const TQString& mountPoint,
				     const TQString& fstype,
				     const TQStringList& options )
{
#ifdef HAVE_TDEHWLIB
	if (!d->deviceUdiMap.contains(dev->blockDeviceName())) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TQString udi = d->deviceUdiMap[dev->blockDeviceName()];
	TDEGenericDevice *hwdevice = d->m_hwdevices->findByUniqueID(udi);
	if (!hwdevice) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}

	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	// FIXME
	// Options from 'options' are not currently loaded into 'mountOptions'
	TDEStorageMountOptions mountOptions;
	TQStringVariantMap mountResult = sdevice->mountDevice(mountPoint, mountOptions);
	TQString mountedPath = mountResult.contains("mountPath") ? mountResult["mountPath"].toString() : TQString::null;
	if (mountedPath.isEmpty()) {
		return ErrorCodes::CommunicationError;
	}
	else {
		return ErrorCodes::Success;
	}
#else
		return ErrorCodes::Device_Volume_NoSuchDevice;
#endif
}


int K3bDevice::Connection::unmount(K3bDevice::Device* dev, const TQStringList& options)
{
#ifdef HAVE_TDEHWLIB
	if (!d->deviceUdiMap.contains(dev->blockDeviceName())) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TQString udi = d->deviceUdiMap[dev->blockDeviceName()];
	TDEGenericDevice *hwdevice = d->m_hwdevices->findByUniqueID(udi);
	if (!hwdevice) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}

	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	// FIXME
	// Options from 'options' are not currently loaded into 'mountOptions'
	TQString mountOptions;

	TQStringVariantMap unmountResult = sdevice->unmountDevice();
	if (unmountResult["result"].toBool() == false) {
		// Unmount failed!
		return ErrorCodes::CommunicationError;
	}
	else {
		return ErrorCodes::Success;
	}
#else
		return ErrorCodes::Device_Volume_NoSuchDevice;
#endif
}


int K3bDevice::Connection::eject(K3bDevice::Device* dev, const TQStringList& options)
{
#ifdef HAVE_TDEHWLIB
	if (!d->deviceUdiMap.contains(dev->blockDeviceName())) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TQString udi = d->deviceUdiMap[dev->blockDeviceName()];
	TDEGenericDevice *hwdevice = d->m_hwdevices->findByUniqueID(udi);
	if (!hwdevice) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}

	if (hwdevice->type() != TDEGenericDeviceType::Disk) {
		return ErrorCodes::Device_Volume_NoSuchDevice;
	}
	TDEStorageDevice* sdevice = static_cast<TDEStorageDevice*>(hwdevice);

	if (sdevice->ejectDriveMedia()) {
		return ErrorCodes::Success;
	}
	else {
		return ErrorCodes::Device_Volume_InvalidEjectOption;
	}
#else
		return ErrorCodes::Device_Volume_NoSuchDevice;
#endif
}

#include "k3bconnection.moc"
