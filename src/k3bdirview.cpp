/*
 *
 * $Id: k3bdirview.cpp 627621 2007-01-27 12:31:44Z trueg $
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

#include <config.h>

#include "k3bdirview.h"
#include "k3bapplication.h"
#include "k3b.h"

#include "rip/k3baudiocdview.h"
#include "rip/k3bvideocdview.h"
#ifdef HAVE_LIBDVDREAD
#include "rip/videodvd/k3bvideodvdrippingview.h"
#endif
#include "k3bfileview.h"
#include "k3bfiletreeview.h"
#include "k3bappdevicemanager.h"
#include "k3bdiskinfoview.h"
#include <k3bdevicehandler.h>
#include <k3bdevice.h>
#include <k3bthememanager.h>
#include <k3bmediacache.h>
#include <k3bexternalbinmanager.h>
#include <k3bpassivepopup.h>

#include <unistd.h>
// TQt includes
#include <tqdir.h>
#include <tqlistview.h>
#include <tqstring.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqpixmap.h>
#include <tqstringlist.h>
#include <tqstrlist.h>
#include <tqheader.h>
#include <tqsplitter.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqiconset.h>
#include <tqvaluelist.h>
#include <tqlabel.h>
#include <tqwidgetstack.h>
#include <tqscrollview.h>
#include <tqpainter.h>
#include <tqsimplerichtext.h>

// KDE-includes
#include <kmimetype.h>
#include <kcursor.h>
#include <tdefiledetailview.h>
#include <tdetoolbar.h>
#include <kiconloader.h>
#include <kurl.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <tdeio/file.h>
#include <tdeio/global.h>
#include <krun.h>
#include <kprocess.h>
#include <tdeio/job.h>
#include <kcombobox.h>
#include <tdefiletreeview.h>
#include <kdialog.h>
#include <tdemessagebox.h>
#include <kstdaction.h>
#include <tdeconfig.h>
#include <tdeaction.h>
#include <kinputdialog.h>



class K3bDirView::Private
{
public:
  bool contextMediaInfoRequested;
};



K3bDirView::K3bDirView(K3bFileTreeView* treeView, TQWidget *parent, const char *name )
  : TQVBox(parent, name),
    m_fileTreeView(treeView),
    m_bViewDiskInfo(false)
{
  d = new Private;
  d->contextMediaInfoRequested = false;

  if( !m_fileTreeView ) {
    m_mainSplitter = new TQSplitter( this );
    m_fileTreeView = new K3bFileTreeView( m_mainSplitter );
    m_viewStack    = new TQWidgetStack( m_mainSplitter );
  }
  else {
    m_viewStack    = new TQWidgetStack( this );
    m_mainSplitter = 0;
  }

  m_fileTreeView->header()->hide();

  m_fileView     = new K3bFileView(m_viewStack, "fileView");
  m_cdView       = new K3bAudioCdView(m_viewStack, "cdview");
  m_videoView    = new K3bVideoCdView(m_viewStack, "videoview");
  m_infoView     = new K3bDiskInfoView(m_viewStack, "infoView");
#ifdef HAVE_LIBDVDREAD
  m_movieView    = new K3bVideoDVDRippingView(m_viewStack, "movieview");
#endif

  m_viewStack->raiseWidget( m_fileView );

  m_fileTreeView->addDefaultBranches();
  m_fileTreeView->addCdDeviceBranches( k3bcore->deviceManager() );
  m_fileTreeView->setCurrentDevice( k3bappcore->appDeviceManager()->currentDevice() );

  m_fileView->setAutoUpdate( true ); // in case we look at the mounted path

  if( m_mainSplitter ) {
    // split
    TQValueList<int> sizes = m_mainSplitter->sizes();
    int all = sizes[0] + sizes[1];
    sizes[1] = all*2/3;
    sizes[0] = all - sizes[1];
    m_mainSplitter->setSizes( sizes );
  }

  connect( m_fileTreeView, TQ_SIGNAL(urlExecuted(const KURL&)),
	   this, TQ_SLOT(slotDirActivated(const KURL&)) );
  connect( m_fileTreeView, TQ_SIGNAL(deviceExecuted(K3bDevice::Device*)),
	   this, TQ_SLOT(showDevice(K3bDevice::Device*)) );
  connect( m_fileTreeView, TQ_SIGNAL(deviceExecuted(K3bDevice::Device*)),
	   this, TQ_SIGNAL(deviceSelected(K3bDevice::Device*)) );
  connect( m_fileTreeView, TQ_SIGNAL(contextMenu(K3bDevice::Device*, const TQPoint&)),
	   this, TQ_SLOT(slotFileTreeContextMenu(K3bDevice::Device*, const TQPoint&)) );

  connect( m_fileView, TQ_SIGNAL(urlEntered(const KURL&)), m_fileTreeView, TQ_SLOT(followUrl(const KURL&)) );
  connect( m_fileView, TQ_SIGNAL(urlEntered(const KURL&)), this, TQ_SIGNAL(urlEntered(const KURL&)) );

  connect( k3bappcore->appDeviceManager(), TQ_SIGNAL(mountFinished(const TQString&)),
	   this, TQ_SLOT(slotMountFinished(const TQString&)) );
  connect( k3bappcore->appDeviceManager(), TQ_SIGNAL(unmountFinished(bool)),
	   this, TQ_SLOT(slotUnmountFinished(bool)) );
  connect( k3bappcore->appDeviceManager(), TQ_SIGNAL(detectingDiskInfo(K3bDevice::Device*)),
	   this, TQ_SLOT(slotDetectingDiskInfo(K3bDevice::Device*)) );
}

K3bDirView::~K3bDirView()
{
  delete d;
}


void K3bDirView::showUrl( const KURL& url )
{
  slotDirActivated( url );
}


void K3bDirView::showDevice( K3bDevice::Device* dev )
{
  d->contextMediaInfoRequested = true;
  m_fileTreeView->setSelectedDevice( dev );
  showMediumInfo( k3bappcore->mediaCache()->medium( dev ) );
}


void K3bDirView::slotDetectingDiskInfo( K3bDevice::Device* dev )
{
  d->contextMediaInfoRequested = false;
  m_fileTreeView->setSelectedDevice( dev );
  showMediumInfo( k3bappcore->mediaCache()->medium( dev ) );
}


void K3bDirView::showMediumInfo( const K3bMedium& medium )
{
  if( !d->contextMediaInfoRequested ||
      medium.diskInfo().diskState() == K3bDevice::STATE_EMPTY ||
      medium.diskInfo().diskState() == K3bDevice::STATE_NO_MEDIA ) {
    
    // show cd info
    m_viewStack->raiseWidget( m_infoView );
    m_infoView->reload( medium );
    return;
  }

#ifdef HAVE_LIBDVDREAD
  else if( medium.content() & K3bMedium::CONTENT_VIDEO_DVD ) {
    KMessageBox::ButtonCode r = KMessageBox::Yes;
    if( KMessageBox::shouldBeShownYesNo( "videodvdripping", r ) ) {
      r = (KMessageBox::ButtonCode)
	KMessageBox::questionYesNoCancel( this,
					  i18n("<p>You have selected the K3b Video DVD ripping tool."
					       "<p>It is intended to <em>rip single titles</em> from a video DVD "
					       "into a compressed format such as XviD. Menu structures are completely ignored."
					       "<p>If you intend to copy the plain Video DVD vob files from the DVD "
					       "(including decryption) for further processing with another application, "
					       "please use the following link to access the Video DVD file structure: "
					       "<a href=\"videodvd:/\">videodvd:/</a>"
					       "<p>If you intend to make a copy of the entire Video DVD including all menus "
					       "and extras it is recommended to use the K3b DVD Copy tool."),
					  i18n("Video DVD ripping"),
					  i18n("Continue"),
					  i18n("Open DVD Copy Dialog"),
					  "videodvdripping",
					  KMessageBox::AllowLink );
    }
    else { // if we do not show the dialog we always continue with the ripping. Everything else would be confusing
      r = KMessageBox::Yes;
    }

    if( r == KMessageBox::Cancel ) {
      //      m_viewStack->raiseWidget( m_fileView );
    }
    else if( r == KMessageBox::No ) {
      m_viewStack->raiseWidget( m_fileView );
      static_cast<K3bMainWindow*>( kapp->mainWidget() )->slotDvdCopy();
    }
    else {
      m_movieView->reload( medium );
      m_viewStack->raiseWidget( m_movieView );
    }

    return;
  }
#endif
  
  else if( medium.content() & K3bMedium::CONTENT_DATA ) {
    bool mount = true;
    if( medium.content() & K3bMedium::CONTENT_VIDEO_CD ) {
      if( !k3bcore ->externalBinManager() ->foundBin( "vcdxrip" ) ) {
	KMessageBox::sorry( this,
			    i18n("K3b uses vcdxrip from the vcdimager package to rip Video CDs. "
				 "Please make sure it is installed.") );
      }
      else {
	if( KMessageBox::questionYesNo( this,
					i18n("Found %1. Do you want K3b to mount the data part "
					     "or show all the tracks?").arg( i18n("Video CD") ),
					i18n("Video CD"),
					i18n("Mount CD"),
					i18n("Show Video Tracks") ) == KMessageBox::No ) {
	  mount = false;
	  m_viewStack->raiseWidget( m_videoView );
	  m_videoView->reload( medium );
	}
      }
    }
    else if( medium.content() & K3bMedium::CONTENT_AUDIO ) {
      if( KMessageBox::questionYesNo( this,
				      i18n("Found %1. Do you want K3b to mount the data part "
					   "or show all the tracks?").arg( i18n("Audio CD") ),
				      i18n("Audio CD"),
				      i18n("Mount CD"),
				      i18n("Show Audio Tracks") ) == KMessageBox::No ) {
	mount = false;
	m_viewStack->raiseWidget( m_cdView );
	m_cdView->reload( medium );
      }
    }
      
    if( mount )
      k3bappcore->appDeviceManager()->mountDisk( medium.device() );
  }

  else if( medium.content() & K3bMedium::CONTENT_AUDIO ) {
    m_viewStack->raiseWidget( m_cdView );
    m_cdView->reload( medium );
  }

  else {
    // show cd info
    m_viewStack->raiseWidget( m_infoView );
    m_infoView->reload( medium );
  }

  d->contextMediaInfoRequested = false;
}


void K3bDirView::slotMountFinished( const TQString& mp )
{
  if( !mp.isEmpty() ) {
    slotDirActivated( mp );
    m_fileView->reload(); // HACK to get the contents shown... FIXME
  }
  else {
    m_viewStack->raiseWidget( m_fileView );
    K3bPassivePopup::showPopup( i18n("<p>K3b was unable to mount medium <b>%1</b> in device <em>%2 - %3</em>")
				.arg( k3bappcore->mediaCache()->medium( k3bappcore->appDeviceManager()->currentDevice() ).shortString() )
				.arg( k3bappcore->appDeviceManager()->currentDevice()->vendor() )
				.arg( k3bappcore->appDeviceManager()->currentDevice()->description() ),
				i18n("Mount Failed"),
				K3bPassivePopup::Warning );
  }
}


void K3bDirView::slotUnmountFinished( bool success )
{
  if( success ) {
    // TODO: check if the fileview is still displaying a folder from the medium
  }
  else {
    K3bPassivePopup::showPopup( i18n("<p>K3b was unable to unmount medium <b>%1</b> in device <em>%2 - %3</em>")
				.arg( k3bappcore->mediaCache()->medium( k3bappcore->appDeviceManager()->currentDevice() ).shortString() )
				.arg( k3bappcore->appDeviceManager()->currentDevice()->vendor() )
				.arg( k3bappcore->appDeviceManager()->currentDevice()->description() ),
				i18n("Unmount Failed"),
				K3bPassivePopup::Warning );
  }
}


void K3bDirView::slotFileTreeContextMenu( K3bDevice::Device* /*dev*/, const TQPoint& p )
{
  TDEAction* a = k3bappcore->appDeviceManager()->actionCollection()->action( "device_popup" );
  if( TDEActionMenu* m = dynamic_cast<TDEActionMenu*>(a) )
    m->popup( p );
}


void K3bDirView::slotDirActivated( const TQString& url )
{
//   m_urlCombo->insertItem( url, 0 );
  slotDirActivated( KURL::fromPathOrURL(url) );
}


void K3bDirView::slotDirActivated( const KURL& url )
{
  m_fileView->setUrl(url, true);
//   m_urlCombo->setEditText( url.path() );

  m_viewStack->raiseWidget( m_fileView );
}


void K3bDirView::home()
{
  slotDirActivated( TQDir::homeDirPath() );
}


void K3bDirView::saveConfig( TDEConfig* c )
{
  m_fileView->saveConfig(c);
}


void K3bDirView::readConfig( TDEConfig* c )
{
  m_fileView->readConfig(c);
}

#include "k3bdirview.moc"
