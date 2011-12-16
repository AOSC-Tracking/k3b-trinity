/*
 *
 * $Id: k3bdeviceselectiondialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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



#include "k3bdeviceselectiondialog.h"
#include <k3bdevice.h>
#include <k3bdevicecombobox.h>
#include <k3bcore.h>
#include <k3bdevicemanager.h>

#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqstring.h>
#include <tqframe.h>

#include <klocale.h>


class K3bDeviceSelectionDialog::Private
{
public:
  K3bDeviceComboBox* comboDevices;
};


K3bDeviceSelectionDialog::K3bDeviceSelectionDialog( TQWidget* parent, 
						    const char* name, 
						    const TQString& text,
						    bool modal )
  : KDialogBase( KDialogBase::Plain, 
		 i18n("Device Selection"), 
		 Ok|Cancel, 
		 Ok,
		 parent,
		 name,
		 modal )
{
  d = new Private();

  TQGridLayout* lay = new TQGridLayout( plainPage() );

  TQLabel* label = new TQLabel( text.isEmpty() ? i18n("Please select a device:") : text, plainPage() );
  d->comboDevices = new K3bDeviceComboBox( plainPage() );

  //  lay->setMargin( marginHint() );
  lay->setSpacing( spacingHint() );
  lay->addWidget( label, 0, 0 );
  lay->addWidget( d->comboDevices, 1, 0 );
  lay->setRowStretch( 2, 1 );
}


K3bDeviceSelectionDialog::~K3bDeviceSelectionDialog()
{
  delete d;
}


void K3bDeviceSelectionDialog::addDevice( K3bDevice::Device* dev )
{
  d->comboDevices->addDevice( dev );
}


void K3bDeviceSelectionDialog::addDevices( const TQPtrList<K3bDevice::Device>& list )
{
  d->comboDevices->addDevices( list );
}


K3bDevice::Device* K3bDeviceSelectionDialog::selectedDevice() const
{
  return d->comboDevices->selectedDevice();
}


void K3bDeviceSelectionDialog::setSelectedDevice( K3bDevice::Device* dev )
{
  d->comboDevices->setSelectedDevice( dev );
}


K3bDevice::Device* K3bDeviceSelectionDialog::selectDevice( TQWidget* parent, 
							       const TQPtrList<K3bDevice::Device>& devices,
							       const TQString& text )
{
  if( devices.isEmpty() )
    return 0;
  if( devices.count() == 1 )
    return devices.getFirst();

  K3bDeviceSelectionDialog dlg( parent, 0, text );
  dlg.addDevices( devices );

  if( dlg.exec() == Accepted )
    return dlg.selectedDevice();
  else
    return 0;
}

K3bDevice::Device* K3bDeviceSelectionDialog::selectDevice( TQWidget* parent, 
							       const TQString& text )
{
  return selectDevice( parent, k3bcore->deviceManager()->allDevices(), text );


}


K3bDevice::Device* K3bDeviceSelectionDialog::selectWriter( TQWidget* parent, const TQString& text )
{
  return selectDevice( parent, k3bcore->deviceManager()->burningDevices(), text );
}


#include "k3bdeviceselectiondialog.moc"
