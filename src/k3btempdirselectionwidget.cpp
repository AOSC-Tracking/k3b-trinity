/*
 *
 * $Id: k3btempdirselectionwidget.cpp 690207 2007-07-20 10:40:19Z trueg $
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


#include "k3btempdirselectionwidget.h"
#include <k3bglobals.h>
#include <k3bcore.h>

#include <tqlabel.h>
#include <tqgroupbox.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqhbox.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqfileinfo.h>

#include <tdeconfig.h>
#include <tdelocale.h>
#include <tdefiledialog.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <tdeio/global.h>
#include <tdeconfig.h>
#include <klineedit.h>


K3bTempDirSelectionWidget::K3bTempDirSelectionWidget( TQWidget *parent, const char *name )
  : TQGroupBox( 4, TQt::Vertical, parent, name ),
    m_labelCdSize(0),
    m_defaultImageFileName( "k3b_image.iso" )
{
  layout()->setSpacing( KDialog::spacingHint() );
  layout()->setMargin( KDialog::marginHint() );

  m_imageFileLabel = new TQLabel( this );
  m_editDirectory = new KURLRequester( this, "m_editDirectory" );

  m_imageFileLabel->setBuddy( m_editDirectory );

  TQHBox* freeTempSpaceBox = new TQHBox( this );
  freeTempSpaceBox->setSpacing( KDialog::spacingHint() );
  (void)new TQLabel( i18n( "Free space in temporary directory:" ), freeTempSpaceBox, "TextLabel2" );
  m_labelFreeSpace = new TQLabel( "                       ",freeTempSpaceBox, "m_labelFreeSpace" );
  m_labelFreeSpace->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );


  connect( m_editDirectory, TQ_SIGNAL(openFileDialog(KURLRequester*)),
	   this, TQ_SLOT(slotTempDirButtonPressed(KURLRequester*)) );
  connect( m_editDirectory, TQ_SIGNAL(textChanged(const TQString&)),
	   this, TQ_SLOT(slotUpdateFreeTempSpace()) );
  connect( m_editDirectory->lineEdit(), TQ_SIGNAL(lostFocus()),
           this, TQ_SLOT(slotFixTempPath()) );

  // choose a default
  setSelectionMode( DIR );

  m_editDirectory->setURL( K3b::defaultTempPath() );
  slotUpdateFreeTempSpace();

  // ToolTips
  // --------------------------------------------------------------------------------
  TQToolTip::add( m_editDirectory, i18n("The directory in which to save the image files") );

  // What's This info
  // --------------------------------------------------------------------------------
  TQWhatsThis::add( m_editDirectory, i18n("<p>This is the directory in which K3b will save the <em>image files</em>."
					 "<p>Please make sure that it resides on a partition that has enough free space.") );
}


K3bTempDirSelectionWidget::~K3bTempDirSelectionWidget()
{
}


unsigned long K3bTempDirSelectionWidget::freeTempSpace() const
{
  TQString path = m_editDirectory->url();

  if( !TQFile::exists( path ) )
    path.truncate( path.findRev('/') );

  unsigned long size;
  K3b::kbFreeOnFs( path, size, m_freeTempSpace );

  return m_freeTempSpace;
}


void K3bTempDirSelectionWidget::slotUpdateFreeTempSpace()
{
  // update the temp space
  freeTempSpace();

  m_labelFreeSpace->setText( TDEIO::convertSizeFromKB(m_freeTempSpace) );

  if( m_labelCdSize ) {
    if( m_freeTempSpace < m_requestedSize/1024 )
      m_labelCdSize->setPaletteForegroundColor( red );
    else
      m_labelCdSize->setPaletteForegroundColor( m_labelFreeSpace->paletteForegroundColor() );
  }
  TQTimer::singleShot( 1000, this, TQ_SLOT(slotUpdateFreeTempSpace()) );
}


void K3bTempDirSelectionWidget::slotTempDirButtonPressed( KURLRequester* r )
{
  // set the correct mode for the filedialog
  if( m_mode == DIR ) {
    r->setCaption( i18n("Select Temporary Directory") );
    r->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
  }
  else {
    r->setCaption( i18n("Select Temporary File") );
    r->setMode( KFile::File | KFile::LocalOnly );
  }
}


void K3bTempDirSelectionWidget::setTempPath( const TQString& dir )
{
  m_editDirectory->setURL( dir );
  slotUpdateFreeTempSpace();
}


TQString K3bTempDirSelectionWidget::tempPath() const
{
  TQFileInfo fi( m_editDirectory->url() );

  if( fi.exists() ) {
    if( m_mode == DIR ) {
      if( fi.isDir() )
	return fi.absFilePath();
      else
	return fi.dirPath( true );
    }
    else {
      if( fi.isFile() )
	return fi.absFilePath();
      else
	return fi.absFilePath() + "/k3b_image.iso";
    }
  }
  else {
    return fi.absFilePath();
  }
}


TQString K3bTempDirSelectionWidget::plainTempPath() const
{
  return m_editDirectory->url();
}


TQString K3bTempDirSelectionWidget::tempDirectory() const
{
  TQString td( m_editDirectory->url() );

  // remove a trailing slash
  while( !td.isEmpty() && td[td.length()-1] == '/' )
    td.truncate( td.length()-1 );

  TQFileInfo fi( td );
  if( fi.exists() && fi.isDir() )
    return td + "/";

  // now we treat the last section as a filename and return the path
  // in front of it
  td.truncate( td.findRev( '/' ) + 1 );
  return td;
}


void K3bTempDirSelectionWidget::setSelectionMode( int mode )
{
  m_mode = mode;

  if( m_mode == DIR ) {
    m_imageFileLabel->setText( i18n( "Wri&te image files to:" ) );
    setTitle( i18n("Temporary Directory") );
  }
  else {
    m_imageFileLabel->setText( i18n( "Wri&te image file to:" ) );
    setTitle( i18n("Temporary File") );
  }
}


void K3bTempDirSelectionWidget::setNeededSize( TDEIO::filesize_t bytes )
{
  m_requestedSize = bytes;
  if( !m_labelCdSize ) {
    TQHBox* cdSizeBox = new TQHBox( this );
    cdSizeBox->setSpacing( KDialog::spacingHint() );
    (void)new TQLabel( i18n( "Size of project:" ), cdSizeBox, "TextLabel4" );
    m_labelCdSize = new TQLabel( TDEIO::convertSize(bytes), cdSizeBox, "m_labelCdSize" );
    m_labelCdSize->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );
  }
  m_labelCdSize->setText( TDEIO::convertSize(bytes) );
}


void K3bTempDirSelectionWidget::saveConfig()
{
  TDEConfigGroup grp( k3bcore->config(), "General Options" );
  grp.writePathEntry( "Temp Dir", tempDirectory() );
}


void K3bTempDirSelectionWidget::readConfig( TDEConfigBase* c )
{
  setTempPath( c->readPathEntry( "image path", K3b::defaultTempPath() ) );
}


void K3bTempDirSelectionWidget::saveConfig( TDEConfigBase* c )
{
  c->writePathEntry( "image path", tempPath() );
}


void K3bTempDirSelectionWidget::setDefaultImageFileName( const TQString& name )
{
    if ( !name.isEmpty() ) {
        bool changeImageName = false;
        if ( selectionMode() == FILE ) {
            if ( plainTempPath().section( '/', -1 ) == m_defaultImageFileName ) {
                changeImageName = true;
            }
        }

        m_defaultImageFileName = name;
        if ( !m_defaultImageFileName.contains( '.' ) ) {
            m_defaultImageFileName += ".iso";
        }
        fixTempPath( changeImageName );
    }
}


void K3bTempDirSelectionWidget::slotFixTempPath()
{
    fixTempPath( false );
}


void K3bTempDirSelectionWidget::fixTempPath( bool forceNewImageName )
{
    // if in file selection mode and no image file is specified or
    // forceNewImageName is true set the default image file name
    if ( selectionMode() == FILE ) {
        if ( forceNewImageName ||
             TQFileInfo( plainTempPath() ).isDir() ) {
            setTempPath( tempDirectory() + m_defaultImageFileName );
        }
    }
}

#include "k3btempdirselectionwidget.moc"
