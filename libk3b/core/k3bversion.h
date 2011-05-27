/* 
 *
 * $Id: k3bversion.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_VERSION_H_
#define _K3B_VERSION_H_

#include <tqstring.h>
#include "k3b_export.h"
/**
 * \brief Representation of a version.
 *
 * K3bVersion represents a version consisting of a major version (accessible via majorVersion()),
 * a minor version (accessible via minorVersion()), a patchLevel (accessible via patchLevel()),
 * and a suffix (accessible via suffix()).
 *
 * The major version is mandatory while all other fields are optional (in case of the minor version 
 * and the patchlevel -1 means that the field is undefined).
 *
 * K3bVersion tries to treat version suffixes in an "intelligent" way to properly compare versions
 * (see compareSuffix() for more details).
 *
 * K3bVersion may also be used everywhere a TQString is needed as it automatically converts to a
 * string representation using createVersionString().
 */
class LIBK3B_EXPORT K3bVersion 
{
 public:
  /**
   * construct an empty version object
   * which is invalid
   * @ see isValid()
   */
  K3bVersion();

  /**
   * copy constructor
   */
  K3bVersion( const K3bVersion& );

  /**
   * this constructor tries to parse the given version string
   */
  K3bVersion( const TQString& version );

  /**
   * sets the version and generates a version string from it
   */
  K3bVersion( int majorVersion, int minorVersion, int pachlevel = -1, const TQString& suffix = TQString() );

  /**
   * tries to parse the version string
   * used by the constructor
   */
  void setVersion( const TQString& );

  bool isValid() const;

  /**
   * sets the version and generates a version string from it
   * used by the constructor
   *
   * If minorVersion or pachlevel are -1 they will not be used when generating the version string.
   */
  void setVersion( int majorVersion, int minorVersion = -1, int patchlevel = -1, const TQString& suffix = TQString() );

  const TQString& versionString() const { return m_versionString; }
  int majorVersion() const { return m_majorVersion; }
  int minorVersion() const { return m_minorVersion; }
  int patchLevel() const { return m_patchLevel; }
  const TQString& suffix() const { return m_suffix; }

  /**
   * just to make it possible to use as a TQString
   */
  operator const TQString& () const { return m_versionString; }
  K3bVersion& operator=( const TQString& v );

  /**
   * \return A new K3bVersion object which equals this one except that the suffix is empty.
   */
  K3bVersion simplify() const;

  /**
   * If minorVersion or pachlevel are -1 they will not be used when generating the version string.
   * If minorVersion is -1 patchlevel will be ignored.
   */
  static TQString createVersionString( int majorVersion, 
				      int minorVersion = -1, 
				      int patchlevel = -1, 
				      const TQString& suffix = TQString() );

  /**
   * "Intelligent" comparison of two version suffixes.
   *
   * This method checks for the following types of suffixes and treats them in the
   * following order:
   *
   * [empty prefix] > rcX > preX > betaX > alphaX = aX (where X is a number)
   *
   * Every other suffixes are compared alphanumerical.
   * An empty prefix is always considered newer than an unknown non-emtpy suffix (e.g. not one of the above.)
   *
   * @return \li -1 if suffix1 is less than suffix2
   *         \li 0 if suffix1 equals suffix2 (be aware that this is not the same as comparing to strings as 
   *             alphaX equals aX in this case.)
   *         \li 1 if suffix1 is greater than suffix2
   */
  static int compareSuffix( const TQString& suffix1, const TQString& suffix2 );

 private:
  static void splitVersionString( const TQString& s, int& num, TQString& suffix );

  TQString m_versionString;
  int m_majorVersion;
  int m_minorVersion;
  int m_patchLevel;
  TQString m_suffix;
};


LIBK3B_EXPORT bool operator<( const K3bVersion& v1, const K3bVersion& v2 );
LIBK3B_EXPORT bool operator>( const K3bVersion& v1, const K3bVersion& v2 );
LIBK3B_EXPORT bool operator==( const K3bVersion& v1, const K3bVersion& v2 );
LIBK3B_EXPORT bool operator<=( const K3bVersion& v1, const K3bVersion& v2 );
LIBK3B_EXPORT bool operator>=( const K3bVersion& v1, const K3bVersion& v2 );


#endif
