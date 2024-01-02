/* 
 *
 * $Id: k3bdeviceoptiontab.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bdeviceoptiontab.h"
#include <k3bdevicemanager.h>
#include "k3bdevicewidget.h"
#include <k3bglobals.h>
#include <k3bcore.h>

#include <tqlabel.h>
#include <tqstring.h>
#include <tqlayout.h>
#include <tqcursor.h>
#include <tqapplication.h>

#include <tdeapplication.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>


K3bDeviceOptionTab::K3bDeviceOptionTab( TQWidget* parent, const char* name )
  : TQWidget( parent, name )
{
  TQGridLayout* frameLayout = new TQGridLayout( this );
  frameLayout->setSpacing( KDialog::spacingHint() );
  frameLayout->setMargin( 0 );


  // Info Label
  // ------------------------------------------------
  m_labelDevicesInfo = new TQLabel( this, "m_labelDevicesInfo" );
  m_labelDevicesInfo->setAlignment( int( TQLabel::WordBreak | TQLabel::AlignVCenter | TQLabel::AlignLeft ) );
  m_labelDevicesInfo->setText( i18n( "K3b tries to detect all your devices properly. "
				     "You can add devices that have not been detected and change "
				     "the black values by clicking in the list. If K3b is unable "
				     "to detect your drive, you need to modify their permissions "
				     "to give K3b write access to all devices." ) );
  // ------------------------------------------------

  m_deviceWidget = new K3bDeviceWidget( k3bcore->deviceManager(), this );

  frameLayout->addWidget( m_labelDevicesInfo, 0, 0 );
  frameLayout->addWidget( m_deviceWidget, 1, 0 );

  connect( m_deviceWidget, TQ_SIGNAL(refreshButtonClicked()), this, TQ_SLOT(slotRefreshButtonClicked()) );
}


K3bDeviceOptionTab::~K3bDeviceOptionTab()
{
}


void K3bDeviceOptionTab::readDevices()
{
  m_deviceWidget->init();
}



void K3bDeviceOptionTab::saveDevices()
{
  // save changes to deviceManager
  m_deviceWidget->apply();

  // save the config
  k3bcore->deviceManager()->saveConfig( kapp->config() );
}


void K3bDeviceOptionTab::slotRefreshButtonClicked()
{
  TQApplication::setOverrideCursor( TQCursor(TQt::WaitCursor) );
  k3bcore->deviceManager()->clear();
  k3bcore->deviceManager()->scanBus();
  m_deviceWidget->init();
  TQApplication::restoreOverrideCursor();
}

#include "k3bdeviceoptiontab.moc"
