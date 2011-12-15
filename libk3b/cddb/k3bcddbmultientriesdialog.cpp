/* 
 *
 * $Id: k3bcddbmultientriesdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bcddbmultientriesdialog.h"

#include <layout.h>
#include <tqframe.h>
#include <tqlabel.h>

#include <klistbox.h>
#include <klocale.h>



K3bCddbMultiEntriesDialog::K3bCddbMultiEntriesDialog( TQWidget* parent, const char* name )
  : KDialogBase( Plain, i18n("CDDB Database Entry"), Ok|Cancel, Ok, parent, name ) 
{
  TQFrame* frame = plainPage();
  TQVBoxLayout* layout = new TQVBoxLayout( frame );
  layout->setAutoAdd( true );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );

  TQLabel* infoLabel = new TQLabel( i18n("K3b found multiple inexact CDDB entries. Please select one."), frame );
  infoLabel->setAlignment( WordBreak );

  m_listBox = new KListBox( frame, "list_box");

  setMinimumSize( 280, 200 );
}

K3bCddbResultHeader K3bCddbMultiEntriesDialog::selectCddbEntry( K3bCddbQuery* query, TQWidget* parent )
{
  K3bCddbMultiEntriesDialog d( parent );

  const TQValueList<K3bCddbResultHeader> headers = query->getInexactMatches();

  int i = 1;
  for( TQValueListConstIterator<K3bCddbResultHeader> it = headers.begin();
       it != headers.end(); ++it ) {
    d.m_listBox->insertItem( TQString::number(i) + " " + 
			     (*it).artist + " - " + 
			     (*it).title + " (" + 
			     (*it).category + ")" );
    ++i;
  }

  d.m_listBox->setSelected( 0, true );

  if( d.exec() == TQDialog::Accepted )
    return headers[ d.m_listBox->currentItem() >= 0 ? d.m_listBox->currentItem() : 0 ];
  else
    return K3bCddbResultHeader();
}


K3bCddbMultiEntriesDialog::~K3bCddbMultiEntriesDialog(){
}


#include "k3bcddbmultientriesdialog.moc"
