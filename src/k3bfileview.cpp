/*
 *
 * $Id: k3bfileview.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bfileview.h"
#include "k3b.h"
#include "k3bdiroperator.h"
#include "k3btoolbox.h"
#include "k3bapplication.h"

#include <tqwidget.h>
#include <tqdragobject.h>
#include <tqlayout.h>
#include <tqdir.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqtoolbutton.h>

#include <tdefiledetailview.h>
#include <tdelistview.h>
#include <tdeaction.h>
#include <tdetoolbar.h>
#include <tdetoolbarbutton.h>
#include <kurl.h>
#include <kurldrag.h>
#include <tdefilefiltercombo.h>
#include <tdelocale.h>
#include <tdefileitem.h>
#include <tdemessagebox.h>
#include <kdirlister.h>
#include <kprogress.h>


K3bFileView::K3bFileView(TQWidget *parent, const char *name ) 
  : K3bContentsView( false, parent, name)
{
  setupGUI();
}


K3bFileView::~K3bFileView()
{
}


TDEActionCollection* K3bFileView::actionCollection() const
{
  return m_dirOp->actionCollection();
}


void K3bFileView::setupGUI()
{
  TQVBoxLayout* layout = new TQVBoxLayout( this );
  //  layout->setAutoAdd( true );

  m_dirOp = new K3bDirOperator( KURL::fromPathOrURL(TQDir::home().absPath()), this );
  m_toolBox = new K3bToolBox( this );

  layout->addWidget( m_toolBox );
  layout->addWidget( m_dirOp );
  layout->setStretchFactor( m_dirOp, 1 );

  // setup actions
  TDEAction* actionHome = m_dirOp->actionCollection()->action("home");
  TDEAction* actionBack = m_dirOp->actionCollection()->action("back");
  TDEAction* actionUp = m_dirOp->actionCollection()->action("up");
  TDEAction* actionReload = m_dirOp->actionCollection()->action("reload");

  m_toolBox->addButton( actionUp );
  m_toolBox->addButton( actionBack );
  m_toolBox->addButton( actionHome );
  m_toolBox->addButton( actionReload );
  m_toolBox->addSpacing();
  m_toolBox->addButton( m_dirOp->actionCollection()->action("short view") );
  m_toolBox->addButton( m_dirOp->actionCollection()->action("detailed view") );
  m_toolBox->addSpacing();
  m_toolBox->addButton( m_dirOp->bookmarkMenu() );
  m_toolBox->addSpacing();

  // create filter selection combobox
  m_toolBox->addSpacing();
  m_toolBox->addLabel( i18n("Filter:") );
  m_toolBox->addSpacing();
  m_filterWidget = new KFileFilterCombo( m_toolBox, "filterwidget" );
  m_toolBox->addWidget( m_filterWidget );
  m_toolBox->addStretch();
  m_toolBox->addWidget( m_dirOp->progressBar() );

  m_filterWidget->setEditable( true );
  TQString filter = i18n("*|All Files");
  filter += "\n" + i18n("audio/x-mp3 audio/x-wav application/x-ogg |Sound Files");
  filter += "\n" + i18n("audio/x-wav |Wave Sound Files");
  filter += "\n" + i18n("audio/x-mp3 |MP3 Sound Files");
  filter += "\n" + i18n("application/x-ogg |Ogg Vorbis Sound Files");
  filter += "\n" + i18n("video/mpeg |MPEG Video Files");
  m_filterWidget->setFilter(filter);

  connect( m_filterWidget, TQ_SIGNAL(filterChanged()), TQ_SLOT(slotFilterChanged()) );

  connect( m_dirOp, TQ_SIGNAL(fileHighlighted(const KFileItem*)), this, TQ_SLOT(slotFileHighlighted(const KFileItem*)) );
  connect( m_dirOp, TQ_SIGNAL(urlEntered(const KURL&)), this, TQ_SIGNAL(urlEntered(const KURL&)) );
  connect( m_dirOp, TQ_SIGNAL(fileSelected(const KFileItem*)), m_dirOp, TQ_SLOT(slotAddFilesToProject()) );

  slotFileHighlighted(0);
}

void K3bFileView::setDir( const TQString& dir )
{
  KURL url;
  url.setPath(dir);
  setUrl( url );
}


void K3bFileView::setUrl(const KURL& url, bool forward)
{
  m_dirOp->setURL( url, forward );
}

KURL K3bFileView::url()
{
  return m_dirOp->url();
}

void K3bFileView::setAutoUpdate( bool b )
{
  m_dirOp->dirLister()->setAutoUpdate( b );
}

void K3bFileView::slotFileHighlighted( const KFileItem* )
{
}


void K3bFileView::slotFilterChanged()
{
  TQString filter = m_filterWidget->currentFilter();
  m_dirOp->clearFilter();

  if( filter.find( '/' ) > -1 ) {
    TQStringList types = TQStringList::split( " ", filter );
    types.prepend( "inode/directory" );
    m_dirOp->setMimeFilter( types );
  }
  else
    m_dirOp->setNameFilter( filter );
  
  m_dirOp->rereadDir();
  //  emit filterChanged( filter );
}


void K3bFileView::reload()
{
  m_dirOp->actionCollection()->action("reload")->activate();
}


void K3bFileView::saveConfig( TDEConfig* c )
{
  m_dirOp->writeConfig( c, "file view" );
}


void K3bFileView::readConfig( TDEConfig* c )
{
  m_dirOp->readConfig( c, "file view" );
}

#include "k3bfileview.moc"
