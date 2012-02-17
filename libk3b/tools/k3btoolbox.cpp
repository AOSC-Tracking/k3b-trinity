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

#include "k3btoolbox.h"

#include <kaction.h>
#include <kpopupmenu.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>

#include <tqtoolbutton.h>
#include <tqsizepolicy.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqlabel.h>
#include <tqvbox.h>
#include <tqstyle.h>
#include <tqpainter.h>
#include <tqevent.h>
#include <tqobjectlist.h>


/**
 * internal class. Do not use!
 */
class K3bToolBoxSeparator : public TQWidget
{
  //  Q_OBJECT
  

 public:
  K3bToolBoxSeparator( K3bToolBox* parent );
  
  TQSize sizeHint() const;
  
 protected:
  void paintEvent( TQPaintEvent * );
};


K3bToolBoxSeparator::K3bToolBoxSeparator( K3bToolBox* parent )
  : TQWidget( parent )
{
  setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum ) );
}


TQSize K3bToolBoxSeparator::sizeHint() const
{
  int extent = style().pixelMetric( TQStyle::PM_DockWindowSeparatorExtent,
				    this );
  return TQSize( extent, 0 );
}


void K3bToolBoxSeparator::paintEvent( TQPaintEvent* )
{
  TQPainter p( this );
  TQStyle::SFlags flags = TQStyle::Style_Default|TQStyle::Style_Horizontal;

  style().tqdrawPrimitive( TQStyle::PE_DockWindowSeparator, &p, rect(),
			 colorGroup(), flags );
}



K3bToolBoxButton::K3bToolBoxButton( KAction* action, TQWidget* parent )
  : TQToolButton( parent ),
    m_popupMenu(0)
{
  setSizePolicy( TQSizePolicy(TQSizePolicy::Fixed, sizePolicy().verData()) );
  setAutoRaise( true );

  setIconSet( action->iconSet() );
  setTextLabel( action->text() );
  setEnabled( action->isEnabled() );

  TQWhatsThis::add( this, action->whatsThis() );
  if( action->toolTip().isEmpty() )
    TQToolTip::add( this, action->text() );
  else
    TQToolTip::add( this, action->toolTip() );

//   if( KToggleAction* ta = dynamic_cast<KToggleAction*>( action ) ) {
//     setToggleButton( true );
    
//     // initial state
//     if( ta->isChecked() )
//       toggle();
    
//     connect( ta, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(toggle()) );
//     connect( this, TQT_SIGNAL(toggled(bool)), ta, TQT_SLOT(setChecked(bool)) );
//   }

//  else
  if( KActionMenu* am = dynamic_cast<KActionMenu*>( action ) ) {
    m_popupMenu = am->popupMenu();
    connect( this, TQT_SIGNAL(pressed()), this, TQT_SLOT(slotPopupActivated()) );
    setPopup( m_popupMenu );
  }

  else {
    connect( this, TQT_SIGNAL(clicked()), action, TQT_SLOT(activate()) );
  }

  connect( action, TQT_SIGNAL(enabled(bool)), this, TQT_SLOT(setEnabled(bool)) );
}


K3bToolBoxButton::K3bToolBoxButton( const TQString& text, const TQString& icon, 
				    const TQString& tooltip, const TQString& whatsthis,
				    TQObject* receiver, const char* slot,
				    TQWidget* parent )
  : TQToolButton( parent ),
    m_popupMenu(0)
{
  setSizePolicy( TQSizePolicy(TQSizePolicy::Fixed, sizePolicy().verData()) );
  setAutoRaise( true );

  setTextLabel( text );

  if( icon.isEmpty() )
    setUsesTextLabel( true );
  else
    setIconSet( SmallIconSet( icon ) );

  TQWhatsThis::add( this, whatsthis );
  TQToolTip::add( this, tooltip );

  if( receiver && slot )
    connect( this, TQT_SIGNAL(clicked()), receiver, slot );
}


void K3bToolBoxButton::slotPopupActivated()
{
  // force the toolbutton to open the popupmenu instantly
  openPopup();
}


void K3bToolBoxButton::resizeEvent( TQResizeEvent* e )
{
  TQToolButton::resizeEvent( e );

  // force icon-only buttons to be square
  if( e->oldSize().height() != e->size().height() &&
      !usesTextLabel() )
    setFixedWidth( e->size().height() );
}







K3bToolBox::K3bToolBox( TQWidget* parent, const char* name )
  : TQFrame( parent, name )
{
  setSizePolicy( TQSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Fixed) );

  m_mainLayout = new TQGridLayout( this );
  m_mainLayout->setMargin( 1 );
  m_mainLayout->setSpacing( 0 );
}


K3bToolBox::~K3bToolBox()
{
  clear();
}


K3bToolBoxButton* K3bToolBox::addButton( KAction* action, bool forceText )
{
  if( action ) {
    K3bToolBoxButton* b = new K3bToolBoxButton( action, this );
    if( forceText ) {
      b->setUsesTextLabel( true );
      b->setTextPosition( TQToolButton::BesideIcon );
    }
    addWidget( b );
    return b;
  }
  else
    return 0;
}


K3bToolBoxButton* K3bToolBox::addButton( const TQString& text, const TQString& icon, 
					 const TQString& tooltip, const TQString& whatsthis,
					 TQObject* receiver, const char* slot,
					 bool forceText )
{
  K3bToolBoxButton* b = new K3bToolBoxButton( text, icon, tooltip, whatsthis, receiver, slot, this );
  if( forceText ) {
    b->setUsesTextLabel( true );
    b->setTextPosition( TQToolButton::BesideIcon );
  }
  addWidget( b );
  return b;
}


void K3bToolBox::addSpacing()
{
  int lastStretch = m_mainLayout->colStretch( m_mainLayout->numCols()-1 );
  m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 0 );
  m_mainLayout->addColSpacing( m_mainLayout->numCols()-1, 8 );
  m_mainLayout->setColStretch( m_mainLayout->numCols(), lastStretch );
}


void K3bToolBox::addSeparator()
{
  K3bToolBoxSeparator* s = new K3bToolBoxSeparator( this );
  addWidget( s );
}


void K3bToolBox::addStretch()
{
  // add an empty widget
  addWidget( new TQWidget( this ) );
  m_mainLayout->setColStretch( m_mainLayout->numCols(), 1 );
}


void K3bToolBox::addLabel( const TQString& text )
{
  TQLabel* label = new TQLabel( text, this );
  label->setSizePolicy( TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed) );

  addWidget( label );
}


void K3bToolBox::addWidget( TQWidget* w )
{
  w->reparent( this, TQPoint() );

  m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 0 );

  m_mainLayout->addWidget( w, 0, m_mainLayout->numCols()-1 );

  if( w->sizePolicy().horData() == TQSizePolicy::Fixed || w->sizePolicy().horData() == TQSizePolicy::Maximum )
    m_mainLayout->setColStretch( m_mainLayout->numCols(), 1 );
  else {
    m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 1 );
    m_mainLayout->setColStretch( m_mainLayout->numCols(), 0 );
  }
}


K3bToolBoxButton* K3bToolBox::addToggleButton( KToggleAction* action )
{
  return addButton( action );
}


void K3bToolBox::addWidgetAction( KWidgetAction* action )
{
  addWidget( action->widget() );
  m_doNotDeleteWidgets.append( action->widget() );
}


void K3bToolBox::clear()
{
  // we do not want to delete the widgets from the widgetactions becasue they
  // might be used afterwards
  for( TQPtrListIterator<TQWidget> it( m_doNotDeleteWidgets ); it.current(); ++it )
    it.current()->reparent( 0L, TQPoint() );

  for( TQObjectListIterator it2( childrenListObject() ); it2.current(); ++it2 )
    if( it2.current()->isWidgetType() )
      delete it2.current();
}

#include "k3btoolbox.moc"
