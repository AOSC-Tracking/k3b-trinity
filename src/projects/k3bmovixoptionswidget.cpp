/* 
 *
 * $Id: k3bmovixoptionswidget.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bmovixoptionswidget.h"
#include "k3bmovixdoc.h"
#include "k3bmovixprogram.h"

#include <kcombobox.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <kdebug.h>

#include <tqcheckbox.h>
#include <tqspinbox.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <tqlabel.h>


class K3bMovixOptionsWidget::LanguageSelectionHelper
{
public:
  LanguageSelectionHelper( TQComboBox* box )
    : m_box(box) {
  }

  void insertLanguages( const TQStringList& langs ) {
    m_box->clear();
    m_langMap.clear();

    for( TQStringList::const_iterator it = langs.begin(); it != langs.end(); ++it ) {
      if( *it == i18n("default") )
	m_box->insertItem( *it );
      else {
	TQString lang = TDEGlobal::locale()->twoAlphaToLanguageName( *it );
	if( lang.isEmpty() )
	  lang = *it;

	m_langMap[m_box->count()] = *it;
	m_indexMap[*it] = m_box->count();
	m_box->insertItem( lang );
      }
    }
  }

  TQString selectedLanguage() const {
    if( m_box->currentItem() == 0 )
      return i18n("default");
    else
      return m_langMap[m_box->currentItem()];
  }

  void setLanguage( const TQString& l ) {
    TQMap<TQString,int>::const_iterator it = m_indexMap.find(l);
    if( it == m_indexMap.end() )
      m_box->setCurrentItem( 0 );
    else
      m_box->setCurrentItem( it.data() );
  }

private:
  TQComboBox* m_box;
  TQMap<int,TQString> m_langMap;
  TQMap<TQString,int> m_indexMap;
};


K3bMovixOptionsWidget::K3bMovixOptionsWidget( TQWidget* parent, const char* name )
  : base_K3bMovixOptionsWidget( parent, name )
{
  m_keyboardLangHelper = new LanguageSelectionHelper( m_comboKeyboardLayout );
  m_helpLangHelper = new LanguageSelectionHelper( m_comboBootMessageLanguage );
}


K3bMovixOptionsWidget::~K3bMovixOptionsWidget()
{
  delete m_keyboardLangHelper;
  delete m_helpLangHelper;
}


void K3bMovixOptionsWidget::init( const K3bMovixBin* bin )
{
  m_labelAudioBackground->setShown( bin->hasFeature( "newfiles" ) );
  m_comboAudioBackground->setShown( bin->hasFeature( "newfiles" ) );
  m_labelKeyboardLayout->setShown( bin->hasFeature( "newfiles" ) );
  m_comboKeyboardLayout->setShown( bin->hasFeature( "newfiles" ) );

  m_comboSubtitleFontset->insertStringList( bin->supportedSubtitleFonts() );
  m_helpLangHelper->insertLanguages( bin->supportedLanguages() );
  m_comboDefaultBootLabel->insertStringList( bin->supportedBootLabels() );
  m_keyboardLangHelper->insertLanguages( bin->supportedKbdLayouts() );
  m_comboAudioBackground->insertStringList( bin->supportedBackgrounds() );
}


void K3bMovixOptionsWidget::readSettings( K3bMovixDoc* doc )
{
  m_comboSubtitleFontset->setCurrentItem( doc->subtitleFontset(), false );
  m_spinLoop->setValue( doc->loopPlaylist() );
  m_editAdditionalMplayerOptions->setText( doc->additionalMPlayerOptions() );
  m_editUnwantedMplayerOptions->setText( doc->unwantedMPlayerOptions() );
  m_helpLangHelper->setLanguage( doc->bootMessageLanguage() );
  m_comboDefaultBootLabel->setCurrentItem( doc->defaultBootLabel(), false );
  m_comboAudioBackground->setCurrentItem( doc->audioBackground(), false );
  m_keyboardLangHelper->setLanguage( doc->keyboardLayout() );
  m_checkShutdown->setChecked( doc->shutdown() );
  m_checkReboot->setChecked( doc->reboot() );
  m_checkEject->setChecked( doc->ejectDisk() );
  m_checkRandomPlay->setChecked( doc->randomPlay() );
  m_checkNoDma->setChecked( doc->noDma() );
}


void K3bMovixOptionsWidget::saveSettings( K3bMovixDoc* doc )
{
  doc->setShutdown( m_checkShutdown->isChecked() );
  doc->setReboot( m_checkReboot->isChecked() );
  doc->setEjectDisk( m_checkEject->isChecked() );
  doc->setSubtitleFontset( m_comboSubtitleFontset->currentText() );
  doc->setBootMessageLanguage( m_helpLangHelper->selectedLanguage() );
  doc->setDefaultBootLabel( m_comboDefaultBootLabel->currentText() );
  doc->setKeyboardLayout( m_keyboardLangHelper->selectedLanguage() );
  doc->setAudioBackground( m_comboAudioBackground->currentText() );
  doc->setAdditionalMPlayerOptions( m_editAdditionalMplayerOptions->text() );
  doc->setUnwantedMPlayerOptions( m_editUnwantedMplayerOptions->text() );
  doc->setLoopPlaylist( m_spinLoop->value() );
  doc->setRandomPlay( m_checkRandomPlay->isChecked() );
  doc->setNoDma( m_checkNoDma->isChecked() );
}


void K3bMovixOptionsWidget::loadDefaults()
{
  m_comboSubtitleFontset->setCurrentItem( 0 ); // default
  m_comboAudioBackground->setCurrentItem( 0 ); // default
  m_comboKeyboardLayout->setCurrentItem( 0 ); // default
  m_spinLoop->setValue( 1 );
  m_editAdditionalMplayerOptions->setText( TQString() );
  m_editUnwantedMplayerOptions->setText( TQString() );
  m_comboBootMessageLanguage->setCurrentItem( 0 ); // default
  m_comboDefaultBootLabel->setCurrentItem( 0 );  // default
  m_checkShutdown->setChecked( false );
  m_checkReboot->setChecked( false );
  m_checkEject->setChecked( false );
  m_checkRandomPlay->setChecked( false );
  m_checkNoDma->setChecked( false );
}


void K3bMovixOptionsWidget::loadConfig( TDEConfigBase* c )
{
  TQString s = c->readEntry("subtitle_fontset");
  if( !s.isEmpty() && s != "none" && m_comboSubtitleFontset->contains(s) )
    m_comboSubtitleFontset->setCurrentItem( s, false );
  else
    m_comboSubtitleFontset->setCurrentItem( 0 ); // none

  m_spinLoop->setValue( c->readNumEntry("loop", 1 ) );
  m_editAdditionalMplayerOptions->setText( c->readEntry( "additional_mplayer_options" ) );
  m_editUnwantedMplayerOptions->setText( c->readEntry( "unwanted_mplayer_options" ) );

  s = c->readEntry("boot_message_language");
  m_helpLangHelper->setLanguage( s == "default" ? TQString() : s );

  s = c->readEntry( "default_boot_label" );
  if( !s.isEmpty() && s != "default" && m_comboDefaultBootLabel->contains(s) )
    m_comboDefaultBootLabel->setCurrentItem( s, false );
  else
    m_comboDefaultBootLabel->setCurrentItem( 0 );  // default

  s = c->readEntry("audio_background");
  if( !s.isEmpty() && s != "default" && m_comboAudioBackground->contains(s) )
    m_comboAudioBackground->setCurrentItem( s, false );
  else
    m_comboAudioBackground->setCurrentItem( 0 ); // default

  s = c->readEntry("keyboard_layout");
  m_keyboardLangHelper->setLanguage( s == "default" ? TQString() : s );

  m_checkShutdown->setChecked( c->readBoolEntry( "shutdown", false) );
  m_checkReboot->setChecked( c->readBoolEntry( "reboot", false ) );
  m_checkEject->setChecked( c->readBoolEntry( "eject", false ) );
  m_checkRandomPlay->setChecked( c->readBoolEntry( "random_play", false ) );
  m_checkNoDma->setChecked( c->readBoolEntry( "no_dma", false ) );
}


void K3bMovixOptionsWidget::saveConfig( TDEConfigBase* c )
{
  if( m_comboSubtitleFontset->currentItem() == 0 )
    c->writeEntry( "subtitle_fontset", "none" );
  else
    c->writeEntry( "subtitle_fontset", m_comboSubtitleFontset->currentText() );

  c->writeEntry( "loop", m_spinLoop->value() );
  c->writeEntry( "additional_mplayer_options", m_editAdditionalMplayerOptions->text() );
  c->writeEntry( "unwanted_mplayer_options", m_editUnwantedMplayerOptions->text() );

  if( m_comboBootMessageLanguage->currentItem() == 0 )
    c->writeEntry( "boot_message_language", "default" );
  else
    c->writeEntry( "boot_message_language", m_helpLangHelper->selectedLanguage() );

  if( m_comboDefaultBootLabel->currentItem() == 0 )
    c->writeEntry( "default_boot_label", "default" );
  else
    c->writeEntry( "default_boot_label", m_comboDefaultBootLabel->currentText() );

  if( m_comboAudioBackground->currentItem() == 0 )
    c->writeEntry( "audio_background", "default" );
  else
    c->writeEntry( "audio_background", m_comboAudioBackground->currentText() );

  if( m_comboKeyboardLayout->currentItem() == 0 )
    c->writeEntry( "keyboard_layout", "default" );
  else
    c->writeEntry( "keyboard_layout", m_keyboardLangHelper->selectedLanguage() );

  c->writeEntry( "shutdown", m_checkShutdown->isChecked() );
  c->writeEntry( "reboot", m_checkReboot->isChecked() );
  c->writeEntry( "eject", m_checkEject->isChecked() );
  c->writeEntry( "random_play", m_checkRandomPlay->isChecked() );
  c->writeEntry( "no_dma", m_checkNoDma->isChecked() );
}

#include "k3bmovixoptionswidget.moc"

