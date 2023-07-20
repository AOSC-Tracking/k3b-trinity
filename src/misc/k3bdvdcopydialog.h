/* 
 *
 * $Id: k3bdvdcopydialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_DVD_COPY_DIALOG_H_
#define _K3B_DVD_COPY_DIALOG_H_

#include <k3binteractiondialog.h>

#include <tdeio/global.h>


namespace K3bDevice {
  class Device;
  class DeviceManager;
}

class K3bTempDirSelectionWidget;
class K3bWriterSelectionWidget;
class K3bMediaSelectionComboBox;
class TQCheckBox;
class TQSpinBox;
class K3bWritingModeWidget;


class K3bDvdCopyDialog : public K3bInteractionDialog
{
  TQ_OBJECT
  

 public:
  K3bDvdCopyDialog( TQWidget* parent = 0, const char* name = 0, bool modal = true );
  ~K3bDvdCopyDialog();

  void setReadingDevice( K3bDevice::Device* );
  K3bDevice::Device* readingDevice() const;

 private slots:
  void slotStartClicked();
  void slotSourceMediumChanged( K3bDevice::Device* );
  void updateOverrideDevice();

 protected:
  void init();
  void toggleAll();

 private:
  void loadUserDefaults( TDEConfigBase* );
  void saveUserDefaults( TDEConfigBase* );
  void loadK3bDefaults();

  TDEIO::filesize_t neededSize() const;

  K3bWriterSelectionWidget* m_writerSelectionWidget;
  K3bTempDirSelectionWidget* m_tempDirSelectionWidget;
  K3bMediaSelectionComboBox* m_comboSourceDevice;
  TQCheckBox* m_checkSimulate;
  TQCheckBox* m_checkDeleteImages;
  TQCheckBox* m_checkOnlyCreateImage;
  TQCheckBox* m_checkCacheImage;
  TQCheckBox* m_checkVerifyData;
  TQSpinBox* m_spinCopies;
  TQSpinBox* m_spinRetries;
  TQCheckBox* m_checkIgnoreReadErrors;
  K3bWritingModeWidget* m_writingModeWidget;
};

#endif
