/*
 *
 * $Id$
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

#include "k3bcddboptiontab.h"

#include <tqvariant.h>
#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqlistbox.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqtabwidget.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqstringlist.h>
#include <tqcombobox.h>
#include <tqtoolbutton.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <krun.h>
#include <tdelistview.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <knuminput.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <tdeversion.h>


K3bCddbOptionTab::K3bCddbOptionTab( TQWidget* parent,  const char* name )
    : base_K3bCddbOptionTab( parent, name )
{
  // fix all the margins and spacings that have been corrupted by TQDesigner ;-)
  // -----------------------------------------------------------------------------

  base_K3bCddbOptionTabLayout->setMargin( 0 );
  base_K3bCddbOptionTabLayout->setSpacing( KDialog::spacingHint() );

  m_mainTabbed->page(0)->layout()->setMargin( KDialog::marginHint() );
  m_mainTabbed->page(0)->layout()->setSpacing( KDialog::spacingHint() );
  m_mainTabbed->page(1)->layout()->setMargin( KDialog::marginHint() );
  m_mainTabbed->page(1)->layout()->setSpacing( KDialog::spacingHint() );

//   m_groupLocalDir->layout()->setMargin( 0 );
//   m_groupLocalDirLayout->setMargin( KDialog::marginHint() );
//   m_groupLocalDirLayout->setSpacing( KDialog::spacingHint() );

//   m_groupCddbServer->layout()->setMargin( 0 );
//   m_groupCddbServerLayout->setMargin( KDialog::marginHint() );
//   m_groupCddbServerLayout->setSpacing( KDialog::spacingHint() );

  m_groupCgi->layout()->setMargin( 0 );
  m_groupCgiLayout->setMargin( KDialog::marginHint() );
  m_groupCgiLayout->setSpacing( KDialog::spacingHint() );

  m_boxLocalDirectoryLayout->setSpacing( KDialog::spacingHint() );
  m_boxCddbServerLayout->setSpacing( KDialog::spacingHint() );
  // -----------------------------------------------------------------------------


  m_viewLocalDir->setSorting(-1);
  m_viewLocalDir->setAcceptDrops( true );
  m_viewLocalDir->setDragEnabled( true );
  m_viewLocalDir->setDropVisualizer( true );

  m_viewCddbServer->setSorting(-1);
  m_viewCddbServer->setAcceptDrops( true );
  m_viewCddbServer->setDragEnabled( true );
  m_viewCddbServer->setDropVisualizer( true );

  m_comboCddbType->insertItem( "Http" );
  m_comboCddbType->insertItem( "Cddbp" );


  // set icons for the buttons
  m_buttonAddLocalDir->setPixmap( SmallIcon("ok") );
  m_buttonRemoveLocalDir->setPixmap( SmallIcon("process-stop") );
  m_buttonLocalDirUp->setPixmap( SmallIcon("go-up") );
  m_buttonLocalDirDown->setPixmap( SmallIcon("go-down") );
  m_buttonAddCddbServer->setPixmap( SmallIcon("ok") );
  m_buttonRemoveCddbServer->setPixmap( SmallIcon("process-stop") );
  m_buttonCddbServerUp->setPixmap( SmallIcon("go-up") );
  m_buttonCddbServerDown->setPixmap( SmallIcon("go-down") );


  // setup connections
  // -----------------------------------------------------------------------------
  connect( m_buttonAddLocalDir, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotLocalDirAdd()) );
  connect( m_buttonRemoveLocalDir, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotLocalDirRemove()) );
  connect( m_buttonLocalDirUp, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotLocalDirUp()) );
  connect( m_buttonLocalDirDown, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotLocalDirDown()) );

  connect( m_buttonAddCddbServer, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotCddbServerAdd()) );
  connect( m_buttonRemoveCddbServer, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotCddbServerRemove()) );
  connect( m_buttonCddbServerUp, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotCddbServerUp()) );
  connect( m_buttonCddbServerDown, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotCddbServerDown()) );

  connect( m_editLocalDir, TQ_SIGNAL(textChanged(const TQString&)), this, TQ_SLOT(enDisableButtons()) );
  connect( m_editCddbServer, TQ_SIGNAL(textChanged(const TQString&)), this, TQ_SLOT(enDisableButtons()) );
  connect( m_viewLocalDir, TQ_SIGNAL(selectionChanged()), this, TQ_SLOT(enDisableButtons()) );
  connect( m_viewCddbServer, TQ_SIGNAL(selectionChanged()), this, TQ_SLOT(enDisableButtons()) );
  connect( m_comboCddbType, TQ_SIGNAL(highlighted(int)),
	   this, TQ_SLOT(slotServerTypeChanged()) );
  connect( m_comboCddbType, TQ_SIGNAL(activated(int)),
	   this, TQ_SLOT(slotServerTypeChanged()) );
  // -----------------------------------------------------------------------------

  enDisableButtons();
}


K3bCddbOptionTab::~K3bCddbOptionTab()
{
}


void K3bCddbOptionTab::readSettings()
{
  TDEConfig* c = kapp->config();

  c->setGroup( "Cddb" );

  // old config <= 0.7.3
  TQStringList cddbpServer = c->readListEntry( "cddbp server" );
  TQStringList httpServer = c->readListEntry( "http server" );

  // new config
  TQStringList cddbServer = c->readListEntry( "cddb server" );

  TQStringList localCddbDirs = c->readPathListEntry( "local cddb dirs" );

  m_checkRemoteCddb->setChecked( c->readBoolEntry( "use remote cddb", true ) );
  m_checkUseLocalCddb->setChecked( c->readBoolEntry( "use local cddb query", true ) );
  m_checkSaveLocalEntries->setChecked( c->readBoolEntry( "save cddb entries locally", true ) );
  m_checkManualCgiPath->setChecked( c->readBoolEntry( "use manual cgi path", false ) );
  m_editManualCgiPath->setText( c->readEntry( "cgi path", "/~cddb/cddb.cgi" ) );

  if( localCddbDirs.isEmpty() )
    localCddbDirs.append( "~/.cddb/" );

  for( TQStringList::const_iterator it = localCddbDirs.begin(); it != localCddbDirs.end(); ++it )
    (void)new TDEListViewItem( m_viewLocalDir, m_viewLocalDir->lastItem(), *it );


  // old config <= 0.7.3
  if( !httpServer.isEmpty() ) {
    for( TQStringList::iterator it = httpServer.begin(); it != httpServer.end(); ++it ) {
      cddbServer.append( "Http " + *it );
    }
  }
  if( !cddbpServer.isEmpty() ) {
    for( TQStringList::iterator it = cddbpServer.begin(); it != cddbpServer.end(); ++it ) {
      cddbServer.append( "Cddbp " + *it );
    }
  }

  if( cddbServer.isEmpty() )
    cddbServer.append( "Http freedb2.org:80" );

  for( TQStringList::const_iterator it = cddbServer.begin(); it != cddbServer.end(); ++it ) {
    const TQString& s = *it;
    TQStringList buf = TQStringList::split( ":", s.mid( s.find(" ")+1 ) );
    TQString server = buf[0];
    int port = buf[1].toInt();
    if( s.startsWith("Http") )
      (void)new TDEListViewItem( m_viewCddbServer, m_viewCddbServer->lastItem(), "Http", server, TQString::number(port) );
    else
      (void)new TDEListViewItem( m_viewCddbServer, m_viewCddbServer->lastItem(), "Cddbp", server, TQString::number(port) );
  }

  enDisableButtons();
}


void K3bCddbOptionTab::apply()
{
  TDEConfig* c = kapp->config();

  c->setGroup( "Cddb" );

  c->writeEntry( "use remote cddb", m_checkRemoteCddb->isChecked() );
  c->writeEntry( "use local cddb query", m_checkUseLocalCddb->isChecked() );
  c->writeEntry( "save cddb entries locally", m_checkSaveLocalEntries->isChecked() );
  c->writeEntry( "use manual cgi path", m_checkManualCgiPath->isChecked() );
  c->writeEntry( "cgi path", m_editManualCgiPath->text() );

  TQStringList cddbServer;
  TQStringList localCddbDirs;

  TQListViewItemIterator it( m_viewLocalDir );
  while( it.current() ) {
    localCddbDirs.append( it.current()->text(0) );
    ++it;
  }

  TQListViewItemIterator it1( m_viewCddbServer );
  while( it1.current() ) {
    cddbServer.append( it1.current()->text(0) + " " + it1.current()->text(1) + ":" + it1.current()->text(2) );
    ++it1;
  }

  // new config
  c->writeEntry( "cddb server", cddbServer );

  // old config <= 0.7.3
  if( c->hasKey( "http server" ) )
    c->deleteEntry( "http server" );
  if( c->hasKey( "cddbp server" ) )
    c->deleteEntry( "cddbp server" );

  c->writePathEntry( "local cddb dirs", localCddbDirs );
}


void K3bCddbOptionTab::slotLocalDirAdd()
{
  if( !m_editLocalDir->text().isEmpty() ) {
      TQString localDir( m_editLocalDir->text() );
      TQListViewItemIterator it( m_viewLocalDir );
      while( it.current() ) {
          if ( it.current()->text(0) == localDir )
              return;
          ++it;
      }
    (void)new TDEListViewItem( m_viewLocalDir, m_viewLocalDir->lastItem(),
			     localDir );

    enDisableButtons();
  }
}


void K3bCddbOptionTab::slotLocalDirRemove()
{
  if( TQListViewItem* item = m_viewLocalDir->selectedItem() )
    delete item;

  enDisableButtons();
}


void K3bCddbOptionTab::slotCddbServerAdd()
{
  if( !m_editCddbServer->text().isEmpty() ) {
    (void)new TDEListViewItem( m_viewCddbServer, m_viewCddbServer->lastItem(),
			     m_comboCddbType->currentText(),
			     m_editCddbServer->text(),
			     TQString::number( m_editCddbPort->value() ) );

    enDisableButtons();
  }
}


void K3bCddbOptionTab::slotCddbServerRemove()
{
  if( TQListViewItem* item = m_viewCddbServer->selectedItem() )
    delete item;

  enDisableButtons();
}


void K3bCddbOptionTab::enDisableButtons()
{
  m_buttonAddLocalDir->setDisabled( m_editLocalDir->text().isEmpty() );
  m_buttonRemoveLocalDir->setDisabled( m_viewLocalDir->selectedItem() == 0 );
  m_buttonLocalDirUp->setDisabled( m_viewLocalDir->selectedItem() == 0 ||
				   m_viewLocalDir->selectedItem() == m_viewLocalDir->firstChild() );
  m_buttonLocalDirDown->setDisabled( m_viewLocalDir->selectedItem() == 0 ||
				     m_viewLocalDir->selectedItem() == m_viewLocalDir->lastItem() );

  m_buttonAddCddbServer->setDisabled( m_editCddbServer->text().isEmpty() );
  m_buttonRemoveCddbServer->setDisabled( m_viewCddbServer->selectedItem() == 0 );
  m_buttonCddbServerUp->setDisabled( m_viewCddbServer->selectedItem() == 0 ||
				     m_viewCddbServer->selectedItem() == m_viewCddbServer->firstChild() );
  m_buttonCddbServerDown->setDisabled( m_viewCddbServer->selectedItem() == 0 ||
				       m_viewCddbServer->selectedItem() == m_viewCddbServer->lastItem() );

  if( m_viewLocalDir->childCount() <= 0 ) {
    m_checkSaveLocalEntries->setChecked(false);
  }
  m_checkSaveLocalEntries->setDisabled( m_viewLocalDir->childCount() <= 0 );
}


void K3bCddbOptionTab::slotServerTypeChanged()
{
  m_editCddbPort->setValue( m_comboCddbType->currentText() == "Http" ? 80 : 8880 );
}


void K3bCddbOptionTab::slotLocalDirDown()
{
  TQListViewItem* sel = m_viewLocalDir->selectedItem();
  m_viewLocalDir->moveItem( sel, 0, sel->nextSibling() );
  m_viewLocalDir->setSelected( sel, true );
}


void K3bCddbOptionTab::slotLocalDirUp()
{
  TQListViewItem* sel = m_viewLocalDir->selectedItem();
  m_viewLocalDir->moveItem( sel, 0, sel->itemAbove()->itemAbove() );
  m_viewLocalDir->setSelected( sel, true );
}


void K3bCddbOptionTab::slotCddbServerDown()
{
  TQListViewItem* sel = m_viewCddbServer->selectedItem();
  m_viewCddbServer->moveItem( sel, 0, sel->nextSibling() );
  m_viewCddbServer->setSelected( sel, true );
}


void K3bCddbOptionTab::slotCddbServerUp()
{
  TQListViewItem* sel = m_viewCddbServer->selectedItem();
  m_viewCddbServer->moveItem( sel, 0, sel->itemAbove()->itemAbove() );
  m_viewCddbServer->setSelected( sel, true );
}

#include "k3bcddboptiontab.moc"
