/* 
 *
 * $Id: k3bcutcombobox.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bcutcombobox.h"

#include <k3bstringutils.h>

#include <tqfontmetrics.h>
#include <tqevent.h>
#include <tqstringlist.h>
#include <tqrect.h>
#include <tqsize.h>
#include <tqpixmap.h>
#include <tqstyle.h>
#include <tqsizepolicy.h>


class K3bCutComboBox::Private
{
public:
  Private() {
    method = CUT;
  }

  TQStringList originalItems;

  int method;
  int width;
};


K3bCutComboBox::K3bCutComboBox( TQWidget* parent, const char* name )
  : KComboBox( parent, name )
{
  d = new Private();
  //  setSizePolicy( TQSizePolicy::Maximum, sizePolicy().horData(), sizePolicy().hasHeightForWidth() );
}


K3bCutComboBox::K3bCutComboBox( int method, TQWidget* parent, const char* name )
  : KComboBox( parent, name )
{
  d = new Private();
  d->method = method;
}


K3bCutComboBox::~K3bCutComboBox()
{
  delete d;
}


void K3bCutComboBox::setMethod( int m )
{
  d->method = m;
  cutText();
}


TQSize K3bCutComboBox::sizeHint() const
{
//   TQSize s(KComboBox::sizeHint());

//   for( int i = 0; i < count(); i++ ) {
//     int w = fontMetrics().width(d->originalItems[i]) + 
//       ( d->pixmaps[i].isNull() ? 0 : d->pixmaps[i].width() + 4);
//     if( w > s.width() )
//       s.setWidth( w );
//   }

  return KComboBox::sizeHint();
}

TQSize K3bCutComboBox::minimumSizeHint() const
{
  return KComboBox::minimumSizeHint();
}


void K3bCutComboBox::setCurrentText( const TQString& s )
{
  int i;
  for( i = 0; i < count(); i++ )
    if ( d->originalItems[i] == s )
      break;
  if ( i < count() ) {
    setCurrentItem(i);
  }
  else if( !d->originalItems.isEmpty() ) {
    d->originalItems[currentItem()] = s;
    cutText();
  }
}


void K3bCutComboBox::insertStringList( const TQStringList&, int )
{
  // FIXME
}


void K3bCutComboBox::insertStrList( const TQStrList&, int )
{
  // FIXME
}

void K3bCutComboBox::insertStrList( const TQStrList*, int )
{
  // FIXME
}

void K3bCutComboBox::insertStrList( const char**, int, int)
{
  // FIXME
}

void K3bCutComboBox::insertItem( const TQString& text, int index )
{
  insertItem( TQPixmap(), text, index );
}

void K3bCutComboBox::insertItem( const TQPixmap& pix, int i )
{
  insertItem( pix, "", i );
}

void K3bCutComboBox::insertItem( const TQPixmap& pixmap, const TQString& text, int index )
{
  if( index != -1 )
    d->originalItems.insert( d->originalItems.at(index), text );
  else
    d->originalItems.append( text );

  if( !pixmap.isNull() )
    KComboBox::insertItem( pixmap, "xx", index );
  else
    KComboBox::insertItem( "xx", index );

  cutText();
}

void K3bCutComboBox::removeItem( int i )
{
  d->originalItems.erase( d->originalItems.at(i) );
  KComboBox::removeItem( i );
}

void K3bCutComboBox::changeItem( const TQString& s, int i )
{
  d->originalItems[i] = s;
  cutText();
}

void K3bCutComboBox::changeItem( const TQPixmap& pix, const TQString& s, int i )
{
  KComboBox::changeItem( pix, i );
  changeItem( s, i );
}


TQString K3bCutComboBox::text( int i ) const
{
  if( i < (int)d->originalItems.count() )
    return d->originalItems[i];
  else
    return TQString();
}


TQString K3bCutComboBox::currentText() const
{
  if( currentItem() < (int)d->originalItems.count() )
    return d->originalItems[currentItem()];
  else
    return TQString();
}


void K3bCutComboBox::clear()
{
  KComboBox::clear();
  d->originalItems.clear();
}

void K3bCutComboBox::resizeEvent( TQResizeEvent* e )
{
  cutText();

  KComboBox::resizeEvent(e);
}


void K3bCutComboBox::cutText()
{
  d->width = TQStyle::visualRect( tqstyle().querySubControlMetrics(TQStyle::CC_ComboBox, this,
								TQStyle::SC_ComboBoxEditField), this ).width();

  for( int i = 0; i < (int)d->originalItems.count(); ++i ) {
    int w = d->width;
    if ( pixmap(i) && !pixmap(i)->isNull() )
      w -= ( pixmap(i)->width() + 4 );

    TQString text;
    if( d->method == STQUEEZE )
      text = K3b::squeezeTextToWidth( fontMetrics(), d->originalItems[i], w );
    else
      text = K3b::cutToWidth( fontMetrics(), d->originalItems[i], w );

    // now insert the cut text
    if( pixmap(i) )
      KComboBox::changeItem( *pixmap(i), text, i );
    else
      KComboBox::changeItem( text, i );
  }
}

#include "k3bcutcombobox.moc"
