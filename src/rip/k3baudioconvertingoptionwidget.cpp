/* 
 *
 * $Id: k3baudioconvertingoptionwidget.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudioconvertingoptionwidget.h"

#include <k3bpluginmanager.h>
#include <k3baudioencoder.h>
#include <k3bcore.h>
#include <k3bglobals.h>

#include <kcombobox.h>
#include <kurlrequester.h>
#include <kio/global.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <tqintdict.h>
#include <tqmap.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqcheckbox.h>



class K3bAudioConvertingOptionWidget::Private
{
public:
  TQIntDict<K3bAudioEncoder> encoderMap;
  TQMap<int, TQString> extensionMap;

  TQTimer freeSpaceUpdateTimer;

  KIO::filesize_t neededSize;

  int getDefaultFormat() {
    // we prefere formats in this order:
    // 1. ogg
    // 2. mp3
    // 3. flac
    // 4. wave
    int ogg = -1;
    int mp3 = -1;
    int flac = -1;
    for( TQMap<int, TQString>::const_iterator it = extensionMap.constBegin();
	 it != extensionMap.constEnd(); ++it ) {
      if( it.data() == "ogg" )
	ogg = it.key();
      else if( it.data() == "mp3" )
	mp3 = it.key();
      else if( it.data() == "flac" )
	flac = it.key();
    }

    if( ogg > -1 )
      return ogg;
    else if( mp3 > -1 )
      return mp3;
    else if( flac > -1 )
      return flac;
    else
	return 0;
  }
};


K3bAudioConvertingOptionWidget::K3bAudioConvertingOptionWidget( TQWidget* tqparent, const char* name )
  : base_K3bAudioRippingOptionWidget( tqparent, name )
{
  d = new Private();

  connect( m_editBaseDir, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SLOT(slotUpdateFreeTempSpace()) );
  connect( m_comboFileType, TQT_SIGNAL(activated(int)), 
	   this, TQT_SLOT(slotEncoderChanged()) );
  connect( &d->freeSpaceUpdateTimer, TQT_SIGNAL(timeout()),
	   this, TQT_SLOT(slotUpdateFreeTempSpace()) );
  connect( m_checkCreatePlaylist, TQT_SIGNAL(toggled(bool)), this, TQT_SIGNAL(changed()) );
  connect( m_checkSingleFile, TQT_SIGNAL(toggled(bool)), this, TQT_SIGNAL(changed()) );
  connect( m_checkWriteCueFile, TQT_SIGNAL(toggled(bool)), this, TQT_SIGNAL(changed()) );
  connect( m_comboFileType, TQT_SIGNAL(activated(int)), this, TQT_SIGNAL(changed()) );
  connect( m_editBaseDir, TQT_SIGNAL(textChanged(const TQString&)), this, TQT_SIGNAL(changed()) );
  connect( m_buttonConfigurePlugin, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotConfigurePlugin()) );

  m_editBaseDir->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
  m_buttonConfigurePlugin->setIconSet( SmallIconSet( "gear" ) );

  // FIXME: see if sox and the sox encoder are installed and if so do not put the internal wave
  //        writer in the list of encoders.

  d->encoderMap.clear();
  d->extensionMap.clear();
  m_comboFileType->clear();
  m_comboFileType->insertItem( i18n("Wave") );
  d->extensionMap[0] = "wav";

  // check the available encoding plugins
  TQPtrList<K3bPlugin> fl = k3bcore->pluginManager()->plugins( "AudioEncoder" );
  for( TQPtrListIterator<K3bPlugin> it( fl ); it.current(); ++it ) {
    K3bAudioEncoder* f = (K3bAudioEncoder*)it.current();
    TQStringList exL = f->extensions();

    for( TQStringList::const_iterator exIt = exL.begin();
	 exIt != exL.end(); ++exIt ) {
      d->extensionMap.insert( m_comboFileType->count(), *exIt );
      d->encoderMap.insert( m_comboFileType->count(), f );
      m_comboFileType->insertItem( f->fileTypeComment(*exIt) );
    }
  }

  // refresh every 2 seconds
  d->freeSpaceUpdateTimer.start(2000);
  slotUpdateFreeTempSpace();
}


K3bAudioConvertingOptionWidget::~K3bAudioConvertingOptionWidget()
{
  delete d;
}


TQString K3bAudioConvertingOptionWidget::baseDir() const
{
  return m_editBaseDir->url();
}


void K3bAudioConvertingOptionWidget::setBaseDir( const TQString& path )
{
  m_editBaseDir->setURL( path );
}


void K3bAudioConvertingOptionWidget::setNeededSize( KIO::filesize_t size )
{
  d->neededSize = size;
  if( size > 0 )
    m_labelNeededSpace->setText( KIO::convertSize( size ) );
  else
    m_labelNeededSpace->setText( i18n("unknown") );

  slotUpdateFreeTempSpace();
}


void K3bAudioConvertingOptionWidget::slotConfigurePlugin()
{
  // 0 for wave
  K3bAudioEncoder* encoder = d->encoderMap[m_comboFileType->currentItem()];
  if( encoder )
    k3bcore->pluginManager()->execPluginDialog( encoder, this );
}


void K3bAudioConvertingOptionWidget::slotUpdateFreeTempSpace()
{
  TQString path = m_editBaseDir->url();

  if( !TQFile::exists( path ) )
    path.truncate( path.findRev('/') );

  unsigned long size, avail;
  if( K3b::kbFreeOnFs( path, size, avail ) ) {
    m_labelFreeSpace->setText( KIO::convertSizeFromKB(avail) );
    if( avail < d->neededSize/1024 )
      m_labelNeededSpace->setPaletteForegroundColor( TQt::red );
    else
      m_labelNeededSpace->setPaletteForegroundColor( paletteForegroundColor() );
  }
  else {
    m_labelFreeSpace->setText("-");
    m_labelNeededSpace->setPaletteForegroundColor( paletteForegroundColor() );
  }
}


void K3bAudioConvertingOptionWidget::slotEncoderChanged()
{
  // 0 for wave
  m_buttonConfigurePlugin->setEnabled( d->encoderMap[m_comboFileType->currentItem()] != 0 );
}


K3bAudioEncoder* K3bAudioConvertingOptionWidget::encoder() const
{
  return d->encoderMap[m_comboFileType->currentItem()];  // 0 for wave
}


TQString K3bAudioConvertingOptionWidget::extension() const
{
  return d->extensionMap[m_comboFileType->currentItem()];
}


void K3bAudioConvertingOptionWidget::loadDefaults()
{
  m_editBaseDir->setURL( TQDir::homeDirPath() );
  m_checkSingleFile->setChecked( false );
  m_checkWriteCueFile->setChecked( false );
  m_comboFileType->setCurrentItem( d->getDefaultFormat() );
  m_checkCreatePlaylist->setChecked(false);
  m_checkPlaylistRelative->setChecked(false);

  slotEncoderChanged();
}


void K3bAudioConvertingOptionWidget::loadConfig( KConfigBase* c )
{
  m_editBaseDir->setURL( c->readPathEntry( "last ripping directory", TQDir::homeDirPath() ) );

  m_checkSingleFile->setChecked( c->readBoolEntry( "single_file", false ) );
  m_checkWriteCueFile->setChecked( c->readBoolEntry( "write_cue_file", false ) );

  m_checkCreatePlaylist->setChecked( c->readBoolEntry( "create_playlist", false ) );
  m_checkPlaylistRelative->setChecked( c->readBoolEntry( "relative_path_in_playlist", false ) );

  TQString filetype = c->readEntry( "filetype", d->extensionMap[d->getDefaultFormat()] );
  if( filetype == "wav" )
    m_comboFileType->setCurrentItem(0);
  else {
    for( TQMap<int, TQString>::iterator it = d->extensionMap.begin();
	 it != d->extensionMap.end(); ++it ) {
      if( it.data() == filetype ) {
	m_comboFileType->setCurrentItem( it.key() );
	break;
      }
    }
  }

  slotEncoderChanged();
}


void K3bAudioConvertingOptionWidget::saveConfig( KConfigBase* c )
{
  c->writePathEntry( "last ripping directory", m_editBaseDir->url() );

  c->writeEntry( "single_file", m_checkSingleFile->isChecked() );
  c->writeEntry( "write_cue_file", m_checkWriteCueFile->isChecked() );

  c->writeEntry( "create_playlist", m_checkCreatePlaylist->isChecked() );
  c->writeEntry( "relative_path_in_playlist", m_checkPlaylistRelative->isChecked() );

  if( d->extensionMap.contains(m_comboFileType->currentItem()) )
    c->writeEntry( "filetype", d->extensionMap[m_comboFileType->currentItem()] );
  else
    c->writeEntry( "filetype", "wav" );
}

#include "k3baudioconvertingoptionwidget.moc"
