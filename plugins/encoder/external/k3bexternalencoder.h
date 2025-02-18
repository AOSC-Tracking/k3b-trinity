/* 
 *
 * $Id: k3bexternalencoder.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_EXTERNAL_ENCODER_H_
#define _K3B_EXTERNAL_ENCODER_H_


#include <k3baudioencoder.h>


class base_K3bExternalEncoderConfigWidget;
class TDEProcess;


class K3bExternalEncoder : public K3bAudioEncoder
{
  TQ_OBJECT
  

 public:
  K3bExternalEncoder( TQObject* parent = 0, const char* name = 0 );
  ~K3bExternalEncoder();

  TQStringList extensions() const;
  
  TQString fileTypeComment( const TQString& ) const;

  int pluginSystemVersion() const { return 3; }

  K3bPluginConfigWidget* createConfigWidget( TQWidget* parent, 
					     const char* name ) const;

  /**
   * reimplemented since the external program is intended to write the file
   * TODO: allow writing to stdout.
   */
  bool openFile( const TQString& ext, const TQString& filename, const K3b::Msf& length );
  void closeFile();

  class Command;

 private slots:
  void slotExternalProgramFinished( TDEProcess* );
  void slotExternalProgramOutputLine( const TQString& );

 private:
  void finishEncoderInternal();
  bool initEncoderInternal( const TQString& extension );
  long encodeInternal( const char* data, TQ_ULONG len );
  void setMetaDataInternal( MetaDataField, const TQString& );
  bool writeWaveHeader();

  class Private;
  Private* d;
};

#endif
