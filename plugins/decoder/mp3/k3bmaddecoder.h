/* 
 *
 * $Id$
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

#ifndef K3BMP3MODULE_H
#define K3BMP3MODULE_H


#include <k3baudiodecoder.h>

extern "C" {
#include <mad.h>
}


class K3bMadDecoderFactory : public K3bAudioDecoderFactory
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bMadDecoderFactory( TQObject* tqparent = 0, const char* name = 0 );
  ~K3bMadDecoderFactory();

  bool canDecode( const KURL& filename );

  int pluginSystemVersion() const { return 3; }

  K3bAudioDecoder* createDecoder( TQObject* tqparent = 0, 
				  const char* name = 0 ) const;
};


class K3bMadDecoder : public K3bAudioDecoder
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bMadDecoder( TQObject* tqparent = 0, const char* name = 0 );
  ~K3bMadDecoder();

  TQString metaInfo( MetaDataField );

  void cleanup();

  bool seekInternal( const K3b::Msf& );

  TQString fileType() const;
  TQStringList supportedTechnicalInfos() const;
  TQString technicalInfo( const TQString& ) const;

 protected:
  bool analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch );
  bool initDecoderInternal();

  int decodeInternal( char* _data, int maxLen );
 
 private:
  unsigned long countFrames();
  inline unsigned short linearRound( mad_fixed_t fixed );
  bool createPcmSamples( mad_synth* );

  static int MaxAllowedRecoverableErrors;

  class MadDecoderPrivate;
  MadDecoderPrivate* d;
};

#endif
