/*
 *
 * $Id: k3bdatapropertiesdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bdatapropertiesdialog.h"

#include "k3bdiritem.h"
#include "k3bfileitem.h"
#include <kcutlabel.h>
#include <k3bvalidators.h>

#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqframe.h>
#include <tqcheckbox.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqtabwidget.h>
#include <tqvalidator.h>

#include <klineedit.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include <tdeio/global.h>
#include <tdefileitem.h>



K3bDataPropertiesDialog::K3bDataPropertiesDialog( K3bDataItem* dataItem, TQWidget* parent, const char* name )
  : KDialogBase( Plain, i18n("File Properties"), Ok|Cancel, Ok, parent, name, true, false )
{
  m_dataItem = dataItem;

  TQLabel* labelMimeType = new TQLabel( plainPage() );
  TQLabel* extraInfoLabel = new TQLabel( plainPage() );
  m_editName = new KLineEdit( plainPage() );
  m_labelType = new TQLabel( plainPage() );
  m_labelLocation = new KCutLabel( plainPage() );
  m_labelSize = new TQLabel( plainPage() );
  m_labelBlocks = new TQLabel( plainPage() );
  m_labelLocalName = new KCutLabel( plainPage() );
  m_labelLocalLocation = new KCutLabel( plainPage() );


  TQGridLayout* grid = new TQGridLayout( plainPage() );
  grid->setSpacing( spacingHint() );
  grid->setMargin( marginHint() );

  grid->addWidget( labelMimeType, 0, 0 );
  grid->addWidget( m_editName, 0, 2 );
  TQFrame* line = new TQFrame( plainPage() );
  line->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
  grid->addMultiCellWidget( line, 1, 1, 0, 2 );
  grid->addWidget( new TQLabel( i18n("Type:"), plainPage() ), 2, 0 );
  grid->addWidget( new TQLabel( i18n("Location:"), plainPage() ), 4, 0 );
  grid->addWidget( new TQLabel( i18n("Size:"), plainPage() ), 5, 0 );
  grid->addWidget( new TQLabel( i18n("Used blocks:"), plainPage() ), 6, 0 );
  grid->addWidget( m_labelType, 2, 2 );
  grid->addWidget( extraInfoLabel, 3, 2 );
  grid->addWidget( m_labelLocation, 4, 2 );
  grid->addWidget( m_labelSize, 5, 2 );
  grid->addWidget( m_labelBlocks, 6, 2 );
  line = new TQFrame( plainPage() );
  line->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
  grid->addMultiCellWidget( line, 7, 7, 0, 2 );
  TQLabel* label1 = new TQLabel( i18n("Local name:"), plainPage() );
  grid->addWidget(  label1, 8, 0 );
  TQLabel* label2 = new TQLabel( i18n("Local location:"), plainPage() );
  grid->addWidget( label2, 9, 0 );
  grid->addWidget( m_labelLocalName, 8, 2 );
  grid->addWidget( m_labelLocalLocation, 9, 2 );

  grid->addColSpacing( 1, 50 );
  grid->setColStretch( 2, 1 );



  if( K3bFileItem* fileItem = dynamic_cast<K3bFileItem*>(dataItem) ) {
    KFileItem kFileItem( KFileItem::Unknown, KFileItem::Unknown, KURL::fromPathOrURL(fileItem->localPath()) );
    labelMimeType->setPixmap( kFileItem.pixmap(TDEIcon::SizeLarge) );
    if( fileItem->isSymLink() )
      m_labelType->setText( i18n("Link to %1").arg(kFileItem.mimeComment()) );
    else
      m_labelType->setText( kFileItem.mimeComment() );
    m_labelLocalName->setText( kFileItem.name() );
    TQString localLocation = kFileItem.url().path(-1);
    localLocation.truncate( localLocation.findRev('/') );
    m_labelLocalLocation->setText( localLocation );
    m_labelSize->setText( TDEIO::convertSize(dataItem->size()) );
  }
  else if( K3bDirItem* dirItem = dynamic_cast<K3bDirItem*>(dataItem) ) {
    labelMimeType->setPixmap( KMimeType::pixmapForURL( KURL( "/" )) );
    m_labelType->setText( i18n("Directory") );
    label1->hide();
    label2->hide();
    m_labelLocalName->hide();
    m_labelLocalLocation->hide();
    line->hide();
    m_labelSize->setText( TDEIO::convertSize(dataItem->size()) + "\n(" +
			  i18n("in 1 file", "in %n files", dirItem->numFiles()) + " " +
			  i18n("and 1 directory", "and %n directories", dirItem->numDirs()) + ")" );
  }
  else {
    labelMimeType->setPixmap( DesktopIcon("unknown", TDEIcon::SizeLarge) );
    m_labelType->setText( i18n("Special file") );
    m_labelLocalName->hide();
    m_labelLocalLocation->hide();
    label1->hide();
    label2->hide();
    line->hide();
    m_labelSize->setText( TDEIO::convertSize(dataItem->size()) );
  }

  m_editName->setText( dataItem->k3bName() );
  m_labelBlocks->setText( TQString::number(dataItem->blocks().lba()) );

  TQString location = "/" + dataItem->k3bPath();
  if( location[location.length()-1] == '/' )
    location.truncate( location.length()-1 );
  location.truncate( location.findRev('/') );
  if( location.isEmpty() )
    location = "/";
  m_labelLocation->setText( location );
  extraInfoLabel->setText( TQString( "(%1)" ).arg(dataItem->extraInfo()) );
  if( dataItem->extraInfo().isEmpty() )
    extraInfoLabel->hide();

  // OPTIONS
  // /////////////////////////////////////////////////
  TQTabWidget* optionTab = new TQTabWidget( plainPage() );
  line = new TQFrame( plainPage() );
  line->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );

  grid->addMultiCellWidget( line, 10, 10, 0, 2 );
  grid->addMultiCellWidget( optionTab, 12, 12, 0, 2 );
  grid->setRowStretch( 11, 1 );

  TQWidget* hideBox = new TQWidget( optionTab );
  TQGridLayout* hideBoxGrid = new TQGridLayout( hideBox );
  hideBoxGrid->setSpacing( spacingHint() );
  hideBoxGrid->setMargin( marginHint() );
  m_checkHideOnRockRidge = new TQCheckBox( i18n("Hide on Rockridge"), hideBox );
  m_checkHideOnJoliet = new TQCheckBox( i18n("Hide on Joliet"), hideBox );
  hideBoxGrid->addWidget( m_checkHideOnRockRidge, 0, 0 );
  hideBoxGrid->addWidget( m_checkHideOnJoliet, 1, 0 );
  hideBoxGrid->setRowStretch( 2, 1 );
//   grid->addMultiCellWidget( m_checkHideOnRockRidge, 10, 10, 0, 2 );
//   grid->addMultiCellWidget( m_checkHideOnJoliet, 11, 11, 0, 2 );

  TQWidget* sortingBox = new TQWidget( optionTab );
  TQGridLayout* sortingBoxGrid = new TQGridLayout( sortingBox );
  sortingBoxGrid->setSpacing( spacingHint() );
  sortingBoxGrid->setMargin( marginHint() );
  m_editSortWeight = new KLineEdit( sortingBox );
  m_editSortWeight->setValidator( new TQIntValidator( -2147483647, 2147483647, m_editSortWeight ) );
  m_editSortWeight->setAlignment( TQt::AlignRight );
  sortingBoxGrid->addWidget( new TQLabel( i18n("Sort weight:"), sortingBox ), 0, 0 );
  sortingBoxGrid->addWidget( m_editSortWeight, 0, 1 );
  sortingBoxGrid->setColStretch( 1, 1 );
  sortingBoxGrid->setRowStretch( 1, 1 );

  optionTab->addTab( hideBox, i18n("Settings") );
  optionTab->addTab( sortingBox, i18n("Advanced") );


  m_checkHideOnJoliet->setChecked( dataItem->hideOnJoliet() );
  m_checkHideOnRockRidge->setChecked( dataItem->hideOnRockRidge() );
  m_editSortWeight->setText( TQString::number(dataItem->sortWeight()) );

  // if the parent is hidden the value cannot be changed (see K3bDataItem::setHide...)
  if( dataItem->parent() ) {
    m_checkHideOnRockRidge->setDisabled( dataItem->parent()->hideOnRockRidge() );
    m_checkHideOnJoliet->setDisabled( dataItem->parent()->hideOnJoliet() );
  }

  if( !dataItem->isHideable() ) {
    m_checkHideOnJoliet->setDisabled(true);
    m_checkHideOnRockRidge->setDisabled(true);
    //    line->hide();
  }

  TQToolTip::add( m_checkHideOnRockRidge, i18n("Hide this file in the RockRidge filesystem") );
  TQToolTip::add( m_checkHideOnJoliet, i18n("Hide this file in the Joliet filesystem") );
  TQToolTip::add( m_editSortWeight, i18n("Modify the physical sorting") );
  TQWhatsThis::add( m_checkHideOnRockRidge, i18n("<p>If this option is checked, the file or directory "
						"(and its entire contents) will be hidden on the "
						"ISO9660 and RockRidge filesystem.</p>"
						"<p>This is useful, for example, for having different README "
						"files for RockRidge and Joliet, which can be managed "
						"by hiding README.joliet on RockRidge and README.rr "
						"on the Joliet filesystem.</p>") );
  TQWhatsThis::add( m_checkHideOnJoliet, i18n("<p>If this option is checked, the file or directory "
					     "(and its entire contents) will be hidden on the "
					     "Joliet filesystem.</p>"
					     "<p>This is useful, for example, for having different README "
					     "files for RockRidge and Joliet, which can be managed "
					     "by hiding README.joliet on RockRidge and README.rr "
					     "on the Joliet filesystem.</p>") );
  TQWhatsThis::add( m_editSortWeight, i18n("<p>This value modifies the physical sort order of the files "
					  "in the ISO9660 filesystem. A higher weighting means that the "
					  "file will be located closer to the beginning of the image "
					  "(and the disk)."
					  "<p>This option is useful in order to optimize the data layout "
					  "on a CD/DVD."
					  "<p><b>Caution:</b> This does not sort the order of the file "
					  "names that appear in the ISO9660 directory."
					  "It sorts the order in which the file data is "
					  "written to the image.") );

  m_editName->setValidator( K3bValidators::iso9660Validator( false, this ) );
  m_editName->setReadOnly( !dataItem->isRenameable() );
  m_editName->setFocus();
}


K3bDataPropertiesDialog::~K3bDataPropertiesDialog()
{
}


void K3bDataPropertiesDialog::slotOk()
{
  // save filename
  m_dataItem->setK3bName( m_editName->text() );
  m_dataItem->setHideOnRockRidge( m_checkHideOnRockRidge->isChecked() );
  m_dataItem->setHideOnJoliet( m_checkHideOnJoliet->isChecked() );
  m_dataItem->setSortWeight( m_editSortWeight->text().toInt() );

  KDialogBase::slotOk();
}


#include "k3bdatapropertiesdialog.moc"
