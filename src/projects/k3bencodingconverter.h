/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
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

#ifndef _K3B_ENCODING_CONVERTER_H_
#define _K3B_ENCODING_CONVERTER_H_

#include <tqcstring.h>
#include <tqstring.h>

class TQWidget;

class K3bEncodingConverter
{
 public:
  K3bEncodingConverter();
  ~K3bEncodingConverter();

  /**
   * Check if a string is encoded using the local codeset
   *
   * \return True if the string is encoded in the local encoding.
   */
  bool encodedLocally( const TQCString& );

  /**
   * Tries to fix the encoding of a string to fit the local
   * encoding.
   * It presents a dialog to the user that let's them choose
   * the proper encoding based on example conversions.
   *
   * \param s The string to be fixed.
   * \param tqparent The tqparent widget to be used when showing the encoding selection dialog.
   * \param cacheEncoding If true the codeset used for successful conversion is cached and
   *                      reused for the next call to fixEncoding.
   *
   * \return True if the conversion was successful.
   */
  bool fixEncoding( const TQCString& s, TQCString& result, TQWidget* tqparent = 0, bool cacheEncoding = true );

 private:
  bool convert( const TQCString& s, TQCString& result, const TQString& from, const TQString& to );

  class Private;
  Private* d;
};

#endif
