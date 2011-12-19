/*
*
* $Id: k3bvideocdview.cpp 619556 2007-01-03 17:38:12Z trueg $
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

// kde includes
#include <kaction.h>
#include <kcutlabel.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstdaction.h>

// qt includes
#include <tqfont.h>
#include <tqframe.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcursor.h>
#include <tqapplication.h>

// k3b includes
#include "k3bvideocdview.h"
#include "k3bvideocdrippingdialog.h"

#include <k3bdevice.h>
#include <k3bmsf.h>
#include <k3btoc.h>
#include <k3bcore.h>
#include <k3blistview.h>
#include <k3bstdguiitems.h>
#include <k3btoolbox.h>


class K3bVideoCdView::VideoTrackViewItem : public TQListViewItem
{
    public:
        VideoTrackViewItem( TQListViewItem* parent, TQListViewItem* after )
                : TQListViewItem( parent, after )
        {
            setSelectable( false );
        }

        VideoTrackViewItem( TQListView* parent, TQListViewItem* after )
                : TQListViewItem( parent, after )
        {
            setSelectable( false );
        }
        
        VideoTrackViewItem( TQListViewItem* parent,
                            const TQString& name,
                            const TQString& id,
                            int _trackNumber,
                            const K3b::Msf& length )
                : TQListViewItem( parent )
        {
            setText( 0, TQString( "%1. %2" ).arg( _trackNumber ).arg( id ) );
            setText( 1, name );
            if ( length > 0 ) {
                setText( 2, length.toString() );
                setText( 3, KIO::convertSize( length.mode2Form2Bytes() ) );
            }

            trackNumber = _trackNumber;
            setSelectable( false );
        }

        int trackNumber;

        void updateData( const K3bVideoCdInfoResultEntry& resultEntry )
        {
            setText( 0, TQString( "%1. %2" ).arg( trackNumber ).arg( resultEntry.id ) );
            setText( 1, resultEntry.name );
        }

};

class K3bVideoCdView::VideoTrackViewCheckItem : public TQCheckListItem
{
    public:
        VideoTrackViewCheckItem( TQListViewItem* parent,
                                 const TQString& desc )
                : TQCheckListItem( parent,
                                  TQString(),
                                  TQCheckListItem::CheckBox )
        {
            setText( 0, desc );

            setOn( true );
        }

        VideoTrackViewCheckItem( TQListView* parent,
                                 const TQString& desc )
                : TQCheckListItem( parent,
                                  TQString(),
                                  TQCheckListItem::CheckBox )
        {
            setText( 0, desc );

            setOn( true );
        }

        VideoTrackViewCheckItem( VideoTrackViewCheckItem* parent,
                                 const TQString& desc )
                : TQCheckListItem( parent,
                                  TQString(),
                                  TQCheckListItem::CheckBox )
        {
            setText( 0, desc );

            setOn( true );
        }

        void updateData( const K3b::Msf& length, bool form2 = false )
        {
            setText( 2, length.toString() );
            if ( form2 )
                setText( 3, KIO::convertSize( length.mode2Form2Bytes() ) );
            else
                setText( 3, KIO::convertSize( length.mode2Form1Bytes() ) );
        }

};

K3bVideoCdView::K3bVideoCdView( TQWidget* parent, const char *name )
        : K3bMediaContentsView( true,
				K3bMedium::CONTENT_VIDEO_CD,
				K3bDevice::MEDIA_CD_ALL,
				K3bDevice::STATE_INCOMPLETE|K3bDevice::STATE_COMPLETE,
				parent, name )
{
    TQGridLayout * mainGrid = new TQGridLayout( mainWidget() );

    // toolbox
    // ----------------------------------------------------------------------------------
    TQHBoxLayout* toolBoxLayout = new TQHBoxLayout( 0, 0, 0, "toolBoxLayout" );
    m_toolBox = new K3bToolBox( mainWidget() );
    toolBoxLayout->addWidget( m_toolBox );
    TQSpacerItem* spacer = new TQSpacerItem( 10, 10, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
    toolBoxLayout->addItem( spacer );
    m_labelLength = new TQLabel( mainWidget() );
    m_labelLength->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );
    toolBoxLayout->addWidget( m_labelLength );

    // the track view
    // ----------------------------------------------------------------------------------
    m_trackView = new K3bListView( mainWidget() );
    m_trackView->setFullWidth( true );
    m_trackView->setAllColumnsShowFocus( true );
    m_trackView->setSelectionMode( TQListView::Single );
    m_trackView->setDragEnabled( true );
    m_trackView->addColumn( i18n( "Item Name" ) );
    m_trackView->addColumn( i18n( "Extracted Name" ) );
    m_trackView->addColumn( i18n( "Length" ) );
    m_trackView->addColumn( i18n( "Size" ) );

    m_trackView->header() ->setClickEnabled( false );

    m_trackView->setItemsRenameable( false );
    m_trackView->setRootIsDecorated( true );

    connect( m_trackView, TQT_SIGNAL( contextMenu( KListView*, TQListViewItem*, const TQPoint& ) ),
             this, TQT_SLOT( slotContextMenu( KListView*, TQListViewItem*, const TQPoint& ) ) );
    connect( m_trackView, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
             this, TQT_SLOT( slotTrackSelectionChanged( TQListViewItem* ) ) );
    connect( m_trackView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
             this, TQT_SLOT( slotStateChanged( TQListViewItem* ) ) );
    connect( m_trackView, TQT_SIGNAL( spacePressed( TQListViewItem* ) ),
             this, TQT_SLOT( slotStateChanged( TQListViewItem* ) ) );


    mainGrid->addLayout( toolBoxLayout, 0, 0 );
    mainGrid->addWidget( m_trackView, 1, 0 );

    initActions();
    slotTrackSelectionChanged( 0 );

    m_videocdinfo = 0L;
    m_videooptions = new K3bVideoCdRippingOptions();
    
    m_contentList.clear();
}


K3bVideoCdView::~K3bVideoCdView()
{
    delete m_videocdinfo;
    delete m_videooptions;
}


void K3bVideoCdView::reloadMedium()
{
    m_toc = medium().toc();

    m_trackView->clear();

    m_trackView->setEnabled( false );
    m_toolBox->setEnabled( false );
    TQApplication::setOverrideCursor( TQCursor(TQt::WaitCursor) );

    m_contentList.append( new VideoTrackViewCheckItem( m_trackView, i18n("Video CD MPEG tracks") ) );
    m_contentList.append( new VideoTrackViewCheckItem( m_trackView, i18n("Video CD DATA track" ) ) );

    ( ( VideoTrackViewCheckItem* ) m_contentList[ 0 ] ) ->setOpen( true );

    // create a listviewItem for every video track
    int index = 0;
    m_videocddatasize = 0;
    m_videocdmpegsize = 0;
            
    K3b::Msf sequenceSize;

    for ( K3bDevice::Toc::const_iterator it = m_toc.begin();
            it != m_toc.end(); ++it ) {

        if ( index > 0 ) {
            K3b::Msf length( ( *it ).length() );
            sequenceSize += length;
            m_videocdmpegsize += length.mode2Form2Bytes();
            ( void ) new VideoTrackViewItem( ( VideoTrackViewCheckItem* ) m_contentList[ 0 ], i18n( "Sequence-%1" ).arg( index ), "", index, length );
        } else {
            K3b::Msf length( ( *it ).length() );
            m_videocddatasize += length.mode2Form1Bytes();
            ( ( VideoTrackViewCheckItem* ) m_contentList[ 1 ] ) ->updateData( length );
            ( void ) new VideoTrackViewCheckItem( ( VideoTrackViewCheckItem* ) m_contentList[ 1 ], i18n( "Files" ) );
            ( void ) new VideoTrackViewCheckItem( ( VideoTrackViewCheckItem* ) m_contentList[ 1 ], i18n( "Segments" ) );
        }

        index++;
    }

    ( ( VideoTrackViewCheckItem* ) m_contentList[ 0 ] ) ->updateData( sequenceSize, true );

    m_videooptions ->setVideoCdSource( device()->devicename() );
    
    m_videocdinfo = new K3bVideoCdInfo( TQT_TQOBJECT(this) );
    m_videocdinfo->info( device()->devicename() );

    connect( m_videocdinfo, TQT_SIGNAL( infoFinished( bool ) ),
             this, TQT_SLOT( slotVideoCdInfoFinished( bool ) ) );

}

void K3bVideoCdView::slotVideoCdInfoFinished( bool success )
{
    if ( success ) {
        m_videocdinfoResult = m_videocdinfo->result();
        updateDisplay();
    }

    m_trackView->setEnabled( true );
    m_toolBox->setEnabled( true );
    TQApplication::restoreOverrideCursor();

}

void K3bVideoCdView::updateDisplay()
{
    // update the listview

    VideoTrackViewItem * item = ( VideoTrackViewItem* ) m_contentList[ 0 ] ->firstChild();
    int index = 0;
    while ( item ) {
        item->updateData( m_videocdinfoResult.entry( index, K3bVideoCdInfoResult::SEQUENCE ) );
        item = ( VideoTrackViewItem* ) item->nextSibling();
        index++;
    }

    VideoTrackViewCheckItem* check_item = ( VideoTrackViewCheckItem* ) m_contentList[ 1 ] ->firstChild();
    while ( check_item ) {
        if ( check_item->key( 0, false ).compare( i18n( "Files" ) ) == 0 ) {
            if ( domTree.setContent( m_videocdinfoResult.xmlData ) ) {

                TQDomElement root = domTree.documentElement();
                TQDomNode node;
                node = root.firstChild();
                while ( !node.isNull() ) {
                    if ( node.isElement() && node.nodeName() == "filesystem" ) {
                        TQDomElement body = node.toElement();
                        buildTree( check_item, body );
                        break;
                    }
                    node = node.nextSibling();
                }
            }
        } else {
            for ( index = 0; index < m_videocdinfoResult.foundEntries( K3bVideoCdInfoResult::SEGMENT ); index++ ) {
                ( void ) new VideoTrackViewItem( check_item, m_videocdinfoResult.entry( index, K3bVideoCdInfoResult::SEGMENT ).name, m_videocdinfoResult.entry( index, K3bVideoCdInfoResult::SEGMENT ).id , index + 1, 0 );
            }
        }
        check_item = ( VideoTrackViewCheckItem* ) check_item->nextSibling();
    }

    if ( !m_videocdinfoResult.volumeId.isEmpty() ) {
        TQString description = m_videocdinfoResult.volumeId + " (" + m_videocdinfoResult.type + " " + m_videocdinfoResult.version + ")" ;
        setTitle( description );
        m_videooptions ->setVideoCdDescription( description );
    }
    else
        setTitle( i18n( "Video CD" ) );

    m_labelLength->setText( i18n( "1 track (%1)", "%n tracks (%1)", m_toc.count() ).arg( K3b::Msf( m_toc.length() ).toString() ) );
}


void K3bVideoCdView::initActions()
{
    m_actionCollection = new KActionCollection( this );

    KAction* actionSelectAll = KStdAction::selectAll( TQT_TQOBJECT(this), TQT_SLOT( slotSelectAll() ),
                               m_actionCollection, "select_all" );
    KAction* actionDeselectAll = KStdAction::deselect( TQT_TQOBJECT(this), TQT_SLOT( slotDeselectAll() ),
                                 m_actionCollection, "deselect_all" );
    actionDeselectAll->setText( i18n( "Dese&lect All" ) );
    KAction* actionSelect = new KAction( i18n( "Select Track" ), 0, 0, TQT_TQOBJECT(this),
                                         TQT_SLOT( slotSelect() ), actionCollection(),
                                         "select_track" );
    KAction* actionDeselect = new KAction( i18n( "Deselect Track" ), 0, 0, TQT_TQOBJECT(this),
                                           TQT_SLOT( slotDeselect() ), actionCollection(),
                                           "deselect_track" );

    KAction* actionStartRip = new KAction( i18n( "Start Ripping" ), "run", 0, TQT_TQOBJECT(this),
                                           TQT_SLOT( startRip() ), actionCollection(), "start_rip" );

    // TODO: set the actions tooltips and whatsthis infos

    // setup the popup menu
    m_popupMenu = new KActionMenu( actionCollection(), "popup_menu" );
    KAction* separator = new KActionSeparator( actionCollection(), "separator" );
    m_popupMenu->insert( actionSelect );
    m_popupMenu->insert( actionDeselect );
    m_popupMenu->insert( actionSelectAll );
    m_popupMenu->insert( actionDeselectAll );
    m_popupMenu->insert( separator );
    m_popupMenu->insert( actionStartRip );

    // setup the toolbox
    m_toolBox->addButton( actionStartRip, true );
}


void K3bVideoCdView::slotContextMenu( KListView*, TQListViewItem*, const TQPoint& p )
{
    m_popupMenu->popup( p );
}


void K3bVideoCdView::slotTrackSelectionChanged( TQListViewItem* item )
{
    actionCollection() ->action( "select_track" ) ->setEnabled( item != 0 );
    actionCollection() ->action( "deselect_track" ) ->setEnabled( item != 0 );
}

void K3bVideoCdView::slotStateChanged( TQListViewItem* item )
{
    /* > QT 3.1
    if ( !item == 0 && item ->isSelectable() ) {
        if ( ( ( VideoTrackViewCheckItem* ) item) ->state() == TQCheckListItem::On)
            slotSelect();
        else if ( ( ( VideoTrackViewCheckItem* ) item) ->state() == TQCheckListItem::Off)
            slotDeselect();
    }
    */
    if ( !item == 0 && item ->isSelectable() ) {
        if ( ( ( VideoTrackViewCheckItem* ) item) ->isOn() )
            slotSelect();
        else
            slotDeselect();
    }
}

void K3bVideoCdView::startRip()
{

    int selectedItems  = 0;
    for ( TQListViewItemIterator it( m_trackView ); it.current(); ++it ) {
        if ( it.current() ->isSelectable() ) {
            if ( ( ( ( VideoTrackViewCheckItem* ) it.current()) ->key( 0, false ).compare( i18n("Video CD MPEG tracks" ) ) == 0 ) && ( ( VideoTrackViewCheckItem* ) it.current() ) ->isOn() ) {
                m_videooptions ->setVideoCdRipSequences( true );
                selectedItems++;
            }
            else if ( ( ( ( VideoTrackViewCheckItem* ) it.current()) ->key( 0, false ).compare( i18n("Files" ) ) == 0 ) && ( ( VideoTrackViewCheckItem* ) it.current() ) ->isOn() ) {
                m_videooptions ->setVideoCdRipFiles( true );
                selectedItems++;
            }
            else if ( ( ( ( VideoTrackViewCheckItem* ) it.current()) ->key( 0, false ).compare( i18n("Segments" ) ) == 0 ) && ( ( VideoTrackViewCheckItem* ) it.current() ) ->isOn() ) {
                m_videooptions ->setVideoCdRipSegments( true );
                selectedItems++;
            }
        }
    }

    if( selectedItems == 0 ) {
        KMessageBox::error( this, i18n("Please select the tracks to rip."), i18n("No Tracks Selected") );
    }
    else {
        unsigned long videocdsize = 0;
        // TODO: split SegmentSize and FileSize. Have no infos now
        if ( m_videooptions ->getVideoCdRipSegments() || m_videooptions ->getVideoCdRipFiles())
            videocdsize += m_videocddatasize;
        if ( m_videooptions ->getVideoCdRipSequences() )
            videocdsize += m_videocdmpegsize;

        kdDebug() << TQString("(K3bVideoCdView::startRip())  m_videooptions ->setVideoCdSize( %1)").arg( videocdsize ) << endl;
        m_videooptions ->setVideoCdSize( videocdsize );
        K3bVideoCdRippingDialog rip( m_videooptions, this );
        rip.exec();
    }
}

void K3bVideoCdView::slotSelectAll()
{
    for ( TQListViewItemIterator it( m_trackView ); it.current(); ++it )
        if ( it.current() ->isSelectable() )
            ( ( VideoTrackViewCheckItem* ) it.current() ) ->setOn( true );
}

void K3bVideoCdView::slotDeselectAll()
{
    for ( TQListViewItemIterator it( m_trackView ); it.current(); ++it )
        if ( it.current() ->isSelectable() )
            ( ( VideoTrackViewCheckItem* ) it.current() ) ->setOn( false );
}

void K3bVideoCdView::slotSelect()
{
    if ( TQListViewItem * sel = m_trackView->selectedItem() ) {
        ( ( VideoTrackViewCheckItem* ) sel) ->setOn( true );
        TQListViewItem * item = sel ->firstChild();
        while ( item ) {
            if ( item ->isSelectable() )
                ( ( VideoTrackViewCheckItem* ) item) ->setOn( true );

            item = item->nextSibling();
        }
    }
}

void K3bVideoCdView::slotDeselect()
{
    if ( TQListViewItem * sel = m_trackView->selectedItem() ) {
        ( ( VideoTrackViewCheckItem* ) sel) ->setOn( false );
        TQListViewItem * item = sel ->firstChild();
        while ( item ) {
            if ( item ->isSelectable() )
                ( ( VideoTrackViewCheckItem* ) item) ->setOn( false );

            item = item->nextSibling();
        }
    }
}

void K3bVideoCdView::enableInteraction( bool b )
{
  actionCollection()->action( "start_rip" )->setEnabled( b );
}

void K3bVideoCdView::buildTree( TQListViewItem *parentItem, const TQDomElement &parentElement, const TQString& pname )
{
    VideoTrackViewItem * thisItem = 0;
    TQDomNode node = parentElement.firstChild();

    while ( !node.isNull() ) {
        if ( node.isElement() && node.nodeName() == "folder" || node.nodeName() == "file" ) {
            if ( parentItem == 0 )
                thisItem = new VideoTrackViewItem( m_trackView, thisItem );
            else
                thisItem = new VideoTrackViewItem( parentItem, thisItem );

            TQString txt = node.firstChild().toElement().text();
            thisItem->setText( 0, txt);
            if ( node.nodeName() == "folder" ) {
		buildTree( thisItem, node.toElement(), pname + "_" + txt.lower() );
            }
            else {
                thisItem->setText( 1, pname + "_" + txt.lower() );
		buildTree( thisItem, node.toElement(), pname );
            }
        } else if ( node.isElement() && node.nodeName() == "segment-item" || node.nodeName() == "sequence-item" ) {
            if ( parentItem == 0 )
                thisItem = new VideoTrackViewItem( m_trackView, thisItem );
            else
                thisItem = new VideoTrackViewItem( parentItem, thisItem );

            thisItem->setText( 0, node.toElement().attribute( "src" ) );

            buildTree( thisItem, node.toElement() );
        }

        node = node.nextSibling();
    }
}

#include "k3bvideocdview.moc"
