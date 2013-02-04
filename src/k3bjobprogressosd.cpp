/* 
 *
 * $Id: k3bjobprogressosd.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bjobprogressosd.h"

#include <k3bthememanager.h>
#include <k3bapplication.h>

#include <twin.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcursor.h>
#include <tdeconfig.h>
#include <klocale.h>
#include <tdepopupmenu.h>

#include <tqpixmap.h>
#include <tqpainter.h>
#include <tqapplication.h>

#include <X11/Xlib.h>


K3bJobProgressOSD::K3bJobProgressOSD( TQWidget* parent, const char* name )
  : TQWidget( parent, name, WType_TopLevel | WNoAutoErase | WStyle_Customize | WX11BypassWM | WStyle_StaysOnTop ),
    m_dirty(true),
    m_progress(0),
    m_dragging(false),
    m_screen(0),
    m_position(s_outerMargin, s_outerMargin)
{
  setFocusPolicy( TQ_NoFocus );
  setBackgroundMode( NoBackground );

  // dummy size
  resize( 20, 20 );

  // make sure we are always visible
  KWin::setOnAllDesktops( winId(), true );

  connect( k3bappcore->themeManager(), TQT_SIGNAL(themeChanged()),
	   this, TQT_SLOT(refresh()) );
  connect( kapp, TQT_SIGNAL(appearanceChanged()),
	   this, TQT_SLOT(refresh()) );
}


K3bJobProgressOSD::~K3bJobProgressOSD()
{
}


void K3bJobProgressOSD::show()
{
  // start with 0 progress
  setProgress(0);

  if( m_dirty )
    renderOSD();
  
  TQWidget::show();
}


void K3bJobProgressOSD::setText( const TQString& text )
{
  if( m_text != text ) {
    m_text = text;
    refresh();
  }
}


void K3bJobProgressOSD::setProgress( int p )
{
  if( m_progress != p ) {
    m_progress = p;
    refresh();
  }
}


void K3bJobProgressOSD::setPosition( const TQPoint& p )
{
  m_position = p;
  reposition();
}


void K3bJobProgressOSD::refresh()
{
  if( isVisible() )
    renderOSD();
  else
    m_dirty = true;
}


void K3bJobProgressOSD::renderOSD()
{
  // ----------------------------------------
  // |        Copying CD                    |
  // |  K3B   ========== 40%                |
  // |                                      |
  // ----------------------------------------

  // calculate needed size 
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    TQPixmap icon = TDEGlobal::iconLoader()->loadIcon( "k3b", TDEIcon::NoGroup, 32 );
    int margin = 10;
    int textWidth = fontMetrics().width( m_text );

    // do not change the size every time the text changes, just in case we are too small
    TQSize newSize( TQMAX( TQMAX( 2*margin + icon.width() + margin + textWidth, 100 ), width() ),
		   TQMAX( 2*margin + icon.height(), 2*margin + fontMetrics().height()*2 ) );
    
    m_osdBuffer.resize( newSize );
    TQPainter p( &m_osdBuffer );
    
    p.setPen( theme->foregroundColor() );
    
    // draw the background and the frame
    TQRect thisRect( 0, 0, newSize.width(), newSize.height() );
    p.fillRect( thisRect, theme->backgroundColor() );
    p.drawRect( thisRect );
    
    // draw the k3b icon
    p.drawPixmap( margin, (newSize.height()-icon.height())/2, icon );
    
    // draw the text
    TQSize textSize = fontMetrics().size( 0, m_text );
    int textX = 2*margin + icon.width();
    int textY = margin + fontMetrics().ascent();
    p.drawText( textX, textY, m_text );
    
    // draw the progress
    textY += fontMetrics().descent() + 4;
    TQRect progressRect( textX, textY, newSize.width()-textX-margin, newSize.height()-textY-margin );
    p.drawRect( progressRect );
    progressRect.setWidth( m_progress > 0 ? m_progress*progressRect.width()/100 : 0 );
    p.fillRect( progressRect, theme->foregroundColor() );

    // reposition the osd
    reposition( newSize );

    m_dirty = false;

    update();
  }
}


void K3bJobProgressOSD::setScreen( int screen )
{
  const int n = TQApplication::desktop()->numScreens();
  m_screen = (screen >= n) ? n-1 : screen;
  reposition();
}


void K3bJobProgressOSD::reposition( TQSize newSize )
{
  if( !newSize.isValid() )
    newSize = size();

  TQPoint newPos = m_position;
  const TQRect& screen = TQApplication::desktop()->screenGeometry( m_screen );

  // now to properly resize if put into one of the corners we interpret the position
  // depending on the quadrant
  int midH = screen.width()/2;
  int midV = screen.height()/2;
  if( newPos.x() > midH )
    newPos.rx() -= newSize.width();
  if( newPos.y() > midV )
    newPos.ry() -= newSize.height();

  newPos = fixupPosition( newPos );
 
  // correct for screen position
  newPos += screen.topLeft();
  
  // ensure we are painted before we move
  if( isVisible() )
    paintEvent( 0 );

  // fancy X11 move+resize, reduces visual artifacts
  XMoveResizeWindow( x11Display(), winId(), newPos.x(), newPos.y(), newSize.width(), newSize.height() );
}


void K3bJobProgressOSD::paintEvent( TQPaintEvent* )
{
  bitBlt( this, 0, 0, &m_osdBuffer );
}


void K3bJobProgressOSD::mousePressEvent( TQMouseEvent* e )
{
  m_dragOffset = e->pos();

  if( e->button() == Qt::LeftButton && !m_dragging ) {
    grabMouse( KCursor::sizeAllCursor() );
    m_dragging = true;
  }
  else if( e->button() == Qt::RightButton ) {
    TDEPopupMenu m;
    if( m.insertItem( i18n("Hide OSD") ) == m.exec( e->globalPos() ) )
      hide();
  }
}


void K3bJobProgressOSD::mouseReleaseEvent( TQMouseEvent* )
{
  if( m_dragging ) {
    m_dragging = false;
    releaseMouse();
  }
}


void K3bJobProgressOSD::mouseMoveEvent( TQMouseEvent* e )
{
  if( m_dragging && this == mouseGrabber() ) {

    // check if the osd has been dragged out of the current screen
    int currentScreen = TQApplication::desktop()->screenNumber( e->globalPos() );
    if( currentScreen != -1 )
      m_screen = currentScreen;

    const TQRect& screen = TQApplication::desktop()->screenGeometry( m_screen );
    
    // make sure the position is valid
    m_position = fixupPosition( e->globalPos() - m_dragOffset - screen.topLeft() );

    // move us to the new position
    move( m_position );

    // fix the position
    int midH = screen.width()/2;
    int midV = screen.height()/2;
    if( m_position.x() + width() > midH )
      m_position.rx() += width();
    if( m_position.y() + height() > midV )
      m_position.ry() += height();
  }
}


TQPoint K3bJobProgressOSD::fixupPosition( const TQPoint& pp )
{
  TQPoint p(pp);
  const TQRect& screen = TQApplication::desktop()->screenGeometry( m_screen );
  int maxY = screen.height() - height() - s_outerMargin;
  int maxX = screen.width() - width() - s_outerMargin;

  if( p.y() < s_outerMargin )
    p.ry() = s_outerMargin;
  else if( p.y() > maxY )
    p.ry() = maxY;

  if( p.x() < s_outerMargin )
    p.rx() = s_outerMargin;
  else if( p.x() > maxX )
    p.rx() = screen.width() - s_outerMargin - width();

  p += screen.topLeft();

  return p;
}


void K3bJobProgressOSD::readSettings( TDEConfigBase* c )
{
  TDEConfigGroup grp( c, "OSD Position" );

  setPosition( grp.readPointEntry( "Position", 0 ) );
  setScreen( grp.readNumEntry( "Screen", 0 ) );
}


void K3bJobProgressOSD::saveSettings( TDEConfigBase* c )
{
  TDEConfigGroup grp( c, "OSD Position" );

  grp.writeEntry( "Position", m_position );
  grp.writeEntry( "Screen", m_screen );
}

#include "k3bjobprogressosd.moc"
