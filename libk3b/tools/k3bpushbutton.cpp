/* 
 *
 * $Id: k3bpushbutton.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bpushbutton.h"

#include <tqtimer.h>
#include <tqpopupmenu.h>
#include <tqevent.h>

#include <tdeglobalsettings.h>
#include <tdeapplication.h>



class K3bPushButton::Private
{
public:
  Private() 
    : popupTimer(0) {
  }

  TQTimer* popupTimer;
  TQPoint mousePressPos;
};



K3bPushButton::K3bPushButton( TQWidget* parent, const char* name )
  : KPushButton( parent, name )
{
  d = new Private();
  installEventFilter(this);
}


K3bPushButton::K3bPushButton( const TQString& text, TQWidget* parent, const char* name )
  : KPushButton( text, parent, name )
{
  d = new Private();
  installEventFilter(this);
}


K3bPushButton::K3bPushButton( const TQIconSet& icon, const TQString& text,
			      TQWidget* parent, const char* name )
  : KPushButton( icon, text, parent, name )
{
  d = new Private();
  installEventFilter(this);
}


K3bPushButton::K3bPushButton( const KGuiItem& item, TQWidget* parent, const char* name )
  : KPushButton( item, parent, name )
{
  d = new Private();
  installEventFilter(this);
}


K3bPushButton::~K3bPushButton()
{
  delete d;
}


void K3bPushButton::setDelayedPopupMenu( TQPopupMenu* popup )
{
  if( !d->popupTimer ) {
    d->popupTimer = new TQTimer( this );
    connect( d->popupTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotDelayedPopup()) );
  }

  setPopup( popup );

  // we need to do the popup handling ourselves so we cheat a little
  // TQPushButton connects a popup slot to the pressed signal which we disconnect here
  disconnect( this );
}


bool K3bPushButton::eventFilter( TQObject* o, TQEvent* ev )
{
  if( dynamic_cast<K3bPushButton*>(o) == this ) {

    // Popup the menu when the left mousebutton is pressed and the mouse
    // is moved by a small distance.
    if( popup() ) {
      if( ev->type() == TQEvent::MouseButtonPress ) {
        TQMouseEvent* mev = static_cast<TQMouseEvent*>(ev);
        d->mousePressPos = mev->pos();
	d->popupTimer->start( TQApplication::startDragTime() );
      }
      else if( ev->type() == TQEvent::MouseMove ) {
        TQMouseEvent* mev = static_cast<TQMouseEvent*>(ev);
        if( ( mev->pos() - d->mousePressPos).manhattanLength() > TDEGlobalSettings::dndEventDelay() ) {
	  d->popupTimer->stop();
	  slotDelayedPopup();
          return true;
        }
      }
    }
  }

  return KPushButton::eventFilter( o, ev );
}


void K3bPushButton::slotDelayedPopup()
{
  d->popupTimer->stop();

  if( isDown() ) {
    // popup the menu.
    // this has been taken from the TQPushButton code
    if( mapToGlobal( TQPoint( 0, rect().bottom() ) ).y() + popup()->sizeHint().height() <= tqApp->desktop()->height() )
      popup()->exec( mapToGlobal( rect().bottomLeft() ) );
    else
      popup()->exec( mapToGlobal( rect().topLeft() - TQPoint( 0, popup()->sizeHint().height() ) ) );
    setDown( false );
  }
}

#include "k3bpushbutton.moc"
