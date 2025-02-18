/* 
 *
 * $Id: k3baudioconvertingoptionwidget.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_AUDIO_CONVERTING_OPTION_WIDGET_H_
#define _K3B_AUDIO_CONVERTING_OPTION_WIDGET_H_

#include "base_k3baudiorippingoptionwidget.h"

#include <tqcheckbox.h>
#include <tdeio/global.h>

class K3bAudioEncoder;
class TDEConfigBase;


/**
 * Internally used by K3bAudioConvertingDialog
 */
class K3bAudioConvertingOptionWidget : public base_K3bAudioRippingOptionWidget
{
  TQ_OBJECT
  

 public:
  K3bAudioConvertingOptionWidget( TQWidget* parent, const char* name = 0 );
  ~K3bAudioConvertingOptionWidget();

  void setBaseDir( const TQString& path );

  void setNeededSize( TDEIO::filesize_t );

  /**
   * @returns 0 if wave is selected
   */
  K3bAudioEncoder* encoder() const;
  TQString extension() const;

  TQString baseDir() const;

  bool createPlaylist() const { return m_checkCreatePlaylist->isChecked(); }
  bool playlistRelativePath() const { return m_checkPlaylistRelative->isChecked(); }
  bool createSingleFile() const { return m_checkSingleFile->isChecked(); }
  bool createCueFile() const { return m_checkWriteCueFile->isChecked(); }

 public slots:
  void loadDefaults();
  void loadConfig( TDEConfigBase* );
  void saveConfig( TDEConfigBase* );

 signals:
  void changed();

 private slots:
  void slotConfigurePlugin();
  void slotUpdateFreeTempSpace();
  void slotEncoderChanged();

 private:
  class Private;
  Private* d;
};

#endif
