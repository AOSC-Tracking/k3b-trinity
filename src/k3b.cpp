/*
 *
 * $Id: k3b.cpp 642063 2007-03-13 09:40:13Z trueg $
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
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


// include files for TQt
#include <tqdir.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqtoolbutton.h>
#include <tqstring.h>
#include <tqsplitter.h>
#include <tqevent.h>
#include <tqvaluelist.h>
#include <tqfont.h>
#include <tqpalette.h>
#include <tqwidgetstack.h>
#include <tqtimer.h>

#include <kdockwidget.h>
#include <kkeydialog.h>
// include files for KDE
#include <kiconloader.h>
#include <tdemessagebox.h>
#include <tdefiledialog.h>
#include <tdemenubar.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstdaction.h>
#include <klineeditdlg.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kurl.h>
#include <kurllabel.h>
#include <tdetoolbar.h>
#include <kstatusbar.h>
#include <tdeglobalsettings.h>
#include <kdialog.h>
#include <kedittoolbar.h>
#include <ksystemtray.h>
#include <tdeaboutdata.h>
#include <ktip.h>
#include <kxmlguifactory.h>
#include <kstdguiitem.h>
#include <tdeio/global.h>
#include <tdeio/netaccess.h>
#include <tderecentdocument.h>

#include <stdlib.h>

// application specific includes
#include "k3b.h"
#include "k3bapplication.h"
#include <k3bglobals.h>
#include "k3bview.h"
#include "k3bdirview.h"
#include <k3baudiodoc.h>
#include "k3baudioview.h"
#include "k3bappdevicemanager.h"
#include "k3baudiotrackdialog.h"
#include "option/k3boptiondialog.h"
#include "k3bprojectburndialog.h"
#include <k3bdatadoc.h>
#include "k3bdataview.h"
#include <k3bdvddoc.h>
#include "k3bdvdview.h"
#include <k3bvideodvddoc.h>
#include "k3bvideodvdview.h"
#include <k3bmixeddoc.h>
#include "k3bmixedview.h"
#include <k3bvcddoc.h>
#include "k3bvcdview.h"
#include <k3bmovixdoc.h>
#include "k3bmovixview.h"
#include <k3bmovixdvddoc.h>
#include "k3bmovixdvdview.h"
#include "misc/k3bblankingdialog.h"
#include "misc/k3bcdimagewritingdialog.h"
#include "misc/k3bisoimagewritingdialog.h"
#include <k3bexternalbinmanager.h>
#include "k3bprojecttabwidget.h"
#include "misc/k3bcdcopydialog.h"
#include "k3btempdirselectionwidget.h"
#include "k3bstatusbarmanager.h"
#include "k3bfiletreecombobox.h"
#include "k3bfiletreeview.h"
#include "k3bsidepanel.h"
#include "k3bstdguiitems.h"
#include "misc/k3bdvdformattingdialog.h"
#include "misc/k3bdvdcopydialog.h"
//#include "dvdcopy/k3bvideodvdcopydialog.h"
#include "k3bprojectmanager.h"
#include "k3bwelcomewidget.h"
#include <k3bpluginmanager.h>
#include <k3bplugin.h>
#include "k3bsystemproblemdialog.h"
#include <k3baudiodecoder.h>
#include <k3bthememanager.h>
#include <k3biso9660.h>
#include <k3bcuefileparser.h>
#include <k3bdeviceselectiondialog.h>
#include <k3bjob.h>
#include <k3bsignalwaiter.h>
#include "k3bmediaselectiondialog.h"
#include "k3bmediacache.h"
#include "k3bmedium.h"
#include "projects/k3bdatasessionimportdialog.h"
#include "k3bpassivepopup.h"
#include "k3bthemedheader.h"
#include <k3baudioserver.h>


class K3bMainWindow::Private
{
public:
  K3bDoc* lastDoc;

  TQWidgetStack* documentStack;
  K3bWelcomeWidget* welcomeWidget;
  TQWidget* documentHull;

  TQLabel* leftDocPicLabel;
  TQLabel* centerDocLabel;
  TQLabel* rightDocPicLabel;
};


K3bMainWindow::K3bMainWindow()
  : DockMainWindow(0,"K3bMainwindow")
{
  //setup splitter behavior
  manager()->setSplitterHighResolution(true);
  manager()->setSplitterOpaqueResize(true);
  manager()->setSplitterKeepSize(true);

  d = new Private;
  d->lastDoc = 0;

  setPlainCaption( i18n("K3b - The CD and DVD Kreator") );

  m_config = kapp->config();

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initActions();
  initView();
  initStatusBar();
  createGUI(0L);

  // we need the actions for the welcomewidget
  d->welcomeWidget->loadConfig( config() );

  // fill the tabs action menu
  m_documentTab->insertAction( actionFileSave );
  m_documentTab->insertAction( actionFileSaveAs );
  m_documentTab->insertAction( actionFileClose );

  // /////////////////////////////////////////////////////////////////
  // disable actions at startup
  slotStateChanged( "state_project_active", KXMLGUIClient::StateReverse );

  connect( k3bappcore->projectManager(), TQ_SIGNAL(newProject(K3bDoc*)), this, TQ_SLOT(createClient(K3bDoc*)) );
  connect( k3bcore->deviceManager(), TQ_SIGNAL(changed()), this, TQ_SLOT(slotCheckSystemTimed()) );
  connect( K3bAudioServer::instance(), TQ_SIGNAL(error(const TQString&)), this, TQ_SLOT(slotAudioServerError(const TQString&)) );

  // FIXME: now make sure the welcome screen is displayed completely
  resize( 780, 550 );
//   getMainDockWidget()->resize( getMainDockWidget()->size().expandedTo( d->welcomeWidget->sizeHint() ) );
//   m_dirTreeDock->resize( TQSize( m_dirTreeDock->sizeHint().width(), m_dirTreeDock->height() ) );

  readOptions();
}

K3bMainWindow::~K3bMainWindow()
{
  delete mainDock;
  delete m_contentsDock;

  delete d;
}


void K3bMainWindow::showEvent( TQShowEvent* e )
{
  slotCheckDockWidgetStatus();
  KDockMainWindow::showEvent( e );
}


void K3bMainWindow::initActions()
{
  // merge in the device actions from the device manager
  // operator+= is deprecated but I know no other way to do this. Why does the KDE app framework
  // need to have all actions in the mainwindow's actioncollection anyway (or am I just to stupid to
  // see the correct solution?)
  *actionCollection() += *k3bappcore->appDeviceManager()->actionCollection();

  actionFileOpen = KStdAction::open(this, TQ_SLOT(slotFileOpen()), actionCollection());
  actionFileOpenRecent = KStdAction::openRecent(this, TQ_SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  actionFileSave = KStdAction::save(this, TQ_SLOT(slotFileSave()), actionCollection());
  actionFileSaveAs = KStdAction::saveAs(this, TQ_SLOT(slotFileSaveAs()), actionCollection());
  actionFileSaveAll = new TDEAction( i18n("Save All"), "save_all", 0, this, TQ_SLOT(slotFileSaveAll()), 
				   actionCollection(), "file_save_all" );
  actionFileClose = KStdAction::close(this, TQ_SLOT(slotFileClose()), actionCollection());
  actionFileCloseAll = new TDEAction( i18n("Close All"), 0, 0, this, TQ_SLOT(slotFileCloseAll()), 
				    actionCollection(), "file_close_all" );
  actionFileQuit = KStdAction::quit(this, TQ_SLOT(slotFileQuit()), actionCollection());
  actionViewStatusBar = KStdAction::showStatusbar(this, TQ_SLOT(slotViewStatusBar()), actionCollection());
  actionSettingsConfigure = KStdAction::preferences(this, TQ_SLOT(slotSettingsConfigure()), actionCollection() );

  // the tip action
  (void)KStdAction::tipOfDay(this, TQ_SLOT(slotShowTips()), actionCollection() );
  (void)KStdAction::keyBindings( this, TQ_SLOT( slotConfigureKeys() ), actionCollection() );

  KStdAction::configureToolbars(this, TQ_SLOT(slotEditToolbars()), actionCollection());
  setStandardToolBarMenuEnabled(true);
  KStdAction::showMenubar( this, TQ_SLOT(slotShowMenuBar()), actionCollection() );

  actionFileNewMenu = new TDEActionMenu( i18n("&New Project"), "document-new", actionCollection(), "file_new" );
  actionFileNewAudio = new TDEAction(i18n("New &Audio CD Project"), "audiocd", 0, this, TQ_SLOT(slotNewAudioDoc()),
			     actionCollection(), "file_new_audio");
  actionFileNewData = new TDEAction(i18n("New Data &CD Project"), "datacd", 0, this, TQ_SLOT(slotNewDataDoc()),
			    actionCollection(), "file_new_data");
  actionFileNewMixed = new TDEAction(i18n("New &Mixed Mode CD Project"), "mixedcd", 0, this, TQ_SLOT(slotNewMixedDoc()),
				   actionCollection(), "file_new_mixed");
  actionFileNewVcd = new TDEAction(i18n("New &Video CD Project"), "videocd", 0, this, TQ_SLOT(slotNewVcdDoc()),
				   actionCollection(), "file_new_vcd");
  actionFileNewMovix = new TDEAction(i18n("New &eMovix CD Project"), "emovix", 0, this, TQ_SLOT(slotNewMovixDoc()),
				   actionCollection(), "file_new_movix");
  actionFileNewMovixDvd = new TDEAction(i18n("New &eMovix DVD Project"), "emovix", 0, this, TQ_SLOT(slotNewMovixDvdDoc()),
				      actionCollection(), "file_new_movix_dvd");
  actionFileNewDvd = new TDEAction(i18n("New Data &DVD Project"), "datadvd", 0, this, TQ_SLOT(slotNewDvdDoc()),
				 actionCollection(), "file_new_dvd");
  actionFileNewVideoDvd = new TDEAction(i18n("New V&ideo DVD Project"), "videodvd", 0, this, TQ_SLOT(slotNewVideoDvdDoc()),
				      actionCollection(), "file_new_video_dvd");
  actionFileContinueMultisession = new TDEAction( i18n("Continue Multisession Project"), "datacd", 0, this, TQ_SLOT(slotContinueMultisession()),
						actionCollection(), "file_continue_multisession" );

  actionFileNewMenu->setDelayed( false );
  actionFileNewMenu->insert( actionFileNewData );
  actionFileNewMenu->insert( actionFileNewDvd );
  actionFileNewMenu->insert( actionFileContinueMultisession );
  actionFileNewMenu->insert( new TDEActionSeparator( this ) );
  actionFileNewMenu->insert( actionFileNewAudio );
  actionFileNewMenu->insert( new TDEActionSeparator( this ) );
  actionFileNewMenu->insert( actionFileNewMixed );
  actionFileNewMenu->insert( new TDEActionSeparator( this ) );
  actionFileNewMenu->insert( actionFileNewVcd );
  actionFileNewMenu->insert( actionFileNewVideoDvd );
  actionFileNewMenu->insert( new TDEActionSeparator( this ) );
  actionFileNewMenu->insert( actionFileNewMovix );
  actionFileNewMenu->insert( actionFileNewMovixDvd );





  actionProjectAddFiles = new TDEAction( i18n("&Add Files..."), "document-new", 0, this, TQ_SLOT(slotProjectAddFiles()),
				       actionCollection(), "project_add_files");

  TDEAction* actionClearProject = new TDEAction( i18n("&Clear Project"), TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase", 0, 
					     this, TQ_SLOT(slotClearProject()), actionCollection(), "project_clear_project" );

  actionViewDirTreeView = new TDEToggleAction(i18n("Show Directories"), 0, this, TQ_SLOT(slotShowDirTreeView()),
					    actionCollection(), "view_dir_tree");

  actionViewContentsView = new TDEToggleAction(i18n("Show Contents"), 0, this, TQ_SLOT(slotShowContentsView()),
					     actionCollection(), "view_contents");

  actionViewDocumentHeader = new TDEToggleAction(i18n("Show Document Header"), 0, this, TQ_SLOT(slotViewDocumentHeader()),
					       actionCollection(), "view_document_header");

  actionToolsBlankCdrw = new TDEAction( i18n("&Erase CD-RW..."), "erasecd", 0, this, TQ_SLOT(slotBlankCdrw()),
				      actionCollection(), "tools_blank_cdrw" );
  TDEAction* actionToolsFormatDVD = new TDEAction( i18n("&Format DVD%1RW...").arg("�"), "formatdvd", 0, this, 
					       TQ_SLOT(slotFormatDvd()), actionCollection(), "tools_format_dvd" );
  actionToolsWriteCdImage = new TDEAction(i18n("&Burn CD Image..."), "burn_cdimage", 0, this, TQ_SLOT(slotWriteCdImage()),
					 actionCollection(), "tools_write_cd_image" );
  TDEAction* actionToolsWriteDvdImage = new TDEAction(i18n("&Burn DVD ISO Image..."), "burn_dvdimage", 0, this, TQ_SLOT(slotWriteDvdIsoImage()),
						 actionCollection(), "tools_write_dvd_iso" );

  actionCdCopy = new TDEAction(i18n("&Copy CD..."), "cdcopy", 0, this, TQ_SLOT(slotCdCopy()),
			     actionCollection(), "tools_copy_cd" );

  TDEAction* actionToolsDvdCopy = new TDEAction(i18n("Copy &DVD..."), "dvdcopy", 0, this, TQ_SLOT(slotDvdCopy()),
					    actionCollection(), "tools_copy_dvd" );

  actionToolsCddaRip = new TDEAction( i18n("Rip Audio CD..."), "cddarip", 0, this, TQ_SLOT(slotCddaRip()),
				    actionCollection(), "tools_cdda_rip" );
  actionToolsVideoDvdRip = new TDEAction( i18n("Rip Video DVD..."), "videodvd", 0, this, TQ_SLOT(slotVideoDvdRip()),
				    actionCollection(), "tools_videodvd_rip" );
  actionToolsVideoCdRip = new TDEAction( i18n("Rip Video CD..."), "videocd", 0, this, TQ_SLOT(slotVideoCdRip()),
				       actionCollection(), "tools_videocd_rip" );

  (void)new TDEAction( i18n("System Check"), 0, 0, this, TQ_SLOT(slotCheckSystem()),
		     actionCollection(), "help_check_system" );

#ifdef HAVE_K3BSETUP
  actionSettingsK3bSetup = new TDEAction(i18n("&Setup System Permissions..."), "configure", 0, this, TQ_SLOT(slotK3bSetup()),
				       actionCollection(), "settings_k3bsetup" );
#endif

#ifdef K3B_DEBUG
  (void)new TDEAction( "Test Media Selection ComboBox", 0, 0, this, 
		     TQ_SLOT(slotMediaSelectionTester()), actionCollection(),
		     "test_media_selection" );
#endif

  actionFileNewMenu->setToolTip(i18n("Creates a new project"));
  actionFileNewData->setToolTip( i18n("Creates a new data CD project") );
  actionFileNewAudio->setToolTip( i18n("Creates a new audio CD project") );
  actionFileNewMovixDvd->setToolTip( i18n("Creates a new eMovix DVD project") );
  actionFileNewDvd->setToolTip( i18n("Creates a new data DVD project") );
  actionFileNewMovix->setToolTip( i18n("Creates a new eMovix CD project") );
  actionFileNewVcd->setToolTip( i18n("Creates a new Video CD project") );
  actionToolsBlankCdrw->setToolTip( i18n("Open the CD-RW erasing dialog") );
  actionToolsFormatDVD->setToolTip( i18n("Open the DVD%1RW formatting dialog").arg("�") );
  actionCdCopy->setToolTip( i18n("Open the CD copy dialog") );
  actionToolsWriteCdImage->setToolTip( i18n("Write an Iso9660, cue/bin, or cdrecord clone image to CD") );
  actionToolsWriteDvdImage->setToolTip( i18n("Write an Iso9660 image to DVD") );
  actionToolsDvdCopy->setToolTip( i18n("Open the DVD copy dialog") );
  actionFileOpen->setToolTip(i18n("Opens an existing project"));
  actionFileOpenRecent->setToolTip(i18n("Opens a recently used file"));
  actionFileSave->setToolTip(i18n("Saves the current project"));
  actionFileSaveAs->setToolTip(i18n("Saves the current project to a new url"));
  actionFileSaveAll->setToolTip(i18n("Saves all open projects"));
  actionFileClose->setToolTip(i18n("Closes the current project"));
  actionFileCloseAll->setToolTip(i18n("Closes all open projects"));
  actionFileQuit->setToolTip(i18n("Quits the application"));
  actionSettingsConfigure->setToolTip( i18n("Configure K3b settings") );
#ifdef HAVE_K3BSETUP
  actionSettingsK3bSetup->setToolTip( i18n("Setup the system permissions (requires root privileges)") );
#endif
  actionToolsCddaRip->setToolTip( i18n("Digitally extract tracks from an audio CD") );
  actionToolsVideoDvdRip->setToolTip( i18n("Transcode Video DVD titles") );
  actionToolsVideoCdRip->setToolTip( i18n("Extract tracks from a Video CD") );
  actionProjectAddFiles->setToolTip( i18n("Add files to the current project") );
  actionClearProject->setToolTip( i18n("Clear the current project") );

  // make sure the tooltips are used for the menu
  actionCollection()->setHighlightingEnabled( true );
}



const TQPtrList<K3bDoc>& K3bMainWindow::projects() const
{
  return k3bappcore->projectManager()->projects();
}


void K3bMainWindow::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
}

void K3bMainWindow::initStatusBar()
{
  m_statusBarManager = new K3bStatusBarManager( this );
}


void K3bMainWindow::initView()
{
  // setup main docking things
  mainDock = createDockWidget( "project_view", SmallIcon("idea"), 0,
			       kapp->makeStdCaption( i18n("Project View") ), i18n("Project View") );
  mainDock->setDockSite( KDockWidget::DockCorner );
  mainDock->setEnableDocking( KDockWidget::DockNone );
  setView( mainDock );
  setMainDockWidget( mainDock );

  // --- Document Dock ----------------------------------------------------------------------------
  d->documentStack = new TQWidgetStack( mainDock );
  mainDock->setWidget( d->documentStack );

  d->documentHull = new TQWidget( d->documentStack );
  d->documentStack->addWidget( d->documentHull );
  TQGridLayout* documentHullLayout = new TQGridLayout( d->documentHull );
  documentHullLayout->setMargin( 2 );
  documentHullLayout->setSpacing( 0 );

  m_documentHeader = new K3bThemedHeader( d->documentHull );
  m_documentHeader->setTitle( i18n("Current Projects") );
  m_documentHeader->setAlignment( TQt::AlignHCenter | TQt::AlignVCenter );
  m_documentHeader->setLeftPixmap( K3bTheme::PROJECT_LEFT );
  m_documentHeader->setRightPixmap( K3bTheme::PROJECT_RIGHT );

  // add the document tab to the styled document box
  m_documentTab = new K3bProjectTabWidget( d->documentHull );

  documentHullLayout->addWidget( m_documentHeader, 0, 0 );
  documentHullLayout->addWidget( m_documentTab, 1, 0 );

  connect( m_documentTab, TQ_SIGNAL(currentChanged(TQWidget*)), this, TQ_SLOT(slotCurrentDocChanged()) );

  d->welcomeWidget = new K3bWelcomeWidget( this, m_documentTab );
  m_documentTab->addTab( d->welcomeWidget, i18n("Quickstart") );

//   d->documentStack->addWidget( d->welcomeWidget );
//   d->documentStack->raiseWidget( d->welcomeWidget );
  // ---------------------------------------------------------------------------------------------

  // --- Directory Dock --------------------------------------------------------------------------
  m_dirTreeDock = createDockWidget( "directory_tree", SmallIcon("folder"), 0,
				    kapp->makeStdCaption( i18n("Sidepanel") ), i18n("Sidepanel") );
  m_dirTreeDock->setEnableDocking( KDockWidget::DockCorner );

  K3bFileTreeView* sidePanel = new K3bFileTreeView( m_dirTreeDock );
  //K3bSidePanel* sidePanel = new K3bSidePanel( this, m_dirTreeDock, "sidePanel" );

  m_dirTreeDock->setWidget( sidePanel );
  m_dirTreeDock->manualDock( mainDock, KDockWidget::DockTop, 4000 );
  connect( m_dirTreeDock, TQ_SIGNAL(iMBeingClosed()), this, TQ_SLOT(slotDirTreeDockHidden()) );
  connect( m_dirTreeDock, TQ_SIGNAL(hasUndocked()), this, TQ_SLOT(slotDirTreeDockHidden()) );
  // ---------------------------------------------------------------------------------------------

  // --- Contents Dock ---------------------------------------------------------------------------
  m_contentsDock = createDockWidget( "contents_view", SmallIcon("idea"), 0,
			      kapp->makeStdCaption( i18n("Contents View") ), i18n("Contents View") );
  m_contentsDock->setEnableDocking( KDockWidget::DockCorner );
  m_dirView = new K3bDirView( sidePanel/*->fileTreeView()*/, m_contentsDock );
  m_contentsDock->setWidget( m_dirView );
  m_contentsDock->manualDock( m_dirTreeDock, KDockWidget::DockRight, 2000 );

  connect( m_contentsDock, TQ_SIGNAL(iMBeingClosed()), this, TQ_SLOT(slotContentsDockHidden()) );
  connect( m_contentsDock, TQ_SIGNAL(hasUndocked()), this, TQ_SLOT(slotContentsDockHidden()) );
  // ---------------------------------------------------------------------------------------------

  // --- filetreecombobox-toolbar ----------------------------------------------------------------
  K3bFileTreeComboBox* m_fileTreeComboBox = new K3bFileTreeComboBox( 0 );
  connect( m_fileTreeComboBox, TQ_SIGNAL(urlExecuted(const KURL&)), m_dirView, TQ_SLOT(showUrl(const KURL& )) );
  connect( m_fileTreeComboBox, TQ_SIGNAL(deviceExecuted(K3bDevice::Device*)), m_dirView,
	   TQ_SLOT(showDevice(K3bDevice::Device* )) );
  connect( m_dirView, TQ_SIGNAL(urlEntered(const KURL&)), m_fileTreeComboBox, TQ_SLOT(setUrl(const KURL&)) );
  connect( m_dirView, TQ_SIGNAL(deviceSelected(K3bDevice::Device*)), m_fileTreeComboBox, TQ_SLOT(setDevice(K3bDevice::Device*)) );

  KWidgetAction* fileTreeComboAction = new KWidgetAction( m_fileTreeComboBox,
							  i18n("&Quick Dir Selector"),
							  0, 0, 0,
							  actionCollection(), "quick_dir_selector" );
  fileTreeComboAction->setAutoSized(true);
  (void)new TDEAction( i18n("Go"), "key_enter", 0, m_fileTreeComboBox, TQ_SLOT(slotGoUrl()), actionCollection(), "go_url" );
  // ---------------------------------------------------------------------------------------------
}


void K3bMainWindow::createClient( K3bDoc* doc )
{
  // create the proper K3bView (maybe we should put this into some other class like K3bProjectManager)
  K3bView* view = 0;
  switch( doc->type() ) {
  case K3bDoc::AUDIO:
    view = new K3bAudioView( static_cast<K3bAudioDoc*>(doc), m_documentTab );
    break;
  case K3bDoc::DATA:
    view = new K3bDataView( static_cast<K3bDataDoc*>(doc), m_documentTab );
    break; 
  case K3bDoc::MIXED:
    {
      K3bMixedDoc* mixedDoc = static_cast<K3bMixedDoc*>(doc);
      view = new K3bMixedView( mixedDoc, m_documentTab );
      mixedDoc->dataDoc()->setView( view );
      mixedDoc->audioDoc()->setView( view );
      break; 
    }
  case K3bDoc::VCD:
    view = new K3bVcdView( static_cast<K3bVcdDoc*>(doc), m_documentTab );
    break; 
  case K3bDoc::MOVIX:
    view = new K3bMovixView( static_cast<K3bMovixDoc*>(doc), m_documentTab );
    break;
  case K3bDoc::MOVIX_DVD:
    view = new K3bMovixDvdView( static_cast<K3bMovixDvdDoc*>(doc), m_documentTab );
    break;
  case K3bDoc::DVD:
    view = new K3bDvdView( static_cast<K3bDvdDoc*>(doc), m_documentTab );
    break;
  case K3bDoc::VIDEODVD:
    view = new K3bVideoDvdView( static_cast<K3bVideoDvdDoc*>(doc), m_documentTab );
    break;
  }

  doc->setView( view );
  view->setCaption( doc->URL().fileName() );

  m_documentTab->insertTab( doc );
  m_documentTab->showPage( view );

  slotCurrentDocChanged();
}


K3bView* K3bMainWindow::activeView() const
{
  TQWidget* w = m_documentTab->currentPage();
  if( K3bView* view = dynamic_cast<K3bView*>(w) )
    return view;
  else
    return 0;
}


K3bDoc* K3bMainWindow::activeDoc() const
{
  if( activeView() )
    return activeView()->getDocument();
  else
    return 0;
}


K3bDoc* K3bMainWindow::openDocument(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  //
  // First we check if this is an iso image in case someone wants to open one this way
  //
  if( !isCdDvdImageAndIfSoOpenDialog( url ) ) {

    // see if it's an audio cue file
    K3bCueFileParser parser( url.path() );
    if( parser.isValid() && parser.toc().contentType() == K3bDevice::AUDIO ) {
      K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::AUDIO );
      doc->addUrl( url );
      return doc;
    }
    else {
      // check, if document already open. If yes, set the focus to the first view
      K3bDoc* doc = k3bappcore->projectManager()->findByUrl( url );
      if( doc ) {
	doc->view()->setFocus();
	return doc;
      }

      doc = k3bappcore->projectManager()->openProject( url );

      if( doc == 0 ) {
	KMessageBox::error (this,i18n("Could not open document!"), i18n("Error!"));
	return 0;
      }

      actionFileOpenRecent->addURL(url);

      return doc;
    }
  }
  else
    return 0;
}


void K3bMainWindow::saveOptions()
{
  actionFileOpenRecent->saveEntries( config(), "Recent Files" );

  // save dock positions!
  manager()->writeConfig( config(), "Docking Config" );

  m_dirView->saveConfig( config() );

  saveMainWindowSettings( config(), "main_window_settings" );

  k3bcore->saveSettings( config() );

  d->welcomeWidget->saveConfig( config() );

  TDEConfigGroup grp( m_config, "General Options" );
  grp.writeEntry( "Show Document Header", actionViewDocumentHeader->isChecked() );
}


void K3bMainWindow::readOptions()
{
  TDEConfigGroup grp( m_config, "General Options" );

  bool bViewDocumentHeader = grp.readBoolEntry("Show Document Header", true);
  actionViewDocumentHeader->setChecked(bViewDocumentHeader);

  // initialize the recent file list
  actionFileOpenRecent->loadEntries( config(), "Recent Files" );

  // do not read dock-positions from a config that has been saved by an old version
  K3bVersion configVersion( grp.readEntry( "config version", "0.1" ) );
  if( configVersion >= K3bVersion("0.12") )
    manager()->readConfig( config(), "Docking Config" );
  else
    kdDebug() << "(K3bMainWindow) ignoring docking config from K3b version " << configVersion << endl;

  applyMainWindowSettings( config(), "main_window_settings" );

  m_dirView->readConfig( config() );

  slotViewDocumentHeader();
  slotCheckDockWidgetStatus();
}


void K3bMainWindow::saveProperties( TDEConfig* c )
{
  // 1. put saved projects in the config
  // 2. save every modified project in  "~/.trinity/share/apps/k3b/sessions/" + KApp->sessionId()
  // 3. save the url of the project (might be something like "AudioCD1") in the config
  // 4. save the status of every project (modified/saved)

  TQString saveDir = TDEGlobal::dirs()->saveLocation( "appdata", "sessions/" + tqApp->sessionId() + "/", true );

  // FIXME: for some reason the config entries are not properly stored when using the default
  //        TDEMainWindow session config. Since I was not able to find the bug I use another config object
  // ----------------------------------------------------------
  c = new KSimpleConfig( saveDir + "list", false );
  c->setGroup( "Saved Session" );
  // ----------------------------------------------------------

  const TQPtrList<K3bDoc>& docs = k3bappcore->projectManager()->projects();
  c->writeEntry( "Number of projects", docs.count() );

  int cnt = 1;
  for( TQPtrListIterator<K3bDoc> it( docs ); *it; ++it ) {
    // the "name" of the project (or the original url if isSaved())
    c->writePathEntry( TQString("%1 url").arg(cnt), (*it)->URL().url() );

    // is the doc modified
    c->writeEntry( TQString("%1 modified").arg(cnt), (*it)->isModified() );

    // has the doc already been saved?
    c->writeEntry( TQString("%1 saved").arg(cnt), (*it)->isSaved() );

    // where does the session management save it? If it's not modified and saved this is
    // the same as the url
    KURL saveUrl = (*it)->URL();
    if( !(*it)->isSaved() || (*it)->isModified() )
      saveUrl = KURL::fromPathOrURL( saveDir + TQString::number(cnt) );
    c->writePathEntry( TQString("%1 saveurl").arg(cnt), saveUrl.url() );

    // finally save it
    k3bappcore->projectManager()->saveProject( *it, saveUrl );

    ++cnt;
  }

  // FIXME: for some reason the config entries are not properly stored when using the default
  //        TDEMainWindow session config. Since I was not able to find the bug I use another config object
  // ----------------------------------------------------------
  delete c;
  // ----------------------------------------------------------
}


// FIXME:move this to K3bProjectManager
void K3bMainWindow::readProperties( TDEConfig* c )
{
  // FIXME: do not delete the files here. rather do it when the app is exited normally
  //        since that's when we can be sure we never need the session stuff again.

  // 1. read all projects from the config
  // 2. simply open all of themg
  // 3. reset the saved urls and the modified state
  // 4. delete "~/.trinity/share/apps/k3b/sessions/" + KApp->sessionId()

  TQString saveDir = TDEGlobal::dirs()->saveLocation( "appdata", "sessions/" + tqApp->sessionId() + "/", true );

  // FIXME: for some reason the config entries are not properly stored when using the default
  //        TDEMainWindow session config. Since I was not able to find the bug I use another config object
  // ----------------------------------------------------------
  c = new KSimpleConfig( saveDir + "list", true );
  c->setGroup( "Saved Session" );
  // ----------------------------------------------------------

  int cnt = c->readNumEntry( "Number of projects", 0 );
  kdDebug() << "(K3bMainWindow::readProperties) number of projects from last session in " << saveDir << ": " << cnt << endl
	    << "                                read from config group " << c->group() << endl;

  for( int i = 1; i <= cnt; ++i ) {
    // in this case the constructor works since we saved as url()
    KURL url = c->readPathEntry( TQString("%1 url").arg(i) );

    bool modified = c->readBoolEntry( TQString("%1 modified").arg(i) );

    bool saved = c->readBoolEntry( TQString("%1 saved").arg(i) );

    KURL saveUrl = c->readPathEntry( TQString("%1 saveurl").arg(i) );

    // now load the project
    if( K3bDoc* doc = k3bappcore->projectManager()->openProject( saveUrl ) ) {

      // reset the url
      doc->setURL( url );
      doc->setModified( modified );
      doc->setSaved( saved );
    }
    else
      kdDebug() << "(K3bMainWindow) could not open session saved doc " << url.path() << endl;

    // remove the temp file
    if( !saved || modified )
      TQFile::remove( saveUrl.path() );
  }

  // and now remove the temp dir
  TDEIO::del( KURL::fromPathOrURL(saveDir), false, false );

  // FIXME: for some reason the config entries are not properly stored when using the default
  //        TDEMainWindow session config. Since I was not able to find the bug I use another config object
  // ----------------------------------------------------------
  delete c;
  // ----------------------------------------------------------
}


bool K3bMainWindow::queryClose()
{
  //
  // Check if a job is currently running
  // For now K3b only allows for one major job at a time which means that we only need to cancel
  // this one job.
  //
  if( k3bcore->jobsRunning() ) {

    // pitty, but I see no possibility to make this work. It always crashes because of the event
    // management thing mentioned below. So until I find a solution K3b simply will refuse to close
    // while a job i running
    return false;

//     kdDebug() << "(K3bMainWindow::queryClose) jobs running." << endl;
//     K3bJob* job = k3bcore->runningJobs().getFirst();
    
//     // now search for the major job (to be on the safe side although for now no subjobs register with the k3bcore)
//     K3bJobHandler* jh = job->jobHandler();
//     while( jh->isJob() ) {
//       job = static_cast<K3bJob*>( jh );
//       jh = job->jobHandler();
//     }

//     kdDebug() << "(K3bMainWindow::queryClose) main job found: " << job->jobDescription() << endl;

//     // now job is the major job and jh should be a widget
//     TQWidget* progressDialog = dynamic_cast<TQWidget*>( jh );

//     kdDebug() << "(K3bMainWindow::queryClose) job active: " << job->active() << endl;

//     // now ask the user if he/she really wants to cancel this job
//     if( job->active() ) {
//       if( KMessageBox::questionYesNo( progressDialog ? progressDialog : this,
// 				      i18n("Do you really want to cancel?"), 
// 				      i18n("Cancel") ) == KMessageBox::Yes ) {
// 	// cancel the job
// 	kdDebug() << "(K3bMainWindow::queryClose) canceling job." << endl;
// 	job->cancel();

// 	// wait for the job to finish
// 	kdDebug() << "(K3bMainWindow::queryClose) waiting for job to finish." << endl;
// 	K3bSignalWaiter::waitForJob( job );

// 	// close the progress dialog
// 	if( progressDialog ) {
// 	  kdDebug() << "(K3bMainWindow::queryClose) closing progress dialog." << endl;
// 	  progressDialog->close();
// 	  //
// 	  // now here we have the problem that due to the whole TQt event thing the exec call (or
// 	  // in this case most likely the startJob call) does not return until we leave this method.
// 	  // That means that the progress dialog might be deleted by it's parent below (when we 
// 	  // close docs) before it is deleted by the creator (most likely a projectburndialog).
// 	  // That would result in a double deletion and thus a crash.
// 	  // So we just reparent the dialog to 0 here so it's (former) parent won't delete it.
// 	  //
// 	  progressDialog->reparent( 0, TQPoint(0,0) );
// 	}

// 	kdDebug() << "(K3bMainWindow::queryClose) job cleanup done." << endl;
//       }
//       else
// 	return false;
//     }
  }

  //
  // if we are closed by the session manager everything is fine since we store the
  // current state in saveProperties
  //
  if( kapp->sessionSaving() ) 
    return true;

  // FIXME: do not close the docs here. Just ask for them to be saved and return false
  //        if the user chose cancel for some doc

  // ---------------------------------
  // we need to manually close all the views to ensure that
  // each of them receives a close-event and
  // the user is asked for every modified doc to save the changes
  // ---------------------------------

  while( K3bView* view = activeView() ) {
    if( !canCloseDocument(view->doc()) )
      return false;
    closeProject(view->doc());
  }

  return true;
}


bool K3bMainWindow::canCloseDocument( K3bDoc* doc )
{
  if( !doc->isModified() )
    return true;

  if( !TDEConfigGroup( config(), "General Options" ).readBoolEntry( "ask_for_saving_changes_on_exit", true ) )
    return true;

  switch ( KMessageBox::warningYesNoCancel( this, 
					    i18n("%1 has unsaved data.").arg( doc->URL().fileName() ),
					    i18n("Closing Project"), 
					    KStdGuiItem::save(),
					    KGuiItem( i18n("&Discard"), "editshred" ) ) )
    {
    case KMessageBox::Yes:
      fileSave( doc );
    case KMessageBox::No:
      return true;

    default:
      return false;
    }
}

bool K3bMainWindow::queryExit()
{
  // TODO: call this in K3bApplication somewhere
  saveOptions();
  return true;
}



/////////////////////////////////////////////////////////////////////
// TQ_SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void K3bMainWindow::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KURL::List urls = KFileDialog::getOpenURLs( ":k3b-projects-folder",
					      i18n("*.k3b|K3b Projects"),
					      this,
					      i18n("Open Files") );
  for( KURL::List::iterator it = urls.begin(); it != urls.end(); ++it ) {
    openDocument( *it );
    actionFileOpenRecent->addURL( *it );
  }
}

void K3bMainWindow::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  openDocument(url);
}


void K3bMainWindow::slotFileSaveAll()
{
  for( TQPtrListIterator<K3bDoc> it( k3bappcore->projectManager()->projects() );
       *it; ++it ) {
    fileSave( *it );
  }
}


void K3bMainWindow::slotFileSave()
{
  if( K3bDoc* doc = activeDoc() ) {
    fileSave( doc );
  }
}

void K3bMainWindow::fileSave( K3bDoc* doc )
{
  slotStatusMsg(i18n("Saving file..."));

  if( doc == 0 ) {
    doc = activeDoc();
  }
  if( doc != 0 ) {
    if( !doc->isSaved() )
      fileSaveAs( doc );
    else if( !k3bappcore->projectManager()->saveProject( doc, doc->URL()) )
      KMessageBox::error (this,i18n("Could not save the current document!"), i18n("I/O Error"));
  }
}


void K3bMainWindow::slotFileSaveAs()
{
  if( K3bDoc* doc = activeDoc() ) {
    fileSaveAs( doc );
  }
}


void K3bMainWindow::fileSaveAs( K3bDoc* doc )
{
  slotStatusMsg(i18n("Saving file with a new filename..."));

  if( doc == 0 ) {
    doc = activeDoc();
  }

  if( doc != 0 ) {
    // we do not use the static KFileDialog method here to be able to specify a filename suggestion
    KFileDialog dlg( ":k3b-projects-folder", i18n("*.k3b|K3b Projects"), this, "filedialog", true );
    dlg.setCaption( i18n("Save As") );
    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setSelection( doc->name() );
    dlg.exec();
    KURL url = dlg.selectedURL();

    if( url.isValid() ) {
      TDERecentDocument::add( url );

      bool exists = TDEIO::NetAccess::exists( url, false, 0 );
      if( !exists ||
	  ( exists &&
	    KMessageBox::warningContinueCancel( this, i18n("Do you want to overwrite %1?").arg( url.prettyURL() ), 
						i18n("File Exists"), i18n("Overwrite") )
	    == KMessageBox::Continue ) ) {

	if( !k3bappcore->projectManager()->saveProject( doc, url ) ) {
	  KMessageBox::error (this,i18n("Could not save the current document!"), i18n("I/O Error"));
	  return;
	}
	else
	  actionFileOpenRecent->addURL(url);
      }
    }
  }
}


void K3bMainWindow::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));
  if( K3bView* pView = activeView() ) {
    if( pView ) {
      K3bDoc* pDoc = pView->doc();

      if( canCloseDocument(pDoc) ) {
	closeProject(pDoc);
      }
    }
  }

  slotCurrentDocChanged();
}


void K3bMainWindow::slotFileCloseAll()
{
  while( K3bView* pView = activeView() ) {
    if( pView ) {
      K3bDoc* pDoc = pView->doc();

      if( canCloseDocument(pDoc) )
	closeProject(pDoc);
      else
	break;
    }
  }

  slotCurrentDocChanged();
}


void K3bMainWindow::closeProject( K3bDoc* doc )
{
  // unplug the actions
  if( factory() ) {
    if( d->lastDoc == doc ) {
      factory()->removeClient( static_cast<K3bView*>(d->lastDoc->view()) );
      d->lastDoc = 0;
    }
  }

  // remove the view from the project tab
  m_documentTab->removePage( doc->view() );

  // remove the project from the manager
  k3bappcore->projectManager()->removeProject( doc );

  // delete view and doc
  delete doc->view();
  delete doc;
}


void K3bMainWindow::slotFileQuit()
{
  close();
}


void K3bMainWindow::slotViewStatusBar()
{
  //turn Statusbar on or off
  if(actionViewStatusBar->isChecked()) {
    statusBar()->show();
  }
  else {
    statusBar()->hide();
  }
}


void K3bMainWindow::slotStatusMsg(const TQString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
//   statusBar()->clear();
//   statusBar()->changeItem(text,1);

  statusBar()->message( text, 2000 );
}


void K3bMainWindow::slotSettingsConfigure()
{
  K3bOptionDialog d( this, "SettingsDialog", true );

  d.exec();

  // emit a changed signal every time since we do not know if the user selected
  // "apply" and "cancel" or "ok"
  emit configChanged( m_config );
}


void K3bMainWindow::showOptionDialog( int index )
{
  K3bOptionDialog d( this, "SettingsDialog", true );

  d.showPage( index );

  d.exec();

  // emit a changed signal every time since we do not know if the user selected
  // "apply" and "cancel" or "ok"
  emit configChanged( m_config );
}


K3bDoc* K3bMainWindow::slotNewAudioDoc()
{
  slotStatusMsg(i18n("Creating new Audio CD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::AUDIO );

  return doc;
}

K3bDoc* K3bMainWindow::slotNewDataDoc()
{
  slotStatusMsg(i18n("Creating new Data CD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::DATA );

  return doc;
}


K3bDoc* K3bMainWindow::slotNewDvdDoc()
{
  slotStatusMsg(i18n("Creating new Data DVD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::DVD );

  return doc;
}


K3bDoc* K3bMainWindow::slotContinueMultisession()
{
  return K3bDataSessionImportDialog::importSession( 0, this );
}


K3bDoc* K3bMainWindow::slotNewVideoDvdDoc()
{
  slotStatusMsg(i18n("Creating new VideoDVD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::VIDEODVD );

  return doc;
}


K3bDoc* K3bMainWindow::slotNewMixedDoc()
{
  slotStatusMsg(i18n("Creating new Mixed Mode CD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::MIXED );

  return doc;
}

K3bDoc* K3bMainWindow::slotNewVcdDoc()
{
  slotStatusMsg(i18n("Creating new Video CD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::VCD );

  return doc;
}


K3bDoc* K3bMainWindow::slotNewMovixDoc()
{
  slotStatusMsg(i18n("Creating new eMovix CD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::MOVIX );

  return doc;
}


K3bDoc* K3bMainWindow::slotNewMovixDvdDoc()
{
  slotStatusMsg(i18n("Creating new eMovix DVD Project."));

  K3bDoc* doc = k3bappcore->projectManager()->createProject( K3bDoc::MOVIX_DVD );

  return doc;
}


void K3bMainWindow::slotCurrentDocChanged()
{
  // check the doctype
  K3bView* v = activeView();
  if( v ) {
    k3bappcore->projectManager()->setActive( v->doc() );

    //
    // There are two possiblities to plug the project actions:
    // 1. Through KXMLGUIClient::plugActionList
    //    This way we just ask the View for the actionCollection (which it should merge with
    //    the doc's) and plug it into the project menu.
    //    Advantage: easy and clear to handle
    //    Disadvantage: we may only plug all actions at once into one menu
    //
    // 2. Through merging the doc as a KXMLGUIClient
    //    This way every view is a KXMLGUIClient and it's GUI is just merged into the MainWindow's.
    //    Advantage: flexible
    //    Disadvantage: every view needs it's own XML file
    //
    //

    if( factory() ) {
      if( d->lastDoc )
	factory()->removeClient( static_cast<K3bView*>(d->lastDoc->view()) );
      factory()->addClient( v );
      d->lastDoc = v->doc();
    }
    else
      kdDebug() << "(K3bMainWindow) ERROR: could not get KXMLGUIFactory instance." << endl;
  }
  else
    k3bappcore->projectManager()->setActive( 0L );

  if( k3bappcore->projectManager()->isEmpty() ) {
    slotStateChanged( "state_project_active", KXMLGUIClient::StateReverse );
  }
  else {
    d->documentStack->raiseWidget( d->documentHull );
    slotStateChanged( "state_project_active", KXMLGUIClient::StateNoReverse );
  }

  // make sure the document header is shown (or not)
  slotViewDocumentHeader();
}


void K3bMainWindow::slotEditToolbars()
{
  saveMainWindowSettings( m_config, "main_window_settings" );
  KEditToolbar dlg( factory() );
  connect(&dlg, TQ_SIGNAL(newToolbarConfig()), TQ_SLOT(slotNewToolBarConfig()));
  dlg.exec();
}


void K3bMainWindow::slotNewToolBarConfig()
{
  applyMainWindowSettings( m_config, "main_window_settings" );
}


bool K3bMainWindow::eject()
{
  TDEConfigGroup c( config(), "General Options" );
  return !c.readBoolEntry( "No cd eject", false );
}


void K3bMainWindow::slotErrorMessage(const TQString& message)
{
  KMessageBox::error( this, message );
}


void K3bMainWindow::slotWarningMessage(const TQString& message)
{
  KMessageBox::sorry( this, message );
}


void K3bMainWindow::slotWriteCdImage()
{
  K3bCdImageWritingDialog d( this );
  d.exec(false);
}


void K3bMainWindow::slotWriteDvdIsoImage()
{
  K3bIsoImageWritingDialog d( this );
  d.exec(false);
}


void K3bMainWindow::slotWriteDvdIsoImage( const KURL& url )
{
  K3bIsoImageWritingDialog d( this );
  d.setImage( url );
  d.exec(false);
}


void K3bMainWindow::slotWriteCdImage( const KURL& url )
{
  K3bCdImageWritingDialog d( this );
  d.setImage( url );
  d.exec(false);
}


void K3bMainWindow::slotProjectAddFiles()
{
  K3bView* view = activeView();

  if( view ) {
    TQStringList files = KFileDialog::getOpenFileNames( ":k3b-project-add-files", 
						       i18n("*|All Files"), 
						       this, 
						       i18n("Select Files to Add to Project") );

    KURL::List urls;
    for( TQStringList::ConstIterator it = files.begin();
         it != files.end(); it++ ) {
      KURL url;
      url.setPath(*it);
      urls.append( url );
    }

    if( !urls.isEmpty() )
      view->addUrls( urls );
  }
  else
    KMessageBox::error( this, i18n("Please create a project before adding files"), i18n("No Active Project"));
}


void K3bMainWindow::slotK3bSetup()
{
  TDEProcess p;
  p << "tdesu" << "tdecmshell k3bsetup2 --lang " + TDEGlobal::locale()->language();
  if( !p.start( TDEProcess::DontCare ) )
    KMessageBox::error( 0, i18n("Could not find tdesu to run K3bSetup with root privileges. "
				"Please run it manually as root.") );
}


void K3bMainWindow::blankCdrw( K3bDevice::Device* dev )
{
  K3bBlankingDialog dlg( this, "blankingdialog" );
  dlg.setDevice( dev );
  dlg.exec(false);
}


void K3bMainWindow::slotBlankCdrw()
{
  blankCdrw( 0 );
}


void K3bMainWindow::formatDvd( K3bDevice::Device* dev )
{
  K3bDvdFormattingDialog d( this );
  d.setDevice( dev );
  d.exec(false);
}


void K3bMainWindow::slotFormatDvd()
{
  formatDvd( 0 );
}


void K3bMainWindow::cdCopy( K3bDevice::Device* dev )
{
  K3bCdCopyDialog d( this );
  d.setReadingDevice( dev );
  d.exec(false);
}


void K3bMainWindow::slotCdCopy()
{
  cdCopy( 0 );
}


void K3bMainWindow::dvdCopy( K3bDevice::Device* dev )
{
  K3bDvdCopyDialog d( this );
  d.setReadingDevice( dev );
  d.exec(false);
}


void K3bMainWindow::slotDvdCopy()
{
  dvdCopy( 0 );
}


// void K3bMainWindow::slotVideoDvdCopy()
// {
//   K3bVideoDvdCopyDialog d( this );
//   d.exec();
// }


void K3bMainWindow::slotShowDirTreeView()
{
  m_dirTreeDock->changeHideShowState();
  slotCheckDockWidgetStatus();
}


void K3bMainWindow::slotShowContentsView()
{
  m_contentsDock->changeHideShowState();
  slotCheckDockWidgetStatus();
}


void K3bMainWindow::slotShowMenuBar() 
{ 
  if( menuBar()->isVisible() ) 
    menuBar()->hide(); 
  else 
    menuBar()->show(); 
}


void K3bMainWindow::slotShowTips()
{
  KTipDialog::showTip( this, TQString(), true );
}


void K3bMainWindow::slotDirTreeDockHidden()
{
  actionViewDirTreeView->setChecked( false );
}


void K3bMainWindow::slotContentsDockHidden()
{
  actionViewContentsView->setChecked( false );
}


void K3bMainWindow::slotCheckDockWidgetStatus()
{
  actionViewContentsView->setChecked( m_contentsDock->isVisible() );
  actionViewDirTreeView->setChecked( m_dirTreeDock->isVisible() );
}


void K3bMainWindow::slotViewDocumentHeader()
{
  if( actionViewDocumentHeader->isChecked() &&
      !k3bappcore->projectManager()->isEmpty() ) {
    m_documentHeader->show();
  }
  else {
    m_documentHeader->hide();
  }
}


K3bExternalBinManager* K3bMainWindow::externalBinManager() const
{
  return k3bcore->externalBinManager();
}


K3bDevice::DeviceManager* K3bMainWindow::deviceManager() const
{
  return k3bcore->deviceManager();
}


void K3bMainWindow::slotDataImportSession()
{
  if( activeView() ) {
    if( K3bDataView* view = dynamic_cast<K3bDataView*>(activeView()) ) {
      view->importSession();
    }
  }
}


void K3bMainWindow::slotDataClearImportedSession()
{
  if( activeView() ) {
    if( K3bDataView* view = dynamic_cast<K3bDataView*>(activeView()) ) {
      view->clearImportedSession();
    }
  }
}


void K3bMainWindow::slotEditBootImages()
{
  if( activeView() ) {
    if( K3bDataView* view = dynamic_cast<K3bDataView*>(activeView()) ) {
      view->editBootImages();
    }
  }
}


void K3bMainWindow::slotCheckSystemTimed()
{
  // run the system check from the event queue so we do not
  // mess with the device state resetting throughout the app
  // when called from K3bDeviceManager::changed
  TQTimer::singleShot( 0, this, TQ_SLOT(slotCheckSystem()) );
}


void K3bMainWindow::slotCheckSystem()
{
  K3bSystemProblemDialog::checkSystem( this );
}


void K3bMainWindow::addUrls( const KURL::List& urls )
{
  if( K3bView* view = activeView() ) {
    view->addUrls( urls );
  }
  else {
    // check if the files are all audio we can handle. If so create an audio project
    bool audio = true;
    TQPtrList<K3bPlugin> fl = k3bcore->pluginManager()->plugins( "AudioDecoder" );
    for( KURL::List::const_iterator it = urls.begin(); it != urls.end(); ++it ) {
      const KURL& url = *it;

      if( TQFileInfo(url.path()).isDir() ) {
	audio = false;
	break;
      }

      bool a = false;
      for( TQPtrListIterator<K3bPlugin> it( fl ); it.current(); ++it ) {
	if( static_cast<K3bAudioDecoderFactory*>(it.current())->canDecode( url ) ) {
	  a = true;
	  break;
	}
      }
      if( !a ) {
	audio = a;
	break;
      }
    }

    if( !audio && urls.count() == 1 ) {
      // see if it's an audio cue file
      K3bCueFileParser parser( urls.first().path() );
      if( parser.isValid() && parser.toc().contentType() == K3bDevice::AUDIO ) {
	audio = true;
      }
    }

    if( audio )
      static_cast<K3bView*>(slotNewAudioDoc()->view())->addUrls( urls );
    else if( urls.count() > 1 || !isCdDvdImageAndIfSoOpenDialog( urls.first() ) )
      static_cast<K3bView*>(slotNewDataDoc()->view())->addUrls( urls );
  }
}


void K3bMainWindow::slotClearProject()
{
  K3bDoc* doc = k3bappcore->projectManager()->activeDoc();
  if( doc ) {
    if( KMessageBox::warningContinueCancel( this,
				    i18n("Do you really want to clear the current project?"),
				    i18n("Clear Project"),
				    i18n("Clear"),
				    "clear_current_project_dontAskAgain" ) == KMessageBox::Continue ) {
      doc->newDocument();
      k3bappcore->projectManager()->loadDefaults( doc );
    }
  }
}


bool K3bMainWindow::isCdDvdImageAndIfSoOpenDialog( const KURL& url )
{
  K3bIso9660 iso( url.path() );
  if( iso.open() ) {
    iso.close();
    // very rough dvd image size test
    if( K3b::filesize( url ) > 1000*1024*1024 )
      slotWriteDvdIsoImage( url );
    else
      slotWriteCdImage( url );

    return true;
  }
  else
    return false;
}


void K3bMainWindow::slotCddaRip()
{
  cddaRip( 0 );
}


void K3bMainWindow::cddaRip( K3bDevice::Device* dev )
{
  if( !dev ||
      !(k3bappcore->mediaCache()->medium( dev ).content() & K3bMedium::CONTENT_AUDIO ) )
    dev = K3bMediaSelectionDialog::selectMedium( K3bDevice::MEDIA_CD_ALL, 
						 K3bDevice::STATE_COMPLETE|K3bDevice::STATE_INCOMPLETE,
						 K3bMedium::CONTENT_AUDIO, 
						 this,
						 i18n("Audio CD Rip") );

  if( dev )
    m_dirView->showDevice( dev );
}


void K3bMainWindow::videoDvdRip( K3bDevice::Device* dev )
{
  if( !dev ||
      !(k3bappcore->mediaCache()->medium( dev ).content() & K3bMedium::CONTENT_VIDEO_DVD ) )
    dev = K3bMediaSelectionDialog::selectMedium( K3bDevice::MEDIA_DVD_ALL, 
						 K3bDevice::STATE_COMPLETE,
						 K3bMedium::CONTENT_VIDEO_DVD, 
						 this,
						 i18n("Video DVD Rip") );

  if( dev )
    m_dirView->showDevice( dev );
}


void K3bMainWindow::slotVideoDvdRip()
{
  videoDvdRip( 0 );
}


void K3bMainWindow::videoCdRip( K3bDevice::Device* dev )
{
  if( !dev ||
      !(k3bappcore->mediaCache()->medium( dev ).content() & K3bMedium::CONTENT_VIDEO_CD ) )
    dev = K3bMediaSelectionDialog::selectMedium( K3bDevice::MEDIA_CD_ALL, 
						 K3bDevice::STATE_COMPLETE,
						 K3bMedium::CONTENT_VIDEO_CD, 
						 this,
						 i18n("Video CD Rip") );

  if( dev )
    m_dirView->showDevice( dev );
}


void K3bMainWindow::slotVideoCdRip()
{
  videoCdRip( 0 );
}


void K3bMainWindow::slotAudioServerError( const TQString& error )
{
  K3bPassivePopup::showPopup( error, i18n("Audio Output Problem") );
}

#include "k3b.moc"

