/* 
 *
 * $Id: k3bcddbpatternwidget.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_CDDB_PATTERN_WIDGET_H_
#define _K3B_CDDB_PATTERN_WIDGET_H_

#include "base_k3bcddbpatternwidget.h"

class TDEConfigBase;


class K3bCddbPatternWidget : public base_K3bCddbPatternWidget
{
  TQ_OBJECT
  

 public:
  K3bCddbPatternWidget( TQWidget* parent = 0, const char* name = 0 );
  ~K3bCddbPatternWidget();

  TQString filenamePattern() const;
  TQString playlistPattern() const;
  TQString blankReplaceString() const;
  bool replaceBlanks() const;

 signals:
  void changed();

 public slots:
  void loadConfig( TDEConfigBase* );
  void saveConfig( TDEConfigBase* );
  void loadDefaults();

 private slots:
  void slotSeeSpecialStrings();
  void slotSeeConditionalInclusion();
};

#endif
