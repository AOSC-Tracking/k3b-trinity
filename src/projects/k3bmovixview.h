/* 
 *
 * $Id: k3bmovixview.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef _K3B_MOVIX_VIEW_H_
#define _K3B_MOVIX_VIEW_H_

#include <k3bview.h>

class K3bMovixDoc;
class K3bMovixListView;
class KAction;
class KPopupMenu;
class TQListViewItem;
class TQPoint;
class TQLineEdit;


class K3bMovixView : public K3bView
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bMovixView( K3bMovixDoc* doc, TQWidget* parent = 0, const char* name = 0 );
  virtual ~K3bMovixView();

 private slots:
  void slotContextMenuRequested(TQListViewItem*, const TQPoint& , int );
  void slotRemoveItems();
  void slotRemoveSubTitleItems();
  void showPropertiesDialog();
  void slotAddSubTitleFile();
  void slotDocChanged();

 protected:
  virtual K3bProjectBurnDialog* newBurnDialog( TQWidget* parent = 0, const char* name = 0 );

  K3bMovixListView* m_listView;

 private:
  K3bMovixDoc* m_doc;

  KAction* m_actionProperties;
  KAction* m_actionRemove;
  KAction* m_actionRemoveSubTitle;
  KAction* m_actionAddSubTitle;
  KPopupMenu* m_popupMenu;

  TQLineEdit* m_volumeIDEdit;
};

#endif
