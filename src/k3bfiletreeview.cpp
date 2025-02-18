/*
 *
 * $Id: k3bfiletreeview.cpp 657858 2007-04-25 08:25:11Z trueg $
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


#include "k3bfiletreeview.h"
#include "k3bappdevicemanager.h"
#include "k3bapplication.h"
#include "k3bmediacache.h"
#include "k3btooltip.h"
#include "k3bthememanager.h"

#include <k3bdevice.h>
#include <k3bdiskinfo.h>
#include <k3bglobals.h>

#include <kiconloader.h>
#include <tdelocale.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <tdeaction.h>
#include <tdeio/global.h>
#include <tdefileitem.h>
#include <tdeapplication.h>
#include <tdeglobalsettings.h>

#include <tqdir.h>
#include <tqevent.h>
#include <tqdragobject.h>
#include <tqcursor.h>
#include <tqnamespace.h>
#include <tqmap.h>
#include <tqptrdict.h>
#include <tqpainter.h>
#include <tqfont.h>
#include <tqstyle.h>
#include <tqlabel.h>
#include <tqlayout.h>


K3bDeviceBranch::K3bDeviceBranch( KFileTreeView* view, K3bDevice::Device* dev, KFileTreeViewItem* item )
  : KFileTreeBranch( view,
		     KURL( "media:/" + dev->blockDeviceName() ),
		     TQString("%1 - %2").arg(dev->vendor()).arg(dev->description()),
		     ( dev->burner()
		       ? SmallIcon("cd-rw-unmounted")
		       : SmallIcon("media-optical-cdrom-unmounted") ),
		     false,
		     item == 0
		     ? new K3bDeviceBranchViewItem( view, dev, this )
		     : item ),
    m_device( dev ),
    m_showBlockDeviceName( false )
{
  setAutoUpdate(true);
  root()->setExpandable(false);
  updateLabel();

  connect( k3bappcore->mediaCache(), TQ_SIGNAL(mediumChanged(K3bDevice::Device*)),
	   this, TQ_SLOT(slotMediumChanged(K3bDevice::Device*)) );
}


bool K3bDeviceBranch::populate( const KURL& url,  KFileTreeViewItem *currItem )
{
  // FIXME: we somehow need to "unpopulate" once the medium is unmounted

  // make sure we do not try to populate in case we are not mounted
  if( K3b::isMounted( m_device ) ) {
    bool b = KFileTreeBranch::populate( url, currItem );
    populateFinished( currItem );
    return b;
  }
  else
    populateFinished( currItem );

  return true;
}


void K3bDeviceBranch::setCurrent( bool c )
{
  static_cast<K3bDeviceBranchViewItem*>( root() )->setCurrent( c );
}


void K3bDeviceBranch::updateLabel()
{
  if( m_showBlockDeviceName )
    setName( TQString("%1 %2 (%3)")
	     .arg(m_device->vendor())
	     .arg(m_device->description())
	     .arg(m_device->blockDeviceName()) );
  else
    setName( TQString("%1 %2")
	     .arg(m_device->vendor())
	     .arg(m_device->description()) );

  if( k3bappcore->mediaCache() ) {
    root()->setMultiLinesEnabled( true );
    root()->setText( 0, name() + "\n" + k3bappcore->mediaCache()->mediumString( m_device ) );
    static_cast<K3bFileTreeView*>( root()->listView() )->updateMinimumWidth();
  }
  else {
    root()->setMultiLinesEnabled( false );
    root()->setText( 0, name() );
  }
}


void K3bDeviceBranch::slotMediumChanged( K3bDevice::Device* dev )
{
  if( dev == m_device )
    updateLabel();
}


void K3bDeviceBranch::showBlockDeviceName( bool b )
{
  m_showBlockDeviceName = b;
  updateLabel();
}


K3bFileTreeBranch::K3bFileTreeBranch( KFileTreeView* view,
				      const KURL& url,
				      const TQString& name,
				      const TQPixmap& pix,
				      bool showHidden,
				      KFileTreeViewItem* item )
  : KFileTreeBranch( view, url, name, pix, showHidden,
		     item == 0
		     ? new K3bFileTreeViewItem( view,
						new KFileItem( url, "inode/directory",
							       S_IFDIR  ),
						this )
		     : item )
{
}



K3bDeviceBranchViewItem::K3bDeviceBranchViewItem( KFileTreeViewItem* parent,
						  K3bDevice::Device* dev,
						  K3bDeviceBranch* branch )
  : KFileTreeViewItem( parent,
		       new KFileItem( KURL( "media:/" + dev->blockDeviceName() ),
				      "inode/directory",
				      S_IFDIR  ),
		       branch ),
    m_bCurrent( false ),
    m_device( dev )
{
}


K3bDeviceBranchViewItem::K3bDeviceBranchViewItem( KFileTreeView* parent,
						  K3bDevice::Device* dev,
						  K3bDeviceBranch* branch )
  : KFileTreeViewItem( parent,
		       new KFileItem( KURL( "media:/" + dev->blockDeviceName() ),
				      "inode/directory",
				      S_IFDIR  ),
		       branch ),
    m_bCurrent( false ),
    m_device( dev )
{
}


void K3bDeviceBranchViewItem::setCurrent( bool c )
{
  m_bCurrent = c;
  repaint();
}


void K3bDeviceBranchViewItem::paintCell( TQPainter* p, const TQColorGroup& cg, int /* col */, int width, int align )
{
  p->save();

  int xpos = 1;
  int ypos = 1;
  TQFontMetrics fm( p->fontMetrics() );

  if( isSelected() ) {
    p->fillRect( 0, 0, width, height(),
		 cg.brush( TQColorGroup::Highlight ) );
    p->setPen( cg.highlightedText() );
  }
  else {
    p->fillRect( 0, 0, width, height(), cg.base() );
    p->setPen( cg.text() );
  }

  if( pixmap(0) ) {
    p->drawPixmap( xpos, ypos, *pixmap(0) );
    xpos += pixmap(0)->width() + 5;
  }

  if( m_bCurrent ) {
    TQFont f( listView()->font() );
    f.setBold( true );
    p->setFont( f );
  }

  ypos += fm.ascent();
  TQString line1 = text(0).left( text(0).find('\n') );
  p->drawText( xpos, ypos, line1 );

  TQFont f( listView()->font() );
  f.setItalic( true );
  f.setBold( false );
  f.setPointSize( f.pointSize() - 2 );
  p->setFont( f );

  ypos += p->fontMetrics().height() + 1;
  TQString line2 = text(0).mid( text(0).find('\n')+1 );
  p->drawText( xpos - p->fontMetrics().leftBearing( line2[0] ), ypos, line2 );


  // from TQListViewItem
  if( isOpen() && childCount() ) {
    int textheight = fm.size( align, text(0) ).height() + 2 * listView()->itemMargin();
    textheight = TQMAX( textheight, TQApplication::globalStrut().height() );
    if ( textheight % 2 > 0 )
      textheight++;
    if ( textheight < height() ) {
      int w = listView()->treeStepSize() / 2;
      listView()->style().drawComplexControl( TQStyle::CC_ListView, p, listView(),
					      TQRect( 0, textheight, w + 1, height() - textheight + 1 ), cg,
					      TQStyle::Style_Enabled,
					      TQStyle::SC_ListViewExpand,
					      (uint)TQStyle::SC_All, TQStyleOption( this ) );
    }
  }

  p->restore();
}


int K3bDeviceBranchViewItem::widthHint() const
{
  TQFont f( listView()->font() );
  if ( m_bCurrent ) {
      f.setBold( true );
  }
  int w = TQFontMetrics(f).width( text(0).left( text(0).find('\n') ) );

  f.setItalic( true );
  f.setBold( false );
  f.setPointSize( f.pointSize() - 2 );
  w = TQMAX( w, TQFontMetrics(f).width( text(0).mid( text(0).find('\n')+1 ) ) );

  w++; // see paintCell

  if( pixmap(0) )
    w += pixmap(0)->width() + 5;

  return w;
}


TQString K3bDeviceBranchViewItem::key( int column, bool ascending ) const
{
  return "0" + KFileTreeViewItem::key( column, ascending );
}



K3bFileTreeViewItem::K3bFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch )
  : KFileTreeViewItem( parent, item, branch )
{
}


K3bFileTreeViewItem::K3bFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
  : KFileTreeViewItem( parent, item, branch )
{
}


TQString K3bFileTreeViewItem::key( int column, bool ascending ) const
{
  return "1" + KFileTreeViewItem::key( column, ascending );
}


class K3bDeviceTreeToolTip : public K3bToolTip
{
public:
  K3bDeviceTreeToolTip( TQWidget* parent, K3bFileTreeView* lv );

  void maybeTip( const TQPoint &pos );

private:
  K3bFileTreeView* m_view;
};


K3bDeviceTreeToolTip::K3bDeviceTreeToolTip( TQWidget* parent, K3bFileTreeView* lv )
  : K3bToolTip( parent ),
    m_view( lv )
{
  setTipTimeout( 500 );
}


void K3bDeviceTreeToolTip::maybeTip( const TQPoint& pos )
{
  if( !parentWidget() || !m_view )
    return;

  K3bDeviceBranchViewItem* item = dynamic_cast<K3bDeviceBranchViewItem*>( m_view->itemAt( pos ) );
  if( !item )
    return;

  K3bDevice::Device* dev = static_cast<K3bDeviceBranch*>( item->branch() )->device();

  TQFrame* tooltip = new TQFrame( parentWidget() );
  tooltip->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
  tooltip->setFrameShape( TQFrame::StyledPanel );
  TQGridLayout* lay = new TQGridLayout( tooltip, 2, 2, tooltip->frameWidth()*2 /*margin*/, 6 /*spacing*/ );

  TQString text = k3bappcore->mediaCache()->medium( dev ).longString();
  int detailsStart = text.find( "<p>", 3 );
  TQString details = text.mid( detailsStart );
  text.truncate( detailsStart );

  TQLabel* label = new TQLabel( text, tooltip );
  label->setMargin( 9 );
  lay->addMultiCellWidget( label, 0, 0, 0, 1 );
  label = new TQLabel( details, tooltip );
  label->setMargin( 9 );
  label->setAlignment( TQt::Vertical );
  lay->addMultiCellWidget( label, 1, 2, 0, 0 );
  label = new TQLabel( tooltip );
  lay->addWidget( label, 2, 1 );
  lay->setColStretch( 0, 1 );

  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    tooltip->setPaletteBackgroundColor( theme->backgroundColor() );
    tooltip->setPaletteForegroundColor( theme->foregroundColor() );
    K3bTheme::PixmapType pm;
    int c = k3bappcore->mediaCache()->medium( dev ).content();
    if( c & (K3bMedium::CONTENT_VIDEO_CD|K3bMedium::CONTENT_VIDEO_DVD) )
      pm = K3bTheme::MEDIA_VIDEO;
    else if( c & K3bMedium::CONTENT_AUDIO &&
	      c & K3bMedium::CONTENT_DATA )
      pm = K3bTheme::MEDIA_MIXED;
    else if( c & K3bMedium::CONTENT_AUDIO )
      pm = K3bTheme::MEDIA_AUDIO;
    else if( c & K3bMedium::CONTENT_DATA )
      pm = K3bTheme::MEDIA_DATA;
    else {
      K3bDevice::DiskInfo di = k3bappcore->mediaCache()->diskInfo( dev );
      if( di.diskState() == K3bDevice::STATE_EMPTY )
	pm = K3bTheme::MEDIA_EMPTY;
      else
	pm = K3bTheme::MEDIA_NONE;
    }
    label->setPixmap( theme->pixmap( pm ) );
  }

  // the tooltip will take care of deleting the widget
  tip( m_view->itemRect( item ), tooltip );
}



class K3bFileTreeView::Private
{
public:
  Private()
    : deviceManager(0),
      currentDeviceBranch(0) {
  }

  TQPtrDict<K3bDeviceBranch> deviceBranchDict;
  TQMap<KFileTreeBranch*, K3bDevice::Device*> branchDeviceMap;

  K3bDevice::DeviceManager* deviceManager;
  K3bDeviceBranch* currentDeviceBranch;

  K3bDeviceTreeToolTip* toolTip;
};

K3bFileTreeView::K3bFileTreeView( TQWidget *parent, const char *name )
  : KFileTreeView( parent,  name )
{
  d = new Private();

  d->toolTip = new K3bDeviceTreeToolTip( viewport(), this );

  addColumn( i18n("Directories") );
  setDragEnabled( true );
  setAlternateBackground( TQColor() );
  setFullWidth(true);
  //  setRootIsDecorated(true);
  setSorting(0);

  m_dirOnlyMode = true;
  m_menuEnabled = false;

  connect( this, TQ_SIGNAL(executed(TQListViewItem*)), this, TQ_SLOT(slotItemExecuted(TQListViewItem*)) );
  connect( this, TQ_SIGNAL(returnPressed(TQListViewItem*)), this, TQ_SLOT(slotItemExecuted(TQListViewItem*)) );
  connect( this, TQ_SIGNAL(contextMenu(TDEListView*, TQListViewItem* , const TQPoint& )),
	   this, TQ_SLOT(slotContextMenu(TDEListView*, TQListViewItem* , const TQPoint& )) );

  // we always simulate the single click
  slotSettingsChangedK3b(TDEApplication::SETTINGS_MOUSE);
  if( kapp )
    connect( kapp, TQ_SIGNAL(settingsChanged(int)), TQ_SLOT(slotSettingsChangedK3b(int)) );

  initActions();
}


K3bFileTreeView::~K3bFileTreeView()
{
  delete d;
}


void K3bFileTreeView::clear()
{
  KFileTreeView::clear();
  if( d->deviceManager )
    d->deviceManager->disconnect( this );
  d->deviceManager = 0;
}


void K3bFileTreeView::initActions()
{
//   m_actionCollection = new TDEActionCollection( this );

//   m_devicePopupMenu = new TDEActionMenu( m_actionCollection, "device_popup_menu" );
//   m_urlPopupMenu = new TDEActionMenu( m_actionCollection, "url_popup_menu" );

//   TDEAction* actionDiskInfo = new TDEAction( i18n("&Disk Info"), "application-vnd.tde.info", 0, this, TQ_SLOT(slotShowDiskInfo()),
// 					 m_actionCollection, "disk_info");
//   TDEAction* actionUnmount = new TDEAction( i18n("&Unmount"), "media-optical-cdrom-unmounted", 0, this, TQ_SLOT(slotUnmountDisk()),
// 					m_actionCollection, "disk_unmount");
//   TDEAction* actionEject = new TDEAction( i18n("&Eject"), "", 0, this, TQ_SLOT(slotEjectDisk()),
// 					m_actionCollection, "disk_eject");

//   m_devicePopupMenu->insert( actionDiskInfo );
//   m_devicePopupMenu->insert( new TDEActionSeparator( this ) );
//   m_devicePopupMenu->insert( actionUnmount );
//   m_devicePopupMenu->insert( actionEject );

}


void K3bFileTreeView::addDefaultBranches()
{
  KURL home = KURL::fromPathOrURL( TQDir::homeDirPath() );
  KURL root = KURL( "file:/" );

  KFileTreeBranch* treeBranch = addBranch( new K3bFileTreeBranch( this, root, i18n("Root"), SmallIcon("folder_red") ) );
  treeBranch = addBranch( new K3bFileTreeBranch( this, home, i18n("Home"), SmallIcon("folder_home") ) );
  treeBranch->setOpen( true );
}


void K3bFileTreeView::addCdDeviceBranches( K3bDevice::DeviceManager* dm )
{
  kdDebug() << "(K3bFileTreeView::addCdDeviceBranches)" << endl;

  // remove all previous added device branches
  for( TQMap<KFileTreeBranch*, K3bDevice::Device*>::Iterator it = d->branchDeviceMap.begin();
       it != d->branchDeviceMap.end(); ++it ) {
    removeBranch( it.key() );
  }

  // clear the maps
  d->branchDeviceMap.clear();
  d->deviceBranchDict.clear();

  for( TQPtrListIterator<K3bDevice::Device> it( dm->allDevices() ); *it; ++it )
    addDeviceBranch( *it );

  if( dm != d->deviceManager ) {
    if( d->deviceManager )
      d->deviceManager->disconnect( this );
    d->deviceManager = dm;

    // make sure we get changes to the config
    connect( dm, TQ_SIGNAL(changed(K3bDevice::DeviceManager*)),
	     this, TQ_SLOT(addCdDeviceBranches(K3bDevice::DeviceManager*)) );

    if( K3bAppDeviceManager* appDevM = dynamic_cast<K3bAppDeviceManager*>( dm ) )
      connect( appDevM, TQ_SIGNAL(currentDeviceChanged(K3bDevice::Device*)),
	       this, TQ_SLOT(setCurrentDevice(K3bDevice::Device*)) );
  }

  K3bDevice::Device* currentDevice = k3bappcore->appDeviceManager()->currentDevice();
  if ( !currentDevice && !k3bappcore->appDeviceManager()->allDevices().isEmpty() ) {
      k3bappcore->appDeviceManager()->setCurrentDevice( k3bappcore->appDeviceManager()->allDevices().getFirst() );
  }

  d->currentDeviceBranch = d->deviceBranchDict[currentDevice];
  if( d->currentDeviceBranch ) {
    d->currentDeviceBranch->setCurrent( true );
  }

  kdDebug() << "(K3bFileTreeView::addCdDeviceBranches) done" << endl;
}


void K3bFileTreeView::addDeviceBranch( K3bDevice::Device* dev )
{
  K3bDeviceBranch* newBranch = new K3bDeviceBranch( this, dev );
  addBranch( newBranch );

  // search for an equal device
  int equalCnt = 0;
  K3bDeviceBranch* equalBranch = 0;
  for( TQMap<KFileTreeBranch*, K3bDevice::Device*>::Iterator it = d->branchDeviceMap.begin();
       it != d->branchDeviceMap.end(); ++it ) {
    K3bDevice::Device* itDev = it.data();
    K3bDeviceBranch* itBranch = (K3bDeviceBranch*)it.key();
    if( itDev->vendor() == dev->vendor() &&
	itDev->description() == dev->description() ) {
      ++equalCnt;
      equalBranch = itBranch;
    }
  }

  // if there is at least one equal device add the block device name
  // if there is more than one equal device they have been updated after
  // adding the last one so there is no need to update more than two
  if( equalCnt > 0 ) {
    kdDebug() << "(K3bFileTreeView) equal branch" << endl;
    newBranch->showBlockDeviceName(true);
    equalBranch->showBlockDeviceName(true);
  }

  // add to maps
  d->branchDeviceMap.insert( newBranch, dev );
  d->deviceBranchDict.insert( (void*)dev, newBranch );

  updateMinimumWidth();
}


KFileTreeBranch* K3bFileTreeView::addBranch( KFileTreeBranch* branch )
{
  KFileTreeBranch* newBranch = KFileTreeView::addBranch( branch );
  newBranch->setChildRecurse( false );
  setDirOnlyMode( newBranch, m_dirOnlyMode );

  return newBranch;
}


KFileTreeBranch* K3bFileTreeView::addBranch( const KURL& url, const TQString& name, const TQPixmap& pix, bool showHidden )
{
  KFileTreeBranch* newBranch = KFileTreeView::addBranch( url, name, pix, showHidden );
  newBranch->setChildRecurse( false );
  setDirOnlyMode( newBranch, m_dirOnlyMode );

  return newBranch;
}


void K3bFileTreeView::slotItemExecuted( TQListViewItem* item )
{
  KFileTreeViewItem* treeItem = static_cast<KFileTreeViewItem*>(item);
  if( d->branchDeviceMap.contains( treeItem->branch() ) &&
      treeItem == treeItem->branch()->root() ) {
    K3bDevice::Device* dev = d->branchDeviceMap[treeItem->branch()];
      k3bappcore->appDeviceManager()->setCurrentDevice( dev );
    emit deviceExecuted( dev );
  }
  else
    emit urlExecuted( treeItem->url() );
}


void K3bFileTreeView::setTreeDirOnlyMode( bool b )
{
  m_dirOnlyMode = b;
}


void K3bFileTreeView::followUrl( const KURL& url )
{
  // TODO: first try the current branch
  KFileTreeBranchIterator it( branches() );
  for( ; *it; ++it ) {
    if( !d->branchDeviceMap.contains( *it ) )
      if( KFileTreeViewItem* item = (*it)->findTVIByURL( url ) ) {
	setCurrentItem( item );
	setSelected(item, true);
	ensureItemVisible( item );
	return;
      }
  }
}


void K3bFileTreeView::slotContextMenu( TDEListView*, TQListViewItem* item, const TQPoint& p )
{
  KFileTreeViewItem* treeItem = dynamic_cast<KFileTreeViewItem*>(item);
  if( treeItem ) {
    K3bDevice::Device* device = 0;
    TQMap<KFileTreeBranch*, K3bDevice::Device*>::iterator devIt =
      d->branchDeviceMap.find( treeItem->branch() );
    if( devIt != d->branchDeviceMap.end() )
      device = devIt.data();

    setCurrentItem( treeItem );
    setSelected( treeItem, true);

    if( device ) {
      k3bappcore->appDeviceManager()->setCurrentDevice( device );
      emit contextMenu( device, p );
    }
    else
      emit contextMenu( treeItem->url(), p );
  }
  else
    kdWarning() << "(K3bFileTreeView) found viewItem that is no KFileTreeViewItem!" << endl;
}


K3bDevice::Device* K3bFileTreeView::selectedDevice() const
{
  KFileTreeViewItem* treeItem = dynamic_cast<KFileTreeViewItem*>(selectedItem());
  if( treeItem ) {
    if( d->branchDeviceMap.contains( treeItem->branch() ) )
      return d->branchDeviceMap[treeItem->branch()];
  }
  return 0;
}


KURL K3bFileTreeView::selectedUrl() const
{
  KFileTreeViewItem* treeItem = dynamic_cast<KFileTreeViewItem*>(selectedItem());
  if( treeItem ) {
    if( !d->branchDeviceMap.contains( treeItem->branch() ) )
      return treeItem->url();
  }
  return KURL();
}


void K3bFileTreeView::setCurrentDevice( K3bDevice::Device* dev )
{
  if( d->currentDeviceBranch )
    d->currentDeviceBranch->setCurrent( false );

  if( dev ) {
    if ( ( d->currentDeviceBranch = branch( dev ) ) )
        d->currentDeviceBranch->setCurrent( true );
  }

  updateMinimumWidth();
}


void K3bFileTreeView::setSelectedDevice( K3bDevice::Device* dev )
{
  setCurrentDevice( dev );
  K3bDeviceBranch* b = branch( dev );
  setCurrentItem( b->root() );
  setSelected( b->root(), true );
}


K3bDeviceBranch* K3bFileTreeView::branch( K3bDevice::Device* dev )
{
  return d->deviceBranchDict.find( (void*)dev );
}


void K3bFileTreeView::slotSettingsChangedK3b(int category)
{
  // we force single click like konqueror does. This really should be done in KFileTreeView

  if( category == TDEApplication::SETTINGS_MOUSE ) {
    disconnect(this, TQ_SIGNAL(mouseButtonClicked(int, TQListViewItem*, const TQPoint &, int)),
	       this, TQ_SLOT(slotMouseButtonClickedK3b(int, TQListViewItem*, const TQPoint &, int)));

    if( !TDEGlobalSettings::singleClick() )
      connect(this, TQ_SIGNAL(mouseButtonClicked(int, TQListViewItem*, const TQPoint &, int)),
	      this, TQ_SLOT(slotMouseButtonClickedK3b(int, TQListViewItem*, const TQPoint &, int)));
  }
}


void K3bFileTreeView::slotMouseButtonClickedK3b( int btn, TQListViewItem *item, const TQPoint &pos, int c )
{
  if( (btn == TQt::LeftButton) && item )
    emitExecute(item, pos, c);
}


void K3bFileTreeView::updateMinimumWidth()
{
  //
  // only handle the device branches, we don't care about the folders.
  //
  int w = 0;
  for( TQMap<KFileTreeBranch*, K3bDevice::Device*>::Iterator it = d->branchDeviceMap.begin();
       it != d->branchDeviceMap.end(); ++it ) {
    w = TQMAX( w, static_cast<K3bDeviceBranchViewItem*>( it.key()->root() )->widthHint() );
  }

  // width of the items + scrollbar width + the frame + a little eyecandy spacing
  setMinimumWidth( w + verticalScrollBar()->sizeHint().width() + 2*frameWidth() + 2 );
}

#include "k3bfiletreeview.moc"
