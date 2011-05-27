/* 
 *
 * $Id: k3baudiocdview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_AUDIOCD_VIEW_H_
#define _K3B_AUDIOCD_VIEW_H_

#include <k3bmediacontentsview.h>
#include <k3bmedium.h>

#include <k3btoc.h>
#include <k3bcddbresult.h>
#include <k3bcdtext.h>

class K3bListView;
class KListView;
class TQListViewItem;
class TQPoint;
class KActionCollection;
class KActionMenu;
class K3bCddb;
class TQLabel;
class K3bToolBox;
class TQDragObject;


namespace K3bDevice {
  class Device;
}


class K3bAudioCdView : public K3bMediaContentsView
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bAudioCdView( TQWidget* tqparent = 0, const char * name = 0 );
  ~K3bAudioCdView();

  KActionCollection* actionCollection() const { return m_actionCollection; }

  /**
   * internal
   */
  TQDragObject* dragObject();

 public slots:
  void queryCddb();

 private slots:
  void slotContextMenu( KListView*, TQListViewItem*, const TQPoint& );
  void slotItemRenamed( TQListViewItem*, const TQString&, int );
  void slotCddbQueryFinished( int );
  void slotTrackSelectionChanged( TQListViewItem* );
  void slotSaveCddbLocally();

  void slotEditTrackCddb();
  void slotEditAlbumCddb();
  void startRip();
  void slotCheckAll();
  void slotUncheckAll();
  void slotSelect();
  void slotDeselect();

 private:
  void reloadMedium();

  void initActions();
  void updateDisplay();
  void enableInteraction( bool );
  void showBusyLabel( bool );

  K3bDevice::Toc m_toc;
  K3bDevice::Device* m_device;

  K3bCddbResultEntry m_cddbInfo;

  KActionCollection* m_actionCollection;
  KActionMenu* m_popupMenu;

  K3bListView* m_trackView;
  K3bToolBox* m_toolBox;
  TQLabel* m_labelLength;

  class AudioTrackViewItem;

  K3bCddb* m_cddb;

  K3bDevice::CdText m_cdText;

  TQLabel* m_busyInfoLabel;
};


#endif
