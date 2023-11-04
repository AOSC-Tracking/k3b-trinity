/*
 *
 * $Id: k3bflatbutton.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bflatbutton.h"
#include "k3bthememanager.h"
#include "k3bapplication.h"

#include <tdeaction.h>
#include <kiconloader.h>
#include <tdeglobal.h>

#include <tqpainter.h>
#include <tqtooltip.h>
#include <tqfontmetrics.h>
#include <tqpixmap.h>


K3bFlatButton::K3bFlatButton( TQWidget *parent, const char *name )
  : TQFrame( parent, name/*, WNoAutoErase*/ ),
    m_pressed(false)
{
  init();
}


K3bFlatButton::K3bFlatButton( const TQString& text, TQWidget *parent, const char *name )
  : TQFrame( parent, name/*, WNoAutoErase*/ ),
    m_pressed(false)
{
  init();
  setText( text );
}


K3bFlatButton::K3bFlatButton( TDEAction* a, TQWidget *parent, const char *name )
  : TQFrame( parent, name/*, WNoAutoErase*/ ),
    m_pressed(false)
{
  init();

  setText( a->text() );
  TQToolTip::add( this, a->toolTip() );
  setPixmap( TDEGlobal::iconLoader()->loadIcon( a->icon(), TDEIcon::NoGroup, 32 ) );
  connect( this, TQT_SIGNAL(clicked()), a, TQT_SLOT(activate()) );
}


K3bFlatButton::~K3bFlatButton() {}


void K3bFlatButton::init()
{
  setHover(false);
  setMargin(5);
  setFrameStyle( TQFrame::Box|TQFrame::Plain );

  connect( k3bappcore->themeManager(), TQT_SIGNAL(themeChanged()), this, TQT_SLOT(slotThemeChanged()) );
  connect( kapp, TQT_SIGNAL(appearanceChanged()), this, TQT_SLOT(slotThemeChanged()) );
  slotThemeChanged();
}


void K3bFlatButton::setText( const TQString& s )
{
  m_text = s;
  m_text.remove( '&' );

  update();
}


void K3bFlatButton::setPixmap( const TQPixmap& p )
{
  m_pixmap = p;
  update();
}


void K3bFlatButton::enterEvent( TQEvent* )
{
  setHover(true);
}


void K3bFlatButton::leaveEvent( TQEvent* )
{
  setHover(false);
}


void K3bFlatButton::mousePressEvent( TQMouseEvent* e )
{
  if( e->button() == TQt::LeftButton ) {
    emit pressed();
    m_pressed = true;
  }
  else
    e->ignore();
}


void K3bFlatButton::mouseReleaseEvent( TQMouseEvent* e )
{
  if( e->button() == TQt::LeftButton ) {
    if( m_pressed  )
      emit clicked();
    m_pressed = false;
  }
  else
    e->ignore();
}


void K3bFlatButton::setHover( bool b )
{
  if( b ) {
    setPaletteBackgroundColor( m_foreColor );
    setPaletteForegroundColor( m_backColor );
  } else {
    setPaletteBackgroundColor( m_backColor );
    setPaletteForegroundColor( m_foreColor );
  }

  m_hover = b;

  update();
}


TQSize K3bFlatButton::sizeHint() const
{
  // height: pixmap + 5 spacing + font height + frame width
  // width: max( pixmap, text) + frame width
  return TQSize( TQMAX( m_pixmap.width(), fontMetrics().width( m_text ) ) + frameWidth()*2, 
		m_pixmap.height() + fontMetrics().height() + 5 + frameWidth()*2 );
}


void K3bFlatButton::drawContents( TQPainter* p )
{
  TQRect rect = contentsRect();

//   if( m_hover )
//     p->fillRect( rect, m_foreColor );
//   else if( parentWidget() ) {
//     TQRect r( mapToParent( TQPoint(lineWidth(), lineWidth()) ), 
// 	     mapToParent( TQPoint(width()-2*lineWidth(), height()-2*lineWidth() )) );
    
//     parentWidget()->repaint( r );
//   }

  p->save();

  TQRect textRect = fontMetrics().boundingRect( m_text );
  int textX = TQMAX( 0, ( rect.width() - textRect.width() ) / 2 );
  int textY = textRect.height();

  if( !m_pixmap.isNull() ) {
    p->translate( rect.left(), rect.top() );
    textX = TQMAX( textX, (m_pixmap.width() - textRect.width()) / 2 );
    textY += 5 + m_pixmap.height();

    int pixX = TQMAX( TQMAX( 0, (textRect.width() - m_pixmap.width()) / 2 ), 
		     ( rect.width() - m_pixmap.width() ) / 2 );
    p->drawPixmap( pixX, 0, m_pixmap );
    p->drawText( textX, textY, m_text ); 
  }
  else
    p->drawText( rect, TQt::AlignCenter, m_text );

  p->restore();
}


void K3bFlatButton::setColors( const TQColor& fore, const TQColor& back )
{
  m_foreColor = fore;
  m_backColor = back;

  setHover( m_hover );
}


void K3bFlatButton::slotThemeChanged()
{
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    setColors( theme->foregroundColor(), theme->backgroundColor() );
  }
}

#include "k3bflatbutton.moc"
