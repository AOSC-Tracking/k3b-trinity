/*
*
* $Id: k3bvideocdrippingdialog.cpp 640370 2007-03-07 19:44:32Z trueg $
* Copyright (C) 2003 Christian Kvasny <chris@k3b.org>
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


// kde include
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// qt includes
#include <tqgroupbox.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqstringlist.h>
#include <tqhbox.h>

// k3b includes
#include "k3bvideocdrippingdialog.h"
#include "k3bvideocdrip.h"

#include <k3bjobprogressdialog.h>
#include <k3bcore.h>
#include <k3bglobals.h>
#include <k3bstdguiitems.h>

K3bVideoCdRippingDialog::K3bVideoCdRippingDialog( K3bVideoCdRippingOptions* options, TQWidget* parent, const char* name )
  : K3bInteractionDialog( parent, name,
			  i18n( "Video CD Ripping" ),
			  TQString(),
			  START_BUTTON|CANCEL_BUTTON,
			  START_BUTTON,
			  "Video CD Ripping" ), // config group
    m_videooptions( options )
{
  setupGui();
  setupContextHelp();
}


K3bVideoCdRippingDialog::~K3bVideoCdRippingDialog()
{
}


void K3bVideoCdRippingDialog::setupGui()
{
    TQWidget * frame = mainWidget();
    TQGridLayout* MainLayout = new TQGridLayout( frame );
    MainLayout->setSpacing( KDialog::spacingHint() );
    MainLayout->setMargin( 0 );

    // ---------------------------------------------------- Directory group ---
    TQGroupBox* groupDirectory = new TQGroupBox( 0, Qt::Vertical, i18n( "Destination Directory" ), frame );
    groupDirectory->tqlayout() ->setSpacing( KDialog::spacingHint() );
    groupDirectory->tqlayout() ->setMargin( KDialog::marginHint() );

    TQGridLayout* groupDirectoryLayout = new TQGridLayout( groupDirectory->tqlayout() );
    groupDirectoryLayout->setAlignment( TQt::AlignTop );

    TQLabel* rippathLabel = new TQLabel( i18n( "Rip files to:" ), groupDirectory );
    m_editDirectory = new KURLRequester( groupDirectory, "m_editDirectory" );
    m_editDirectory->setURL( TQDir::homeDirPath() );
    m_editDirectory->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    rippathLabel->setBuddy( m_editDirectory );

    TQHBox* freeSpaceBox = new TQHBox( groupDirectory );
    freeSpaceBox->setSpacing( KDialog::spacingHint() );
    ( void ) new TQLabel( i18n( "Free space in directory:" ), freeSpaceBox, "FreeSpaceLabel" );
    m_labelFreeSpace = new TQLabel( "                       ", freeSpaceBox, "m_labelFreeSpace" );
    m_labelFreeSpace->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

    TQHBox* necessarySizeBox = new TQHBox( groupDirectory );
    necessarySizeBox->setSpacing( KDialog::spacingHint() );
    ( void ) new TQLabel( i18n( "Necessary storage size:" ), necessarySizeBox, "StorSize" );
    m_labelNecessarySize = new TQLabel( "                        ", necessarySizeBox, "m_labelNecessarySize" );
    m_labelNecessarySize->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );


    groupDirectoryLayout->addWidget( rippathLabel, 0, 0 );
    groupDirectoryLayout->addWidget( m_editDirectory, 0, 1 );
    groupDirectoryLayout->addWidget( freeSpaceBox, 1, 1 );
    groupDirectoryLayout->addWidget( necessarySizeBox, 2, 1 );

    // ---------------------------------------------------- Options group ---
    TQGroupBox* groupOptions = new TQGroupBox( 4, Qt::Vertical, i18n( "Settings" ), frame );

    m_ignoreExt = new TQCheckBox( i18n( "Ignore /EXT/PSD_X.VCD" ), groupOptions );

    m_sector2336 = new TQCheckBox( i18n( "Use 2336 byte sector mode for image file" ), groupOptions );
    // Only available for image file ripping
    m_sector2336->setEnabled( false );
    m_sector2336->setChecked( false );

    m_extractXML = new TQCheckBox( i18n( "Extract XML structure" ), groupOptions );


    MainLayout->addWidget( groupDirectory, 0, 0 );
    MainLayout->addWidget( groupOptions, 1, 0 );
    MainLayout->setRowStretch( 0, 1 );

    setStartButtonText( i18n( "Start Ripping" ), i18n( "Starts extracting the selected VideoCd tracks" ) );
    // ----------------------------------------------------------------------------------

    connect( m_editDirectory, TQT_SIGNAL(textChanged(const TQString&)), this, TQT_SLOT(slotUpdateFreeSpace()) );

    m_labelNecessarySize ->setText( KIO::convertSize( m_videooptions ->getVideoCdSize() ) );    
}


void K3bVideoCdRippingDialog::setupContextHelp()
{
    TQToolTip::add( m_labelFreeSpace, i18n("Free space on destination directory: %1").arg( m_editDirectory ->url() ) );

    TQToolTip::add( m_labelNecessarySize, i18n("Necessary space for extracted files") );

    TQToolTip::add( m_ignoreExt, i18n("Ignore extended PSD") );
    TQWhatsThis::add( m_ignoreExt, i18n("<p>Ignore extended PSD (located in the ISO-9660 filesystem under `/EXT/PSD_X.VCD') and use the <em>standard</em> PSD.</p>") );

    TQToolTip::add( m_sector2336, i18n("Assume a 2336-byte sector mode") );
    TQWhatsThis::add( m_sector2336, i18n("<p>This option only makes sense if you are reading from a BIN CD disk image. This indicates to `vcdxrip' to assume a 2336-byte sector mode for image file.</p>"
                                                            "<b>Note: This option is slated to disappear.</b>") );

    TQToolTip::add( m_extractXML, i18n("Create XML description file.") );
    TQWhatsThis::add( m_extractXML, i18n("<p>This option creates an XML description file with all video CD information.</p>"
					"<p>This file will always contain all of the information.</p>"
					"<p>Example: If you only extract sequences, the description file will also hold the information for files and segments.</p>"
					"<p>The filename is the same as the video CD name, with a .xml extension. The default is VIDEOCD.xml.</p>") );
}

void K3bVideoCdRippingDialog::slotStartClicked()
{

    TQStringList filesExists;
    TQDir d;
    d.setPath( m_editDirectory ->url() );
    if( !d.exists() ) {
      if( KMessageBox::warningYesNo( this, i18n("Image folder '%1' does not exist. Do you want K3b to create it?").arg( m_editDirectory->url() ) )
	  == KMessageBox::Yes ) {
	if( !KStandardDirs::makeDir( m_editDirectory->url() ) ) {
	  KMessageBox::error( this, i18n("Failed to create folder '%1'.").arg( m_editDirectory->url() ) );
	  return;
	}
      }
    }
    const TQFileInfoList* list = d.entryInfoList();
    TQFileInfoListIterator it( *list );
    TQFileInfo* fi;
    while ( ( fi = it.current() ) != 0 ) {
        if ( fi ->fileName() != "." && fi ->fileName() != ".." )
            filesExists.append( TQString( "%1 (%2)" ).arg( TQFile::encodeName( fi ->fileName() ).data() ).arg( KIO::convertSize( fi ->size() ) ) );
        ++it;
    }

    if( !filesExists.isEmpty() )
        if( KMessageBox::questionYesNoList( this,
                                i18n("Continue although the folder is not empty?"),
                                filesExists,
                                i18n("Files Exist"),KStdGuiItem::cont(),KStdGuiItem::cancel() ) == KMessageBox::No )
        return;

    m_videooptions ->setVideoCdIgnoreExt( m_ignoreExt ->isChecked() );
    m_videooptions ->setVideoCdSector2336( m_sector2336 ->isChecked() );
    m_videooptions ->setVideoCdExtractXml( m_extractXML ->isChecked() );
    m_videooptions ->setVideoCdDestination( m_editDirectory ->url() );

    K3bJobProgressDialog ripDialog( kapp->mainWidget(), "Ripping" );
    K3bVideoCdRip * rip = new K3bVideoCdRip( &ripDialog, m_videooptions );

    hide();
    ripDialog.startJob( rip );

    delete rip;

    close();
}

void K3bVideoCdRippingDialog::slotFreeSpace(const TQString&,
						  unsigned long,
						  unsigned long,
						  unsigned long kbAvail)
{
    m_labelFreeSpace->setText( KIO::convertSizeFromKB(kbAvail) );

    m_freeSpace = kbAvail;

    if( m_freeSpace < m_videooptions ->getVideoCdSize() /1024 )
        m_labelNecessarySize->setPaletteForegroundColor( red );
    else
        m_labelNecessarySize->setPaletteForegroundColor( m_labelFreeSpace->paletteForegroundColor() );

    TQTimer::singleShot( 1000, this, TQT_SLOT(slotUpdateFreeSpace()) );
}


void K3bVideoCdRippingDialog::slotUpdateFreeSpace()
{
    TQString path = m_editDirectory->url();

    if( !TQFile::exists( path ) )
        path.truncate( path.findRev('/') );

    unsigned long size, avail;
    if( K3b::kbFreeOnFs( path, size, avail ) )
        slotFreeSpace( path, size, 0, avail );
    else
        m_labelFreeSpace->setText("-");
}

void K3bVideoCdRippingDialog::loadK3bDefaults()
{
    m_editDirectory->setURL( TQDir::homeDirPath() );
    m_ignoreExt ->setChecked( false );
    m_sector2336 ->setChecked( false );
    m_extractXML ->setChecked( false );

    slotUpdateFreeSpace();
}

void K3bVideoCdRippingDialog::loadUserDefaults( KConfigBase* c )
{
    m_editDirectory ->setURL( c->readPathEntry( "last ripping directory", TQDir::homeDirPath() ) );
    m_ignoreExt ->setChecked( c->readBoolEntry( "ignore ext", false ) );
    m_sector2336 ->setChecked( c->readBoolEntry( "sector 2336", false ) );
    m_extractXML ->setChecked( c->readBoolEntry( "extract xml", false ) );

    slotUpdateFreeSpace();
}

void K3bVideoCdRippingDialog::saveUserDefaults( KConfigBase* c )
{
    c->writePathEntry( "last ripping directory", m_editDirectory->url() );
    c->writeEntry( "ignore ext", m_ignoreExt ->isChecked( ) );
    c->writeEntry( "sector 2336", m_sector2336 ->isChecked( ) );
    c->writeEntry( "extract xml", m_extractXML ->isChecked( ) );
}

#include "k3bvideocdrippingdialog.moc"
