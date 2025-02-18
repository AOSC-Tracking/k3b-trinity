/* 
 *
 * $Id: k3bview.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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


// include files for TQt
#include <tqlayout.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqptrlist.h>
#include <tqtoolbutton.h>

#include <tdeaction.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kdebug.h>

// application specific includes
#include "k3bview.h"
#include "k3bdoc.h"
#include "k3bfillstatusdisplay.h"
#include "k3bprojectburndialog.h"
#include "k3bprojectplugindialog.h"
#include <k3btoolbox.h>
#include <k3bpluginmanager.h>
#include <k3bprojectplugin.h>
#include <k3bcore.h>


K3bView::K3bView( K3bDoc* pDoc, TQWidget *parent, const char* name )
  : TQWidget( parent, name ),
    m_doc( pDoc )
{
  TQGridLayout* grid = new TQGridLayout( this );

  m_toolBox = new K3bToolBox( this, "toolbox" );
  m_fillStatusDisplay = new K3bFillStatusDisplay( m_doc, this );

  grid->addMultiCellWidget( m_toolBox, 0, 0, 0, 1 );
  grid->addMultiCellWidget( m_fillStatusDisplay, 2, 2, 0, 1 );
  //  grid->addWidget( m_buttonBurn, 2, 1 );
  grid->setRowStretch( 1, 1 );
  grid->setColStretch( 0, 1 );
  grid->setSpacing( 5 );
  grid->setMargin( 2 );

  TDEAction* burnAction = new TDEAction( i18n("&Burn"), "cdburn", CTRL + Key_B, this, TQ_SLOT(slotBurn()),
				     actionCollection(), "project_burn");
  burnAction->setToolTip( i18n("Open the burn dialog for the current project") );
  TDEAction* propAction = new TDEAction( i18n("&Properties"), "edit", CTRL + Key_P, this, TQ_SLOT(slotProperties()),
				     actionCollection(), "project_properties");
  propAction->setToolTip( i18n("Open the properties dialog") );

  m_toolBox->addButton( burnAction, true );
  m_toolBox->addSeparator();

  // this is just for testing (or not?)
  // most likely every project type will have it's rc file in the future
  setXML( "<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">"
	  "<kpartgui name=\"k3bproject\" version=\"1\">"
	  "<MenuBar>"
	  " <Menu name=\"project\"><text>&amp;Project</text>"
	  "  <Action name=\"project_burn\"/>"
	  "  <Action name=\"project_properties\"/>"
	  " </Menu>"
	  "</MenuBar>"
	  "</kpartgui>", true );
}

K3bView::~K3bView()
{
}


void K3bView::setMainWidget( TQWidget* w )
{
  static_cast<TQGridLayout*>(layout())->addMultiCellWidget( w, 1, 1, 0, 1 );
}


void K3bView::slotBurn()
{
  if( m_doc->numOfTracks() == 0 || m_doc->size() == 0 ) {
    KMessageBox::information( this, i18n("Please add files to your project first."),
			      i18n("No Data to Burn"), TQString(), false );
  }
  else {
    K3bProjectBurnDialog* dlg = newBurnDialog( this );
    if( dlg ) {
      dlg->execBurnDialog(true);
      delete dlg;
    }
    else {
      kdDebug() << "(K3bDoc) Error: no burndialog available." << endl;
    }
  }
}


void K3bView::slotProperties()
{
  K3bProjectBurnDialog* dlg = newBurnDialog( this );
  if( dlg ) {
    dlg->execBurnDialog(false);
    delete dlg;
  }
  else {
    kdDebug() << "(K3bDoc) Error: no burndialog available." << endl;
  }
}


// TDEActionCollection* K3bView::actionCollection() const
// {
//   return m_actionCollection; 
// }


void K3bView::addPluginButtons( int projectType )
{
  TQPtrList<K3bPlugin> pl = k3bcore->pluginManager()->plugins( "ProjectPlugin" );
  for( TQPtrListIterator<K3bPlugin> it( pl ); *it; ++it ) {
    K3bProjectPlugin* pp = dynamic_cast<K3bProjectPlugin*>( *it );
    if( pp && (pp->type() & projectType) ) {
      TQToolButton* button = toolBox()->addButton( pp->text(),
						  pp->icon(),
						  pp->toolTip(),
						  pp->whatsThis(),
						  this, 
						  TQ_SLOT(slotPluginButtonClicked()) );
      m_plugins.insert( static_cast<void*>(button), pp );
    }
  }
}


void K3bView::slotPluginButtonClicked()
{
  TQObject* o = const_cast<TQObject*>(sender());
  if( K3bProjectPlugin* p = m_plugins[static_cast<void*>(o)] ) {
    if( p->hasGUI() ) {
      K3bProjectPluginDialog dlg( p, doc(), this );
      dlg.exec();
    }
    else
      p->activate( doc(), this );
  }
}


void K3bView::addUrl( const KURL& url )
{
  KURL::List urls(url);
  addUrls( urls );
}


void K3bView::addUrls( const KURL::List& urls )
{
  doc()->addUrls( urls );
}

#include "k3bview.moc"
