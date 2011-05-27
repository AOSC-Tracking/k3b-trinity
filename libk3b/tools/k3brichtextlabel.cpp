/* 
 *
 * $Id: k3brichtextlabel.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Waldo Bastian <bastian@kde.org>
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

#include "k3brichtextlabel.h"

#include <tqtooltip.h>
#include <tqstylesheet.h>
#include <tqsimplerichtext.h>

#include <kglobalsettings.h>

static TQString qrichtextify( const TQString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  TQStringList lines = TQStringList::split('\n', text);
  for(TQStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = TQStyleSheet::convertFromPlainText( *it, TQStyleSheetItem::WhiteSpaceNormal );
  }

  return lines.join(TQString());
}

K3bRichTextLabel::K3bRichTextLabel( const TQString &text , TQWidget *tqparent, const char *name )
 : TQLabel ( tqparent, name ) {
  m_defaultWidth = TQMIN(400, KGlobalSettings::desktopGeometry(this).width()*2/5);
  tqsetAlignment( TQt::WordBreak );
  setText(text);
}

K3bRichTextLabel::K3bRichTextLabel( TQWidget *tqparent, const char *name )
 : TQLabel ( tqparent, name ) {
  m_defaultWidth = TQMIN(400, KGlobalSettings::desktopGeometry(this).width()*2/5);
  tqsetAlignment( TQt::WordBreak );
}

void K3bRichTextLabel::setDefaultWidth(int defaultWidth)
{
  m_defaultWidth = defaultWidth;
  updateGeometry();
}

TQSizePolicy K3bRichTextLabel::sizePolicy() const
{
  return TQSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::Minimum, false);
}

TQSize K3bRichTextLabel::tqminimumSizeHint() const
{
  TQString qt_text = qrichtextify( text() );
  int pref_width = 0;
  int pref_height = 0;
  TQSimpleRichText rt(qt_text, font());
  pref_width = m_defaultWidth;
  rt.setWidth(pref_width);
  int used_width = rt.widthUsed();
  if (used_width <= pref_width)
  {
    while(true)
    {
      int new_width = (used_width * 9) / 10;
      rt.setWidth(new_width);
      int new_height = rt.height();
      if (new_height > pref_height)
        break;
      used_width = rt.widthUsed();
      if (used_width > new_width)
        break;
    }
    pref_width = used_width;
  }
  else
  {
    if (used_width > (pref_width *2))
      pref_width = pref_width *2;
    else
      pref_width = used_width;
  }

  return TQSize(pref_width, rt.height());
}

TQSize K3bRichTextLabel::tqsizeHint() const
{
  return tqminimumSizeHint();
}

void K3bRichTextLabel::setText( const TQString &text ) {
  TQLabel::setText(text);
}

void K3bRichTextLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "k3brichtextlabel.moc"
