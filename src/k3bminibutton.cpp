/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * K3bMiniButton is based on KDockButton_Private
 * Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
 * Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>
 * Copyright (C) 2005 Dominik Haumann <dhdev@gmx.de> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bminibutton.h"

#include <tqpainter.h>


K3bMiniButton::K3bMiniButton( TQWidget *parent, const char * name )
  :TQPushButton( parent, name ),
   m_mouseOver( false )
{
  setFocusPolicy( TQ_NoFocus );
}

K3bMiniButton::~K3bMiniButton()
{
}


void K3bMiniButton::drawButton( TQPainter* p )
{
  p->fillRect( 0,0, width(), height(), TQBrush(tqcolorGroup().brush(TQColorGroup::Background)) );
  p->drawPixmap( (width() - pixmap()->width()) / 2, (height() - pixmap()->height()) / 2, *pixmap() );
  if( m_mouseOver && !isDown() ){
    p->setPen( white );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( tqcolorGroup().dark() );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
  if( isOn() || isDown() ){
    p->setPen( tqcolorGroup().dark() );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( white );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
}

void K3bMiniButton::enterEvent( TQEvent * )
{
  m_mouseOver = true;
  tqrepaint();
}


void K3bMiniButton::leaveEvent( TQEvent * )
{
  m_mouseOver = false;
  tqrepaint();
}

#include "k3bminibutton.moc"
