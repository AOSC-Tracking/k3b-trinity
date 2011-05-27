/* 
 *
 * $Id: k3blameencoder.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_LAME_ENCODER_H_
#define _K3B_LAME_ENCODER_H_


#include <k3baudioencoder.h>
#include <k3bpluginconfigwidget.h>

#include "base_k3blameencodersettingswidget.h"
#include "base_k3bmanualbitratesettingswidget.h"


class KDialogBase;


class K3bLameEncoder : public K3bAudioEncoder
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bLameEncoder( TQObject* tqparent = 0, const char* name = 0 );
  ~K3bLameEncoder();

  bool openFile( const TQString& extension, const TQString& filename, const K3b::Msf& length );
  bool isOpen() const;
  void closeFile();
  const TQString& filename() const;

  TQStringList extensions() const;
  
  TQString fileTypeComment( const TQString& ) const;

  long long fileSize( const TQString&, const K3b::Msf& msf ) const;

  int pluginSystemVersion() const { return 3; }

  K3bPluginConfigWidget* createConfigWidget( TQWidget* tqparent = 0, 
					     const char* name = 0 ) const;

 private:
  void finishEncoderInternal();
  bool initEncoderInternal( const TQString& extension, const K3b::Msf& length );
  long encodeInternal( const char* data, TQ_ULONG len );
  void setMetaDataInternal( MetaDataField, const TQString& );

  class Private;
  Private* d;
};


class K3bLameEncoderSettingsWidget : public K3bPluginConfigWidget
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bLameEncoderSettingsWidget( TQWidget* tqparent = 0, const char* name = 0 );
  ~K3bLameEncoderSettingsWidget();

 public slots:
  void loadConfig();
  void saveConfig();

 private slots:
  void slotQualityLevelChanged( int val );
  void slotShowManualSettings();
  void updateManualSettingsLabel();

 private:
  base_K3bLameEncoderSettingsWidget* m_w;
  base_K3bManualBitrateSettingsWidget* m_brW;
  KDialogBase* m_manualSettingsDlg;
};

#endif
