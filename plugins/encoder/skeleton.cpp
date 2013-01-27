/* 
 *
 * $Id: skeleton.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <config.h>

#include "k3b<name>encoder.h"

#include <klocale.h>
#include <tdeconfig.h>
#include <kdebug.h>


K_EXPORT_COMPONENT_FACTORY( libk3b<name>encoder, K3bPluginFactory<K3b<name>Encoder>( "libk3b<name>encoder" ) )


K3b<name>Encoder::K3b<name>Encoder( TQObject* parent, const char* name )
  : K3bAudioEncoder( parent, name )
{
}


K3b<name>Encoder::~K3b<name>Encoder()
{
}


bool K3b<name>Encoder::initEncoderInternal( const TQString& )
{
  // PUT YOUR CODE HERE
  return false;
}


long K3b<name>Encoder::encodeInternal( const char* data, TQ_ULONG len )
{
  // PUT YOUR CODE HERE
  return false;
}


void K3b<name>Encoder::finishEncoderInternal()
{
  // PUT YOUR CODE HERE
}


void K3b<name>Encoder::setMetaDataInternal( K3bAudioEncoder::MetaDataField f, const TQString& value )
{
  // PUT YOUR CODE HERE
}


TQStringList K3b<name>Encoder::extensions() const
{
  // PUT YOUR CODE HERE
  return TQStringList( "" );
}


TQString K3b<name>Encoder::fileTypeComment( const TQString& ) const
{
  // PUT YOUR CODE HERE
  return "";
}


long long K3b<name>Encoder::fileSize( const TQString&, const K3b::Msf& msf ) const
{
  // PUT YOUR CODE HERE
  return -1;
}


K3bPluginConfigWidget* K3b<name>Encoder::createConfigWidget( TQWidget* parent, 
							     const char* name ) const
{
  return new K3b<name>EncoderSettingsWidget( parent, name );
}



K3b<name>EncoderSettingsWidget::K3b<name>EncoderSettingsWidget( TQWidget* parent, const char* name )
  : K3bPluginConfigWidget( parent, name )
{
}


K3b<name>EncoderSettingsWidget::~K3b<name>EncoderSettingsWidget()
{
}


void K3b<name>EncoderSettingsWidget::loadConfig()
{
  TDEConfig* c = k3bcore->config();
  c->setGroup( "K3b<name>EncoderPlugin" );

  // PUT YOUR CODE HERE
}


void K3b<name>EncoderSettingsWidget::saveConfig()
{
  TDEConfig* c = k3bcore->config();
  c->setGroup( "K3b<name>EncoderPlugin" );

  // PUT YOUR CODE HERE
}


#include "k3b<name>encoder.moc"
