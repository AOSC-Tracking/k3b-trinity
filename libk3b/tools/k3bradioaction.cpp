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

#include <ktoolbarbutton.h>

K3bRadioAction::K3bRadioAction( const TQString& text, const KShortcut& cut,
				TQObject* parent, const char* name )
  : KToggleAction( text, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const KShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : KToggleAction( text, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQIconSet& pix,
				const KShortcut& cut,
				TQObject* parent, const char* name )
  : KToggleAction( text, pix, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQString& pix,
				const KShortcut& cut,
				TQObject* parent, const char* name )
  : KToggleAction( text, pix, cut, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQIconSet& pix,
				const KShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : KToggleAction( text, pix, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( const TQString& text, const TQString& pix,
				const KShortcut& cut,
				const TQObject* receiver, const char* slot,
				TQObject* parent, const char* name )
  : KToggleAction( text, pix, cut, receiver, slot, parent, name ),
    m_alwaysEmit(false)
{
}

K3bRadioAction::K3bRadioAction( TQObject* parent, const char* name )
  : KToggleAction( parent, name ),
    m_alwaysEmit(false)
{
}

void K3bRadioAction::slotActivated()
{
  if( isChecked() ) {
    if( m_alwaysEmit )
      emit activated();

    const TQObject *senderObj = TQT_TQOBJECT_CONST(const_cast<const TQT_BASE_OBJECT_NAME*>(sender()));
    
    if ( !senderObj || !::tqqt_cast<const KToolBarButton *>( senderObj ) )
      return;
    
    const_cast<KToolBarButton *>( static_cast<const KToolBarButton *>( TQT_TQWIDGET_CONST(senderObj) ) )->on( true );
    
    return;
  }

  KToggleAction::slotActivated();
}

#include "k3bradioaction.moc"
