/*
 *
 * $Id: k3bcdimagewritingdialog.cpp 630454 2007-02-05 13:06:45Z trueg $
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

#include "k3bcdimagewritingdialog.h"
#include "k3biso9660imagewritingjob.h"
#include "k3bbinimagewritingjob.h"
#include "k3bcuefileparser.h"
#include "k3bclonetocreader.h"
#include "k3baudiocuefilewritingjob.h"
#include <k3bclonejob.h>

#include <k3btempdirselectionwidget.h>
#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bwriterselectionwidget.h>
#include <k3bburnprogressdialog.h>
#include <kcutlabel.h>
#include <k3bstdguiitems.h>
#include <k3bmd5job.h>
#include <k3bdatamodewidget.h>
#include <k3bglobals.h>
#include <k3bwritingmodewidget.h>
#include <k3bcore.h>
#include <k3blistview.h>
#include <k3biso9660.h>
#include <k3btoc.h>
#include <k3btrack.h>
#include <k3bcdtext.h>

#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kio/global.h>
#include <kurl.h>
#include <kinputdialog.h>
#include <kurldrag.h>

#include <tqheader.h>
#include <tqgroupbox.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqptrlist.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqfont.h>
#include <tqfontmetrics.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqspinbox.h>
#include <tqmap.h>
#include <tqptrqueue.h>
#include <tqpopupmenu.h>
#include <tqclipboard.h>


class K3bCdImageWritingDialog::Private
{
public:
  Private()
    : md5SumItem(0),
      haveMd5Sum( false ),
      foundImageType( IMAGE_UNKNOWN ),
      imageForced( false ) {
  }

  K3bListViewItem* md5SumItem;
  TQString lastCheckedFile;

  K3bMd5Job* md5Job;
  bool haveMd5Sum;

  int foundImageType;

  TQMap<int,int> imageTypeSelectionMap;
  TQMap<int,int> imageTypeSelectionMapRev;
  TQString imageFile;
  TQString tocFile;

  TQTabWidget* optionTabbed;

  TQWidget* advancedTab;
  TQWidget* tempPathTab;
  bool advancedTabVisible;
  bool tempPathTabVisible;

  bool imageForced;
};


K3bCdImageWritingDialog::K3bCdImageWritingDialog( TQWidget* parent, const char* name, bool modal )
  : K3bInteractionDialog( parent, name, 
			  i18n("Burn CD Image"), 
			  "iso cue toc",
			  START_BUTTON|CANCEL_BUTTON,
			  START_BUTTON,
			  "image writing", // config group
			  modal )
{
  d = new Private();

  setAcceptDrops( true );

  setupGui();

  d->md5Job = new K3bMd5Job( 0, TQT_TQOBJECT(this) );
  connect( d->md5Job, TQT_SIGNAL(finished(bool)),
	   this, TQT_SLOT(slotMd5JobFinished(bool)) );
  connect( d->md5Job, TQT_SIGNAL(percent(int)),
	   this, TQT_SLOT(slotMd5JobPercent(int)) );

  connect( m_writerSelectionWidget, TQT_SIGNAL(writerChanged()),
	   this, TQT_SLOT(slotToggleAll()) );
  connect( m_writerSelectionWidget, TQT_SIGNAL(writingAppChanged(int)),
	   this, TQT_SLOT(slotToggleAll()) );
  connect( m_writerSelectionWidget, TQT_SIGNAL(writerChanged(K3bDevice::Device*)), 
	   m_writingModeWidget, TQT_SLOT(setDevice(K3bDevice::Device*)) );
  connect( m_comboImageType, TQT_SIGNAL(activated(int)),
	   this, TQT_SLOT(slotToggleAll()) );
  connect( m_writingModeWidget, TQT_SIGNAL(writingModeChanged(int)),
	   this, TQT_SLOT(slotToggleAll()) );
  connect( m_editImagePath, TQT_SIGNAL(textChanged(const TQString&)), 
	   this, TQT_SLOT(slotUpdateImage(const TQString&)) );
  connect( m_checkDummy, TQT_SIGNAL(toggled(bool)),
	   this, TQT_SLOT(slotToggleAll()) );
  connect( m_checkCacheImage, TQT_SIGNAL(toggled(bool)),
	   this, TQT_SLOT(slotToggleAll()) );
}


K3bCdImageWritingDialog::~K3bCdImageWritingDialog()
{
  d->md5Job->cancel();
  delete d;
}


void K3bCdImageWritingDialog::init()
{
  if( !d->imageForced ) {
    // when opening the dialog first the default settings are loaded and afterwards we set the 
    // last written image because that's what most users want
    KConfig* c = k3bcore->config();
    c->setGroup( configGroup() );
    TQString image = c->readPathEntry( "last written image" );
    if( TQFile::exists( image ) )
      m_editImagePath->setURL( image );
  }
}


void K3bCdImageWritingDialog::setupGui()
{
  TQWidget* frame = mainWidget();

  // image
  // -----------------------------------------------------------------------
  TQGroupBox* groupImageUrl = new TQGroupBox( 1, Qt::Horizontal, i18n("Image to Burn"), frame );
  m_editImagePath = new KURLRequester( groupImageUrl );
  m_editImagePath->setMode( KFile::File|KFile::ExistingOnly );
  m_editImagePath->setCaption( i18n("Choose Image File") );
  m_editImagePath->setFilter( i18n("*.iso *.toc *.ISO *.TOC *.cue *.CUE|Image Files") 
			      + "\n"
			      + i18n("*.iso *.ISO|ISO9660 Image Files")
			      + "\n"
			      + i18n("*.cue *.CUE|Cue Files")
			      + "\n"
			      + i18n("*.toc *.TOC|Cdrdao TOC Files and Cdrecord Clone Images")
			      + "\n" 
			      + i18n("*|All Files") );
  
  TQGroupBox* groupImageType = new TQGroupBox( 1, Qt::Horizontal, i18n("Image Type"), frame );
  m_comboImageType = new TQComboBox( groupImageType );
  m_comboImageType->insertItem( i18n("Auto Detection") );
  m_comboImageType->insertItem( i18n("ISO9660 Image") );
  m_comboImageType->insertItem( i18n("Cue/Bin Image") );
  m_comboImageType->insertItem( i18n("Audio Cue File") );
  m_comboImageType->insertItem( i18n("Cdrdao TOC File") );
  m_comboImageType->insertItem( i18n("Cdrecord Clone Image") );
  d->imageTypeSelectionMap[1] = IMAGE_ISO;
  d->imageTypeSelectionMap[2] = IMAGE_CUE_BIN;
  d->imageTypeSelectionMap[3] = IMAGE_AUDIO_CUE;
  d->imageTypeSelectionMap[4] = IMAGE_CDRDAO_TOC;
  d->imageTypeSelectionMap[5] = IMAGE_CDRECORD_CLONE;
  d->imageTypeSelectionMapRev[IMAGE_ISO] = 1;
  d->imageTypeSelectionMapRev[IMAGE_CUE_BIN] = 2;
  d->imageTypeSelectionMapRev[IMAGE_AUDIO_CUE] = 3;
  d->imageTypeSelectionMapRev[IMAGE_CDRDAO_TOC] = 4;
  d->imageTypeSelectionMapRev[IMAGE_CDRECORD_CLONE] = 5;


  // image info
  // -----------------------------------------------------------------------
  m_infoView = new K3bListView( frame );
  m_infoView->addColumn( "key" );
  m_infoView->addColumn( "value" );
  m_infoView->header()->hide();
  m_infoView->setNoItemText( i18n("No image file selected") );
  m_infoView->setSorting( -1 );
  m_infoView->setAlternateBackground( TQColor() );
  m_infoView->setFullWidth(true);
  m_infoView->setSelectionMode( TQListView::NoSelection );

  connect( m_infoView, TQT_SIGNAL(contextMenu(KListView*, TQListViewItem*, const TQPoint&)),
	   this, TQT_SLOT(slotContextMenu(KListView*, TQListViewItem*, const TQPoint&)) );


  m_writerSelectionWidget = new K3bWriterSelectionWidget( frame );
  m_writerSelectionWidget->setWantedMediumType( K3bDevice::MEDIA_WRITABLE_CD );
  m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_EMPTY );

  // options
  // -----------------------------------------------------------------------
  d->optionTabbed = new TQTabWidget( frame );

  TQWidget* optionTab = new TQWidget( d->optionTabbed );
  TQGridLayout* optionTabLayout = new TQGridLayout( optionTab );
  optionTabLayout->setAlignment( TQt::AlignTop );
  optionTabLayout->setSpacing( spacingHint() );
  optionTabLayout->setMargin( marginHint() );

  TQGroupBox* writingModeGroup = new TQGroupBox( 1,Qt::Vertical, i18n("Writing Mode"), optionTab );
  writingModeGroup->setInsideMargin( marginHint() );
  m_writingModeWidget = new K3bWritingModeWidget( writingModeGroup );


  // copies --------
  TQGroupBox* groupCopies = new TQGroupBox( 2, Qt::Horizontal, i18n("Copies"), optionTab );
  groupCopies->setInsideSpacing( spacingHint() );
  groupCopies->setInsideMargin( marginHint() );
  TQLabel* pixLabel = new TQLabel( groupCopies );
  pixLabel->setPixmap( SmallIcon( "cdcopy", KIcon::SizeMedium ) );
  pixLabel->setScaledContents( false );
  m_spinCopies = new TQSpinBox( groupCopies );
  m_spinCopies->setMinValue( 1 );
  m_spinCopies->setMaxValue( 999 );
  // -------- copies

  TQGroupBox* optionGroup = new TQGroupBox( 3,Qt::Vertical, i18n("Settings"), optionTab );
  optionGroup->setInsideMargin( marginHint() );
  optionGroup->setInsideSpacing( spacingHint() );
  m_checkDummy = K3bStdGuiItems::simulateCheckbox( optionGroup );
  m_checkCacheImage = K3bStdGuiItems::createCacheImageCheckbox( optionGroup );
  m_checkVerify = K3bStdGuiItems::verifyCheckBox( optionGroup );

  optionTabLayout->addWidget( writingModeGroup, 0, 0 );
  optionTabLayout->addWidget( groupCopies, 1, 0 );
  optionTabLayout->addMultiCellWidget( optionGroup, 0, 1, 1, 1 );
  optionTabLayout->setRowStretch( 1, 1 );
  optionTabLayout->setColStretch( 1, 1 );

  d->optionTabbed->addTab( optionTab, i18n("Settings") );


  // image tab ------------------------------------
  d->tempPathTab = new TQWidget( d->optionTabbed );
  TQGridLayout* imageTabGrid = new TQGridLayout( d->tempPathTab );
  imageTabGrid->setSpacing( spacingHint() );
  imageTabGrid->setMargin( marginHint() );

  m_tempDirSelectionWidget = new K3bTempDirSelectionWidget( d->tempPathTab );

  imageTabGrid->addWidget( m_tempDirSelectionWidget, 0, 0 );

  d->optionTabbed->addTab( d->tempPathTab, i18n("&Image") );
  d->tempPathTabVisible = true;
  // -------------------------------------------------------------


  // advanced ---------------------------------
  d->advancedTab = new TQWidget( d->optionTabbed );
  TQGridLayout* advancedTabLayout = new TQGridLayout( d->advancedTab );
  advancedTabLayout->setAlignment( TQt::AlignTop );
  advancedTabLayout->setSpacing( spacingHint() );
  advancedTabLayout->setMargin( marginHint() );
    
  m_dataModeWidget = new K3bDataModeWidget( d->advancedTab );
  m_checkNoFix = K3bStdGuiItems::startMultisessionCheckBox( d->advancedTab );
    
  advancedTabLayout->addWidget( new TQLabel( i18n("Data mode:"), d->advancedTab ), 0, 0 );
  advancedTabLayout->addWidget( m_dataModeWidget, 0, 1 );
  advancedTabLayout->addMultiCellWidget( m_checkNoFix, 1, 1, 0, 2 );
  advancedTabLayout->setRowStretch( 2, 1 );
  advancedTabLayout->setColStretch( 2, 1 );
    
  d->optionTabbed->addTab( d->advancedTab, i18n("Advanced") );
  d->advancedTabVisible = true;
  // -----------------------------------------------------------------------




  TQGridLayout* grid = new TQGridLayout( frame );
  grid->setSpacing( spacingHint() );
  grid->setMargin( 0 );

  grid->addWidget( groupImageUrl, 0, 0 );
  grid->addWidget( groupImageType, 0, 1 );
  grid->addMultiCellWidget( m_infoView, 1, 1, 0, 1 );
  grid->addMultiCellWidget( m_writerSelectionWidget, 2, 2, 0, 1 );
  grid->addMultiCellWidget( d->optionTabbed, 3, 3, 0, 1 );

  grid->setRowStretch( 1, 1 );
}


void K3bCdImageWritingDialog::slotStartClicked()
{
  // FIXME: this results in a call to slotMd5JobFinished
  //        if this dialog is deleted becasue it is not opened with exec(false)
  //        this results in a crash.
  //        For now this is not a problem in K3b since the dialog is not deleted
  //        when hiding (due to the exec(false) call in k3b.cpp
  d->md5Job->cancel();

  // save the path
  KConfig* c = k3bcore->config();
  c->setGroup( configGroup() );
  c->writePathEntry( "last written image", imagePath() );

  if( d->imageFile.isEmpty() )
    d->imageFile = imagePath();
  if( d->tocFile.isEmpty() )
    d->tocFile = imagePath();

  // create a progresswidget
  K3bBurnProgressDialog dlg( kapp->mainWidget(), "burnProgress", true );

  // create the job
  K3bBurnJob* job = 0;
  switch( currentImageType() ) {
  case IMAGE_CDRECORD_CLONE:
    {
      K3bCloneJob* _job = new K3bCloneJob( &dlg, TQT_TQOBJECT(this) );
      _job->setWriterDevice( m_writerSelectionWidget->writerDevice() );
      _job->setImagePath( d->imageFile );
      _job->setSimulate( m_checkDummy->isChecked() );
      _job->setWriteSpeed( m_writerSelectionWidget->writerSpeed() );
      _job->setCopies( m_checkDummy->isChecked() ? 1 : m_spinCopies->value() );
      _job->setOnlyBurnExistingImage( true );
      
      job = _job;
    }
    break;

  case IMAGE_AUDIO_CUE: 
    {
      K3bAudioCueFileWritingJob* job_ = new K3bAudioCueFileWritingJob( &dlg, TQT_TQOBJECT(this) );
      
      job_->setBurnDevice( m_writerSelectionWidget->writerDevice() );
      job_->setSpeed( m_writerSelectionWidget->writerSpeed() );
      job_->setSimulate( m_checkDummy->isChecked() );
      job_->setWritingMode( m_writingModeWidget->writingMode() );
      job_->setCueFile( d->tocFile );
      job_->setCopies( m_checkDummy->isChecked() ? 1 : m_spinCopies->value() );
      job_->setOnTheFly( !m_checkCacheImage->isChecked() );
      job_->setTempDir( m_tempDirSelectionWidget->tempPath() );

      job = job_;
    }
    break;

  case IMAGE_CUE_BIN:
    // for now the K3bBinImageWritingJob decides if it's a toc or a cue file
  case IMAGE_CDRDAO_TOC:
    {
      K3bBinImageWritingJob* job_ = new K3bBinImageWritingJob( &dlg, TQT_TQOBJECT(this) );

      job_->setWriter( m_writerSelectionWidget->writerDevice() );
      job_->setSpeed( m_writerSelectionWidget->writerSpeed() );
      job_->setTocFile( d->tocFile );
      job_->setSimulate(m_checkDummy->isChecked());
      job_->setMulti( false /*m_checkNoFix->isChecked()*/ );
      job_->setCopies( m_checkDummy->isChecked() ? 1 : m_spinCopies->value() );
      
      job = job_;
    }
    break;

  case IMAGE_ISO:
    {
      K3bIso9660 isoFs( d->imageFile );
      if( isoFs.open() ) {
	if( K3b::filesize( KURL::fromPathOrURL(d->imageFile) ) < (KIO::filesize_t)(isoFs.primaryDescriptor().volumeSpaceSize*2048) ) {
	  if( KMessageBox::questionYesNo( this, 
					  i18n("<p>This image has an invalid file size."
					       "If it has been downloaded make sure the download is complete."
					       "<p>Only continue if you know what you are doing."),
					  i18n("Warning"),
					  i18n("Continue"),
					  i18n("Cancel") ) == KMessageBox::No )
	    return;
	}
      }

      K3bIso9660ImageWritingJob* job_ = new K3bIso9660ImageWritingJob( &dlg );
      
      job_->setBurnDevice( m_writerSelectionWidget->writerDevice() );
      job_->setSpeed( m_writerSelectionWidget->writerSpeed() );
      job_->setSimulate( m_checkDummy->isChecked() );
      job_->setWritingMode( m_writingModeWidget->writingMode() );
      job_->setVerifyData( m_checkVerify->isChecked() );
      job_->setNoFix( m_checkNoFix->isChecked() );
      job_->setDataMode( m_dataModeWidget->dataMode() );
      job_->setImagePath( d->imageFile );
      job_->setCopies( m_checkDummy->isChecked() ? 1 : m_spinCopies->value() );
      
      job = job_;
    }
    break;

  default:
    kdDebug() << "(K3bCdImageWritingDialog) this should really not happen!" << endl;
    break;
  }

  if( job ) {
    job->setWritingApp( m_writerSelectionWidget->writingApp() );

    if( !exitLoopOnHide() )
      hide();
    
    dlg.startJob(job);
    
    delete job;

    if( KConfigGroup( k3bcore->config(), "General Options" ).readBoolEntry( "keep action dialogs open", false ) &&
	!exitLoopOnHide() )
      show();
    else
      close();
  }
}


void K3bCdImageWritingDialog::slotUpdateImage( const TQString& )
{
  TQString path = imagePath();

  // check the image types

  d->haveMd5Sum = false;
  d->md5Job->cancel();
  m_infoView->clear();
  m_infoView->header()->resizeSection( 0, 20 );
  d->md5SumItem = 0;
  d->foundImageType = IMAGE_UNKNOWN;
  d->tocFile.truncate(0);
  d->imageFile.truncate(0);

  TQFileInfo info( path );
  if( info.isFile() ) {

    // ------------------------------------------------
    // Test for iso9660 image
    // ------------------------------------------------
    K3bIso9660 isoF( path );
    if( isoF.open() ) {
#ifdef K3B_DEBUG
      isoF.debug();
#endif

      createIso9660InfoItems( &isoF );
      isoF.close();
      calculateMd5Sum( path );

      d->foundImageType = IMAGE_ISO;
      d->imageFile = path;
    }

    if( d->foundImageType == IMAGE_UNKNOWN ) {
     
      // check for cdrecord clone image
      // try both path and path.toc as tocfiles
      K3bCloneTocReader cr;

      if( path.right(4) == ".toc" ) {
	cr.openFile( path );
	if( cr.isValid() ) {
	  d->tocFile = path;
	  d->imageFile = cr.imageFilename();
	}
      }
      if( d->imageFile.isEmpty() ) {
	cr.openFile( path + ".toc" );
	if( cr.isValid() ) {
	  d->tocFile = cr.filename();
	  d->imageFile = cr.imageFilename();
	}
      }

      if( !d->imageFile.isEmpty() ) {
	// we have a cdrecord clone image
	createCdrecordCloneItems( d->tocFile, d->imageFile );
	calculateMd5Sum( d->imageFile );

	d->foundImageType = IMAGE_CDRECORD_CLONE;
      }
    }

    if( d->foundImageType == IMAGE_UNKNOWN ) {

      // check for cue/bin stuff
      // once again we try both path and path.cue
      K3bCueFileParser cp;

      if( path.right(4).lower() == ".cue" )
	cp.openFile( path );
      else if( path.right(4).lower() == ".bin" )
	cp.openFile( path.left( path.length()-3) + "cue" );

      if( cp.isValid() ) {
	d->tocFile = cp.filename();
	d->imageFile = cp.imageFilename();
      }
      
      if( d->imageFile.isEmpty() ) {
	cp.openFile( path + ".cue" );
	if( cp.isValid() ) {
	  d->tocFile = cp.filename();
	  d->imageFile = cp.imageFilename();
	}
      }

      if( !d->imageFile.isEmpty() ) {
	// we have a cue file
	if( cp.toc().contentType() == K3bDevice::AUDIO ) {
	  d->foundImageType = IMAGE_AUDIO_CUE;
	  createAudioCueItems( cp );
	}
	else {
	  d->foundImageType = IMAGE_CUE_BIN;  // we cannot be sure if writing will work... :(
	  createCueBinItems( d->tocFile, d->imageFile );
	  calculateMd5Sum( d->imageFile );
	}
      }
    }

    if( d->foundImageType == IMAGE_UNKNOWN ) {
      // TODO: check for cdrdao tocfile
    }



    if( d->foundImageType == IMAGE_UNKNOWN ) {
      K3bListViewItem* item = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						   i18n("Seems not to be a usable image") );
      item->setForegroundColor( 0, TQt::red );
      item->setPixmap( 0, SmallIcon( "stop") );
    }
  }
  else {
    K3bListViewItem* item = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						 i18n("File not found") );
    item->setForegroundColor( 0, TQt::red );
    item->setPixmap( 0, SmallIcon( "stop") );
  }

  slotToggleAll();
}


void K3bCdImageWritingDialog::createIso9660InfoItems( K3bIso9660* isoF )
{
  K3bListViewItem* isoRootItem = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						      i18n("Detected:"),
						      i18n("Iso9660 image") );
  isoRootItem->setForegroundColor( 0, palette().disabled().foreground() );
  isoRootItem->setPixmap( 0, SmallIcon( "cdimage") );

  KIO::filesize_t size = K3b::filesize( KURL::fromPathOrURL(isoF->fileName()) );
  K3bListViewItem* item = new K3bListViewItem( isoRootItem, m_infoView->lastItem(),
					       i18n("Filesize:"), 
					       KIO::convertSize( size ) );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("System Id:"), 
			      isoF->primaryDescriptor().systemId.isEmpty()
			      ? TQString("-") 
			      : isoF->primaryDescriptor().systemId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Volume Id:"), 
			      isoF->primaryDescriptor().volumeId.isEmpty() 
			      ? TQString("-") 
			      : isoF->primaryDescriptor().volumeId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Volume Set Id:"), 
			      isoF->primaryDescriptor().volumeSetId.isEmpty()
			      ? TQString("-")
			      : isoF->primaryDescriptor().volumeSetId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Publisher Id:"), 
			      isoF->primaryDescriptor().publisherId.isEmpty() 
			      ? TQString("-") 
			      : isoF->primaryDescriptor().publisherId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Preparer Id:"), 
			      isoF->primaryDescriptor().preparerId.isEmpty() 
			      ? TQString("-") : isoF->primaryDescriptor().preparerId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Application Id:"), 
			      isoF->primaryDescriptor().applicationId.isEmpty()
			      ? TQString("-") 
			      : isoF->primaryDescriptor().applicationId );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  isoRootItem->setOpen( true );
}


void K3bCdImageWritingDialog::createCdrecordCloneItems( const TQString& tocFile, const TQString& imageFile )
{
  K3bListViewItem* isoRootItem = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						      i18n("Detected:"),
						      i18n("Cdrecord clone image") );
  isoRootItem->setForegroundColor( 0, palette().disabled().foreground() );
  isoRootItem->setPixmap( 0, SmallIcon( "cdimage") );

  K3bListViewItem* item = new K3bListViewItem( isoRootItem, m_infoView->lastItem(),
					       i18n("Filesize:"), KIO::convertSize( K3b::filesize(KURL::fromPathOrURL(imageFile)) ) );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Image file:"), 
			      imageFile );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("TOC file:"), 
			      tocFile );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  isoRootItem->setOpen( true );
}


void K3bCdImageWritingDialog::createCueBinItems( const TQString& cueFile, const TQString& imageFile )
{
  K3bListViewItem* isoRootItem = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						      i18n("Detected:"),
						      i18n("Cue/bin image") );
  isoRootItem->setForegroundColor( 0, palette().disabled().foreground() );
  isoRootItem->setPixmap( 0, SmallIcon( "cdimage") );

  K3bListViewItem* item = new K3bListViewItem( isoRootItem, m_infoView->lastItem(),
					       i18n("Filesize:"), KIO::convertSize( K3b::filesize(KURL::fromPathOrURL(imageFile)) ) );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Image file:"), 
			      imageFile );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  item = new K3bListViewItem( isoRootItem, 
			      m_infoView->lastItem(),
			      i18n("Cue file:"), 
			      cueFile );
  item->setForegroundColor( 0, palette().disabled().foreground() );

  isoRootItem->setOpen( true );
}


void K3bCdImageWritingDialog::createAudioCueItems( const K3bCueFileParser& cp )
{
  K3bListViewItem* rootItem = new K3bListViewItem( m_infoView, m_infoView->lastItem(),
						   i18n("Detected:"),
						   i18n("Audio Cue Image") );
  rootItem->setForegroundColor( 0, palette().disabled().foreground() );
  rootItem->setPixmap( 0, SmallIcon( "sound") );

  K3bListViewItem* trackParent = new K3bListViewItem( rootItem,
						      i18n("%n track", "%n tracks", cp.toc().count() ),
						      cp.toc().length().toString() );
  if( !cp.cdText().isEmpty() )
    trackParent->setText( 1,
			  TQString("%1 (%2 - %3)")
			  .arg(trackParent->text(1))
			  .arg(cp.cdText().performer())
			  .arg(cp.cdText().title()) );

  unsigned int i = 1;
  for( K3bDevice::Toc::const_iterator it = cp.toc().begin();
       it != cp.toc().end(); ++it ) {

    K3bListViewItem* trackItem = 
      new K3bListViewItem( trackParent, m_infoView->lastItem(),
			   i18n("Track") + " " + TQString::number(i).rightJustify( 2, '0' ),
			   "    " + ( i < cp.toc().count() 
				      ? (*it).length().toString() 
				      : TQString("??:??:??") ) );

    if( !cp.cdText().isEmpty() && !cp.cdText()[i-1].isEmpty() )
      trackItem->setText( 1,
			  TQString("%1 (%2 - %3)")
			  .arg(trackItem->text(1))
			  .arg(cp.cdText()[i-1].performer())
			  .arg(cp.cdText()[i-1].title()) );

    ++i;
  }

  rootItem->setOpen( true );
  trackParent->setOpen( true );
}


void K3bCdImageWritingDialog::toggleAll()
{
  // enable the Write-Button if we found a valid image or the user forced an image type
  setButtonEnabled( START_BUTTON, m_writerSelectionWidget->writerDevice() 
		    && currentImageType() != IMAGE_UNKNOWN 
		    && TQFile::exists( imagePath() ) );
  
  // cdrecord clone and cue both need DAO
  if( m_writerSelectionWidget->writingApp() != K3b::CDRDAO 
      && ( currentImageType() == IMAGE_ISO ||
	   currentImageType() == IMAGE_AUDIO_CUE ) )
    m_writingModeWidget->setSupportedModes( K3b::TAO|K3b::DAO|K3b::RAW ); // stuff supported by cdrecord
  else
    m_writingModeWidget->setSupportedModes( K3b::DAO );
  
  // some stuff is only available for iso images
  if( currentImageType() == IMAGE_ISO ) {
    m_checkVerify->show();
    if( !d->advancedTabVisible )
      d->optionTabbed->addTab( d->advancedTab, i18n("Advanced") );
    d->advancedTabVisible = true;
    if( m_checkDummy->isChecked() ) {
      m_checkVerify->setEnabled( false );
      m_checkVerify->setChecked( false );
    }
    else
      m_checkVerify->setEnabled( true );	
  }
  else {
    if( d->advancedTabVisible )
      d->optionTabbed->removePage( d->advancedTab );
    d->advancedTabVisible = false;
    m_checkVerify->hide();
  }
  
  // and some other stuff only makes sense for audio cues
  if( currentImageType() == IMAGE_AUDIO_CUE ) {
    if( !d->tempPathTabVisible )
      d->optionTabbed->addTab( d->tempPathTab, i18n("&Image") );
    d->tempPathTabVisible = true;
    m_tempDirSelectionWidget->setDisabled( !m_checkCacheImage->isChecked() );
  }
  else {
    if( d->tempPathTabVisible )
      d->optionTabbed->removePage( d->tempPathTab );
    d->tempPathTabVisible = false;
  }
  m_checkCacheImage->setShown( currentImageType() == IMAGE_AUDIO_CUE );
  
  m_spinCopies->setEnabled( !m_checkDummy->isChecked() );

  switch( currentImageType() ) {
  case IMAGE_CDRDAO_TOC:
    m_writerSelectionWidget->setSupportedWritingApps( K3b::CDRDAO );
    break;
  case IMAGE_CDRECORD_CLONE:
    m_writerSelectionWidget->setSupportedWritingApps( K3b::CDRECORD );
    break;
  default:
    m_writerSelectionWidget->setSupportedWritingApps( K3b::CDRECORD|K3b::CDRDAO );
    break;
  }

  K3bListViewItem* item = dynamic_cast<K3bListViewItem*>(m_infoView->firstChild());
  if( item )
    item->setForegroundColor( 1, 
			      currentImageType() != d->foundImageType 
			      ? TQt::red
			      : m_infoView->colorGroup().foreground() );
}


void K3bCdImageWritingDialog::setImage( const KURL& url )
{
  d->imageForced = true;
#if KDE_IS_VERSION(3,4,0)
  m_editImagePath->setKURL( url );
#else
  m_editImagePath->setURL( url.path() );
#endif
}


void K3bCdImageWritingDialog::calculateMd5Sum( const TQString& file )
{
  d->haveMd5Sum = false;

  if( !d->md5SumItem )
    d->md5SumItem = new K3bListViewItem( m_infoView, m_infoView->firstChild() );

  d->md5SumItem->setText( 0, i18n("Md5 Sum:") );
  d->md5SumItem->setForegroundColor( 0, palette().disabled().foreground() );
  d->md5SumItem->setProgress( 1, 0 );
  d->md5SumItem->setPixmap( 0, SmallIcon( "exec") );

  if( file != d->lastCheckedFile ) {
    d->lastCheckedFile = file;
    d->md5Job->setFile( file );
    d->md5Job->start();
  }
  else
    slotMd5JobFinished( true );
}


void K3bCdImageWritingDialog::slotMd5JobPercent( int p )
{
  d->md5SumItem->setProgress( 1, p );
}


void K3bCdImageWritingDialog::slotMd5JobFinished( bool success )
{
  if( success ) {
    d->md5SumItem->setText( 1, d->md5Job->hexDigest() );
    d->haveMd5Sum = true;
  }
  else {
    d->md5SumItem->setForegroundColor( 1, TQt::red );
    if( d->md5Job->hasBeenCanceled() )
      d->md5SumItem->setText( 1, i18n("Calculation cancelled") );
    else
      d->md5SumItem->setText( 1, i18n("Calculation failed") );
    d->md5SumItem->setPixmap( 0, SmallIcon( "stop") );
    d->lastCheckedFile.truncate(0);
  }

  d->md5SumItem->setDisplayProgressBar( 1, false );
}


void K3bCdImageWritingDialog::slotContextMenu( KListView*, TQListViewItem*, const TQPoint& pos )
{
  if( !d->haveMd5Sum )
    return;

  TQPopupMenu popup;
  int copyItem = popup.insertItem( i18n("Copy checksum to clipboard") );
  int compareItem = popup.insertItem( i18n("Compare checksum...") );

  int r = popup.exec( pos );

  if( r == compareItem ) {
    bool ok;
    TQString md5sumToCompare = KInputDialog::getText( i18n("MD5 Sum Check"),
						     i18n("Please insert the MD5 Sum to compare:"),
						     TQString(),
						     &ok,
						     this );
    if( ok ) {
      if( md5sumToCompare.lower().utf8() == d->md5Job->hexDigest().lower() )
	KMessageBox::information( this, i18n("The MD5 Sum of %1 equals the specified.").arg(imagePath()),
				  i18n("MD5 Sums Equal") );
      else
	KMessageBox::sorry( this, i18n("The MD5 Sum of %1 differs from the specified.").arg(imagePath()),
			    i18n("MD5 Sums Differ") );
    }
  }
  else if( r == copyItem ) {
    TQApplication::clipboard()->setText( d->md5Job->hexDigest().lower(), TQClipboard::Clipboard );
  }
}


void K3bCdImageWritingDialog::loadUserDefaults( KConfigBase* c )
{
  m_writingModeWidget->loadConfig( c );
  m_checkDummy->setChecked( c->readBoolEntry("simulate", false ) );
  m_checkNoFix->setChecked( c->readBoolEntry("multisession", false ) );
  m_checkCacheImage->setChecked( !c->readBoolEntry("on_the_fly", true ) );

  m_dataModeWidget->loadConfig(c);

  m_spinCopies->setValue( c->readNumEntry( "copies", 1 ) );
 
  m_checkVerify->setChecked( c->readBoolEntry( "verify_data", false ) );

  m_writerSelectionWidget->loadConfig( c );

  if( !d->imageForced ) {
    TQString image = c->readPathEntry( "image path", c->readPathEntry( "last written image" ) );
    if( TQFile::exists( image ) )
      m_editImagePath->setURL( image );
  }

  TQString imageType = c->readEntry( "image type", "auto" );
  int x = 0;
  if( imageType == "iso9660" )
    x = d->imageTypeSelectionMapRev[IMAGE_ISO];
  else if( imageType == "cue-bin" )
    x = d->imageTypeSelectionMapRev[IMAGE_CUE_BIN];
  else if( imageType == "audio-cue" )
    x = d->imageTypeSelectionMapRev[IMAGE_AUDIO_CUE];
  else if( imageType == "cdrecord-clone" )
    x = d->imageTypeSelectionMapRev[IMAGE_CDRECORD_CLONE];
  else if( imageType == "cdrdao-toc" )
    x = d->imageTypeSelectionMapRev[IMAGE_CDRDAO_TOC];

  m_comboImageType->setCurrentItem( x );

  m_tempDirSelectionWidget->setTempPath( K3b::defaultTempPath() );

  slotToggleAll();
}


void K3bCdImageWritingDialog::saveUserDefaults( KConfigBase* c )
{
  m_writingModeWidget->saveConfig( c ),
  c->writeEntry( "simulate", m_checkDummy->isChecked() );
  c->writeEntry( "multisession", m_checkNoFix->isChecked() );
  c->writeEntry( "on_the_fly", !m_checkCacheImage->isChecked() );
  m_dataModeWidget->saveConfig(c);
  
  c->writeEntry( "verify_data", m_checkVerify->isChecked() );

  m_writerSelectionWidget->saveConfig( c );

  c->writePathEntry( "image path", imagePath() );

  c->writeEntry( "copies", m_spinCopies->value() );

  TQString imageType;
  if( m_comboImageType->currentItem() == 0 )
    imageType = "auto";
  else {
    switch( d->imageTypeSelectionMap[m_comboImageType->currentItem()] ) {
    case IMAGE_ISO:
      imageType = "iso9660";
      break;
    case IMAGE_CUE_BIN:
      imageType = "cue-bin";
      break;
    case IMAGE_AUDIO_CUE:
      imageType = "audio-cue";
      break;
    case IMAGE_CDRECORD_CLONE:
      imageType = "cdrecord-clone";
      break;
    case IMAGE_CDRDAO_TOC:
      imageType = "cdrdao-toc";
      break;
    }
  }
  c->writeEntry( "image type", imageType );

  if( m_tempDirSelectionWidget->isEnabled() )
    m_tempDirSelectionWidget->saveConfig();
}

void K3bCdImageWritingDialog::loadK3bDefaults()
{
  m_writerSelectionWidget->loadDefaults();
  m_writingModeWidget->setWritingMode( K3b::WRITING_MODE_AUTO );
  m_checkDummy->setChecked( false );
  m_checkVerify->setChecked( false );
  m_checkNoFix->setChecked( false );
  m_checkCacheImage->setChecked( false );
  m_dataModeWidget->setDataMode( K3b::DATA_MODE_AUTO );
  m_comboImageType->setCurrentItem(0);
  m_spinCopies->setValue( 1 );

  slotToggleAll();
}


int K3bCdImageWritingDialog::currentImageType()
{
  if( m_comboImageType->currentItem() == 0 )
    return d->foundImageType;
  else
    return d->imageTypeSelectionMap[m_comboImageType->currentItem()];    
}


TQString K3bCdImageWritingDialog::imagePath() const
{
  return K3b::convertToLocalUrl( KURL::fromPathOrURL( m_editImagePath->url() ) ).path();
}


void K3bCdImageWritingDialog::dragEnterEvent( TQDragEnterEvent* e )
{
  e->accept( KURLDrag::canDecode(e) );
}


void K3bCdImageWritingDialog::dropEvent( TQDropEvent* e )
{
  KURL::List urls;
  KURLDrag::decode( e, urls );
#if KDE_IS_VERSION(3,4,0)
  m_editImagePath->setKURL( urls.first() );
#else
  m_editImagePath->setURL( urls.first().path() );
#endif
}

#include "k3bcdimagewritingdialog.moc"
