/* 
 *
 * $Id: k3bdataburndialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDATABURNDIALOG_H
#define K3BDATABURNDIALOG_H

#include "k3bprojectburndialog.h"

class TQCheckBox;
class KComboBox;
class TQGroupBox;
class TQLabel;
class TQToolButton;
class TQRadioButton;
class TQButtonGroup;
class K3bWriterSelectionWidget;
class K3bTempDirSelectionWidget;
class K3bDataDoc;
class KLineEdit;
class K3bDataImageSettingsWidget;
class K3bDataModeWidget;
class K3bDataMultiSessionCombobox;


/**
  *@author Sebastian Trueg
  */

class K3bDataBurnDialog : public K3bProjectBurnDialog
{
 Q_OBJECT
  TQ_OBJECT

 public:
   K3bDataBurnDialog(K3bDataDoc*, TQWidget *parent=0, const char *name=0, bool modal = true );
   ~K3bDataBurnDialog();

 protected:
   void setupSettingsTab();
   void loadK3bDefaults();
   void loadUserDefaults( KConfigBase* );
   void saveUserDefaults( KConfigBase* );
   void toggleAll();

   // --- settings tab ---------------------------
   K3bDataImageSettingsWidget* m_imageSettingsWidget;
   // ----------------------------------------------
	
   TQGroupBox* m_groupDataMode;
   K3bDataModeWidget* m_dataModeWidget;
   K3bDataMultiSessionCombobox* m_comboMultisession;

   TQCheckBox* m_checkVerify;

 protected slots:
   void slotStartClicked();
   void saveSettings();
   void readSettings();

   void slotMultiSessionModeChanged();
};

#endif
