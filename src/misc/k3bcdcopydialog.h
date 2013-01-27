/* 
 *
 * $Id: k3bcdcopydialog.h 733470 2007-11-06 12:10:29Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *                    Klaus-Dieter Krannich <kd@k3b.org>
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


#ifndef K3BCDCOPYDIALOG_H
#define K3BCDCOPYDIALOG_H


#include <k3binteractiondialog.h>

#include <tdeio/global.h>

namespace K3bDevice {
  class Device;
  class DeviceManager;
}

class K3bWriterSelectionWidget;
class K3bTempDirSelectionWidget;
class K3bMediaSelectionComboBox;
class TQCheckBox;
class TQSpinBox;
class TQComboBox;
class K3bWritingModeWidget;
class TQButtonGroup;
class TQGroupBox;


/**
  *@author Sebastian Trueg
  */
class K3bCdCopyDialog : public K3bInteractionDialog
{
  Q_OBJECT
  

 public: 
  K3bCdCopyDialog(TQWidget *parent = 0, const char *name = 0, bool modal = true );
  ~K3bCdCopyDialog();

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
  TQCheckBox* m_checkSimulate;
  TQCheckBox* m_checkCacheImage;
  TQCheckBox* m_checkDeleteImages;
  TQCheckBox* m_checkOnlyCreateImage;
  TQCheckBox* m_checkReadCdText;
  TQCheckBox* m_checkPrefereCdText;
  TQCheckBox* m_checkIgnoreDataReadErrors;
  TQCheckBox* m_checkIgnoreAudioReadErrors;
  TQCheckBox* m_checkNoCorrection;
  K3bMediaSelectionComboBox* m_comboSourceDevice;
  TQComboBox* m_comboParanoiaMode;
  TQSpinBox* m_spinCopies;
  TQSpinBox* m_spinDataRetries;
  TQSpinBox* m_spinAudioRetries;
  K3bWritingModeWidget* m_writingModeWidget;
  TQComboBox* m_comboCopyMode;

  TQGroupBox* m_groupAdvancedDataOptions;
  TQGroupBox* m_groupAdvancedAudioOptions;
};

#endif
