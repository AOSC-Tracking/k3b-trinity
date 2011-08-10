/* 
 *
 * $Id: k3bwritingmodewidget.cpp 554512 2006-06-24 07:25:39Z trueg $
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

#include "k3bintmapcombobox.h"

#include <tqwhatsthis.h>
#include <tqmap.h>
#include <tqvaluevector.h>


class K3bIntMapComboBox::Private
{
public:
  TQMap<int, int> valueIndexMap;
  TQMap<int, TQPair<int, TQString> > indexValueDescriptionMap;

  TQString topWhatsThis;
  TQString bottomWhatsThis;
};


K3bIntMapComboBox::K3bIntMapComboBox( TQWidget* parent, const char* name )
  : KComboBox( parent, name )
{
  d = new Private;
  connect( this, TQT_SIGNAL(highlighted(int)),
	   this, TQT_SLOT(slotItemHighlighted(int)) );
  connect( this, TQT_SIGNAL(activated(int)),
	   this, TQT_SLOT(slotItemActivated(int)) );
}


K3bIntMapComboBox::~K3bIntMapComboBox()
{
  delete d;
}


int K3bIntMapComboBox::selectedValue() const
{
  if( (int)d->indexValueDescriptionMap.count() > KComboBox::currentItem() )
    return d->indexValueDescriptionMap[KComboBox::currentItem()].first;
  else
    return 0;
}


void K3bIntMapComboBox::setSelectedValue( int value )
{
  if( d->valueIndexMap.contains( value ) )
    KComboBox::setCurrentItem( d->valueIndexMap[value] );
}


void K3bIntMapComboBox::clear()
{
  d->valueIndexMap.clear();
  d->indexValueDescriptionMap.clear();

  KComboBox::clear();
}


bool K3bIntMapComboBox::insertItem( int value, const TQString& text, const TQString& description, int index )
{
  if( d->valueIndexMap.contains( value ) )
    return false;

  // FIXME: allow inserition at any index
  index = KComboBox::count();

  d->valueIndexMap[value] = index;
  d->indexValueDescriptionMap[index] = tqMakePair<int, TQString>( value, description );

  KComboBox::insertItem( text );

  updateWhatsThis();

  return true;
}


void K3bIntMapComboBox::updateWhatsThis()
{
  TQString ws( d->topWhatsThis );
  for( unsigned int i = 0; i < d->indexValueDescriptionMap.count(); ++i ) {
    ws += "<p><b>" + KComboBox::text( i ) + "</b><br>";
    ws += d->indexValueDescriptionMap[i].second;
  }  
  ws += "<p>" + d->bottomWhatsThis;

  TQWhatsThis::add( this, ws );
}


void K3bIntMapComboBox::slotItemHighlighted( int index )
{
  emit valueHighlighted( d->indexValueDescriptionMap[index].first );
}


void K3bIntMapComboBox::slotItemActivated( int index )
{
  emit valueChanged( d->indexValueDescriptionMap[index].first );
}


void K3bIntMapComboBox::addGlobalWhatsThisText( const TQString& top, const TQString& bottom )
{
  d->topWhatsThis = top;
  d->bottomWhatsThis = bottom;
  updateWhatsThis();
}

#include "k3bintmapcombobox.moc"
