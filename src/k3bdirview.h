/*
 *
 * $Id: k3bdirview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDIRVIEW_H
#define K3BDIRVIEW_H

#include <tqvbox.h>

#include <k3bmedium.h>

class TQSplitter;
class KURL;
class K3bAudioCdView;
class K3bVideoCdView;
class K3bFileView;
class K3bVideoDVDRippingView;
class KComboBox;
class K3bFileTreeView;
class TQWidgetStack;
class K3bDiskInfoView;
class TQScrollView;
class TQLabel;
class TDEConfig;
class K3bDeviceBranch;

namespace K3bDevice {
  class Device;
  class DiskInfo;
}

namespace TDEIO {
  class Job;
}


/**
  *@author Sebastian Trueg
  */
class K3bDirView : public TQVBox
{
  TQ_OBJECT
  

 public:
  K3bDirView(K3bFileTreeView* tree, TQWidget *parent=0, const char *name=0);
  ~K3bDirView();

 public slots:
  void saveConfig( TDEConfig* c );
  void readConfig( TDEConfig* c );
  void showUrl( const KURL& );
  void showDevice( K3bDevice::Device* );
  
 protected slots:
  void slotDirActivated( const KURL& );
  void slotDirActivated( const TQString& );
  void slotMountFinished( const TQString& );
  void slotUnmountFinished( bool );
  void showMediumInfo( const K3bMedium& );
  void slotDetectingDiskInfo( K3bDevice::Device* dev );
  void home();
  void slotFileTreeContextMenu( K3bDevice::Device* dev, const TQPoint& p );

 signals:
  void urlEntered( const KURL& );
  void deviceSelected( K3bDevice::Device* );

 private:
  TQWidgetStack* m_viewStack;
  TQScrollView* m_scroll;

  K3bAudioCdView*   m_cdView;
  K3bVideoCdView*   m_videoView;
  K3bVideoDVDRippingView* m_movieView;
  K3bFileView* m_fileView;
  K3bDiskInfoView* m_infoView;

  KComboBox* m_urlCombo;
  TQSplitter* m_mainSplitter;
  K3bFileTreeView* m_fileTreeView;

  bool m_bViewDiskInfo;

  class Private;
  Private* d;
};

#endif
