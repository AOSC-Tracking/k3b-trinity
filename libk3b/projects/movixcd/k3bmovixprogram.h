/* 
 *
 * $Id: k3bmovixprogram.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_MOVIX_PROGRAM_H_
#define _K3B_MOVIX_PROGRAM_H_

#include <k3bexternalbinmanager.h>
#include "k3b_export.h"

class LIBK3B_EXPORT K3bMovixBin : public K3bExternalBin
{
 public:
  K3bMovixBin( K3bExternalProgram* p )
    : K3bExternalBin( p ) {
  }

  const TQString& movixDataDir() const { return m_movixPath; }

  const TQStringList& supportedBootLabels() const { return m_supportedBootLabels; }
  TQStringList supportedSubtitleFonts() const;
  TQStringList supportedLanguages() const;
  TQStringList supportedKbdLayouts() const;
  TQStringList supportedBackgrounds() const;
  TQStringList supportedCodecs() const;

  /*
   * Unused for eMovix versions 0.9.0 and above
   */
  const TQStringList& movixFiles() const { return m_movixFiles; }

  /*
   * Unused for eMovix versions 0.9.0 and above
   */
  const TQStringList& isolinuxFiles() const { return m_isolinuxFiles; }

  /**
   * returnes empty string if font was not found
   *
   * Unused for eMovix versions 0.9.0 and above
   */
  TQString subtitleFontDir( const TQString& font ) const;

  /**
   * returnes empty string if lang was not found
   *
   * Unused for eMovix versions 0.9.0 and above
   */
  TQString languageDir( const TQString& lang ) const;

  /**
   * Interface for the movix-conf --files interface for
   * versions >= 0.9.0
   */
  TQStringList files( const TQString& kbd = TQString(),
		     const TQString& font = TQString(),
		     const TQString& bg = TQString(),
		     const TQString& lang = TQString(),
		     const TQStringList& codecs = TQStringList() ) const;

 private:
  TQStringList supported( const TQString& ) const;

  TQString m_movixPath;
  TQStringList m_movixFiles;
  TQStringList m_isolinuxFiles;
  TQStringList m_supportedBootLabels;
  TQStringList m_supportedSubtitleFonts;
  TQStringList m_supportedLanguages;

  friend class K3bMovixProgram;
};


class LIBK3B_EXPORT K3bMovixProgram : public K3bExternalProgram
{
 public:
  K3bMovixProgram();

  bool scan( const TQString& );

  bool supportsUserParameters() const { return false; }

 private:
  bool scanNewEMovix( K3bMovixBin* bin, const TQString& );
  bool scanOldEMovix( K3bMovixBin* bin, const TQString& );
  TQStringList determineSupportedBootLabels( const TQString& ) const;
};



#endif
