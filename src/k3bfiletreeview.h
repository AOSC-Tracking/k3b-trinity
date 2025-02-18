/*
 *
 * $Id: k3bfiletreeview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BFILETREEVIEW_H
#define K3BFILETREEVIEW_H


#include <tdefiletreeview.h>

class KFileTreeBranch;
class TDEActionCollection;
class TDEActionMenu;
class TQPoint;
class TQDropEvent;
class TQDragEnterEvent;

namespace K3bDevice {
  class Device;
  class DeviceManager;
}

namespace TDEIO {
  class Job;
}


class K3bDeviceBranch : public KFileTreeBranch
{
  TQ_OBJECT
  

 public:
  K3bDeviceBranch( KFileTreeView*, K3bDevice::Device* dev, KFileTreeViewItem* item = 0 );

  K3bDevice::Device* device() const { return m_device; }

  /**
   * Adds or removes the blockdevicename from the branch name
   */
  void showBlockDeviceName( bool b );

 public slots:
  void setCurrent( bool );

  bool populate( const KURL& url,  KFileTreeViewItem *currItem );

 private slots:
  void slotMediumChanged( K3bDevice::Device* );

 private:
  void updateLabel();

  K3bDevice::Device* m_device;
  bool m_showBlockDeviceName;
};


class K3bFileTreeBranch : public KFileTreeBranch
{
 public:
  K3bFileTreeBranch( KFileTreeView*,
		     const KURL& url,
		     const TQString& name,
		     const TQPixmap& pix,
		     bool showHidden = false,
		     KFileTreeViewItem* item = 0 );
};


class K3bDeviceBranchViewItem : public KFileTreeViewItem
{
 public:
  K3bDeviceBranchViewItem( KFileTreeViewItem*, K3bDevice::Device*, K3bDeviceBranch* );
  K3bDeviceBranchViewItem( KFileTreeView*, K3bDevice::Device*, K3bDeviceBranch* );

  TQString key( int column, bool ascending ) const;

  void setCurrent( bool );

  void paintCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align );

  int widthHint() const;

 private:
  bool m_bCurrent;

  K3bDevice::Device* m_device;
};


class K3bFileTreeViewItem : public KFileTreeViewItem
{
 public:
  K3bFileTreeViewItem( KFileTreeViewItem*, KFileItem*, KFileTreeBranch* );
  K3bFileTreeViewItem( KFileTreeView *, KFileItem*, KFileTreeBranch* );

  TQString key( int column, bool ascending ) const;
};


/**
  *@author Sebastian Trueg
  */
class K3bFileTreeView : public KFileTreeView
{
  TQ_OBJECT
  

 public: 
  K3bFileTreeView( TQWidget *parent = 0, const char *name = 0 );
  ~K3bFileTreeView();


  virtual KFileTreeBranch* addBranch( KFileTreeBranch* );
  virtual KFileTreeBranch* addBranch( const KURL& url, const TQString& name, const TQPixmap& , bool showHidden = false );

  K3bDeviceBranch* branch( K3bDevice::Device* dev );

  /**
   * returns 0 if no device is selected 
   */
  K3bDevice::Device* selectedDevice() const;

  /** 
   * returnes an empty url if no url is selected
   */
  KURL selectedUrl() const;

 public slots:
  /**
   * adds home and root dir branch
   */
  void addDefaultBranches();
  void addCdDeviceBranches( K3bDevice::DeviceManager* );
  void addDeviceBranch( K3bDevice::Device* dev );

  /**
   * Make dev the current device. This does not mean that the device entry
   * will be highlighted but marked otherwise since this means that it is the
   * current device in the application and not the treeview.
   */
  void setCurrentDevice( K3bDevice::Device* dev );

  /**
   * his will highlight the device and also make it the current device.
   */
  void setSelectedDevice( K3bDevice::Device* dev );

  void followUrl( const KURL& url );
  void setTreeDirOnlyMode( bool b );
  void enablePopupMenu( bool b ) { m_menuEnabled = b; }

  /**
   * @reimplemented
   */
  virtual void clear();

  void updateMinimumWidth();

 signals:
  void urlExecuted( const KURL& url );
  void deviceExecuted( K3bDevice::Device* dev );

  /** only gets emitted if the menu is disabled */
  void contextMenu( K3bDevice::Device*, const TQPoint& );
  /** only gets emitted if the menu is disabled */
  void contextMenu( const KURL& url, const TQPoint& );
  
 private slots:
  void slotItemExecuted( TQListViewItem* item );
  void slotContextMenu( TDEListView*, TQListViewItem*, const TQPoint& );
  void slotSettingsChangedK3b(int category);
  void slotMouseButtonClickedK3b( int btn, TQListViewItem *item, const TQPoint &pos, int c );

 private:
  void initActions();

  class Private;
  Private* d;

  bool m_dirOnlyMode;
  TDEActionCollection* m_actionCollection;
  TDEActionMenu* m_devicePopupMenu;
  TDEActionMenu* m_urlPopupMenu;
  bool m_menuEnabled;
};

#endif
