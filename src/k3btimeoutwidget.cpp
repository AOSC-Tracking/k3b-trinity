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

#include "k3btimeoutwidget.h"
#include "k3bthememanager.h"
#include "k3bapplication.h"

#include <kiconloader.h>

#include <tqpainter.h>
#include <tqtimer.h>
#include <tqbitmap.h>


class K3bTimeoutWidget::Private
{
public:
  int timeout;
  int margin;

  TQTimer timer;
  int currentTime;
};


K3bTimeoutWidget::K3bTimeoutWidget( TQWidget* parent )
  : TQWidget( parent )
{
  d = new Private;
  d->timeout = 10000;
  d->margin = 4;
  connect( &d->timer, TQT_SIGNAL(timeout()), this, TQT_SLOT(timeStep()) );
}


K3bTimeoutWidget::~K3bTimeoutWidget()
{
  delete d;
}


void K3bTimeoutWidget::start()
{
  d->currentTime = 0;
  startTimer();
}


void K3bTimeoutWidget::stop()
{
  d->timer.stop();
  d->currentTime = 0;
}


void K3bTimeoutWidget::pause()
{
  d->timer.stop();
}


void K3bTimeoutWidget::resume()
{
  startTimer();
}


void K3bTimeoutWidget::timeStep()
{
  d->currentTime += 100;
  update();
  if( d->currentTime >= d->timeout ) {
    stop();
    emit timeout();
  }
}


TQSize K3bTimeoutWidget::sizeHint() const
{
  return minimumSizeHint();
}


TQSize K3bTimeoutWidget::minimumSizeHint() const
{
  int fw = fontMetrics().width( TQString::number( d->timeout/1000 ) );
  int fh = fontMetrics().height();

  int diam = TQMAX( fw, fh ) + 2*d->margin;

  return TQSize( diam, diam );
}


void K3bTimeoutWidget::setTimeout( int msecs )
{
  d->timeout = msecs;
}


void K3bTimeoutWidget::startTimer()
{
  d->timer.start( 100 );
}


void K3bTimeoutWidget::paintEvent( TQPaintEvent* )
{
  if( d->timer.isActive() ) {
    TQPainter p(this);
    
    if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
      p.setBrush( theme->backgroundColor() );
      p.setPen( theme->backgroundColor() );
    }

    TQRect r;
    r.setSize( minimumSizeHint() );
    r.moveCenter( rect().center() );

    p.drawArc( r, 0, 360*16 );    
    p.drawPie( r, 90*16, 360*16*d->currentTime/d->timeout );
    
    p.setPen( TQt::black );
    p.drawText( rect(), TQt::AlignCenter, TQString::number( (d->timeout - d->currentTime + 500)/1000 ) );
  }
}


void K3bTimeoutWidget::resizeEvent( TQResizeEvent* e )
{
  TQWidget::resizeEvent( e );
}


#include "k3btimeoutwidget.moc"
