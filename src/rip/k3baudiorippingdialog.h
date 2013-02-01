/* 
 *
 * $Id: k3baudiorippingdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_AUDIO_RIPPING_DIALOG_H_
#define _K3B_AUDIO_RIPPING_DIALOG_H_

#include <k3binteractiondialog.h>

#include <tqstringlist.h>

#include <k3bcddbquery.h>

namespace K3bDevice {
  class Device;
  class Toc;
}


class TDEListView;
class TQCheckBox;
class TQSpinBox;
class TQComboBox;
class K3bCddbPatternWidget;
class TQToolButton;
class K3bAudioConvertingOptionWidget;


/**
  *@author Sebastian Trueg
  */
class K3bAudioRippingDialog : public K3bInteractionDialog
{
  Q_OBJECT
  

 public: 
  K3bAudioRippingDialog( const K3bDevice::Toc&, 
			 K3bDevice::Device*,
			 const K3bCddbResultEntry&, 
			 const TQValueList<int>&, 
			 TQWidget *parent = 0, const char *name = 0 );
  ~K3bAudioRippingDialog();

  void setStaticDir( const TQString& path );

 public slots:  
  void refresh();
  void init();

 private:
  K3bDevice::Toc m_toc;
  K3bDevice::Device* m_device;
  K3bCddbResultEntry m_cddbEntry;
  TQValueList<int> m_trackNumbers;

  TDEListView*    m_viewTracks;

  TQComboBox* m_comboParanoiaMode;
  TQSpinBox* m_spinRetries;
  TQCheckBox* m_checkIgnoreReadErrors;
  TQCheckBox* m_checkUseIndex0;

  K3bCddbPatternWidget* m_patternWidget;
  K3bAudioConvertingOptionWidget* m_optionWidget;

  void setupGui();
  void setupContextHelp();

  void loadK3bDefaults();
  void loadUserDefaults( TDEConfigBase* );
  void saveUserDefaults( TDEConfigBase* );

  class Private;
  Private* d;
  
 private slots:
  void slotStartClicked();
};

#endif
