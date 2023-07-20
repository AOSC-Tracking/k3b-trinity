/* 
 *
 * $Id: k3bdatafileview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDATAFILEVIEW_H
#define K3BDATAFILEVIEW_H

#include <k3blistview.h>

#include <kurl.h>

#include <tqmap.h>


class K3bDataDoc;
class K3bDirItem;
class K3bDataView;
class K3bDataViewItem;
class K3bDataItem;
class TQDropEvent;
class TDEActionCollection;
class TDEActionMenu;
class TDEAction;
class K3bDataDirTreeView;
class K3bDataDirViewItem;
class K3bView;
class TQPainter;
class TQDragMoveEvent;
class TQDragLeaveEvent;

/**
  *@author Sebastian Trueg
  */

class K3bDataFileView : public K3bListView  
{
  TQ_OBJECT
  

 public:
  K3bDataFileView( K3bView*, K3bDataDirTreeView*, K3bDataDoc*, TQWidget* parent );
  ~K3bDataFileView();
	
  K3bDirItem* currentDir() const;

  TDEActionCollection* actionCollection() const { return m_actionCollection; }

 signals:
  void dirSelected( K3bDirItem* );
	
 public slots:
  void slotSetCurrentDir( K3bDirItem* );
  void checkForNewItems();

 private slots:
  void slotDataItemRemoved( K3bDataItem* );
  void slotExecuted( TQListViewItem* );
  void slotDropped( TQDropEvent* e, TQListViewItem* after, TQListViewItem* parent );
  void showPopupMenu( TDEListView*, TQListViewItem* _item, const TQPoint& );
  void slotRenameItem();
  void slotRemoveItem();
  void slotNewDir();
  void slotParentDir();
  void slotProperties();
  void slotDoubleClicked( TQListViewItem* item );
  void slotItemAdded( K3bDataItem* );
  void slotAddUrls();
  void slotOpen();

 protected:
  bool acceptDrag(TQDropEvent* e) const;
  void contentsDragMoveEvent( TQDragMoveEvent* e );
  void contentsDragLeaveEvent( TQDragLeaveEvent* e );
  TQDragObject* dragObject();

 private:
  void clearItems();
  void setupActions();

  TDEActionCollection* m_actionCollection;
  TDEActionMenu* m_popupMenu;
  TDEAction* m_actionParentDir;
  TDEAction* m_actionRemove;
  TDEAction* m_actionRename;
  TDEAction* m_actionNewDir;
  TDEAction* m_actionProperties;
  TDEAction* m_actionOpen;

  K3bView* m_view;

  K3bDataDoc* m_doc;
  mutable K3bDirItem* m_currentDir;
  K3bDataDirTreeView* m_treeView;

  K3bDataDirViewItem* m_dropDirItem;

  TQMap<K3bDataItem*, K3bDataViewItem*> m_itemMap;

  // used for the urladdingdialog hack
  KURL::List m_addUrls;
  K3bDirItem* m_addParentDir;
};

#endif
