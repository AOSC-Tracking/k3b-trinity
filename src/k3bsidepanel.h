/* 
 *
 * $Id: k3bsidepanel.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_SIDE_PANEL_H_
#define _K3B_SIDE_PANEL_H_

#include <tqtoolbox.h>

class K3bMainWindow;
class K3bFileTreeView;
class TQFrame;
class TDEAction;


class K3bSidePanel : public TQToolBox
{
  Q_OBJECT
  

 public:
  K3bSidePanel( K3bMainWindow*, TQWidget* parent = 0, const char* name = 0 );
  ~K3bSidePanel();

  /**
   * This should be removed in the future. For now we need it because of the
   * bad design of the dirview. :(
   */
  K3bFileTreeView* fileTreeView() const { return m_fileTreeView; }

 private:
  K3bMainWindow* m_mainWindow;
  K3bFileTreeView* m_fileTreeView;

  TQFrame* createPanel();
  void addButton( TQFrame* frame, TDEAction* action );
};

#endif
