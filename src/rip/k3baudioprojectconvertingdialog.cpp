/*
 *
 * $Id: k3baudioprojectconvertingdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3baudioprojectconvertingdialog.h"
#include "k3baudioprojectconvertingthread.h"
#include "k3bpatternparser.h"
#include "k3bcddbpatternwidget.h"
#include "k3baudioconvertingoptionwidget.h"

#include <k3baudiodoc.h>
#include <k3baudioview.h>
#include <k3baudiotrackplayer.h>
#include <k3baudiotrack.h>
#include <k3bjobprogressdialog.h>
#include <k3bcore.h>
#include <k3bglobals.h>
#include <k3blistview.h>
#include <k3baudioencoder.h>
#include <k3bthreadjob.h>

#include <kcombobox.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdelistview.h>
#include <kurlrequester.h>
#include <tdefiledialog.h>
#include <tdeio/global.h>
#include <kdebug.h>
#include <tdemessagebox.h>

#include <tqgroupbox.h>
#include <tqheader.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqdir.h>
#include <tqstringlist.h>
#include <tqtabwidget.h>



class K3bAudioProjectConvertingDialog::Private
{
public:
  Private() {
  }

  TQValueVector<TQString> filenames;
  TQString playlistFilename;
  TQString cueFilename;
};


K3bAudioProjectConvertingDialog::K3bAudioProjectConvertingDialog( K3bAudioDoc* doc, TQWidget *parent, const char *name )
  : K3bInteractionDialog( parent, name,
			  TQString(),
			  TQString(),
			  START_BUTTON|CANCEL_BUTTON,
			  START_BUTTON,
			  "Audio Project Converting" ), // config group
    m_doc(doc)
{
  d = new Private();

  setupGui();

  setTitle( i18n("Audio Project Conversion"), 
	    i18n("1 track (%1)", "%n tracks (%1)", 
		 m_doc->numOfTracks()).arg(m_doc->length().toString()) );

  refresh();
}


K3bAudioProjectConvertingDialog::~K3bAudioProjectConvertingDialog()
{
  delete d;
}


void K3bAudioProjectConvertingDialog::setupGui()
{
  TQWidget *frame = mainWidget();
  TQGridLayout* Form1Layout = new TQGridLayout( frame );
  Form1Layout->setSpacing( KDialog::spacingHint() );
  Form1Layout->setMargin( 0 );

  m_viewTracks = new K3bListView( frame, "m_viewTracks" );
  m_viewTracks->addColumn(i18n( "Filename (relative to base directory)") );
  m_viewTracks->addColumn(i18n( "Length") );
  m_viewTracks->addColumn(i18n( "File Size") );
  m_viewTracks->setSorting(-1);
  m_viewTracks->setAllColumnsShowFocus(true);
  m_viewTracks->setFullWidth(true);

  TQTabWidget* mainTab = new TQTabWidget( frame );

  m_optionWidget = new K3bAudioConvertingOptionWidget( mainTab );
  mainTab->addTab( m_optionWidget, i18n("Settings") );


  // setup filename pattern page
  // -------------------------------------------------------------------------------------------
  m_patternWidget = new K3bCddbPatternWidget( mainTab );
  mainTab->addTab( m_patternWidget, i18n("File Naming") );
  connect( m_patternWidget, TQT_SIGNAL(changed()), this, TQT_SLOT(refresh()) );

  Form1Layout->addWidget( m_viewTracks, 0, 0 );
  Form1Layout->addWidget( mainTab, 1, 0 );
  Form1Layout->setRowStretch( 0, 1 );

  connect( m_optionWidget, TQT_SIGNAL(changed()), this, TQT_SLOT(refresh()) );
}


void K3bAudioProjectConvertingDialog::slotStartClicked()
{
  // make sure we have the tracks just for ourselves
  static_cast<K3bAudioView*>(m_doc->view())->player()->stop();

  // check if all filenames differ
  if( d->filenames.count() > 1 ) {
    bool differ = true;
    // the most stupid version to compare but most cds have about 12 tracks
    // that's a size where algorithms do not need any optimization! ;)
    for( unsigned int i = 0; i < d->filenames.count(); ++i ) {
      for( unsigned int j = i+1; j < d->filenames.count(); ++j )
	if( d->filenames[i] == d->filenames[j] ) {
	  differ = false;
	  break;
	}
    }

    if( !differ ) {
      KMessageBox::sorry( this, i18n("Please check the naming pattern. All filenames need to be unique.") );
      return;
    }
  }

  // check if we need to overwrite some files...
  TQListViewItemIterator it( m_viewTracks );
  TQStringList filesToOverwrite;
  for( unsigned int i = 0; i < d->filenames.count(); ++i ) {
    if( TQFile::exists( d->filenames[i] ) )
      filesToOverwrite.append( d->filenames[i] );
  }

  if( m_optionWidget->createPlaylist() && TQFile::exists( d->playlistFilename ) )
    filesToOverwrite.append( d->playlistFilename );

  if( !filesToOverwrite.isEmpty() )
    if( KMessageBox::warningContinueCancelList( this, 
					i18n("Do you want to overwrite these files?"),
					filesToOverwrite,
					i18n("Files Exist"), i18n("Overwrite") ) == KMessageBox::Cancel )
      return;


  // just generate a fake m_tracks list for now so we can keep most of the methods
  // like they are in K3bAudioRipThread. This way future combination is easier
  TQValueVector<TQPair<int, TQString> > tracksToRip;
  int i = 0;
  K3bAudioTrack* track = m_doc->firstTrack();
  while( track ) {
    tracksToRip.append( qMakePair( i+1, d->filenames[(m_optionWidget->createSingleFile() ? 0 : i)] ) );
    ++i;
    track = track->next();
  }

  K3bAudioEncoder* encoder = m_optionWidget->encoder();

  K3bAudioProjectConvertingThread* thread = new K3bAudioProjectConvertingThread( m_doc );
  thread->setCddbEntry( createCddbEntryFromDoc( m_doc ) );
  thread->setTracksToRip( tracksToRip );
  thread->setSingleFile( m_optionWidget->createSingleFile() );
  thread->setWriteCueFile( m_optionWidget->createCueFile() );
  thread->setEncoder( encoder );
  thread->setWritePlaylist( m_optionWidget->createPlaylist() );
  thread->setPlaylistFilename( d->playlistFilename );
  thread->setUseRelativePathInPlaylist( m_optionWidget->playlistRelativePath() );
  if( encoder )
    thread->setFileType( m_optionWidget->extension() );

  K3bJobProgressDialog progressDialog( parentWidget() );

  K3bThreadJob job( thread, &progressDialog, TQT_TQOBJECT(this) );

  hide();
  progressDialog.startJob(&job);

  delete thread;

  close();
}


void K3bAudioProjectConvertingDialog::refresh()
{
  m_viewTracks->clear();
  d->filenames.clear();

  // FIXME: this is bad and needs to be improved
  // create a cddb entry from the doc to use in the patternparser
  K3bCddbResultEntry cddbEntry = createCddbEntryFromDoc( m_doc );

  TQString baseDir = K3b::prepareDir( m_optionWidget->baseDir() );

  TQString extension = m_optionWidget->extension();

  TDEIO::filesize_t overallSize = 0;

  if( m_optionWidget->createSingleFile() ) {
    TQString filename;
    long long filesize = 0;
    if( m_optionWidget->encoder() == 0 ) {
      filesize = m_doc->length().audioBytes() + 44;
    }
    else {
      filesize = m_optionWidget->encoder()->fileSize( extension, m_doc->length() );
    }

    if( filesize > 0 )
      overallSize = filesize;

    filename = K3bPatternParser::parsePattern( cddbEntry, 1,
					       m_patternWidget->filenamePattern(),
					       m_patternWidget->replaceBlanks(),
					       m_patternWidget->blankReplaceString() );


    (void)new TDEListViewItem( m_viewTracks,
			     m_viewTracks->lastItem(),
			     filename + "." + extension,
			     m_doc->length().toString(),
			     filesize < 0 ? i18n("unknown") : TDEIO::convertSize( filesize ) );

    d->filenames.append( K3b::fixupPath( baseDir + "/" + filename + "." + extension ) );

    if( m_optionWidget->createCueFile() ) {
      d->cueFilename = K3b::fixupPath( baseDir + "/" + filename + ".cue" );
      (void)new TDEListViewItem( m_viewTracks,
			       m_viewTracks->lastItem(),
			       filename + ".cue",
			       "-",
			       "-",
			       i18n("Cue-file") );
    }
  }
  else {
    K3bAudioTrack* track = m_doc->firstTrack();
    unsigned int i = 1;
    while( track ) {
      long long filesize = 0;
      if( m_optionWidget->encoder() == 0 ) {
	filesize = track->length().audioBytes() + 44;
      }
      else {
	filesize = m_optionWidget->encoder()->fileSize( extension, track->length() );
      }

      if( filesize > 0 )
	overallSize += filesize;

      TQString filename = K3bPatternParser::parsePattern( cddbEntry, i,
							 m_patternWidget->filenamePattern(),
							 m_patternWidget->replaceBlanks(),
							 m_patternWidget->blankReplaceString() ) + "." + extension;

      (void)new TDEListViewItem( m_viewTracks,
			       m_viewTracks->lastItem(),
			       filename,
			       track->length().toString(),
			       filesize < 0 ? i18n("unknown") : TDEIO::convertSize( filesize ) );

      d->filenames.append( K3b::fixupPath( baseDir + "/" + filename ) );

      track = track->next();
      ++i;
    }
  }

  // create playlist item
  if( m_optionWidget->createPlaylist() ) {
    TQString filename = K3bPatternParser::parsePattern( cddbEntry, 1,
						       m_patternWidget->playlistPattern(),
						       m_patternWidget->replaceBlanks(),
						       m_patternWidget->blankReplaceString() ) + ".m3u";

    (void)new TDEListViewItem( m_viewTracks,
			     m_viewTracks->lastItem(),
			     filename,
			     "-",
			     "-",
			     i18n("Playlist") );
    
    d->playlistFilename = K3b::fixupPath( baseDir + "/" + filename );
  }

  if( overallSize > 0 )
    m_optionWidget->setNeededSize( overallSize );
  else
    m_optionWidget->setNeededSize( 0 );
}


void K3bAudioProjectConvertingDialog::setBaseDir( const TQString& path )
{
  m_optionWidget->setBaseDir( path );
}


void K3bAudioProjectConvertingDialog::loadK3bDefaults()
{
  m_optionWidget->loadDefaults();
  m_patternWidget->loadDefaults();

  refresh();
}

void K3bAudioProjectConvertingDialog::loadUserDefaults( TDEConfigBase* c )
{
  m_optionWidget->loadConfig( c );
  m_patternWidget->loadConfig( c );

  refresh();
}


void K3bAudioProjectConvertingDialog::saveUserDefaults( TDEConfigBase* c )
{
  m_optionWidget->saveConfig( c );
  m_patternWidget->saveConfig( c );
}


K3bCddbResultEntry K3bAudioProjectConvertingDialog::createCddbEntryFromDoc( K3bAudioDoc* doc )
{
  K3bCddbResultEntry e;

  // global
  e.cdTitle = doc->title();
  e.cdArtist = doc->artist();
  e.cdExtInfo = doc->cdTextMessage();

  // tracks
  K3bAudioTrack* track = doc->firstTrack();
  while( track ) {
    e.titles.append( track->title() );
    e.artists.append( track->artist() );
    e.extInfos.append( track->cdTextMessage() );

    track = track->next();
  }

  return e;
}

#include "k3baudioprojectconvertingdialog.moc"
