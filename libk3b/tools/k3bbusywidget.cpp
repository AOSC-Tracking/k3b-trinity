/* 
 *
 * $Id: k3bbusywidget.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bbusywidget.h"

#include <tqtimer.h>
#include <tqpainter.h>

#include <tdeglobalsettings.h>


K3bBusyWidget::K3bBusyWidget( TQWidget* parent, const char* name )
  : TQFrame( parent, name )
{
  m_busyTimer = new TQTimer( this );
  m_iBusyPosition = 0;

  connect( m_busyTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(animateBusy()) );

  m_bBusy = false;
}

K3bBusyWidget::~K3bBusyWidget()
{
}


void K3bBusyWidget::showBusy( bool b )
{
  m_bBusy = b;

//   if( b ) {
//     m_iBusyCounter++;
//   }
//   else if( m_iBusyCounter > 0 ) {
//     m_iBusyCounter--;
//   }

  if( m_bBusy ) {
    if( !m_busyTimer->isActive() )
      m_busyTimer->start( 500 );
  }
  else {
    if( m_busyTimer->isActive() )
      m_busyTimer->stop();
    update();
    m_iBusyPosition = 0;
  }
}


void K3bBusyWidget::animateBusy()
{
  m_iBusyPosition++;
  update();
}


TQSize K3bBusyWidget::sizeHint() const
{
  return minimumSizeHint();
}


TQSize K3bBusyWidget::minimumSizeHint() const
{
  return TQSize( 2*frameWidth() + 62, 10 );
}


void K3bBusyWidget::drawContents( TQPainter* p )
{
  TQRect rect = contentsRect();

  int squareSize = 8;

  int pos = 2 + m_iBusyPosition * (squareSize + 2);

  // check if the position is in the visible area
  if( pos + 8 + 2> rect.width() ) {
    m_iBusyPosition = 0;
    pos = 2;
  }

  //  p->eraseRect( rect );
  if( m_bBusy )
    p->fillRect( pos, (rect.height() - squareSize)/2, squareSize, squareSize, TDEGlobalSettings::highlightColor() );
}


#include "k3bbusywidget.moc"
