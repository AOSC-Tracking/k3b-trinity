/*
 *
 * $Id: k3bburningoptiontab.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bburningoptiontab.h"
#include <k3bmsfedit.h>
#include <k3bcore.h>
#include <k3bstdguiitems.h>
#include <k3bglobalsettings.h>

#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tqtabwidget.h>
#include <tqradiobutton.h>
#include <tqvalidator.h>
#include <tqbuttongroup.h>
#include <tqspinbox.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <knuminput.h>
#include <tdeconfig.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <klineedit.h>


K3bBurningOptionTab::K3bBurningOptionTab( TQWidget* parent, const char* name )
  : TQWidget( parent, name )
{
  setupGui();
}


K3bBurningOptionTab::~K3bBurningOptionTab()
{
}


void K3bBurningOptionTab::setupGui()
{
  TQGridLayout* groupAdvancedLayout = new TQGridLayout( this );
  groupAdvancedLayout->setAlignment( TQt::AlignTop );
  groupAdvancedLayout->setSpacing( KDialog::spacingHint() );
  groupAdvancedLayout->setMargin( 0 );


  TQGroupBox* groupWritingApp = new TQGroupBox( 0, TQt::Vertical, i18n("Burning"), this );
  groupWritingApp->layout()->setMargin( 0 );
  TQGridLayout* bufferLayout = new TQGridLayout( groupWritingApp->layout() );
  bufferLayout->setMargin( KDialog::marginHint() );
  bufferLayout->setSpacing( KDialog::spacingHint() );

  m_checkBurnfree = K3bStdGuiItems::burnproofCheckbox( groupWritingApp );
  m_checkOverburn = new TQCheckBox( i18n("Allow overburning (&not supported by cdrecord <= 1.10)"), groupWritingApp );
  m_checkForceUnsafeOperations = new TQCheckBox( i18n("Force unsafe operations"), groupWritingApp );
  m_checkManualWritingBufferSize = new TQCheckBox( i18n("&Manual writing buffer size") + ":", groupWritingApp );
  m_editWritingBufferSize = new KIntNumInput( 4, groupWritingApp );
  m_editWritingBufferSize->setSuffix( " " + i18n("MB") );
  m_checkAllowWritingAppSelection = new TQCheckBox( i18n("Manual writing application &selection"), groupWritingApp );
  bufferLayout->addMultiCellWidget( m_checkBurnfree, 0, 0, 0, 2 );
  bufferLayout->addMultiCellWidget( m_checkOverburn, 1, 1, 0, 2 );
  bufferLayout->addMultiCellWidget( m_checkForceUnsafeOperations, 2, 2, 0, 2 );
  bufferLayout->addWidget( m_checkManualWritingBufferSize, 3, 0 );
  bufferLayout->addWidget( m_editWritingBufferSize, 3, 1 );
  bufferLayout->addMultiCellWidget( m_checkAllowWritingAppSelection, 4, 4, 0, 2 );
  bufferLayout->setColStretch( 2, 1 );

  TQGroupBox* groupMisc = new TQGroupBox( 2, TQt::Vertical, i18n("Miscellaneous"), this );
  m_checkEject = new TQCheckBox( i18n("Do not &eject medium after write process"), groupMisc );
  m_checkAutoErasingRewritable = new TQCheckBox( i18n("Automatically erase CD-RWs and DVD-RWs"), groupMisc );

  groupAdvancedLayout->addWidget( groupWritingApp, 0, 0 );
  groupAdvancedLayout->addWidget( groupMisc, 1, 0 );
  groupAdvancedLayout->setRowStretch( 2, 1 );


  connect( m_checkManualWritingBufferSize, TQ_SIGNAL(toggled(bool)),
           m_editWritingBufferSize, TQ_SLOT(setEnabled(bool)) );
  connect( m_checkManualWritingBufferSize, TQ_SIGNAL(toggled(bool)),
           this, TQ_SLOT(slotSetDefaultBufferSizes(bool)) );


  m_editWritingBufferSize->setDisabled( true );
  // -----------------------------------------------------------------------


  TQToolTip::add( m_checkOverburn, i18n("Allow burning more than the official media capacity") );
  TQToolTip::add( m_checkAllowWritingAppSelection, i18n("Allow to choose between cdrecord and cdrdao") );
  TQToolTip::add( m_checkAutoErasingRewritable, i18n("Automatically erase CD-RWs and DVD-RWs without asking") );
  TQToolTip::add( m_checkEject, i18n("Do not eject the burn medium after a completed burn process") );
  TQToolTip::add( m_checkForceUnsafeOperations, i18n("Force K3b to continue some operations otherwise deemed as unsafe") );

  TQWhatsThis::add( m_checkAllowWritingAppSelection, i18n("<p>If this option is checked K3b gives "
                                                         "the possibility to choose between cdrecord "
                                                         "and cdrdao when writing a cd."
                                                         "<p>This may be useful if one of the programs "
                                                         "does not support the used writer."
                                                         "<p><b>Be aware that K3b does not support both "
                                                         "programs in all project types.</b>") );

  TQWhatsThis::add( m_checkOverburn, i18n("<p>Each medium has an official maximum capacity which is stored in a read-only "
					 "area of the medium and is guaranteed by the vendor. However, this official "
					 "maximum is not always the actual maximum. Many media have an "
					 "actual total capacity that is slightly larger than the official amount."
					 "<p>If this option is checked K3b will disable a safety check that prevents "
					 "burning beyond the offical capacity."
					 "<p><b>Caution:</b> Enabling this option can cause failures in the end of the "
					 "burning process if K3b attempts to write beyond the official capacity. It "
					 "makes sense to first determine the actual maximum capacity of the media brand "
					 "with a simulated burn.") );

  TQWhatsThis::add( m_checkAutoErasingRewritable, i18n("<p>If this option is checked K3b will automatically "
                                                      "erase CD-RWs and format DVD-RWs if one is found instead "
                                                      "of an empty media before writing.") );

  TQWhatsThis::add( m_checkManualWritingBufferSize, i18n("<p>K3b uses a software buffer during the burning process to "
							"avoid gaps in the data stream due to high system load. The default "
							"sizes used are %1 MB for CD and %2 MB for DVD burning."
							"<p>If this option is checked the value specified will be used for both "
							"CD and DVD burning.").arg(4).arg(32) );

  TQWhatsThis::add( m_checkEject, i18n("<p>If this option is checked K3b will not eject the medium once the burn process "
				      "finishes. This can be helpful in case one leaves the computer after starting the "
				      "burning and does not want the tray to be open all the time."
				      "<p>However, on Linux systems a freshly burned medium has to be reloaded. Otherwise "
				      "the system will not detect the changes and still treat it as an empty medium.") );

  TQWhatsThis::add( m_checkForceUnsafeOperations, i18n("<p>If this option is checked K3b will continue in some situations "
						      "which would otherwise be deemed as unsafe."
						      "<p>This setting for example disables the check for medium speed "
						      "verification. Thus, one can force K3b to burn a high speed medium on "
						      "a low speed writer."
						      "<p><b>Caution:</b> Enabling this option may result in damaged media.") );
}


void K3bBurningOptionTab::readSettings()
{
  TDEConfig* c = k3bcore->config();

  c->setGroup( "General Options" );
  m_checkAutoErasingRewritable->setChecked( c->readBoolEntry( "auto rewritable erasing", false ) );
  m_checkAllowWritingAppSelection->setChecked( c->readBoolEntry( "Manual writing app selection", false ) );

  m_checkBurnfree->setChecked( k3bcore->globalSettings()->burnfree() );
  m_checkEject->setChecked( !k3bcore->globalSettings()->ejectMedia() );
  m_checkOverburn->setChecked( k3bcore->globalSettings()->overburn() );
  m_checkForceUnsafeOperations->setChecked( k3bcore->globalSettings()->force() );
  m_checkManualWritingBufferSize->setChecked( k3bcore->globalSettings()->useManualBufferSize() );
  if( k3bcore->globalSettings()->useManualBufferSize() )
    m_editWritingBufferSize->setValue( k3bcore->globalSettings()->bufferSize() );
}


void K3bBurningOptionTab::saveSettings()
{
  TDEConfig* c = k3bcore->config();

  c->setGroup( "General Options" );
  c->writeEntry( "auto rewritable erasing", m_checkAutoErasingRewritable->isChecked() );
  c->writeEntry( "Manual writing app selection", m_checkAllowWritingAppSelection->isChecked() );

  k3bcore->globalSettings()->setEjectMedia( !m_checkEject->isChecked() );
  k3bcore->globalSettings()->setOverburn( m_checkOverburn->isChecked() );
  k3bcore->globalSettings()->setBurnfree( m_checkBurnfree->isChecked() );
  k3bcore->globalSettings()->setUseManualBufferSize( m_checkManualWritingBufferSize->isChecked() );
  k3bcore->globalSettings()->setBufferSize( m_editWritingBufferSize->value() );
  k3bcore->globalSettings()->setForce( m_checkForceUnsafeOperations->isChecked() );

  // FIXME: remove this once libk3b does not use TDEConfig anymore for these values
  k3bcore->globalSettings()->saveSettings( c );
}


void K3bBurningOptionTab::slotSetDefaultBufferSizes( bool b )
{
  if( !b ) {
    m_editWritingBufferSize->setValue( 4 );
  }
}


#include "k3bburningoptiontab.moc"
