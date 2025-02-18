/* 
 *
 * $Id: k3bmovixdvdburndialog.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef _K3B_MOVIX_DVD_BURN_DIALOG_H_
#define _K3B_MOVIX_DVD_BURN_DIALOG_H_

#include "k3bprojectburndialog.h"

class K3bMovixDvdDoc;
class K3bMovixOptionsWidget;
class K3bDataImageSettingsWidget;
class TQCheckBox;


class K3bMovixDvdBurnDialog : public K3bProjectBurnDialog
{
  TQ_OBJECT
  

 public:
  K3bMovixDvdBurnDialog( K3bMovixDvdDoc* doc, TQWidget* parent = 0, const char* name = 0, bool modal = true );
  ~K3bMovixDvdBurnDialog();

 protected slots:
  void slotStartClicked();

 protected:
  void saveSettings();
  void readSettings();
  void loadK3bDefaults();
  void loadUserDefaults( TDEConfigBase* );
  void saveUserDefaults( TDEConfigBase* );
  void toggleAll();

 private:
  K3bMovixDvdDoc* m_doc;
  K3bMovixOptionsWidget* m_movixOptionsWidget;
  K3bDataImageSettingsWidget* m_imageSettingsWidget;

  TQCheckBox* m_checkVerify;
};

#endif

