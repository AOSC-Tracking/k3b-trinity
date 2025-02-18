/*
*
* $Id: k3bvcdlistview.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2004 Christian Kvasny <chris@k3b.org>
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

#ifndef K3BVCDLISTVIEW_H
#define K3BVCDLISTVIEW_H


#include <k3blistview.h>

#include <tqmap.h>

class TQDragEnterEvent;
class TQDragObject;
class TQDropEvent;
class TQTimer;
class TDEPopupMenu;
class TDEAction;
class K3bVcdDoc;
class K3bView;
class K3bVcdTrack;
class TDEActionCollection;
class K3bVcdListViewItem;
class TQPainter;


class K3bVcdListView : public K3bListView
{
        TQ_OBJECT
  

    public:
        K3bVcdListView( K3bView*, K3bVcdDoc*, TQWidget *parent = 0, const char *name = 0 );
        ~K3bVcdListView();

        /**
         * reimplemented from TDEListView
         */
        void insertItem( TQListViewItem* );

        TDEActionCollection* actionCollection() const
        {
            return m_actionCollection;
        }

        TQPtrList<K3bVcdTrack> selectedTracks();

    signals:
        void lengthReady();

    private:
        void setupColumns();
        void setupPopupMenu();
        void setupActions();

        K3bVcdDoc* m_doc;
        K3bView* m_view;

        TDEAction* m_actionProperties;
        TDEAction* m_actionRemove;
        TDEActionCollection* m_actionCollection;

        TDEPopupMenu* m_popupMenu;

        TQMap<K3bVcdTrack*, K3bVcdListViewItem*> m_itemMap;

    private slots:
        void slotDropped( TDEListView*, TQDropEvent* e, TQListViewItem* after );
        void slotUpdateItems();
        void showPopupMenu( TDEListView*, TQListViewItem* item, const TQPoint& );
        void showPropertiesDialog();
        void slotRemoveTracks();
        void slotTrackRemoved( K3bVcdTrack* );

    protected:
        bool acceptDrag( TQDropEvent* e ) const;
        TQDragObject* dragObject();
};

#endif
