/* 
 *
 * $Id: k3bintvalidator.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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


#include <tqwidget.h>
#include <tqstring.h>

#include "k3bintvalidator.h"

#include <tdelocale.h>
#include <tdeglobal.h>
#include <kdebug.h>


K3bIntValidator::K3bIntValidator ( TQWidget * parent, const char * name )
  : TQValidator(parent, name)
{
  m_min = m_max = 0;
}


K3bIntValidator::K3bIntValidator ( int bottom, int top, TQWidget * parent, const char * name )
  : TQValidator(parent, name)
{
  m_min = bottom;
  m_max = top;
}


K3bIntValidator::~K3bIntValidator ()
{
}


TQValidator::State K3bIntValidator::validate ( TQString &str, int & ) const
{
  bool ok;
  int  val = 0;
  TQString newStr;

  newStr = str.stripWhiteSpace();
  newStr = newStr.upper();

  if( newStr.length() ) {
    // check for < 0
    bool minus = newStr.startsWith( "-" );
    if( minus )
      newStr.remove( 0, 1 );

    // check for hex
    bool hex = newStr.startsWith( "0X" );

    if( hex )
      newStr.remove( 0, 2 );

    // a special case
    if( newStr.isEmpty() ) {
      if( minus && m_min && m_min >= 0)
	ok = false;
      else
	return TQValidator::Acceptable;
    }

    val = newStr.toInt( &ok, hex ? 16 : 10 );
    if( minus )
      val *= -1;
  }
  else {
    val = 0;
    ok = true;
  }

  if( !ok )
    return TQValidator::Invalid;

  if( m_min && val > 0 && val < m_min )
    return TQValidator::Acceptable;

  if( m_max && val < 0 && val > m_max )
    return TQValidator::Acceptable;

  if( (m_max && val > m_max) || (m_min && val < m_min) )
    return TQValidator::Invalid;

  return TQValidator::Valid;
}


void K3bIntValidator::fixup ( TQString& ) const
{
  // TODO: remove preceding zeros
}


void K3bIntValidator::setRange ( int bottom, int top )
{
  m_min = bottom;
  m_max = top;

  if( m_max < m_min )
    m_max = m_min;
}


int K3bIntValidator::bottom () const
{
  return m_min;
}


int K3bIntValidator::top () const
{
  return m_max;
}


int K3bIntValidator::toInt( const TQString& s, bool* ok )
{
  if( s.lower().startsWith( "0x" ) )
    return s.right( s.length()-2 ).toInt( ok, 16 );
  else if( s.lower().startsWith( "-0x" ) )
    return -1 * s.right( s.length()-3 ).toInt( ok, 16 );
  else
    return s.toInt( ok, 10 );
}
