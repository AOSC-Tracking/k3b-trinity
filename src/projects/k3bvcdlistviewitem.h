/*
*
* $Id: k3bvcdlistviewitem.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3BVCDLISTVIEWITEM_H
#define K3BVCDLISTVIEWITEM_H

#include <k3blistview.h>

class K3bVcdTrack;

class K3bVcdListViewItem : public K3bListViewItem
{

    public:
        K3bVcdListViewItem( K3bVcdTrack* track, K3bListView* tqparent );
        K3bVcdListViewItem( K3bVcdTrack* track, K3bListView* tqparent, TQListViewItem* after );
        ~K3bVcdListViewItem();

        /** reimplemented from TQListViewItem */
        TQString text( int i ) const;

        /** reimplemented from TQListViewItem */
        void setText( int col, const TQString& text );

        /** reimplemented from TQListViewItem */
        TQString key( int column, bool a ) const;
        bool animate();

        K3bVcdTrack* vcdTrack()
        {
            return m_track;
        }

    private:
        K3bVcdTrack* m_track;
};

#endif
