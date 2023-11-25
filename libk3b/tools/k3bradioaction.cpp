/* 
 *
 * $Id: k3bradioaction.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bradioaction.h"

#include <tdetoolbarbutton.h>

K3bRadioAction::K3bRadioAction( const TQString& text, const TDEShortcut& cut,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TDEShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQIconSet& pix,
				const TDEShortcut& cut,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, pix, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQString& pix,
				const TDEShortcut& cut,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, pix, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQIconSet& pix,
				const TDEShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, pix, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQString& pix,
				const TDEShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : TDEToggleAction( text, pix, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( TQObject* parent, const char* name )
  : TDEToggleAction( parent, name ),
    m_alwaysEmit(false)
{
}

void K3bRadioAction::slotActivated()
{
  if( isChecked() ) {
    if( m_alwaysEmit )
      emit activated();

    const TQObject *senderObj = sender();
    
    if ( !senderObj || !::tqt_cast<const TDEToolBarButton *>( senderObj ) )
      return;
    
    const_cast<TDEToolBarButton *>( static_cast<const TDEToolBarButton *>( senderObj ) )->on( true );
    
    return;
  }

  TDEToggleAction::slotActivated();
}

#include "k3bradioaction.moc"
