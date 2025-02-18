/* 
 *
 * $Id: k3bmpcdecoder.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_MPC_DECODER_H_
#define _K3B_MPC_DECODER_H_

#include <k3baudiodecoder.h>

class K3bMpcWrapper;


class K3bMpcDecoderFactory : public K3bAudioDecoderFactory
{
  TQ_OBJECT
  

 public:
  K3bMpcDecoderFactory( TQObject* parent = 0, const char* name = 0 );
  ~K3bMpcDecoderFactory();

  bool canDecode( const KURL& filename );

  int pluginSystemVersion() const { return 3; }

  K3bAudioDecoder* createDecoder( TQObject* parent = 0, 
				  const char* name = 0 ) const;
};


class K3bMpcDecoder : public K3bAudioDecoder
{
  TQ_OBJECT
  

 public:
  K3bMpcDecoder( TQObject* parent = 0, const char* name = 0 );
  ~K3bMpcDecoder();

  TQString fileType() const;

 protected:
  bool analyseFileInternal( K3b::Msf& frames, int& samplerate, int& ch );
  bool initDecoderInternal();
  bool seekInternal( const K3b::Msf& );

  int decodeInternal( char* _data, int maxLen );

 private:
  K3bMpcWrapper* m_mpc;
};

#endif
