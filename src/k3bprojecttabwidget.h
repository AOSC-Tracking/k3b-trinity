/* 
 *
 * $Id: k3bprojecttabwidget.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BPROJECTTABWIDGET_H
#define K3BPROJECTTABWIDGET_H

#include <tqtabwidget.h>
#include <kurl.h>

class KAction;
class KActionMenu;
class K3bDoc;


/**
 * An enhanced Tab Widget that hides the tabbar in case only one page has been inserted
 * and shows a context menu fpr K3b projects.
 *
 * @author Sebastian Trueg
 */
class K3bProjectTabWidget : public TQTabWidget
{
  Q_OBJECT
  

 public: 
  K3bProjectTabWidget( TQWidget *parent = 0, const char *name = 0, WFlags = 0 );
  ~K3bProjectTabWidget();

  void insertTab( K3bDoc* );
  
  void addTab( TQWidget * child, const TQString & label );
  void addTab( TQWidget * child, const TQIconSet & iconset, const TQString & label );
  void addTab( TQWidget * child, TQTab * tab );
  void insertTab( TQWidget * child, const TQString & label, int index = -1 );
  void insertTab( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index = -1 );
  void insertTab( TQWidget * child, TQTab * tab, int index = -1 );

  /**
   * \return the project for the tab at position \p pos or 0 in case the tab is
   * not a project tab.
   */
  K3bDoc* projectAt( const TQPoint& pos ) const;

  /**
   * inserts the given action into the popup menu for the tabs
   */
  void insertAction( KAction* );

  bool eventFilter( TQObject* o, TQEvent* e );

 public slots:
  void removePage( TQWidget* );

 private slots:
  void slotDocChanged( K3bDoc* );
  void slotDocSaved( K3bDoc* );

 private:
  KActionMenu* m_projectActionMenu;

  class ProjectData;
  TQMap<K3bDoc*, ProjectData> m_projectDataMap;
};

#endif
