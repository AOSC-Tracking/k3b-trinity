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

#include "k3bthememanager.h"

#include <k3bversion.h>

#include <kstandarddirs.h>
#include <tdeglobalsettings.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>

#include <tqpixmap.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqdir.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>


K3bTheme::K3bTheme()
  : m_bgMode(BG_TILE)
{
}


TQColor K3bTheme::backgroundColor() const
{
  if( m_bgColor.isValid() )
    return m_bgColor;
  else
    return TDEGlobalSettings::activeTitleColor();
}


TQColor K3bTheme::foregroundColor() const
{
  if( m_fgColor.isValid() )
    return m_fgColor;
  else
    return TDEGlobalSettings::activeTextColor();
}


const TQPixmap& K3bTheme::pixmap( const TQString& name ) const
{
  TQMap<TQString, TQPixmap>::const_iterator it = m_pixmapMap.find( name );
  if( it != m_pixmapMap.end() )
    return *it;

  // try loading the image
  if( TQFile::exists( m_path + name ) )
    return *m_pixmapMap.insert( name, TQPixmap( m_path + name ) );

  kdDebug() << "(K3bTheme) " << m_name << ": could not load image " << name << endl;

  return m_emptyPixmap;
}


const TQPixmap& K3bTheme::pixmap( K3bTheme::PixmapType t ) const
{
  return pixmap( filenameForPixmapType( t ) );
}


TQString K3bTheme::filenameForPixmapType( PixmapType t )
{
  TQString name;

  switch( t ) {
  case MEDIA_AUDIO:
    name = "media_audio";
    break;
  case MEDIA_DATA:
    name = "media_data";
    break;
  case MEDIA_VIDEO:
    name = "media_video";
    break;
  case MEDIA_EMPTY:
    name = "media_empty";
    break;
  case MEDIA_MIXED:
    name = "media_mixed";
    break;
  case MEDIA_NONE:
    name = "media_none";
    break;
  case MEDIA_LEFT:
    name = "media_left";
    break;
  case PROGRESS_WORKING:
    name = "progress_working";
    break;
  case PROGRESS_SUCCESS:
    name = "progress_success";
    break;
  case PROGRESS_FAIL:
    name = "progress_fail";
    break;
  case PROGRESS_RIGHT:
    name = "progress_right";
    break;
  case DIALOG_LEFT:
    name = "dialog_left";
    break;
  case DIALOG_RIGHT:
    name = "dialog_right";
    break;
  case SPLASH:
    name = "splash";
    break;
  case PROJECT_LEFT:
    name = "project_left";
    break;
  case PROJECT_RIGHT:
    name = "project_right";
    break;
  case WELCOME_BG:
    name = "welcome_bg";
    break;
  default:
    break;
  }

  name.append( ".png" );

  return name;
}


K3bTheme::BackgroundMode K3bTheme::backgroundMode() const
{
  return m_bgMode;
}



class K3bThemeManager::Private
{
public:
  Private()
    : currentTheme(&emptyTheme) {
  }

  TQValueList<K3bTheme*> themes;
  K3bTheme* currentTheme;
  TQString currentThemeName;

  K3bTheme emptyTheme;
};



K3bThemeManager::K3bThemeManager( TQObject* parent, const char* name )
  : TQObject( parent, name )
{
  d = new Private();
  d->emptyTheme.m_name = "Empty Theme";
}


K3bThemeManager::~K3bThemeManager()
{
  delete d;
}


const TQValueList<K3bTheme*>& K3bThemeManager::themes() const
{
  return d->themes;
}


K3bTheme* K3bThemeManager::currentTheme() const
{
  return d->currentTheme;
}


void K3bThemeManager::readConfig( TDEConfigBase* c )
{
  TDEConfigGroup generalOptions( c, "General Options" );

  // allow to override the default theme by packaging a default config file
  TQString defaultTheme = generalOptions.readEntry( "default theme", "quant" );

  K3bVersion configVersion( generalOptions.readEntry( "config version", "0.1" ) );
  if( configVersion >= K3bVersion("0.98") )
    setCurrentTheme( generalOptions.readEntry( "current theme", defaultTheme ) );
  else
    setCurrentTheme( defaultTheme );
}


void K3bThemeManager::saveConfig( TDEConfigBase* c )
{
  if( !d->currentThemeName.isEmpty() )
    TDEConfigGroup( c, "General Options" ).writeEntry( "current theme", d->currentThemeName );
}


void K3bThemeManager::setCurrentTheme( const TQString& name )
{
  if( name != d->currentThemeName ) {
    if( K3bTheme* theme = findTheme( name ) )
      setCurrentTheme( theme );
  }
}


void K3bThemeManager::setCurrentTheme( K3bTheme* theme )
{
  if( !theme )
    theme = d->themes.first();

  if( theme ) {
    if( theme != d->currentTheme ) {
      d->currentTheme = theme;
      d->currentThemeName = theme->name();

      emit themeChanged();
      emit themeChanged( theme );
    }
  }
}


K3bTheme* K3bThemeManager::findTheme( const TQString& name ) const
{
  for( TQValueList<K3bTheme*>::iterator it = d->themes.begin(); it != d->themes.end(); ++it )
    if( (*it)->name() == name )
      return *it;
  return 0;
}


void K3bThemeManager::loadThemes()
{
  // first we cleanup the loaded themes
  for( TQValueList<K3bTheme*>::iterator it = d->themes.begin(); it != d->themes.end(); ++it )
    delete *it;
  d->themes.clear();

  TQStringList dirs = TDEGlobal::dirs()->findDirs( "data", "k3b/pics" );
  // now search for themes. As there may be multiple themes with the same name
  // we only use the names from this list and then use findResourceDir to make sure
  // the local is preferred over the global stuff (like testing a theme by copying it
  // to the .kde dir)
  TQStringList themeNames;
  for( TQStringList::const_iterator dirIt = dirs.begin(); dirIt != dirs.end(); ++dirIt ) {
    TQDir dir( *dirIt );
    TQStringList entries = dir.entryList( TQDir::Dirs );
    entries.remove( "." );
    entries.remove( ".." );
    // every theme dir needs to contain a k3b.theme file
    for( TQStringList::const_iterator entryIt = entries.begin(); entryIt != entries.end(); ++entryIt ) {
      TQString themeDir = *dirIt + *entryIt + "/";
      if( !themeNames.contains( *entryIt ) && TQFile::exists( themeDir + "k3b.theme" ) ) {
	bool themeValid = true;

	// check for all nessessary pixmaps (this is a little evil hacking)
	for( int i = 0; i <= K3bTheme::WELCOME_BG; ++i ) {
	  if( !TQFile::exists( themeDir + K3bTheme::filenameForPixmapType( (K3bTheme::PixmapType)i ) ) ) {
	    kdDebug() << "(K3bThemeManager) theme misses pixmap: " << K3bTheme::filenameForPixmapType( (K3bTheme::PixmapType)i ) << endl;
	    themeValid = false;
	    break;
	  }
	}

	if( themeValid )
	  themeNames.append( *entryIt );
      }
    }
  }

  // now load the themes
  for( TQStringList::const_iterator themeIt = themeNames.begin(); themeIt != themeNames.end(); ++themeIt )
    loadTheme( *themeIt );

  // load the current theme
  setCurrentTheme( findTheme(d->currentThemeName) );
}


void K3bThemeManager::loadTheme( const TQString& name )
{
  TQString path = TDEGlobal::dirs()->findResource( "data", "k3b/pics/" + name + "/k3b.theme" );
  if( !path.isEmpty() ) {
    K3bTheme* t = new K3bTheme();
    t->m_name = name;
    t->m_path = path.left( path.length() - 9 );
    TQFileInfo fi( t->m_path );
    t->m_local = fi.isWritable();

    // load the stuff
    KSimpleConfig cfg( path, true );
    t->m_author = cfg.readEntry( "Author" );
    t->m_comment = cfg.readEntry( "Comment" );
    t->m_version = cfg.readEntry( "Version" );
    t->m_bgColor = cfg.readColorEntry( "Backgroundcolor" );
    t->m_fgColor = cfg.readColorEntry( "Foregroundcolor" );
    t->m_bgMode = ( cfg.readEntry( "BackgroundMode" ) == "Scaled" ? K3bTheme::BG_SCALE : K3bTheme::BG_TILE );

    d->themes.append( t );
  }
}


#include "k3bthememanager.moc"
