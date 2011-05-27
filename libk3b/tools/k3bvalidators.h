/* 
 *
 * $Id: k3bvalidators.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_VALIDATORS_H_
#define _K3B_VALIDATORS_H_

#include <tqvalidator.h>
#include "k3b_export.h"


/**
 * Simple validator that validates a string char by char
 */
class LIBK3B_EXPORT K3bCharValidator : public TQValidator
{
 public:
  K3bCharValidator( TQObject* tqparent = 0, const char* name = 0 );

  virtual State validateChar( const TQChar& ) const = 0;

  virtual State validate( TQString& s, int& pos ) const;

  /**
   * Replaces all invalid chars with the repplace char
   */
  virtual void fixup( TQString& ) const;

  /**
   * Default to '_'
   */
  void setReplaceChar( const TQChar& c ) { m_replaceChar = c; }

 private:
  TQChar m_replaceChar;
};


class LIBK3B_EXPORT K3bLatin1Validator : public K3bCharValidator
{
 public:
  K3bLatin1Validator( TQObject* tqparent = 0, const char* name = 0 );

  virtual State validateChar( const TQChar& ) const;
};


class LIBK3B_EXPORT K3bAsciiValidator : public K3bLatin1Validator
{
 public:
  K3bAsciiValidator( TQObject* tqparent = 0, const char* name = 0 );

  virtual State validateChar( const TQChar& ) const;
};


/**
 * The K3bValidator extends TQRegExpValidator with a fixup method
 * that just replaces all characters that are not allowed with the 
 * replace character. It only makes sense for TQRegExps that simply
 * allow or forbid some characters.
 */
class LIBK3B_EXPORT K3bValidator : public TQRegExpValidator
{
 public:
  K3bValidator( TQObject* tqparent, const char * name = 0 );
  K3bValidator( const TQRegExp& rx, TQObject* tqparent, const char* name = 0 );

  void setReplaceChar( const TQChar& s ) { m_replaceChar = s; }
  const TQChar& replaceChar() const { return m_replaceChar; }

  virtual void fixup( TQString& ) const;

 private:
  TQChar m_replaceChar;
};


namespace K3bValidators
{
  /**
   * just replaces all characters that are not allowed with the 
   * replace character. It only makes sense for TQRegExps that simply
   * allow or forbid some characters.
   */
  LIBK3B_EXPORT TQString fixup( const TQString&, const TQRegExp&, const TQChar& replaceChar = '_' );

  /**
   * Validates an ISRC code of the form "CCOOOYYSSSSS" where:
   * <ul>
   * <li>C: country code (upper case letters or digits)</li>
   * <li>O: owner code (upper case letters or digits)</li>
   * <li>Y: year (digits)</li>
   * <li>S: serial number (digits)</li>
   * </ul>
   */
  LIBK3B_EXPORT K3bValidator* isrcValidator( TQObject* tqparent = 0, const char* name = 0 );
  
  /**
   * This needs to be replaced by something better in the future...
   * Even the name sucks!
   */
  LIBK3B_EXPORT K3bValidator* iso9660Validator( bool allowEmpty = true, TQObject* tqparent = 0, const char* name = 0 );

  /**
   * (1) d-characters are: A-Z, 0-9, _ (see ISO-9660:1988, Annex A, Table 15)
   * (2) a-characters are: A-Z, 0-9, _, space, !, ", %, &, ', (, ), *, +, ,, -, ., /, :, ;, <, =, >, ? 
   * (see ISO-9660:1988, Annex A, Table 14)
   */
  enum Iso646Type {
    Iso646_a, 
    Iso646_d 
  };

  LIBK3B_EXPORT K3bValidator* iso646Validator( int type = Iso646_a, 
				 bool AllowLowerCase = false, 
				 TQObject* tqparent = 0, const char* name = 0 );
}

#endif
