/* 
 *
 * $Id: k3bdatadirtreeview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDATADIRTREEVIEW_H
#define K3BDATADIRTREEVIEW_H


#include <k3blistview.h>
#include <kurl.h>

#include <tqmap.h>

class K3bDataView;
class K3bDataDoc;
class K3bDataDirViewItem;
class K3bDirItem;
class K3bDataItem;
class K3bDataFileView;
class TDEActionCollection;
class TDEActionMenu;
class TDEAction;
class K3bView;
class TQDragMoveEvent;
class TQDragLeaveEvent;


/**
  *@author Sebastian Trueg
  */

class K3bDataDirTreeView : public K3bListView  
{
  TQ_OBJECT
  

 public:
  K3bDataDirTreeView( K3bView*, K3bDataDoc*, TQWidget* parent );
  virtual ~K3bDataDirTreeView();

  K3bDataDirViewItem* root() { return m_root; }
		
  void setFileView( K3bDataFileView* view ) { m_fileView = view; }

  TDEActionCollection* actionCollection() const { return m_actionCollection; }

 public slots:
  void checkForNewItems();
  void setCurrentDir( K3bDirItem* );

 signals:
  //  void urlsDropped( const KURL::List&, TQListViewItem* parent );
  void dirSelected( K3bDirItem* );

 protected:
  bool acceptDrag(TQDropEvent* e) const;
  void contentsDragMoveEvent( TQDragMoveEvent* e );
  void contentsDragLeaveEvent( TQDragLeaveEvent* e );

  TDEActionCollection* m_actionCollection;
  TDEActionMenu* m_popupMenu;
  TDEAction* m_actionRemove;
  TDEAction* m_actionRename;
  TDEAction* m_actionNewDir;
  TDEAction* m_actionProperties;

 protected slots:
  virtual void slotDropped( TQDropEvent* e, TQListViewItem* after, TQListViewItem* parent );

 private:
  void setupActions();
  void startDropAnimation( K3bDirItem* );
  void stopDropAnimation();

  K3bView* m_view;

  K3bDataDoc* m_doc;
  K3bDataDirViewItem* m_root;
  K3bDataFileView* m_fileView;

  /**
   * We save the dirItems in a map to have a fast way
   * for checking for new or removed items
   */
  TQMap<K3bDirItem*, K3bDataDirViewItem*> m_itemMap;

  class Private;
  Private* d;

 private slots:
  void slotExecuted( TQListViewItem* );
  void slotDataItemRemoved( K3bDataItem* );
  void showPopupMenu( TDEListView*, TQListViewItem* _item, const TQPoint& );
  void slotRenameItem();
  void slotRemoveItem();
  void slotNewDir();
  void slotProperties();
  void slotDropAnimate();
  void slotItemAdded( K3bDataItem* );
  void slotAddUrls();
  void slotDocChanged();
};

#endif
