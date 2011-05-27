/* 
 *
 * $Id: k3bstatusbarmanager.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef K3B_STATUSBAR_MANAGER_H
#define K3B_STATUSBAR_MANAGER_H

#include <tqobject.h>

class TQLabel;
class K3bMainWindow;
class TQEvent;
class K3bDoc;
class TQTimer;

class K3bStatusBarManager : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bStatusBarManager( K3bMainWindow* tqparent );
  ~K3bStatusBarManager();

 public slots:
  void update();

 private slots:
  void slotFreeTempSpace( const TQString&, unsigned long, unsigned long, unsigned long );
  void showActionStatusText( const TQString& text );
  void clearActionStatusText();
  void slotActiveProjectChanged( K3bDoc* doc );
  void slotUpdateProjectStats();

 private:
  bool eventFilter( TQObject* o, TQEvent* e );

  TQLabel* m_labelInfoMessage;
  TQLabel* m_pixFreeTemp;
  TQLabel* m_labelFreeTemp;
  TQLabel* m_versionBox;
  TQLabel* m_labelProjectInfo;

  K3bMainWindow* m_mainWindow;

  TQTimer* m_updateTimer;
};

#endif
