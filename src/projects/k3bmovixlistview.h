/* 
 *
 * $Id: k3bmovixlistview.h 628165 2007-01-29 11:01:22Z trueg $
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



#ifndef _K3B_MOVIX_LISTVIEW_H_
#define _K3B_MOVIX_LISTVIEW_H_

#include <k3blistview.h>
#include <kfileitem.h>

#include <tqmap.h>


class K3bMovixDoc;
class K3bMovixFileItem;
class K3bFileItem;


class K3bMovixListViewItem : public K3bListViewItem
{
 public:
  K3bMovixListViewItem( K3bMovixDoc* doc, K3bMovixFileItem*, TQListView* parent, TQListViewItem* after );
  K3bMovixListViewItem( K3bMovixDoc* doc, K3bMovixFileItem*, TQListViewItem* parent );
  ~K3bMovixListViewItem();

  K3bMovixFileItem* fileItem() const { return m_fileItem; }
  K3bMovixDoc* doc() const { return m_doc; }

  virtual bool isMovixFileItem() const { return true; }

 private:
  K3bMovixDoc* m_doc;
  K3bMovixFileItem* m_fileItem;
};


class K3bMovixFileViewItem : public K3bMovixListViewItem, public KFileItem
{
 public:
  K3bMovixFileViewItem( K3bMovixDoc* doc, K3bMovixFileItem*, TQListView* parent, TQListViewItem* );

  TQString text( int ) const;
  void setText(int col, const TQString& text );

  /** always sort according to the playlist order */
  TQString key( int, bool ) const;
};

class K3bMovixSubTitleViewItem : public K3bMovixListViewItem, public KFileItem
{
 public:
  K3bMovixSubTitleViewItem( K3bMovixDoc*, K3bMovixFileItem* item, K3bMovixListViewItem* parent );
  ~K3bMovixSubTitleViewItem();

  TQString text( int ) const;

  bool isMovixFileItem() const { return false; }
};


class K3bMovixListView : public K3bListView
{
  Q_OBJECT
  

 public:
  K3bMovixListView( K3bMovixDoc* doc, TQWidget* parent = 0, const char* name = 0 );
  ~K3bMovixListView();

  TQDragObject* dragObject();

 protected:
  bool acceptDrag(TQDropEvent* e) const;

 private slots:
  void slotNewFileItems();
  void slotFileItemRemoved( K3bMovixFileItem* );
  void slotSubTitleItemRemoved( K3bMovixFileItem* );
  void slotDropped( KListView*, TQDropEvent* e, TQListViewItem* after );
  void slotChanged();

 private:
  K3bMovixDoc* m_doc;

  TQMap<K3bFileItem*, K3bMovixFileViewItem*> m_itemMap;
};

#endif
