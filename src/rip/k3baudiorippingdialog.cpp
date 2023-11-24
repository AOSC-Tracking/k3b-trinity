/*
 *
 * $Id: k3baudiorippingdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3baudiorippingdialog.h"
#include "k3baudioripjob.h"
#include "k3bpatternparser.h"
#include "k3bcddbpatternwidget.h"
#include "k3baudioconvertingoptionwidget.h"

#include <k3bjobprogressdialog.h>
#include <k3bcore.h>
#include <k3bglobals.h>
#include <k3btrack.h>
#include <k3bstdguiitems.h>
#include <k3bfilesysteminfo.h>
#include <k3bpluginmanager.h>
#include <k3baudioencoder.h>

#include <kcombobox.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdelistview.h>
#include <kurlrequester.h>
#include <tdefiledialog.h>
#include <tdeio/global.h>
#include <kiconloader.h>
#include <kstdguiitem.h>
#include <kdebug.h>
#include <tdemessagebox.h>
#include <kurllabel.h>

#include <tqgroupbox.h>
#include <tqheader.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqdir.h>
#include <tqstringlist.h>
#include <tqmessagebox.h>
#include <tqfont.h>
#include <tqhbox.h>
#include <tqtoolbutton.h>
#include <tqtabwidget.h>
#include <tqspinbox.h>
#include <tqptrlist.h>
#include <tqintdict.h>
#include <tqpair.h>
#include <tqvalidator.h>


class K3bAudioRippingDialog::Private
{
public:
  Private() {
  }

  TQValueVector<TQString> filenames;
  TQString playlistFilename;
  K3bFileSystemInfo fsInfo;
};


K3bAudioRippingDialog::K3bAudioRippingDialog(const K3bDevice::Toc& toc, 
					     K3bDevice::Device* device,
					     const K3bCddbResultEntry& entry, 
					     const TQValueList<int>& tracks,
					     TQWidget *parent, const char *name )
  : K3bInteractionDialog( parent, name,
			  TQString(),
			  TQString(),
			  START_BUTTON|CANCEL_BUTTON,
			  START_BUTTON,
			  "Audio Ripping" ), // config group
    m_toc( toc ), 
    m_device( device ),
    m_cddbEntry( entry ), 
    m_trackNumbers( tracks )
{
  d = new Private();

  setupGui();
  setupContextHelp();

  K3b::Msf length;
  for( TQValueList<int>::const_iterator it = m_trackNumbers.begin();
       it != m_trackNumbers.end(); ++it ) {
    length += m_toc[*it-1].length();
  }
  setTitle( i18n("CD Ripping"), 
	    i18n("1 track (%1)", "%n tracks (%1)", 
		 m_trackNumbers.count()).arg(length.toString()) );
}


K3bAudioRippingDialog::~K3bAudioRippingDialog()
{
  delete d;
}


void K3bAudioRippingDialog::setupGui()
{
  TQWidget *frame = mainWidget();
  TQGridLayout* Form1Layout = new TQGridLayout( frame );
  Form1Layout->setSpacing( KDialog::spacingHint() );
  Form1Layout->setMargin( 0 );

  m_viewTracks = new TDEListView( frame, "m_viewTracks" );
  m_viewTracks->addColumn(i18n( "Filename") );
  m_viewTracks->addColumn(i18n( "Length") );
  m_viewTracks->addColumn(i18n( "File Size") );
  m_viewTracks->addColumn(i18n( "Type") );
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


  // setup advanced page
  // -------------------------------------------------------------------------------------------
  TQWidget* advancedPage = new TQWidget( mainTab );
  TQGridLayout* advancedPageLayout = new TQGridLayout( advancedPage );
  advancedPageLayout->setMargin( marginHint() );
  advancedPageLayout->setSpacing( spacingHint() );
  mainTab->addTab( advancedPage, i18n("Advanced") );

  m_comboParanoiaMode = K3bStdGuiItems::paranoiaModeComboBox( advancedPage );
  m_spinRetries = new TQSpinBox( advancedPage );
  m_checkIgnoreReadErrors = new TQCheckBox( i18n("Ignore read errors"), advancedPage );
  m_checkUseIndex0 = new TQCheckBox( i18n("Don't read pregaps"), advancedPage );

  advancedPageLayout->addWidget( new TQLabel( i18n("Paranoia mode:"), advancedPage ), 0, 0 );
  advancedPageLayout->addWidget( m_comboParanoiaMode, 0, 1 );
  advancedPageLayout->addWidget( new TQLabel( i18n("Read retries:"), advancedPage ), 1, 0 );
  advancedPageLayout->addWidget( m_spinRetries, 1, 1 );
  advancedPageLayout->addMultiCellWidget( m_checkIgnoreReadErrors, 2, 2, 0, 1 );
  advancedPageLayout->addMultiCellWidget( m_checkUseIndex0, 3, 3, 0, 1 );
  advancedPageLayout->setRowStretch( 4, 1 );
  advancedPageLayout->setColStretch( 2, 1 );

  // -------------------------------------------------------------------------------------------


  Form1Layout->addWidget( m_viewTracks, 0, 0 );
  Form1Layout->addWidget( mainTab, 1, 0 );
  Form1Layout->setRowStretch( 0, 1 );

  setStartButtonText( i18n( "Start Ripping" ), i18n( "Starts copying the selected tracks") );
  
  connect( m_checkUseIndex0, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(refresh()) );
  connect( m_optionWidget, TQT_SIGNAL(changed()), this, TQT_SLOT(refresh()) );
}


void K3bAudioRippingDialog::setupContextHelp()
{
  TQToolTip::add( m_spinRetries, i18n("Maximal number of read retries") );
  TQWhatsThis::add( m_spinRetries, i18n("<p>This specifies the maximum number of retries to "
				       "read a sector of audio data from the cd. After that "
				       "K3b will either skip the sector if the <em>Ignore Read Errors</em> "
				       "option is enabled or stop the process.") );
  TQToolTip::add( m_checkUseIndex0, i18n("Do not read the pregaps at the end of every track") );
  TQWhatsThis::add( m_checkUseIndex0, i18n("<p>If this option is checked K3b will not rip the audio "
					  "data in the pregaps. Most audio tracks contain an empty "
					  "pregap which does not belong to the track itself.</p>"
					  "<p>Although the default behaviour of nearly all ripping "
					  "software is to include the pregaps for most CDs it makes more "
					  "sense to ignore them. When creating a K3b audio project you "
					  "will regenerate these pregaps anyway.</p>") );
}


void K3bAudioRippingDialog::init()
{
  refresh();
}


void K3bAudioRippingDialog::slotStartClicked()
{
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
    if( KMessageBox::questionYesNoList( this, 
					i18n("Do you want to overwrite these files?"),
					filesToOverwrite,
					i18n("Files Exist"), i18n("Overwrite"), KStdGuiItem::cancel() ) == KMessageBox::No )
      return;


  // prepare list of tracks to rip
  TQValueVector<TQPair<int, TQString> > tracksToRip;
  unsigned int i = 0;
  for( TQValueList<int>::const_iterator trackIt = m_trackNumbers.begin();
       trackIt != m_trackNumbers.end(); ++trackIt ) {
    tracksToRip.append( qMakePair( *trackIt, d->filenames[(m_optionWidget->createSingleFile() ? 0 : i)] ) );
    ++i;
  }

  K3bJobProgressDialog ripDialog( parentWidget(), "Ripping" );

  K3bAudioEncoder* encoder = m_optionWidget->encoder();
  K3bAudioRipJob* job = new K3bAudioRipJob( &ripDialog, TQT_TQOBJECT(this) );
  job->setDevice( m_device );
  job->setCddbEntry( m_cddbEntry );
  job->setTracksToRip( tracksToRip );
  job->setParanoiaMode( m_comboParanoiaMode->currentText().toInt() );
  job->setMaxRetries( m_spinRetries->value() );
  job->setNeverSkip( !m_checkIgnoreReadErrors->isChecked() );
  job->setSingleFile( m_optionWidget->createSingleFile() );
  job->setWriteCueFile( m_optionWidget->createCueFile() );
  job->setEncoder( encoder );
  job->setWritePlaylist( m_optionWidget->createPlaylist() );
  job->setPlaylistFilename( d->playlistFilename );
  job->setUseRelativePathInPlaylist( m_optionWidget->playlistRelativePath() );
  job->setUseIndex0( m_checkUseIndex0->isChecked() );
  if( encoder )
    job->setFileType( m_optionWidget->extension() );

  hide();
  ripDialog.startJob(job);

  kdDebug() << "(K3bAudioRippingDialog) deleting ripjob." << endl;
  delete job;

  close();
}


void K3bAudioRippingDialog::refresh()
{
  m_viewTracks->clear();
  d->filenames.clear();

  TQString baseDir = K3b::prepareDir( m_optionWidget->baseDir() );
  d->fsInfo.setPath( baseDir );

  TDEIO::filesize_t overallSize = 0;

  if( m_optionWidget->createSingleFile() ) {
    long length = 0;
    for( TQValueList<int>::const_iterator it = m_trackNumbers.begin();
	 it != m_trackNumbers.end(); ++it ) {
      length += ( m_checkUseIndex0->isChecked() 
		  ? m_toc[*it-1].realAudioLength().lba()
		  : m_toc[*it-1].length().lba() );
    }

    TQString filename;
    TQString extension;
    long long fileSize = 0;
    if( m_optionWidget->encoder() == 0 ) {
      extension = "wav";
      fileSize = length * 2352 + 44;
    }
    else {
      extension = m_optionWidget->extension();
      fileSize = m_optionWidget->encoder()->fileSize( extension, length );
    }

    if( fileSize > 0 )
      overallSize = fileSize;

    if( (int)m_cddbEntry.titles.count() >= 1 ) {
      filename = K3bPatternParser::parsePattern( m_cddbEntry, 1,
						 m_patternWidget->filenamePattern(),
						 m_patternWidget->replaceBlanks(),
						 m_patternWidget->blankReplaceString() );
    }
    else {
      filename = i18n("Album");
    }

    filename = d->fsInfo.fixupPath( filename );

    (void)new TDEListViewItem( m_viewTracks,
			     m_viewTracks->lastItem(),
			     filename + "." + extension,
			     K3b::Msf(length).toString(),
			     fileSize < 0 ? i18n("unknown") : TDEIO::convertSize( fileSize ),
			     i18n("Audio") );
    d->filenames.append( baseDir + "/" + filename + "." + extension );

    if( m_optionWidget->createCueFile() )
      (void)new TDEListViewItem( m_viewTracks,
			       m_viewTracks->lastItem(),
			       filename + ".cue",
			       "-",
			       "-",
			       i18n("Cue-file") );
  }
  else {
    for( TQValueList<int>::const_iterator it = m_trackNumbers.begin();
	 it != m_trackNumbers.end(); ++it ) {
      int index = *it - 1;

      TQString extension;
      long long fileSize = 0;
      K3b::Msf trackLength = ( m_checkUseIndex0->isChecked() 
			       ? m_toc[index].realAudioLength()
			       : m_toc[index].length() );
      if( m_optionWidget->encoder() == 0 ) {
	extension = "wav";
	fileSize = trackLength.audioBytes() + 44;
      }
      else {
	extension = m_optionWidget->extension();
	fileSize = m_optionWidget->encoder()->fileSize( extension, trackLength );
      }

      if( fileSize > 0 )
	overallSize += fileSize;

      if( m_toc[index].type() == K3bTrack::DATA ) {
	extension = ".iso";
	continue;  // TODO: find out how to rip the iso data
      }


      TQString filename;

      if( (int)m_cddbEntry.titles.count() >= *it ) {
	filename = K3bPatternParser::parsePattern( m_cddbEntry, *it,
						   m_patternWidget->filenamePattern(),
						   m_patternWidget->replaceBlanks(),
						   m_patternWidget->blankReplaceString() ) + "." + extension;
      }
      else {
	filename = i18n("Track%1").arg( TQString::number( *it ).rightJustify( 2, '0' ) ) + "." + extension;
      }

      filename = d->fsInfo.fixupPath( filename );

      (void)new TDEListViewItem( m_viewTracks,
			       m_viewTracks->lastItem(),
			       filename,
			       trackLength.toString(),
			       fileSize < 0 ? i18n("unknown") : TDEIO::convertSize( fileSize ),
			       (m_toc[index].type() == K3bTrack::AUDIO ? i18n("Audio") : i18n("Data") ) );

      d->filenames.append( baseDir + "/" + filename );
    }
  }

  // create playlist item
  if( m_optionWidget->createPlaylist() ) {
    TQString filename = K3bPatternParser::parsePattern( m_cddbEntry, 1,
						       m_patternWidget->playlistPattern(),
						       m_patternWidget->replaceBlanks(),
						       m_patternWidget->blankReplaceString() ) + ".m3u";

    (void)new TDEListViewItem( m_viewTracks,
			     m_viewTracks->lastItem(),
			     filename,
			     "-",
			     "-",
			     i18n("Playlist") );
    
    d->playlistFilename = d->fsInfo.fixupPath( baseDir + "/" + filename );
  }

  if( overallSize > 0 )
    m_optionWidget->setNeededSize( overallSize );
  else
    m_optionWidget->setNeededSize( 0 );
}


void K3bAudioRippingDialog::setStaticDir( const TQString& path )
{
  m_optionWidget->setBaseDir( path );
}


void K3bAudioRippingDialog::loadK3bDefaults()
{
  m_comboParanoiaMode->setCurrentItem( 0 );
  m_spinRetries->setValue(5);
  m_checkIgnoreReadErrors->setChecked( true );
  m_checkUseIndex0->setChecked( false );
  
  m_optionWidget->loadDefaults();
  m_patternWidget->loadDefaults();

  refresh();
}

void K3bAudioRippingDialog::loadUserDefaults( TDEConfigBase* c )
{
  m_comboParanoiaMode->setCurrentItem( c->readNumEntry( "paranoia_mode", 0 ) );
  m_spinRetries->setValue( c->readNumEntry( "read_retries", 5 ) );
  m_checkIgnoreReadErrors->setChecked( !c->readBoolEntry( "never_skip", true ) );
  m_checkUseIndex0->setChecked( c->readBoolEntry( "use_index0", false ) );

  m_optionWidget->loadConfig( c );
  m_patternWidget->loadConfig( c );

  refresh();
}

void K3bAudioRippingDialog::saveUserDefaults( TDEConfigBase* c )
{
  c->writeEntry( "paranoia_mode", m_comboParanoiaMode->currentText().toInt() );
  c->writeEntry( "read_retries", m_spinRetries->value() );
  c->writeEntry( "never_skip", !m_checkIgnoreReadErrors->isChecked() );
  c->writeEntry( "use_index0", m_checkUseIndex0->isChecked() );

  m_optionWidget->saveConfig( c );
  m_patternWidget->saveConfig( c );
}


#include "k3baudiorippingdialog.moc"
