/* 
 *
 * $Id: k3bexternalbinoptiontab.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bexternalbinoptiontab.h"
#include <k3bexternalbinmanager.h>
#include "k3bexternalbinwidget.h"

#include <kmessagebox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <tdelistview.h>

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqfile.h>
#include <tqptrlist.h>



K3bExternalBinOptionTab::K3bExternalBinOptionTab( K3bExternalBinManager* manager, TQWidget* parent, const char* name )
  : TQWidget( parent, name )
{
  m_manager = manager;

  TQGridLayout* frameLayout = new TQGridLayout( this );
  frameLayout->setSpacing( KDialog::spacingHint() );
  frameLayout->setMargin( 0 );

  m_externalBinWidget = new K3bExternalBinWidget( manager, this );
  frameLayout->addWidget( m_externalBinWidget, 1, 0 );

  TQLabel* m_labelInfo = new TQLabel( this, "m_labelInfo" );
  m_labelInfo->setText( i18n( "Specify the paths to the external programs that K3b needs to work properly, "
			      "or press \"Search\" to let K3b search for the programs." ) );
  m_labelInfo->setScaledContents( false );
  m_labelInfo->setAlignment( int( TQLabel::WordBreak | TQLabel::AlignVCenter | TQLabel::AlignLeft ) );

  frameLayout->addWidget( m_labelInfo, 0, 0 );
}


K3bExternalBinOptionTab::~K3bExternalBinOptionTab()
{
}


void K3bExternalBinOptionTab::readSettings()
{
  m_externalBinWidget->load();
}


void K3bExternalBinOptionTab::saveSettings()
{
  m_externalBinWidget->save();
}


#include "k3bexternalbinoptiontab.moc"
