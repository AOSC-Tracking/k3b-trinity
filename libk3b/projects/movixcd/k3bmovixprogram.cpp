/* 
 *
 * $Id: k3bmovixprogram.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bmovixprogram.h"

#include <k3bprocess.h>

#include <kdebug.h>
#include <klocale.h>

#include <tqdir.h>
#include <tqfile.h>
#include <tqtextstream.h>


K3bMovixProgram::K3bMovixProgram()
  : K3bExternalProgram( "eMovix" )
{
}

bool K3bMovixProgram::scan( const TQString& p )
{
  if( p.isEmpty() )
    return false;

  TQString path = p;
  if( path[path.length()-1] != '/' )
    path.append("/");

  // first test if we have a version info (eMovix >= 0.8.0pre3)
  if( !TQFile::exists( path + "movix-version" ) )
    return false;

  K3bMovixBin* bin = 0;

  //
  // probe version and data dir
  //
  KProcess vp, dp;
  vp << path + "movix-version";
  dp << path + "movix-conf";
  K3bProcessOutputCollector vout( &vp ), dout( &dp );
  if( vp.start( KProcess::Block, KProcess::AllOutput ) && dp.start( KProcess::Block, KProcess::AllOutput ) ) {
    // movix-version just gives us the version number on stdout
    if( !vout.output().isEmpty() && !dout.output().isEmpty() ) {
      bin = new K3bMovixBin( this );
      bin->version = vout.output().stripWhiteSpace();
      bin->path = path;
      bin->m_movixPath = dout.output().stripWhiteSpace();
    }
  }
  else {
    kdDebug() << "(K3bMovixProgram) could not start " << path << "movix-version" << endl;
    return false;
  }

  if( bin->version >= K3bVersion( 0, 9, 0 ) )
    return scanNewEMovix( bin, path );
  else
    return scanOldEMovix( bin, path );
}


bool K3bMovixProgram::scanNewEMovix( K3bMovixBin* bin, const TQString& path )
{
  TQStringList files = bin->files();
  for( TQStringList::iterator it = files.begin();
       it != files.end(); ++it ) {
    if( (*it).tqcontains( "isolinux.cfg" ) ) {
      bin->m_supportedBootLabels = determineSupportedBootLabels( TQStringList::split( " ", *it )[1] );
      break;
    }
  }

  // here we simply check for the movix-conf program
  if( TQFile::exists( path + "movix-conf" ) ) {
    bin->addFeature( "newfiles" );
    addBin(bin);
    return true;
  }
  else {
    delete bin;
    return false;
  }
}


bool K3bMovixProgram::scanOldEMovix( K3bMovixBin* bin, const TQString& path )
{
  //
  // first check if all necessary directories are present
  //
  TQDir dir( bin->movixDataDir() );
  TQStringList subdirs = dir.entryList( TQDir::Dirs );
  if( !subdirs.tqcontains( "boot-messages" ) ) {
    kdDebug() << "(K3bMovixProgram) could not find subdir 'boot-messages'" << endl;
    delete bin;
    return false;
  }
  if( !subdirs.tqcontains( "isolinux" ) ) {
    kdDebug() << "(K3bMovixProgram) could not find subdir 'isolinux'" << endl;
    delete bin;
    return false;
  }
  if( !subdirs.tqcontains( "movix" ) ) {
    kdDebug() << "(K3bMovixProgram) could not find subdir 'movix'" << endl;
    delete bin;
    return false;
  }
  if( !subdirs.tqcontains( "mplayer-fonts" ) ) {
    kdDebug() << "(K3bMovixProgram) could not find subdir 'mplayer-fonts'" << endl;
    delete bin;
    return false;
  }


  //
  // check if we have a version of eMovix which contains the movix-files script
  //
  if( TQFile::exists( path + "movix-files" ) ) {
    bin->addFeature( "files" );

    KProcess p;
    K3bProcessOutputCollector out( &p );
    p << bin->path + "movix-files";
    if( p.start( KProcess::Block, KProcess::AllOutput ) ) {
      bin->m_movixFiles = TQStringList::split( "\n", out.output() );
    }
  }

  //
  // fallback: to be compatible with 0.8.0rc2 we just add all files in the movix directory
  //
  if( bin->m_movixFiles.isEmpty() ) {
    TQDir dir( bin->movixDataDir() + "/movix" );
    bin->m_movixFiles = dir.entryList(TQDir::Files);
  }

  //
  // these files are fixed. That should not be a problem
  // since Isolinux is quite stable as far as I know.
  //
  bin->m_isolinuxFiles.append( "initrd.gz" );
  bin->m_isolinuxFiles.append( "isolinux.bin" );
  bin->m_isolinuxFiles.append( "isolinux.cfg" );
  bin->m_isolinuxFiles.append( "kernel/vmlinuz" );
  bin->m_isolinuxFiles.append( "movix.lss" );
  bin->m_isolinuxFiles.append( "movix.msg" );


  //
  // check every single necessary file :(
  //
  for( TQStringList::const_iterator it = bin->m_isolinuxFiles.begin();
       it != bin->m_isolinuxFiles.end(); ++it ) {
    if( !TQFile::exists( bin->movixDataDir() + "/isolinux/" + *it ) ) {
      kdDebug() << "(K3bMovixProgram) Could not find file " << *it << endl;
      delete bin;
      return false;
    }
  }

  //
  // now check the boot-messages languages
  //
  dir.cd( "boot-messages" );
  bin->m_supportedLanguages = dir.entryList(TQDir::Dirs);
  bin->m_supportedLanguages.remove(".");
  bin->m_supportedLanguages.remove("..");
  bin->m_supportedLanguages.remove("CVS");  // the eMovix makefile stuff seems not perfect ;)
  bin->m_supportedLanguages.prepend( i18n("default") );
  dir.cdUp();

  //
  // now check the supported mplayer-fontsets
  // FIXME: every font dir needs to contain the "font.desc" file!
  //
  dir.cd( "mplayer-fonts" );
  bin->m_supportedSubtitleFonts = dir.entryList( TQDir::Dirs );
  bin->m_supportedSubtitleFonts.remove(".");
  bin->m_supportedSubtitleFonts.remove("..");
  bin->m_supportedSubtitleFonts.remove("CVS");  // the eMovix makefile stuff seems not perfect ;)
  // new ttf fonts in 0.8.0rc2
  bin->m_supportedSubtitleFonts += dir.entryList( "*.ttf", TQDir::Files );
  bin->m_supportedSubtitleFonts.prepend( i18n("none") );
  dir.cdUp();
  
  //
  // now check the supported boot labels
  //
  dir.cd( "isolinux" );
  bin->m_supportedBootLabels = determineSupportedBootLabels( dir.filePath("isolinux.cfg") );

  //
  // This seems to be a valid eMovix installation. :)
  //

  addBin(bin);
  return true;
}


TQStringList K3bMovixProgram::determineSupportedBootLabels( const TQString& isoConfigFile ) const
{
  TQStringList list( i18n("default") );

  TQFile f( isoConfigFile );
  if( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "(K3bMovixProgram) could not open file '" << f.name() << "'" << endl;
  }
  else {
    TQTextStream fs( &f );
    TQString line = fs.readLine();
    while( !line.isNull() ) {
      if( line.startsWith( "label" ) )
	list.append( line.mid( 5 ).stripWhiteSpace() );
      
      line = fs.readLine();
    }
    f.close();
  }

  return list;
}


TQString K3bMovixBin::subtitleFontDir( const TQString& font ) const
{
  if( font == i18n("none" ) )
    return "";
  else if( m_supportedSubtitleFonts.tqcontains( font ) )
    return path + "/mplayer-fonts/" + font;
  else
    return "";
}


TQString K3bMovixBin::languageDir( const TQString& lang ) const
{
  if( lang == i18n("default") )
    return languageDir( "en" );
  else if( m_supportedLanguages.tqcontains( lang ) )
    return path + "/boot-messages/" + lang;
  else
    return "";
}


TQStringList K3bMovixBin::supportedSubtitleFonts() const
{
  if( version >= K3bVersion( 0, 9, 0 ) )
    return TQStringList( i18n("default") ) += supported( "font" );
  else
    return m_supportedSubtitleFonts;
}


TQStringList K3bMovixBin::supportedLanguages() const
{
  if( version >= K3bVersion( 0, 9, 0 ) )
    return TQStringList( i18n("default") ) += supported( "lang" );
  else
    return m_supportedLanguages;
}


// only used for eMovix >= 0.9.0
TQStringList K3bMovixBin::supportedKbdLayouts() const
{
  return TQStringList( i18n("default") ) += supported( "kbd" );
}


// only used for eMovix >= 0.9.0
TQStringList K3bMovixBin::supportedBackgrounds() const
{
  return TQStringList( i18n("default") ) += supported( "background" );
}


// only used for eMovix >= 0.9.0
TQStringList K3bMovixBin::supportedCodecs() const
{
  return supported( "codecs" );
}


TQStringList K3bMovixBin::supported( const TQString& type ) const
{
  KProcess p;
  K3bProcessOutputCollector out( &p );
  p << path + "movix-conf" << "--supported=" + type;
  if( p.start( KProcess::Block, KProcess::AllOutput ) )
    return TQStringList::split( "\n", out.output() );
  else
    return TQStringList();
}


TQStringList K3bMovixBin::files( const TQString& kbd,
				const TQString& font,
				const TQString& bg,
				const TQString& lang,
				const TQStringList& codecs ) const
{
  KProcess p;
  K3bProcessOutputCollector out( &p );
  p << path + "movix-conf" << "--files";


  if( !kbd.isEmpty() && kbd != i18n("default") )
    p << "--kbd" << kbd;
  if( !font.isEmpty() && font != i18n("default") )
    p << "--font" << font;
  if( !bg.isEmpty() && bg != i18n("default") )
    p << "--background" << bg;
  if( !lang.isEmpty() && lang != i18n("default") )
    p << "--lang" << lang;
  if( !codecs.isEmpty() )
    p << "--codecs" << codecs.join( "," );

  if( p.start( KProcess::Block, KProcess::AllOutput ) )
    return TQStringList::split( "\n", out.output() );
  else
    return TQStringList();
}
