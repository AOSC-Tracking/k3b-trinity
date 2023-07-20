/* 
 *
 * $Id: k3bdataadvancedimagesettingswidget.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_DATA_ADVANCED_IMAGE_SETTINGS_WIDGET_H
#define K3B_DATA_ADVANCED_IMAGE_SETTINGS_WIDGET_H


#include "base_k3badvanceddataimagesettings.h"

class K3bIsoOptions;
class TQCheckListItem;


class K3bDataAdvancedImageSettingsWidget : public base_K3bAdvancedDataImageSettings
{
  TQ_OBJECT
  

 public:
  K3bDataAdvancedImageSettingsWidget( TQWidget* parent = 0, const char* name =  0 );
  ~K3bDataAdvancedImageSettingsWidget();

  void load( const K3bIsoOptions& );
  void save( K3bIsoOptions& );

 private slots:
  void slotJolietToggled( bool on );

 private:
  TQCheckListItem* m_checkAllowUntranslatedFilenames;
  TQCheckListItem* m_checkAllowMaxLengthFilenames;
  TQCheckListItem* m_checkAllowFullAscii;
  TQCheckListItem* m_checkAllowOther;
  TQCheckListItem* m_checkAllowLowercaseCharacters;
  TQCheckListItem* m_checkAllowMultiDot;
  TQCheckListItem* m_checkOmitVersionNumbers;
  TQCheckListItem* m_checkOmitTrailingPeriod;
  TQCheckListItem* m_checkCreateTransTbl;
  TQCheckListItem* m_checkHideTransTbl;
  TQCheckListItem* m_checkFollowSymbolicLinks;
  TQCheckListItem* m_checkAllow31CharFilenames;
  TQCheckListItem* m_checkAllowBeginningPeriod;
  TQCheckListItem* m_checkJolietLong;
  TQCheckListItem* m_checkDoNotCacheInodes;

  TQCheckListItem* m_isoLevelController;
  TQCheckListItem* m_radioIsoLevel1;
  TQCheckListItem* m_radioIsoLevel2;
  TQCheckListItem* m_radioIsoLevel3;

  class PrivateCheckViewItem;
  class PrivateIsoWhatsThis;

  friend class PrivateIsoWhatsThis;
};


#endif
