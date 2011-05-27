/* 
 *
 * $Id: k3bstdguiitems.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_STD_GUIITEMS_H
#define K3B_STD_GUIITEMS_H
#include "k3b_export.h"

class TQWidget;
class TQCheckBox;
class TQComboBox;
class TQFrame;


namespace K3bStdGuiItems
{
  LIBK3B_EXPORT TQCheckBox* simulateCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* daoCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* burnproofCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* onlyCreateImagesCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* createCacheImageCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* removeImagesCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* onTheFlyCheckbox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* cdTextCheckbox( TQWidget* tqparent = 0, const char* name = 0);
  LIBK3B_EXPORT TQComboBox* paranoiaModeComboBox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* startMultisessionCheckBox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* normalizeCheckBox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* verifyCheckBox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQCheckBox* ignoreAudioReadErrorsCheckBox( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQFrame* horizontalLine( TQWidget* tqparent = 0, const char* name = 0 );
  LIBK3B_EXPORT TQFrame* verticalLine( TQWidget* tqparent = 0, const char* name = 0 );
}

#endif
