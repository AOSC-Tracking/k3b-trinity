/* 
 *
 * $Id: k3bnotifyoptiontab.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include <kdeversion.h>

#include "k3bnotifyoptiontab.h"

#include <knotifydialog.h>
#include <kdebug.h>

#include <tqlayout.h>



K3bNotifyOptionTab::K3bNotifyOptionTab( TQWidget* parent, const char* name )
  : TQWidget( parent, name )
{
  m_notifyWidget = new KNotify::KNotifyWidget( this );

  TQHBoxLayout* box = new TQHBoxLayout( this );
  box->addWidget( m_notifyWidget );
}


K3bNotifyOptionTab::~K3bNotifyOptionTab()
{
}


void K3bNotifyOptionTab::readSettings()
{
  m_notifyWidget->clear();
  KNotify::Application* app = m_notifyWidget->addApplicationEvents( "k3b/eventsrc" );
  if( app )
    m_notifyWidget->addVisibleApp(app);
  else
    kdDebug() << "(K3bNotifyOptionTab) could not find K3b eventsrc." << endl;
}


bool K3bNotifyOptionTab::saveSettings()
{
  m_notifyWidget->save();

  return true;
}

#include "k3bnotifyoptiontab.moc"
