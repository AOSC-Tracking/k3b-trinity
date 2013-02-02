/*
 *
 * $Id: k3bdatafileview.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bdatafileview.h"
#include "k3bdataview.h"
#include <k3bdatadoc.h>
#include <k3bdataitem.h>
#include <k3bdiritem.h>
#include <k3bfileitem.h>
#include <k3bspecialdataitem.h>
#include <k3bsessionimportitem.h>
#include "k3bdataurladdingdialog.h"
#include <k3bvalidators.h>
#include "k3bdatapropertiesdialog.h"
#include "k3bdatadirtreeview.h"
#include "k3bdataviewitem.h"
#include <k3bview.h>


#include <tqdragobject.h>
#include <tqpainter.h>
#include <tqfontmetrics.h>
#include <tqtimer.h>
#include <tqheader.h>
#include <tqfileinfo.h>

#include <klocale.h>
#include <tdeaction.h>
#include <kurldrag.h>
#include <kinputdialog.h>
#include <kdebug.h>
#include <tdeshortcut.h>
#include <krun.h>
#include <tdeversion.h>


K3bDataFileView::K3bDataFileView( K3bView* view, K3bDataDirTreeView* dirTreeView, K3bDataDoc* doc, TQWidget* parent )
  : K3bListView( parent ), 
    m_view(view),
    m_dropDirItem(0)
{
  m_treeView = dirTreeView;

  setAcceptDrops( true );
  setDropVisualizer( false );
  setDropHighlighter( true );
  setDragEnabled( true );
  setItemsMovable( false );
  setAllColumnsShowFocus( true );
  setShowSortIndicator( true );

  setNoItemText( i18n("Use drag'n'drop to add files and directories to the project.\n"
		      "To remove or rename files use the context menu.\n"
		      "After that press the burn button to write the CD.") );


  addColumn( i18n("Name") );
  addColumn( i18n("Type") );
  addColumn( i18n("Size") );
  addColumn( i18n("Local Path") );
  addColumn( i18n("Link") );

  setSelectionModeExt( TDEListView::Extended );

  m_doc = doc;
  m_currentDir = doc->root();
  checkForNewItems();

  connect( m_treeView, TQT_SIGNAL(dirSelected(K3bDirItem*)), TQT_TQOBJECT(this), TQT_SLOT(slotSetCurrentDir(K3bDirItem*)) );
  connect( m_doc, TQT_SIGNAL(itemRemoved(K3bDataItem*)), TQT_TQOBJECT(this), TQT_SLOT(slotDataItemRemoved(K3bDataItem*)) );
  connect( m_doc, TQT_SIGNAL(itemAdded(K3bDataItem*)), TQT_TQOBJECT(this), TQT_SLOT(slotItemAdded(K3bDataItem*)) );
  connect( this, TQT_SIGNAL(executed(TQListViewItem*)), TQT_TQOBJECT(this), TQT_SLOT(slotExecuted(TQListViewItem*)) );
  connect( this, TQT_SIGNAL(contextMenu(TDEListView*, TQListViewItem*, const TQPoint&)),
	   this, TQT_SLOT(showPopupMenu(TDEListView*, TQListViewItem*, const TQPoint&)) );
  connect( this, TQT_SIGNAL(dropped(TQDropEvent*, TQListViewItem*, TQListViewItem*)),
	   this, TQT_SLOT(slotDropped(TQDropEvent*, TQListViewItem*, TQListViewItem*)) );
  connect( this, TQT_SIGNAL(doubleClicked(TQListViewItem*, const TQPoint&, int)),
	   this, TQT_SLOT(slotDoubleClicked(TQListViewItem*)) );

  setupActions();
}


K3bDataFileView::~K3bDataFileView()
{
}


K3bDirItem* K3bDataFileView::currentDir() const
{ 
  if( !m_currentDir )
    m_currentDir = m_doc->root();
  return m_currentDir;
}


void K3bDataFileView::slotSetCurrentDir( K3bDirItem* dir )
{
  if( dir ) {
    m_currentDir = dir;
    clearItems();
    checkForNewItems();
  }
}


void K3bDataFileView::clearItems()
{
  m_itemMap.clear();
  K3bListView::clear();
}


void K3bDataFileView::slotItemAdded( K3bDataItem* item )
{
  if( item->parent() == currentDir() ) {
    K3bDataViewItem* vi = 0;
    if( item->isDir() )
      vi = new K3bDataDirViewItem( static_cast<K3bDirItem*>(item), this );
    else if( item->isFile() )
      vi = new K3bDataFileViewItem( static_cast<K3bFileItem*>(item), this );
    else if( item->isSpecialFile() )
      vi = new K3bSpecialDataViewItem( static_cast<K3bSpecialDataItem*>(item), this );
    else if( item->isFromOldSession() )
      vi = new K3bSessionImportViewItem( static_cast<K3bSessionImportItem*>(item), this );
    else
      kdDebug() << "(K3bDataFileView) ERROR: unknown data item type" << endl;
    
    if( vi )
      m_itemMap[item] = vi;
  }
}


void K3bDataFileView::slotDataItemRemoved( K3bDataItem* item )
{
  if( item->isDir() ) {
    if( static_cast<K3bDirItem*>(item)->isSubItem( currentDir() ) ) {
      slotSetCurrentDir( m_doc->root() );
    }
  }
  
  if( m_itemMap.contains( item ) ) {
    delete m_itemMap[item];
    m_itemMap.remove(item);
  }
}


void K3bDataFileView::checkForNewItems()
{
  hideEditor();

  // add items that are not there yet
  for( TQPtrListIterator<K3bDataItem> it( m_currentDir->children() ); it.current(); ++it ) {
    if( !m_itemMap.contains( it.current() ) ) {
      slotItemAdded( it.current() );
    }
  }

  // now check if some of the items have been moved out of the currently showing dir.
  for( TQListViewItemIterator it( this ); it.current(); ++it ) {
    K3bDataViewItem* dataViewItem = dynamic_cast<K3bDataViewItem*>( it.current() );
    if( dataViewItem && dataViewItem->dataItem()->parent() != currentDir() )
      delete dataViewItem;
  }  
}


TQDragObject* K3bDataFileView::dragObject()
{
  TQPtrList<TQListViewItem> selectedViewItems = selectedItems();
  KURL::List urls;
  for( TQPtrListIterator<TQListViewItem> it( selectedViewItems ); it.current(); ++it ) {
    K3bDataViewItem* dataViewItem = dynamic_cast<K3bDataViewItem*>( it.current() );
    if( dataViewItem ) {
      urls.append( KURL::fromPathOrURL(dataViewItem->dataItem()->localPath()) );
    }
    else
      kdDebug() << "no dataviewitem" << endl;
  }

  if( urls.isEmpty() )
    return 0;

  return KURLDrag::newDrag( urls, viewport() );
}


bool K3bDataFileView::acceptDrag(TQDropEvent* e) const
{
  return ( e->source() == viewport() || 
	   KURLDrag::canDecode(e) || 
	   e->source() == m_treeView->viewport() );
}


void K3bDataFileView::contentsDragMoveEvent( TQDragMoveEvent* e )
{
  K3bListView::contentsDragMoveEvent( e );

  // highlight the folder the items would be added to
  if( m_dropDirItem )
    m_dropDirItem->highlightIcon( false );

  m_dropDirItem = dynamic_cast<K3bDataDirViewItem*>( itemAt(contentsToViewport(e->pos())) );
  if( m_dropDirItem )
    m_dropDirItem->highlightIcon( true );
}


void K3bDataFileView::contentsDragLeaveEvent( TQDragLeaveEvent* e )
{
  K3bListView::contentsDragLeaveEvent( e );

  // remove any highlighting
  if( m_dropDirItem ) {
    m_dropDirItem->highlightIcon( false );
    m_dropDirItem = 0;
  }
}


void K3bDataFileView::slotDropped( TQDropEvent* e, TQListViewItem*, TQListViewItem* )
{
  // remove any highlighting
  if( m_dropDirItem ) {
    m_dropDirItem->highlightIcon( false );
    m_dropDirItem = 0;
  }

  if( !e->isAccepted() )
    return;

  // determine K3bDirItem to add the items to
  m_addParentDir = currentDir();

  if( K3bDataDirViewItem* dirViewItem = dynamic_cast<K3bDataDirViewItem*>( itemAt(contentsToViewport(e->pos())) ) ) {
    // only add to a dir if we drop directly on the name
    if( header()->sectionAt( e->pos().x() ) == 0 )
      m_addParentDir = dirViewItem->dirItem();
  }

  if( m_addParentDir ) {

    // check if items have been moved
    if( e->source() == viewport() ) {
      // move all selected items
      TQPtrList<TQListViewItem> selectedViewItems = selectedItems();
      TQValueList<K3bDataItem*> selectedDataItems;
      TQPtrListIterator<TQListViewItem> it( selectedViewItems );
      for( ; it.current(); ++it ) {
	K3bDataViewItem* dataViewItem = dynamic_cast<K3bDataViewItem*>( it.current() );
	if( dataViewItem )
	  selectedDataItems.append( dataViewItem->dataItem() );
	else
	  kdDebug() << "no dataviewitem" << endl;
      }

      K3bDataUrlAddingDialog::copyMoveItems( selectedDataItems, m_addParentDir, this, e->action() == TQDropEvent::Copy );
    }
    else if( e->source() == m_treeView->viewport() ) {
      // move the selected dir
      if( K3bDataDirViewItem* dirItem = dynamic_cast<K3bDataDirViewItem*>( m_treeView->selectedItem() ) ) {
	TQValueList<K3bDataItem*> selectedDataItems;
	selectedDataItems.append( dirItem->dirItem() );
	K3bDataUrlAddingDialog::copyMoveItems( selectedDataItems, m_addParentDir, this, e->action() == TQDropEvent::Copy );
      }
    }
    else {
      // seems that new items have been dropped
      m_addUrls.clear();
      if( KURLDrag::decode( e, m_addUrls ) ) {
	//
	// This is a small (not to ugly) hack to circumvent problems with the
	// event queues: the url adding dialog will be non-modal regardless of
	// the settings in case we open it directly.
	//
	TQTimer::singleShot( 0, TQT_TQOBJECT(this), TQT_SLOT(slotAddUrls()) );
      }
    }
  }

  // now grab that focus
  setFocus();
}


void K3bDataFileView::slotAddUrls()
{
  K3bDataUrlAddingDialog::addUrls( m_addUrls, m_addParentDir, this );
}


void K3bDataFileView::slotExecuted( TQListViewItem* item )
{
  if( K3bDataDirViewItem* k = dynamic_cast<K3bDataDirViewItem*>( item ) ) {
    hideEditor();  // disable the K3bListView Editor
    slotSetCurrentDir( k->dirItem() );
    emit dirSelected( currentDir() );
  }
}


void K3bDataFileView::setupActions()
{
  m_actionCollection = new TDEActionCollection( this );

  m_actionProperties = new TDEAction( i18n("Properties"), "misc", 0, TQT_TQOBJECT(this), TQT_SLOT(slotProperties()),
				    actionCollection(), "properties" );
  m_actionNewDir = new TDEAction( i18n("New Directory..."), "folder_new", CTRL+Key_N, TQT_TQOBJECT(this), TQT_SLOT(slotNewDir()),
				actionCollection(), "new_dir" );
  m_actionRemove = new TDEAction( i18n("Remove"), "editdelete", Key_Delete, TQT_TQOBJECT(this), TQT_SLOT(slotRemoveItem()),
				actionCollection(), "remove" );
  TDEShortcut renameShortCut( Key_F2 );
  renameShortCut.append( TDEShortcut(CTRL+Key_R) ); // backwards compatibility
  m_actionRename = new TDEAction( i18n("Rename"), "edit", renameShortCut, TQT_TQOBJECT(this), TQT_SLOT(slotRenameItem()),
				actionCollection(), "rename" );
  m_actionParentDir = new TDEAction( i18n("Parent Directory"), "up", 0, TQT_TQOBJECT(this), TQT_SLOT(slotParentDir()),
				   actionCollection(), "parent_dir" );
  m_actionOpen = new TDEAction( i18n("Open"), "fileopen", 0, TQT_TQOBJECT(this), TQT_SLOT(slotOpen()),
				   actionCollection(), "open" );

  m_popupMenu = new TDEActionMenu( m_actionCollection, "contextMenu" );
  m_popupMenu->insert( m_actionParentDir );
  m_popupMenu->insert( new TDEActionSeparator( TQT_TQOBJECT(this) ) );
  m_popupMenu->insert( m_actionRename );
  m_popupMenu->insert( m_actionRemove );
  m_popupMenu->insert( m_actionNewDir );
  m_popupMenu->insert( new TDEActionSeparator( TQT_TQOBJECT(this) ) );
  m_popupMenu->insert( m_actionOpen );
  m_popupMenu->insert( new TDEActionSeparator( TQT_TQOBJECT(this) ) );
  m_popupMenu->insert( m_actionProperties );
  m_popupMenu->insert( new TDEActionSeparator( TQT_TQOBJECT(this) ) );
  m_popupMenu->insert( m_view->actionCollection()->action("project_burn") );
}


void K3bDataFileView::showPopupMenu( TDEListView*, TQListViewItem* item, const TQPoint& point )
{
  if( item ) {
    K3bDataItem* di = static_cast<K3bDataViewItem*>(item)->dataItem();
    m_actionRemove->setEnabled( di->isRemoveable() );
    m_actionRename->setEnabled( di->isRenameable() );
    if( currentDir() == m_doc->root() )
      m_actionParentDir->setEnabled( false );
    else
      m_actionParentDir->setEnabled( true );
    m_actionOpen->setEnabled( di->isFile() );
  }
  else {
    m_actionRemove->setEnabled( false );
    m_actionRename->setEnabled( false );
    m_actionOpen->setEnabled( false );
  }

  m_popupMenu->popup( point );
}


void K3bDataFileView::slotNewDir()
{
  K3bDirItem* parent = currentDir();

  TQString name;
  bool ok;

  name = KInputDialog::getText( i18n("New Directory"),
				i18n("Please insert the name for the new directory:"),
				i18n("New Directory"), &ok, this );

  while( ok && K3bDataDoc::nameAlreadyInDir( name, parent ) ) {
    name = KInputDialog::getText( i18n("New Directory"),
				  i18n("A file with that name already exists. "
				       "Please insert the name for the new directory:"),
				  i18n("New Directory"), &ok, this );
  }

  if( !ok )
    return;


  m_doc->addEmptyDir( name, parent );
}


void K3bDataFileView::slotRenameItem()
{
  if( currentItem() )
    showEditor( (K3bListViewItem*)currentItem(), 0 );
}


void K3bDataFileView::slotRemoveItem()
{
  TQPtrList<TQListViewItem> items = selectedItems();
  TQPtrListIterator<TQListViewItem> it( items );
  for(; it.current(); ++it ) {
    if( K3bDataViewItem* d = dynamic_cast<K3bDataViewItem*>( it.current() ) )
      m_doc->removeItem( d->dataItem() );
  }
}


void K3bDataFileView::slotParentDir()
{
  if( currentDir() != m_doc->root() ) {
    slotSetCurrentDir( currentDir()->parent() );

    emit dirSelected( currentDir() );
  }
}


void K3bDataFileView::slotProperties()
{
  K3bDataItem* dataItem = 0;

  // get selected item
  if( K3bDataViewItem* viewItem = dynamic_cast<K3bDataViewItem*>( selectedItems().first() ) ) {
    dataItem = viewItem->dataItem();
  }
  else {
    // default to current dir
    dataItem = currentDir();
  }

  if( dataItem ) {
    K3bDataPropertiesDialog d( dataItem, this );
    d.exec();
  }
  else
    m_view->slotProperties();
}


void K3bDataFileView::slotOpen()
{
  if( K3bDataViewItem* viewItem = dynamic_cast<K3bDataViewItem*>( selectedItems().first() ) ) {
    K3bDataItem* item = viewItem->dataItem();
    if( item->isFile() ) {
      K3bDataFileViewItem* fvi = static_cast<K3bDataFileViewItem*>( viewItem );
      if( fvi->mimeType() &&
#if KDE_IS_VERSION(3,3,0)
	  !KRun::isExecutableFile( KURL::fromPathOrURL(item->localPath()), 
				   fvi->mimeType()->name() )
#else
	  !TQFileInfo( item->localPath() ).isExecutable()
#endif
	  )
	KRun::runURL( KURL::fromPathOrURL(item->localPath()), 
		      fvi->mimeType()->name() );
      else
	KRun::displayOpenWithDialog( KURL::fromPathOrURL(item->localPath()) );
    }
  }
}


void K3bDataFileView::slotDoubleClicked( TQListViewItem* )
{
  if( K3bDataViewItem* viewItem = dynamic_cast<K3bDataViewItem*>( selectedItems().first() ) ) {
    if( !viewItem->dataItem()->isDir() ) {
      K3bDataPropertiesDialog d( viewItem->dataItem(), this );
      d.exec();
    }
  }
}

#include "k3bdatafileview.moc"
