/* 
 *
 * $Id: k3bmixeddirtreeview.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bmixeddirtreeview.h"

#include "k3bmixeddoc.h"
#include "k3baudiotrackaddingdialog.h"
#include <k3blistview.h>
#include <k3baudiodoc.h>
#include <k3bdataviewitem.h>

#include <tqevent.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kurldrag.h>
#include <klocale.h>


class K3bMixedDirTreeView::PrivateAudioRootViewItem : public K3bListViewItem
{
public:
  PrivateAudioRootViewItem( K3bMixedDoc* doc, TQListView* tqparent, TQListViewItem* after )
    : K3bListViewItem( tqparent, after ),
      m_doc(doc)
  {
    setPixmap( 0, SmallIcon("sound") );
  }

  TQString text( int col ) const {
    if( col == 0 )
      return i18n("Audio Tracks") + TQString(" (%1)").tqarg(m_doc->audioDoc()->numOfTracks());
    else
      return TQString();
  }

  private:
    K3bMixedDoc* m_doc;
};


K3bMixedDirTreeView::K3bMixedDirTreeView( K3bView* view, K3bMixedDoc* doc, TQWidget* tqparent, const char* )
  : K3bDataDirTreeView( view, doc->dataDoc(), tqparent ), m_doc(doc)
{
  m_audioRootItem = new PrivateAudioRootViewItem( doc, this, root() );

  connect( this, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
	   this, TQT_SLOT(slotSelectionChanged(TQListViewItem*)) );
  connect( m_doc->audioDoc(), TQT_SIGNAL(changed()), this, TQT_SLOT(slotNewAudioTracks()) );
}


K3bMixedDirTreeView::~K3bMixedDirTreeView()
{
}


void K3bMixedDirTreeView::slotDropped( TQDropEvent* e, TQListViewItem* tqparent, TQListViewItem* after )
{
  if( !e->isAccepted() )
    return;

  TQListViewItem* droppedItem = itemAt(e->pos());
  if( droppedItem == m_audioRootItem ) {
    KURL::List urls;
    if( KURLDrag::decode( e, urls ) ) {
      K3bAudioTrackAddingDialog::addUrls( urls, m_doc->audioDoc(), 0, 0, 0, this );
    }
  }
  else
    K3bDataDirTreeView::slotDropped( e, tqparent, after );
}


void K3bMixedDirTreeView::slotSelectionChanged( TQListViewItem* i )
{
  if( i == m_audioRootItem )
    emit audioTreeSelected();
  else
    emit dataTreeSelected();
}


void K3bMixedDirTreeView::slotNewAudioTracks()
{
  // update the tracknumber
  m_audioRootItem->tqrepaint();
}

#include "k3bmixeddirtreeview.moc"
