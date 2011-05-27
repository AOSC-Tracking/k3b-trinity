/* 
 *
 * $Id: k3bexternalencoder.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <config.h>

#include "k3bexternalencoder.h"
#include "k3bexternalencodercommand.h"
#include "k3bexternalencoderconfigwidget.h"

#include <k3bpluginfactory.h>
#include <k3bprocess.h>
#include <k3bcore.h>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <tqregexp.h>
#include <tqfile.h>

#include <sys/types.h>
#include <sys/wait.h>


K_EXPORT_COMPONENT_FACTORY( libk3bexternalencoder, K3bPluginFactory<K3bExternalEncoder>( "libk3bexternalencoder" ) )


static const char s_riffHeader[] =
{
  0x52, 0x49, 0x46, 0x46, // 0  "RIFF"
  0x00, 0x00, 0x00, 0x00, // 4  wavSize
  0x57, 0x41, 0x56, 0x45, // 8  "WAVE"
  0x66, 0x6d, 0x74, 0x20, // 12 "fmt "
  0x10, 0x00, 0x00, 0x00, // 16
  0x01, 0x00, 0x02, 0x00, // 20
  0x44, 0xac, 0x00, 0x00, // 24
  0x10, 0xb1, 0x02, 0x00, // 28
  0x04, 0x00, 0x10, 0x00, // 32
  0x64, 0x61, 0x74, 0x61, // 36 "data"
  0x00, 0x00, 0x00, 0x00  // 40 byteCount
};





static K3bExternalEncoderCommand commandByExtension( const TQString& extension )
{
  TQValueList<K3bExternalEncoderCommand> cmds( K3bExternalEncoderCommand::readCommands() );
  for( TQValueList<K3bExternalEncoderCommand>::iterator it = cmds.begin(); it != cmds.end(); ++it )
    if( (*it).extension == extension )
      return *it;

  kdDebug() << "(K3bExternalEncoder) could not find command for extension " << extension << endl;

  return K3bExternalEncoderCommand();
}


class K3bExternalEncoder::Private
{
public:
  Private()
    : process(0) {
  }

  K3bProcess* process;
  TQString fileName;
  TQString extension;
  K3b::Msf length;

  K3bExternalEncoderCommand cmd;

  bool initialized;

  // the metaData we support
  TQString artist;
  TQString title;
  TQString comment;
  TQString trackNumber;
  TQString cdArtist;
  TQString cdTitle;
  TQString cdComment;
  TQString year;
  TQString genre;
};


K3bExternalEncoder::K3bExternalEncoder( TQObject* tqparent, const char* name )
  : K3bAudioEncoder( tqparent, name )
{
  d = new Private();
}


K3bExternalEncoder::~K3bExternalEncoder()
{
  delete d->process;
  delete d;
}


void K3bExternalEncoder::setMetaDataInternal( K3bAudioEncoder::MetaDataField f, const TQString& value )
{
  switch( f ) {
  case META_TRACK_TITLE:
    d->title = value;
    break;
  case META_TRACK_ARTIST:
    d->artist = value;
    break;
  case META_TRACK_COMMENT:
    d->comment = value;
    break;
  case META_TRACK_NUMBER:
    d->trackNumber = value;
    break;
  case META_ALBUM_TITLE:
    d->cdTitle = value;
    break;
  case META_ALBUM_ARTIST:
    d->cdArtist = value;
    break;
  case META_ALBUM_COMMENT:
    d->cdComment = value;
    break;
  case META_YEAR:
    d->year = value;
    break;
  case META_GENRE:
    d->genre = value;
    break;
  }
}


void K3bExternalEncoder::finishEncoderInternal()
{
  if( d->process ) {
    if( d->process->isRunning() ) {
      ::close( d->process->stdinFd() );

      // this is kind of evil... 
      // but we need to be sure the process exited when this method returnes
      ::waitpid( d->process->pid(), 0, 0 );
    }
  }
}


void K3bExternalEncoder::slotExternalProgramFinished( KProcess* p )
{
  if( !p->normalExit() || p->exitStatus() != 0 )
    kdDebug() << "(K3bExternalEncoder) program exited with error." << endl;
}


bool K3bExternalEncoder::openFile( const TQString& ext, const TQString& filename, const K3b::Msf& length )
{
  d->fileName = filename;
  d->extension = ext;
  d->initialized = false;
  d->length = length;

  // delete existing files as some programs (like flac for example) might refuse to overwrite files
  if( TQFile::exists( filename ) )
    TQFile::remove( filename );

  return true;
}


void K3bExternalEncoder::closeFile()
{
  finishEncoderInternal();
}


bool K3bExternalEncoder::initEncoderInternal( const TQString& extension )
{
  d->initialized = true;

  // find the correct command
  d->cmd = commandByExtension( extension );

  if( d->cmd.command.isEmpty() ) {
    setLastError( i18n("Invalid command: the command is empty.") );
    return false;
  }

  // setup the process
  delete d->process;
  d->process = new K3bProcess();
  d->process->setSplitStdout(true);
  d->process->setRawStdin(true);

  connect( d->process, TQT_SIGNAL(processExited(KProcess*)),
	   this, TQT_SLOT(slotExternalProgramFinished(KProcess*)) );
  connect( d->process, TQT_SIGNAL(stderrLine(const TQString&)),
	   this, TQT_SLOT(slotExternalProgramOutputLine(const TQString&)) );
  connect( d->process, TQT_SIGNAL(stdoutLine(const TQString&)),
	   this, TQT_SLOT(slotExternalProgramOutputLine(const TQString&)) );


  // create the commandline
  TQStringList params = TQStringList::split( ' ', d->cmd.command, false );
  for( TQStringList::iterator it = params.begin(); it != params.end(); ++it ) {
    (*it).tqreplace( "%f", d->fileName );
    (*it).tqreplace( "%a", d->artist );
    (*it).tqreplace( "%t", d->title );
    (*it).tqreplace( "%c", d->comment );
    (*it).tqreplace( "%y", d->year );
    (*it).tqreplace( "%m", d->cdTitle );
    (*it).tqreplace( "%r", d->cdArtist );
    (*it).tqreplace( "%x", d->cdComment );
    (*it).tqreplace( "%n", d->trackNumber );
    (*it).tqreplace( "%g", d->genre );

    *d->process << *it;
  }


  kdDebug() << "***** external parameters:" << endl;
  const TQValueList<TQCString>& args = d->process->args();
  TQString s;
  for( TQValueList<TQCString>::const_iterator it = args.begin(); it != args.end(); ++it ) {
    s += *it + " ";
  }
  kdDebug() << s << flush << endl;

  // set one general error message
  setLastError( i18n("Command failed: %1").tqarg( s ) );
  
  if( d->process->start( KProcess::NotifyOnExit, KProcess::All ) ) {
    if( d->cmd.writeWaveHeader )
      return writeWaveHeader();
    else
      return true;
  }
  else {
    TQString commandName = d->cmd.command.section( TQRegExp("\\s+"), 0 );
    if( !KStandardDirs::findExe( commandName ).isEmpty() )
      setLastError( i18n("Could not find program '%1'").tqarg(commandName) );

    return false;
  }
}


bool K3bExternalEncoder::writeWaveHeader()
{
  kdDebug() << "(K3bExternalEncoder) writing wave header" << endl;

  // write the RIFF thing
  if( ::write( d->process->stdinFd(), s_riffHeader, 4 ) != 4 ) {
    kdDebug() << "(K3bExternalEncoder) failed to write riff header." << endl;
    return false;
  }
  
  // write the wave size
  TQ_INT32 dataSize( d->length.audioBytes() );
  TQ_INT32 wavSize( dataSize + 44 - 8 );
  char c[4];

  c[0] = (wavSize   >> 0 ) & 0xff;
  c[1] = (wavSize   >> 8 ) & 0xff;
  c[2] = (wavSize   >> 16) & 0xff;
  c[3] = (wavSize   >> 24) & 0xff;
  
  if( ::write( d->process->stdinFd(), c, 4 ) != 4 ) {
    kdDebug() << "(K3bExternalEncoder) failed to write wave size." << endl;
    return false;
  }

  // write static part of the header
  if( ::write( d->process->stdinFd(), s_riffHeader+8, 32 ) != 32 ) {
    kdDebug() << "(K3bExternalEncoder) failed to write wave header." << endl;
    return false;
  }

  c[0] = (dataSize   >> 0 ) & 0xff;
  c[1] = (dataSize   >> 8 ) & 0xff;
  c[2] = (dataSize   >> 16) & 0xff;
  c[3] = (dataSize   >> 24) & 0xff;

  if( ::write( d->process->stdinFd(), c, 4 ) != 4 ) {
    kdDebug() << "(K3bExternalEncoder) failed to write data size." << endl;
    return false;
  }

  return true;
}


long K3bExternalEncoder::encodeInternal( const char* data, TQ_ULONG len )
{
  if( !d->initialized )
    if( !initEncoderInternal( d->extension ) )
      return -1;

  if( d->process ) {
    if( d->process->isRunning() ) {

      long written = 0;

      //
      // we swap the bytes to reduce user irritation ;)
      // This is a little confused: We used to swap the byte order
      // in older versions of this encoder since little endian seems
      // to "feel" more natural.
      // So now that we have a swap option we have to invert it to ensure
      // compatibility
      //
      if( !d->cmd.swapByteOrder ) {
	char* buffer = new char[len];
	for( unsigned int i = 0; i < len-1; i+=2 ) {
	  buffer[i] = data[i+1];
	  buffer[i+1] = data[i];
	}

	written = ::write( d->process->stdinFd(), (const void*)buffer, len );
	delete [] buffer;
      }
      else
	written = ::write( d->process->stdinFd(), (const void*)data, len );

      return written;
    }
    else
      return -1;
  }
  else
    return -1;
}


void K3bExternalEncoder::slotExternalProgramOutputLine( const TQString& line )
{
  kdDebug() << "(" << d->cmd.name << ") " << line << endl;
}


TQStringList K3bExternalEncoder::extensions() const
{
  TQStringList el;
  TQValueList<K3bExternalEncoderCommand> cmds( K3bExternalEncoderCommand::readCommands() );
  for( TQValueList<K3bExternalEncoderCommand>::iterator it = cmds.begin(); it != cmds.end(); ++it )
    el.append( (*it).extension );

  return el;
}


TQString K3bExternalEncoder::fileTypeComment( const TQString& ext ) const
{
  return commandByExtension( ext ).name;
}


K3bPluginConfigWidget* K3bExternalEncoder::createConfigWidget( TQWidget* tqparent, 
							       const char* name ) const
{
  return new K3bExternalEncoderSettingsWidget( tqparent, name );
}


#include "k3bexternalencoder.moc"
