/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#include "k3btoolbutton.h"

#include <tqstyle.h>
#include <tqpainter.h>
#include <tqevent.h>

#include <tdeglobalsettings.h>
#include <tdeapplication.h>


class K3bToolButton::Private
{
public:
  TQPoint mousePressPos;
  bool instantMenu;
};


K3bToolButton::K3bToolButton( TQWidget* parent )
  : TQToolButton( parent )
{
  d = new Private;
  d->instantMenu = false;
  installEventFilter(this);
}


K3bToolButton::~K3bToolButton()
{
  delete d;
}


void K3bToolButton::setInstantMenu( bool b )
{
  d->instantMenu = b;
}


void K3bToolButton::drawButton( TQPainter* p )
{
  TQToolButton::drawButton( p );

  //
  // code below comes from tdetoolbarbutton.cpp from the tdelibs sources
  // see the file for copyright information
  //
  if( TQToolButton::popup() ) {
    TQStyle::SFlags arrowFlags = TQStyle::Style_Default;
    
    if( isDown() )
      arrowFlags |= TQStyle::Style_Down;
    if( isEnabled() )
      arrowFlags |= TQStyle::Style_Enabled;
    
    style().drawPrimitive(TQStyle::PE_ArrowDown, p,
			  TQRect(width()-7, height()-7, 7, 7), colorGroup(),
			  arrowFlags, TQStyleOption() );
  }
}


bool K3bToolButton::eventFilter( TQObject* o, TQEvent* ev )
{
  if( dynamic_cast<K3bToolButton*>(o) == this ) {

    // Popup the menu when the left mousebutton is pressed and the mouse
    // is moved by a small distance.
    if( TQToolButton::popup() ) {
      if( ev->type() == TQEvent::MouseButtonPress ) {
	TQMouseEvent* mev = TQT_TQMOUSEEVENT(ev);

	if( d->instantMenu ) {
	  setDown(true);
	  openPopup();
	  return true;
	}
	else {
	  d->mousePressPos = mev->pos();
	}
      }
      else if( ev->type() == TQEvent::MouseMove ) {
        TQMouseEvent* mev = TQT_TQMOUSEEVENT(ev);
        if( !d->instantMenu &&
	    ( mev->pos() - d->mousePressPos).manhattanLength() > TDEGlobalSettings::dndEventDelay() ) {
	  openPopup();
          return true;
        }
      }
    }
  }

  return TQToolButton::eventFilter( o, ev );
}
