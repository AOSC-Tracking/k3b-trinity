/*
 *
 * $Id: k3bdeviceselectiondialog.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef K3B_DEVICE_SELECTION_DIALOG_H
#define K3B_DEVICE_SELECTION_DIALOG_H


#include <kdialogbase.h>
#include "k3b_export.h"
#include <tqptrlist.h>

namespace K3bDevice {
  class Device;
}


class LIBK3B_EXPORT K3bDeviceSelectionDialog : public KDialogBase
{
  TQ_OBJECT
  

 public:
  K3bDeviceSelectionDialog( TQWidget* parent = 0, 
			    const char* name = 0, 
			    const TQString& text = TQString(), 
			    bool modal = false );
  ~K3bDeviceSelectionDialog();

  void addDevice( K3bDevice::Device* );
  void addDevices( const TQPtrList<K3bDevice::Device>& );

  void setSelectedDevice( K3bDevice::Device* );

  K3bDevice::Device* selectedDevice() const;

  static K3bDevice::Device* selectWriter( TQWidget* parent, 
					  const TQString& text = TQString() );
  static K3bDevice::Device* selectDevice( TQWidget* parent, 
					  const TQString& text = TQString() );
  static K3bDevice::Device* selectDevice( TQWidget* parent, 
					  const TQPtrList<K3bDevice::Device>& devices,
					  const TQString& text = TQString() );

 private:
  class Private;
  Private* d;
};

#endif
