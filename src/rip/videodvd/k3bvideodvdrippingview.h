/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_VIDEODVD_RIPPING_VIEW_H_
#define _K3B_VIDEODVD_RIPPING_VIEW_H_

#include <k3bmediacontentsview.h>
#include <k3bmedium.h>
#include <k3bvideodvd.h>

class K3bVideoDVDRippingTitleListView;
class K3bToolBox;
class TQLabel;
class TDEActionCollection;
class TDEActionMenu;
class TDEListView;
class TQListViewItem;

class K3bVideoDVDRippingView : public K3bMediaContentsView
{
  TQ_OBJECT
  

 public:
  K3bVideoDVDRippingView( TQWidget* parent = 0, const char * name = 0 );
  ~K3bVideoDVDRippingView();

  TDEActionCollection* actionCollection() const { return m_actionCollection; }

 private slots:
  void slotStartRipping();

  void slotContextMenu( TDEListView*, TQListViewItem*, const TQPoint& );

  void slotCheckAll();
  void slotUncheckAll();
  void slotCheck();
  void slotUncheck();

 private:
  void reloadMedium();
  void enableInteraction( bool enable );
  void initActions();

  TDEActionCollection* m_actionCollection;
  TDEActionMenu* m_popupMenu;

  K3bToolBox* m_toolBox;
  TQLabel* m_labelLength;
  K3bVideoDVDRippingTitleListView* m_titleView;  

  K3bVideoDVD::VideoDVD m_dvd;
};

#endif
