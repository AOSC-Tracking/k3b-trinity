/*
 *
 * $Id: k3bdevicewidget.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDEVICEWIDGET_H
#define K3BDEVICEWIDGET_H

#include <tqwidget.h>
#include <tqptrlist.h>
#include "k3bdevice.h"
#include "k3bdevicemanager.h"

class TQComboBox;
class TQLabel;
class TQGroupBox;
class TQPushButton;
class TQCheckBox;
class K3bListView;
class TQString;
class KIntNumInput;
class TQFrame;
class TQListViewItem;
class TQString;
class TQLineEdit;


/**
  *@author Sebastian Trueg
  */
class K3bDeviceWidget : public TQWidget
{
  Q_OBJECT
  

 public:
  K3bDeviceWidget( K3bDevice::DeviceManager*, TQWidget *parent = 0, const char *name = 0 );
  ~K3bDeviceWidget();

 public slots:
  void init();
  void apply();

 signals:
  void refreshButtonClicked();

 private slots:
  void slotNewDevice();
  void updateDeviceListViews();

 private:
  class PrivateTempDevice;
  class PrivateDeviceViewItem1;

  /** list to save changes to the devices before applying */
  TQPtrList<PrivateTempDevice> m_tempDevices;

  TQListViewItem* m_writerParentViewItem;
  TQListViewItem* m_readerParentViewItem;

  K3bDevice::DeviceManager* m_deviceManager;

  K3bListView*    m_viewDevices;
  TQPushButton* m_buttonRefreshDevices;
  TQPushButton* m_buttonAddDevice;
};

#endif
