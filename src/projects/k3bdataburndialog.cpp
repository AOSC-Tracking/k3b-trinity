/*
 *
 * $Id: k3bdataburndialog.cpp 690207 2007-07-20 10:40:19Z trueg $
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


#include "k3bdataburndialog.h"
#include "k3bdataimagesettingswidget.h"
#include "k3bdatavolumedescwidget.h"
#include "k3bdatamultisessioncombobox.h"
#include "k3bdataview.h"

#include <k3bisooptions.h>
#include <k3bdatadoc.h>
#include <k3bdevice.h>
#include <k3bwriterselectionwidget.h>
#include <k3btempdirselectionwidget.h>
#include <k3bjob.h>
#include <k3bcore.h>
#include <k3bstdguiitems.h>
#include <k3bdatamodewidget.h>
#include <k3bglobals.h>
#include <k3bwritingmodewidget.h>

#include <tqcheckbox.h>
#include <tqframe.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqpoint.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <tqfileinfo.h>
#include <tqtabwidget.h>
#include <tqspinbox.h>
#include <tqfile.h>

#include <tdemessagebox.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdefiledialog.h>
#include <kcombobox.h>
#include <tdeio/global.h>


#include "k3bfilecompilationsizehandler.h"


K3bDataBurnDialog::K3bDataBurnDialog(K3bDataDoc* _doc, TQWidget *parent, const char *name, bool modal )
  : K3bProjectBurnDialog( _doc, parent, name, modal )
{
  prepareGui();

  setTitle( i18n("Data Project"), i18n("Size: %1").arg( TDEIO::convertSize(_doc->size()) ) );

  // for now we just put the verify checkbox on the main page...
  m_checkVerify = K3bStdGuiItems::verifyCheckBox( m_optionGroup );
  m_optionGroupLayout->addWidget( m_checkVerify );

  TQSpacerItem* spacer = new TQSpacerItem( 20, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
  m_optionGroupLayout->addItem( spacer );

  // create image settings tab
  m_imageSettingsWidget = new K3bDataImageSettingsWidget( this );
  addPage( m_imageSettingsWidget, i18n("Filesystem") );

  setupSettingsTab();

  connect( m_comboMultisession, TQ_SIGNAL(activated(int)),
	   this, TQ_SLOT(slotMultiSessionModeChanged()) );

  m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_EMPTY|K3bDevice::STATE_INCOMPLETE );

  m_tempDirSelectionWidget->setSelectionMode( K3bTempDirSelectionWidget::FILE );
  TQString path = _doc->tempDir();
  if( !path.isEmpty() ) {
      m_tempDirSelectionWidget->setTempPath( path );
  }
  if( !_doc->isoOptions().volumeID().isEmpty() ) {
      m_tempDirSelectionWidget->setDefaultImageFileName( _doc->isoOptions().volumeID() + ".iso" );
  }

  connect( m_imageSettingsWidget->m_editVolumeName, TQ_SIGNAL(textChanged(const TQString&)),
           m_tempDirSelectionWidget, TQ_SLOT(setDefaultImageFileName(const TQString&)) );
}

K3bDataBurnDialog::~K3bDataBurnDialog()
{
}


void K3bDataBurnDialog::saveSettings()
{
  K3bProjectBurnDialog::saveSettings();

  // save iso image settings
  K3bIsoOptions o = ((K3bDataDoc*)doc())->isoOptions();
  m_imageSettingsWidget->save( o );
  ((K3bDataDoc*)doc())->setIsoOptions( o );

  // save image file path
  ((K3bDataDoc*)doc())->setTempDir( m_tempDirSelectionWidget->tempPath() );

  // save multisession settings
  ((K3bDataDoc*)doc())->setMultiSessionMode( m_comboMultisession->multiSessionMode() );

  ((K3bDataDoc*)doc())->setDataMode( m_dataModeWidget->dataMode() );

  ((K3bDataDoc*)doc())->setVerifyData( m_checkVerify->isChecked() );
}


void K3bDataBurnDialog::readSettings()
{
  K3bProjectBurnDialog::readSettings();

  // read multisession
  m_comboMultisession->setMultiSessionMode( ((K3bDataDoc*)doc())->multiSessionMode() );

  if( !doc()->tempDir().isEmpty() )
    m_tempDirSelectionWidget->setTempPath( doc()->tempDir() );
  else
    m_tempDirSelectionWidget->setTempPath( K3b::defaultTempPath() + doc()->name() + ".iso" );

  m_checkVerify->setChecked( ((K3bDataDoc*)doc())->verifyData() );

  m_imageSettingsWidget->load( ((K3bDataDoc*)doc())->isoOptions() );

  m_dataModeWidget->setDataMode( ((K3bDataDoc*)doc())->dataMode() );

  toggleAll();
}


void K3bDataBurnDialog::setupSettingsTab()
{
  TQWidget* frame = new TQWidget( this );
  TQGridLayout* frameLayout = new TQGridLayout( frame );
  frameLayout->setSpacing( spacingHint() );
  frameLayout->setMargin( marginHint() );

  m_groupDataMode = new TQGroupBox( 1, TQt::Vertical, i18n("Datatrack Mode"), frame );
  m_dataModeWidget = new K3bDataModeWidget( m_groupDataMode );

  TQGroupBox* groupMultiSession = new TQGroupBox( 1, TQt::Vertical, i18n("Multisession Mode"), frame );
  m_comboMultisession = new K3bDataMultiSessionCombobox( groupMultiSession );

  frameLayout->addWidget( m_groupDataMode, 0, 0 );
  frameLayout->addWidget( groupMultiSession, 1, 0 );
  frameLayout->setRowStretch( 2, 1 );

  addPage( frame, i18n("Misc") );
}


void K3bDataBurnDialog::slotStartClicked()
{
  if( m_checkOnlyCreateImage->isChecked() ||
      m_checkCacheImage->isChecked() ) {
    TQFileInfo fi( m_tempDirSelectionWidget->tempPath() );
    if( fi.isDir() )
      m_tempDirSelectionWidget->setTempPath( fi.filePath() + "/image.iso" );

    if( TQFile::exists( m_tempDirSelectionWidget->tempPath() ) ) {
      if( KMessageBox::warningContinueCancel( this,
					      i18n("Do you want to overwrite %1?").arg(m_tempDirSelectionWidget->tempPath()),
					      i18n("File Exists"), i18n("Overwrite") )
	  == KMessageBox::Continue ) {
	// delete the file here to avoid problems with free space in K3bProjectBurnDialog::slotStartClicked
	TQFile::remove( m_tempDirSelectionWidget->tempPath() );
      }
      else
	return;
    }
  }

  if( m_writingModeWidget->writingMode() == K3b::DAO &&
      m_comboMultisession->multiSessionMode() != K3bDataDoc::NONE &&
      m_writerSelectionWidget->writingApp() == K3b::CDRECORD )
    if( KMessageBox::warningContinueCancel( this,
					    i18n("Most writers do not support writing "
						 "multisession CDs in DAO mode.") )
	== KMessageBox::Cancel )
      return;


  K3bProjectBurnDialog::slotStartClicked();
}


void K3bDataBurnDialog::loadK3bDefaults()
{
  K3bProjectBurnDialog::loadK3bDefaults();

  m_dataModeWidget->setDataMode( K3b::DATA_MODE_AUTO );

  m_imageSettingsWidget->load( K3bIsoOptions::defaults() );
  m_comboMultisession->setMultiSessionMode( K3bDataDoc::AUTO );
  m_checkVerify->setChecked( false );

  toggleAll();
}


void K3bDataBurnDialog::loadUserDefaults( TDEConfigBase* c )
{
  K3bProjectBurnDialog::loadUserDefaults(c);

  m_dataModeWidget->loadConfig(c);
  m_comboMultisession->loadConfig( c );

  K3bIsoOptions o = K3bIsoOptions::load( c );
  m_imageSettingsWidget->load( o );

  m_checkVerify->setChecked( c->readBoolEntry( "verify data", false ) );

  toggleAll();
}


void K3bDataBurnDialog::saveUserDefaults( TDEConfigBase* c )
{
  K3bProjectBurnDialog::saveUserDefaults(c);

  m_dataModeWidget->saveConfig(c);
  m_comboMultisession->saveConfig( c );

  K3bIsoOptions o;
  m_imageSettingsWidget->save( o );
  o.save( c );

  c->writeEntry( "verify data", m_checkVerify->isChecked() );
}


void K3bDataBurnDialog::toggleAll()
{
  K3bProjectBurnDialog::toggleAll();

  if( m_checkSimulate->isChecked() || m_checkOnlyCreateImage->isChecked() ) {
    m_checkVerify->setChecked(false);
    m_checkVerify->setEnabled(false);
  }
  else
    m_checkVerify->setEnabled(true);

  m_comboMultisession->setDisabled( m_checkOnlyCreateImage->isChecked() );
  m_dataModeWidget->setDisabled( m_checkOnlyCreateImage->isChecked() );
}


void K3bDataBurnDialog::slotMultiSessionModeChanged()
{
  if( m_comboMultisession->multiSessionMode() == K3bDataDoc::CONTINUE ||
      m_comboMultisession->multiSessionMode() == K3bDataDoc::FINISH )
    m_spinCopies->setEnabled(false);

  // wait for the proper medium
  // we have to do this in another slot than toggleAll to avoid an endless loop
  // FIXME: K3bInteractionDialog::slotToggleAll is endless loop protected
  if( m_comboMultisession->multiSessionMode() == K3bDataDoc::NONE )
    m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_EMPTY );
  else if( m_comboMultisession->multiSessionMode() == K3bDataDoc::CONTINUE ||
	   m_comboMultisession->multiSessionMode() == K3bDataDoc::FINISH )
    m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_INCOMPLETE );
  else
    m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_EMPTY|K3bDevice::STATE_INCOMPLETE );
}


#include "k3bdataburndialog.moc"
