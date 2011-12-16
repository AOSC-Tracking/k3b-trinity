/* 
 *
 * $Id: k3bcdcontentsview.cpp 582796 2006-09-10 15:31:38Z trueg $
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

#include "k3bcontentsview.h"

#include <k3bthemedheader.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpixmap.h>


K3bContentsView::K3bContentsView( bool withHeader,
				  TQWidget* parent, 
				  const char* name )
  : TQWidget( parent, name ),
    m_header(0),
    m_centerWidget(0)
{
  if( withHeader ) {
    TQVBoxLayout* lay = new TQVBoxLayout( this );
    lay->setMargin( 2 );
    lay->setSpacing( 0 );

    m_header = new K3bThemedHeader( this );
    lay->addWidget( m_header );

    m_header->setLeftPixmap( K3bTheme::MEDIA_LEFT );
    m_header->setRightPixmap( K3bTheme::MEDIA_NONE );
  }
}


K3bContentsView::~K3bContentsView()
{
}


void K3bContentsView::setMainWidget( TQWidget* w )
{
  m_centerWidget = w;
  ((TQVBoxLayout*)tqlayout())->addWidget( w );
}


TQWidget* K3bContentsView::mainWidget()
{
  if( !m_centerWidget )
    setMainWidget( new TQWidget( this ) );
  return m_centerWidget;
}


void K3bContentsView::setTitle( const TQString& s )
{
  if( m_header )
    m_header->setTitle( s );
}


void K3bContentsView::setLeftPixmap( K3bTheme::PixmapType s )
{
  if( m_header )
    m_header->setLeftPixmap( s );
}


void K3bContentsView::setRightPixmap( K3bTheme::PixmapType s )
{
  if( m_header )
    m_header->setRightPixmap( s );
}

#include "k3bcontentsview.moc"
