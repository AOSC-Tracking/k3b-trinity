/*
*
* $Id: k3bvideocdview.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003 Christian Kvasny <chris@k3b.org>
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


#ifndef _K3B_VIDEOCDVIEW_H_
#define _K3B_VIDEOCDVIEW_H_

#include <tqdom.h>

#include <k3bmediacontentsview.h>
#include <k3bmedium.h>

#include "k3bvideocdinfo.h"

class TDEActionCollection;
class TDEActionMenu;
class TDEListView;

class TQLabel;
class TQListViewItem;

class K3bListView;
class K3bToolBox;
class K3bVideoCdRippingOptions;

namespace K3bDevice
{
  class DiskInfoDetector;
  class Toc;
  class Device;
}


class K3bVideoCdView : public K3bMediaContentsView
{
        TQ_OBJECT
  

    public:
        K3bVideoCdView( TQWidget* parent = 0, const char * name = 0 );
        ~K3bVideoCdView();

        TDEActionCollection* actionCollection() const
        {
            return m_actionCollection;
        }

    private slots:
        void slotContextMenu( TDEListView*, TQListViewItem*, const TQPoint& );
        void slotTrackSelectionChanged( TQListViewItem* );
        void slotStateChanged( TQListViewItem* );
        void slotVideoCdInfoFinished( bool );

        void startRip();
        void slotSelectAll();
        void slotDeselectAll();
        void slotSelect();
        void slotDeselect();

    private:

        class VideoTrackViewCheckItem;
        class VideoTrackViewItem;

	void reloadMedium();

        void initActions();
        void updateDisplay();
        void enableInteraction( bool );
        void buildTree( TQListViewItem *parentItem, const TQDomElement &parentElement, const TQString& pname = TQString() );

        K3bDevice::Toc m_toc;

        TDEActionCollection* m_actionCollection;
        TDEActionMenu* m_popupMenu;

        K3bVideoCdInfoResult m_videocdinfoResult;
        K3bVideoCdInfo* m_videocdinfo;
        K3bVideoCdRippingOptions* m_videooptions;

        K3bListView* m_trackView;
        K3bToolBox* m_toolBox;
        TQLabel* m_labelLength;

        TQDomDocument domTree;

        TQValueList<VideoTrackViewCheckItem *> m_contentList;

        unsigned long m_videocddatasize;
        unsigned long m_videocdmpegsize;
        
};

#endif
