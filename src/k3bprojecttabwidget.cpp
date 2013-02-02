/* 
 *
 * $Id: k3bprojecttabwidget.cpp 619556 2007-01-03 17:38:12Z trueg $
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



#include "k3bprojecttabwidget.h"
#include "k3bapplication.h"
#include "k3bprojectmanager.h"

#include <k3bview.h>
#include <k3bdoc.h>

#include <tdeaction.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kurldrag.h>
#include <klocale.h>

#include <tqevent.h>
#include <tqtabbar.h>


class K3bProjectTabWidget::ProjectData
{
public:
  ProjectData()
    : doc(0),
      modified(false) {
  }

  ProjectData( K3bDoc* doc_ )
    : doc(doc_),
      modified(false) {
  }

  // the project
  K3bDoc* doc;

  // is the project marked modified here
  bool modified;
};



K3bProjectTabWidget::K3bProjectTabWidget( TQWidget *parent, const char *name, WFlags f )
  : TQTabWidget( parent, name, f )
{
  tabBar()->setAcceptDrops(true);
  tabBar()->installEventFilter( this );

  m_projectActionMenu = new TDEActionMenu( i18n("Project"), TQT_TQOBJECT(this) );
}


K3bProjectTabWidget::~K3bProjectTabWidget()
{
}


void K3bProjectTabWidget::addTab( TQWidget* child, const TQString& label )
{
  TQTabWidget::addTab( child, label );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::addTab( TQWidget* child, const TQIconSet& iconset, const TQString& label )
{
  TQTabWidget::addTab( child, iconset, label );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::addTab( TQWidget* child, TQTab* tab )
{
  TQTabWidget::addTab( child, tab );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::insertTab( TQWidget* child, const TQString& label, int index )
{
  TQTabWidget::insertTab( child, label, index );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::insertTab( TQWidget* child, const TQIconSet& iconset, const TQString& label, int index )
{
  TQTabWidget::insertTab( child, iconset, label, index );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::insertTab( TQWidget* child, TQTab* tab, int index )
{
  TQTabWidget::insertTab( child, tab, index );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::removePage( TQWidget* w )
{
  TQTabWidget::removePage( w );
  tabBar()->setShown( count() != 1 );
}


void K3bProjectTabWidget::insertTab( K3bDoc* doc )
{
  TQTabWidget::insertTab( doc->view(), doc->URL().fileName() );
  connect( k3bappcore->projectManager(), TQT_SIGNAL(projectSaved(K3bDoc*)), this, TQT_SLOT(slotDocSaved(K3bDoc*)) );
  connect( doc, TQT_SIGNAL(changed(K3bDoc*)), this, TQT_SLOT(slotDocChanged(K3bDoc*)) );

  m_projectDataMap[doc] = ProjectData( doc );

  if( doc->isModified() )
    slotDocChanged( doc );
  else
    slotDocSaved( doc );
}


void K3bProjectTabWidget::insertAction( TDEAction* action )
{
  m_projectActionMenu->insert( action );
}


void K3bProjectTabWidget::slotDocChanged( K3bDoc* doc )
{
  // we need to cache the icon changes since the changed() signal will be emitted very often
  if( !m_projectDataMap[doc].modified ) {
    setTabIconSet( doc->view(), SmallIconSet( "filesave" ) );
    m_projectDataMap[doc].modified = true;

    // we need this one for the session management
    changeTab( doc->view(), doc->URL().fileName() );
  }
}


void K3bProjectTabWidget::slotDocSaved( K3bDoc* doc )
{
  setTabIconSet( doc->view(), TQIconSet() );
  changeTab( doc->view(), doc->URL().fileName() );
}


K3bDoc* K3bProjectTabWidget::projectAt( const TQPoint& pos ) const
{
  TQTab* tab = tabBar()->selectTab( pos );
  if( tab ) {
    TQWidget* w = page( tabBar()->indexOf( tab->identifier() ) );
    if( K3bView* view = dynamic_cast<K3bView*>(w) )
      return view->doc();
  }

  return 0;
}


bool K3bProjectTabWidget::eventFilter( TQObject* o, TQEvent* e )
{
  if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(tabBar()) ) {
    if( e->type() == TQEvent::MouseButtonPress ) {
      TQMouseEvent* me = TQT_TQMOUSEEVENT(e);
      if( me->button() == Qt::RightButton ) {
	if( projectAt( me->pos() ) ) {
	  // we need change the tab because the actions work on the current tab
	  TQTab* clickedTab = tabBar()->selectTab( me->pos() );
	  if( clickedTab ) {
	    tabBar()->setCurrentTab( clickedTab );
	    
	    // show the popup menu
	    m_projectActionMenu->popup( me->globalPos() );
	  }
	}
	return true;
      }
    }

    else if( e->type() == TQEvent::DragMove ) {
      TQDragMoveEvent* de = static_cast<TQDragMoveEvent*>(e);
      de->accept( KURLDrag::canDecode(de) && projectAt(de->pos()) );
      return true;
    }

    else if( e->type() == TQEvent::Drop ) {
      TQDropEvent* de = static_cast<TQDropEvent*>(e);
      KURL::List l;
      if( KURLDrag::decode( de, l ) ) {
	if( K3bDoc* doc = projectAt( de->pos() ) )
	  dynamic_cast<K3bView*>(doc->view())->addUrls( l );
      }
      return true;
    }
  }

  return TQTabWidget::eventFilter( o, e );
}

#include "k3bprojecttabwidget.moc"

