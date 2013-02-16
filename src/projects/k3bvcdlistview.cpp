/*
*
* $Id: k3bvcdlistview.cpp 628165 2007-01-29 11:01:22Z trueg $
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

#include <tqheader.h>
#include <tqtimer.h>
#include <tqdragobject.h>
#include <tqpoint.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqevent.h>
#include <tqpainter.h>
#include <tqfontmetrics.h>

#include <kiconloader.h>
#include <kurl.h>
#include <kurldrag.h>
#include <tdelocale.h>
#include <tdeaction.h>
#include <tdepopupmenu.h>
#include <kdialog.h>

// K3b Includes
#include "k3bvcdlistview.h"
#include "k3bvcdlistviewitem.h"
#include "k3bvcdtrack.h"
#include "k3bvcdtrackdialog.h"
#include "k3bvcddoc.h"
#include <k3bview.h>

K3bVcdListView::K3bVcdListView( K3bView* view, K3bVcdDoc* doc, TQWidget *parent, const char *name )
        : K3bListView( parent, name ), m_doc( doc ), m_view( view )
{
    setAcceptDrops( true );
    setDropVisualizer( true );
    setAllColumnsShowFocus( true );
    setDragEnabled( true );
    setSelectionModeExt( TDEListView::Extended );
    setItemsMovable( false );

    setNoItemText( i18n( "Use drag'n'drop to add MPEG video files to the project." ) + "\n"
                   + i18n( "After that press the burn button to write the CD." ) );

    setSorting( 0 );

    setupActions();
    setupPopupMenu();

    setupColumns();
    header() ->setClickEnabled( false );

    connect( this, TQT_SIGNAL( dropped( TDEListView*, TQDropEvent*, TQListViewItem* ) ),
             this, TQT_SLOT( slotDropped( TDEListView*, TQDropEvent*, TQListViewItem* ) ) );
    connect( this, TQT_SIGNAL( contextMenu( TDEListView*, TQListViewItem*, const TQPoint& ) ),
             this, TQT_SLOT( showPopupMenu( TDEListView*, TQListViewItem*, const TQPoint& ) ) );
    connect( this, TQT_SIGNAL( doubleClicked( TQListViewItem*, const TQPoint&, int ) ),
             this, TQT_SLOT( showPropertiesDialog() ) );

    connect( m_doc, TQT_SIGNAL( changed() ), TQT_TQOBJECT(this), TQT_SLOT( slotUpdateItems() ) );
    connect( m_doc, TQT_SIGNAL( trackRemoved( K3bVcdTrack* ) ), TQT_TQOBJECT(this), TQT_SLOT( slotTrackRemoved( K3bVcdTrack* ) ) );

    slotUpdateItems();
}

K3bVcdListView::~K3bVcdListView()
{}

void K3bVcdListView::setupColumns()
{
    addColumn( i18n( "No." ) );
    addColumn( i18n( "Title" ) );
    addColumn( i18n( "Type" ) );
    addColumn( i18n( "Resolution" ) );
    addColumn( i18n( "High Resolution" ) );
    addColumn( i18n( "Framerate" ) );
    addColumn( i18n( "Muxrate" ) );
    addColumn( i18n( "Duration" ) );
    addColumn( i18n( "File Size" ) );
    addColumn( i18n( "Filename" ) );
}


void K3bVcdListView::setupActions()
{
    m_actionCollection = new TDEActionCollection( this );
    m_actionProperties = new TDEAction( i18n( "Properties" ), "misc", 0, TQT_TQOBJECT(this), TQT_SLOT( showPropertiesDialog() ), actionCollection() );
    m_actionRemove = new TDEAction( i18n( "Remove" ), "editdelete", Key_Delete, TQT_TQOBJECT(this), TQT_SLOT( slotRemoveTracks() ), actionCollection() );

    // disabled by default
    m_actionRemove->setEnabled( false );
}


void K3bVcdListView::setupPopupMenu()
{
    m_popupMenu = new TDEPopupMenu( this, "VcdViewPopupMenu" );
    m_actionRemove->plug( m_popupMenu );
    m_popupMenu->insertSeparator();
    m_actionProperties->plug( m_popupMenu );
    m_popupMenu->insertSeparator();
    m_view->actionCollection() ->action( "project_burn" ) ->plug( m_popupMenu );
}


bool K3bVcdListView::acceptDrag( TQDropEvent* e ) const
{
    // the first is for built-in item moving, the second for dropping urls
    return ( TDEListView::acceptDrag( e ) || KURLDrag::canDecode( e ) );
}


TQDragObject* K3bVcdListView::dragObject()
{
    TQPtrList<TQListViewItem> list = selectedItems();

    if ( list.isEmpty() )
        return 0;

    TQPtrListIterator<TQListViewItem> it( list );
    KURL::List urls;

    for ( ; it.current(); ++it )
        urls.append( KURL( ( ( K3bVcdListViewItem* ) it.current() ) ->vcdTrack() ->absPath() ) );

    return KURLDrag::newDrag( urls, viewport() );
}


void K3bVcdListView::slotDropped( TDEListView*, TQDropEvent* e, TQListViewItem* after )
{
    if ( !e->isAccepted() )
        return ;

    int pos;
    if ( after == 0L )
        pos = 0;
    else
        pos = ( ( K3bVcdListViewItem* ) after ) ->vcdTrack() ->index() + 1;

    if ( e->source() == viewport() ) {
        TQPtrList<TQListViewItem> sel = selectedItems();
        TQPtrListIterator<TQListViewItem> it( sel );
        K3bVcdTrack* trackAfter = ( after ? ( ( K3bVcdListViewItem* ) after ) ->vcdTrack() : 0 );
        while ( it.current() ) {
            K3bVcdTrack * track = ( ( K3bVcdListViewItem* ) it.current() ) ->vcdTrack();
            m_doc->moveTrack( track, trackAfter );
            trackAfter = track;
            ++it;
        }
    } else {
        KURL::List urls;
        KURLDrag::decode( e, urls );

        m_doc->addTracks( urls, pos );
    }

  // now grab that focus
  setFocus();
}


void K3bVcdListView::insertItem( TQListViewItem* item )
{
    TDEListView::insertItem( item );

    // make sure at least one item is selected
    if ( selectedItems().isEmpty() ) {
        setSelected( firstChild(), true );
    }
}

void K3bVcdListView::showPopupMenu( TDEListView*, TQListViewItem* _item, const TQPoint& _point )
{
    if ( _item ) {
        m_actionRemove->setEnabled( true );
    } else {
        m_actionRemove->setEnabled( false );
    }

    m_popupMenu->popup( _point );
}

void K3bVcdListView::showPropertiesDialog()
{
    TQPtrList<K3bVcdTrack> selected = selectedTracks();
    if ( !selected.isEmpty() && selected.count() == 1 ) {
        TQPtrList<K3bVcdTrack> tracks = *m_doc->tracks();
        K3bVcdTrackDialog d( m_doc, tracks, selected, this );
        if ( d.exec() ) {
            repaint();
        }
    } else {
      m_view->slotProperties();
    }
}

TQPtrList<K3bVcdTrack> K3bVcdListView::selectedTracks()
{
    TQPtrList<K3bVcdTrack> selectedTracks;
    TQPtrList<TQListViewItem> selectedVI( selectedItems() );
    for ( TQListViewItem * item = selectedVI.first(); item != 0; item = selectedVI.next() ) {
        K3bVcdListViewItem * vcdItem = dynamic_cast<K3bVcdListViewItem*>( item );
        if ( vcdItem ) {
            selectedTracks.append( vcdItem->vcdTrack() );
        }
    }

    return selectedTracks;
}


void K3bVcdListView::slotRemoveTracks()
{
    TQPtrList<K3bVcdTrack> selected = selectedTracks();
    if ( !selected.isEmpty() ) {

        for ( K3bVcdTrack * track = selected.first(); track != 0; track = selected.next() ) {
            m_doc->removeTrack( track );
        }
    }

    if ( m_doc->numOfTracks() == 0 ) {
        m_actionRemove->setEnabled( false );
    }
}


void K3bVcdListView::slotTrackRemoved( K3bVcdTrack* track )
{
    TQListViewItem * viewItem = m_itemMap[ track ];
    m_itemMap.remove( track );
    delete viewItem;
}


void K3bVcdListView::slotUpdateItems()
{
    // iterate through all doc-tracks and test if we have a listItem, if not, create one
    K3bVcdTrack * track = m_doc->first();
    K3bVcdTrack* lastTrack = 0;
    while ( track != 0 ) {
        if ( !m_itemMap.contains( track ) )
            m_itemMap.insert( track, new K3bVcdListViewItem( track, this, m_itemMap[ lastTrack ] ) );

        lastTrack = track;
        track = m_doc->next();
    }

    if ( m_doc->numOfTracks() > 0 ) {
        m_actionRemove->setEnabled( true );
    } else {
        m_actionRemove->setEnabled( false );
    }

    sort();  // This is so lame!

    header()->setShown( m_doc->numOfTracks() > 0 );
}

#include "k3bvcdlistview.moc"
