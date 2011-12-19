/* 
 *
 * $Id: kcutlabel.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "kcutlabel.h"

#include <k3bstringutils.h>

#include <tqtooltip.h>
#include <tqstringlist.h>
#include <kdebug.h>


KCutLabel::KCutLabel( const TQString &text , TQWidget *parent, const char *name )
 : TQLabel ( parent, name ),
   m_minChars(1) {
  TQSizePolicy myLabelSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed );
  setSizePolicy(myLabelSizePolicy);
  m_fullText = text;
  cutTextToLabel();
}

KCutLabel::KCutLabel( TQWidget *parent, const char *name )
 : TQLabel ( parent, name ),
   m_minChars(1) {
  TQSizePolicy myLabelSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed );
  setSizePolicy(myLabelSizePolicy);
}

TQSize KCutLabel::minimumSizeHint() const
{
  TQSize sh = TQLabel::minimumSizeHint();
  if( m_minChars == 0 )
    sh.setWidth(-1);
  else if( m_minChars < (int)m_fullText.length() )
    sh.setWidth( TQMIN( fontMetrics().width( m_fullText.left(m_minChars) + "..." ), 
		       fontMetrics().width( m_fullText ) ) );

  return sh;
}


void KCutLabel::setMinimumVisibleText( int i )
{
  m_minChars = i;
  cutTextToLabel();
}


void KCutLabel::resizeEvent( TQResizeEvent * )
{
  cutTextToLabel();
}

void KCutLabel::setText( const TQString &text )
{
  m_fullText = text;
  cutTextToLabel();
}


const TQString& KCutLabel::fullText() const
{
  return m_fullText;
}


void KCutLabel::cutTextToLabel()
{
  TQToolTip::remove( this );
  TQToolTip::hide();

  if( m_fullText.contains( "\n" ) ) {
    TQString newText;
    TQStringList lines = TQStringList::split( "\n", m_fullText );
    for( TQStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
      TQString squeezedText = K3b::cutToWidth( fontMetrics(), 
					      *it, 
					      TQMAX( size().width(), 
						    TQMIN( fontMetrics().width( m_fullText.left(m_minChars) + "..." ), 
							  fontMetrics().width( m_fullText ) ) ) );
      newText += squeezedText;
      newText += "\n";
      if( squeezedText != *it )
	TQToolTip::add( this, m_fullText );
    }
    newText.truncate( newText.length() - 1 ); // get rid of the last newline

    TQLabel::setText( newText );
  }
  else {
    TQString squeezedText = K3b::cutToWidth( fontMetrics(), 
					    m_fullText, 
					    TQMAX( size().width(), 
						  TQMIN( fontMetrics().width( m_fullText.left(m_minChars) + "..." ), 
							fontMetrics().width( m_fullText ) ) ) );
    TQLabel::setText( squeezedText );
    if( squeezedText != m_fullText )
      TQToolTip::add( this, m_fullText );      
  }
}

#include "kcutlabel.moc"
