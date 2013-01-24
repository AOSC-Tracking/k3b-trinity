/* 
 *
 * $Id: k3bsoxencoder.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_SOX_ENCODER_H_
#define _K3B_SOX_ENCODER_H_


#include <k3baudioencoder.h>
#include <k3bpluginconfigwidget.h>


class base_K3bSoxEncoderConfigWidget;
class TDEProcess;

class K3bSoxEncoder : public K3bAudioEncoder
{
  Q_OBJECT
  

 public:
  K3bSoxEncoder( TQObject* parent = 0, const char* name = 0 );
  ~K3bSoxEncoder();

  TQStringList extensions() const;
  
  TQString fileTypeComment( const TQString& ) const;

  long long fileSize( const TQString&, const K3b::Msf& msf ) const;

  int pluginSystemVersion() const { return 3; }

  K3bPluginConfigWidget* createConfigWidget( TQWidget* parent = 0, 
					     const char* name = 0 ) const;

  /**
   * reimplemented since sox writes the file itself
   */
  bool openFile( const TQString& ext, const TQString& filename, const K3b::Msf& );
  void closeFile();

 private slots:
  void slotSoxFinished( TDEProcess* );
  void slotSoxOutputLine( const TQString& );

 private:
  void finishEncoderInternal();
  bool initEncoderInternal( const TQString& extension );
  long encodeInternal( const char* data, TQ_ULONG len );

  class Private;
  Private* d;
};


class K3bSoxEncoderSettingsWidget : public K3bPluginConfigWidget
{
  Q_OBJECT
  

 public:
  K3bSoxEncoderSettingsWidget( TQWidget* parent = 0, const char* name = 0 );
  ~K3bSoxEncoderSettingsWidget();

 public slots:
  void loadConfig();
  void saveConfig();

 private:
  base_K3bSoxEncoderConfigWidget* w;
};

#endif
