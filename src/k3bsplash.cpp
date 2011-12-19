/* 
 *
 * $Id: k3bsplash.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bsplash.h"

#include <k3bthememanager.h>
#include <k3bapplication.h>

#include <tqapplication.h>
#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqevent.h>
#include <tqstring.h>
#include <tqfontmetrics.h>
#include <tqpainter.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kaboutdata.h>


K3bSplash::K3bSplash( TQWidget* parent, const char* name )
  : TQVBox( parent, name, 
	   WStyle_Customize|
	   WDestructiveClose|
	   /*	   WStyle_Splash|*/
	   WX11BypassWM|
	   WStyle_NoBorder|
	   WStyle_StaysOnTop )
{
  setMargin( 0 );
  setSpacing( 0 );

  TQLabel* copyrightLabel = new TQLabel( kapp->aboutData()->copyrightStatement(), this );
  copyrightLabel->setMargin( 5 );
  copyrightLabel->setPaletteBackgroundColor( black );
  copyrightLabel->setPaletteForegroundColor( white );
  copyrightLabel->setAlignment( AlignRight );

  TQLabel* picLabel = new TQLabel( this );
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    picLabel->setPaletteBackgroundColor( theme->backgroundColor() );
    picLabel->setPixmap( theme->pixmap( K3bTheme::SPLASH ) );
  }

  m_infoBox = new TQLabel( this );
  m_infoBox->setMargin( 5 );
  m_infoBox->setPaletteBackgroundColor( black );
  m_infoBox->setPaletteForegroundColor( white );

  // Set tqgeometry, with support for Xinerama systems
  TQRect r;
  r.setSize(sizeHint());
  int ps = TQApplication::desktop()->primaryScreen();
  r.moveCenter( TQApplication::desktop()->screenGeometry(ps).center() );
  setGeometry(r);
}


K3bSplash::~K3bSplash()
{
}


void K3bSplash::mousePressEvent( TQMouseEvent* )
{
  close();
}


void K3bSplash::show()
{
  TQVBox::show();
  // make sure the splash screen is shown immediately
  tqApp->processEvents();
}


void K3bSplash::addInfo( const TQString& s )
{
  m_infoBox->setText( s );

  tqApp->processEvents();
}


// void K3bSplash::paintEvent( TQPaintEvent* e )
// {
//   // first let the window paint the background and the child widget
//   TQWidget::paintEvent( e );

//   // now create the text we want to display
//   // find the lower left corner and paint it on top of the pixmap
//   TQPainter p( this );
//   p.setPen( TQt::blue );

//   TQFontMetrics fm = p.fontMetrics();

//   TQString line1 = TQString( "K3b version %1" ).tqarg(VERSION);
//   TQString line2( "(c) 2001 by Sebastian Trueg" );
//   TQString line3( "licenced under the GPL" );

//   TQRect rect1 = fm.boundingRect( line1 );
//   TQRect rect2 = fm.boundingRect( line2 );
//   TQRect rect3 = fm.boundingRect( line3 );

//   int textH = rect1.height() + rect2.height() + rect3.height() + 2 * fm.leading() + 2 + rect2.height() /*hack because the boundingRect method seems not to work properly! :-(*/;
//   int textW = TQMAX( rect1.width(), TQMAX( rect2.width(), rect3.width() ) ) + 2;

//   int startX = 10;
//   int startY = height() - 10 - textH;

//   p.drawText( startX, startY, textW, textH, 0, TQString("%1\n%2\n%3").tqarg(line1).tqarg(line2).tqarg(line3) );
// }


#include "k3bsplash.moc"
