/*
*
* $Id: k3bvcdtrackdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2004 Christian Kvasny <chris@k3b.org>
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

// TQt includes
#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqframe.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqmultilineedit.h>
#include <tqpixmap.h>
#include <tqradiobutton.h>
#include <tqtable.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

// Kde Includes
#include <kiconloader.h>
#include <tdeio/global.h>
#include <tdelocale.h>
#include <kmimetype.h>
#include <knuminput.h>
#include <kurl.h>

// K3b Includes
#include "k3bvcdtrackdialog.h"
#include "k3bvcdtrack.h"
#include <kcutlabel.h>
#include <k3bmsf.h>
#include <k3bglobals.h>
#include <k3bcutcombobox.h>


K3bVcdTrackDialog::K3bVcdTrackDialog( K3bVcdDoc* _doc, TQPtrList<K3bVcdTrack>& tracks, TQPtrList<K3bVcdTrack>& selectedTracks, TQWidget* parent, const char* name )
        : KDialogBase( KDialogBase::Plain, i18n( "Video Track Properties" ), KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply,
                       KDialogBase::Ok, parent, name )
{
    prepareGui();

    setupPbcTab();
    setupPbcKeyTab();
    setupVideoTab();
    setupAudioTab();

    m_tracks = tracks;
    m_selectedTracks = selectedTracks;
    m_vcdDoc = _doc;
    m_numkeysmap.clear();

    if ( !m_selectedTracks.isEmpty() ) {

        K3bVcdTrack * selectedTrack = m_selectedTracks.first();

        m_displayFileName->setText( selectedTrack->fileName() );
        m_displayLength->setText( selectedTrack->duration() );
        m_displaySize->setText( TDEIO::convertSize( selectedTrack->size() ) );
        m_muxrate->setText( i18n( "%1 bit/s" ).arg( selectedTrack->muxrate() ) );

        if ( selectedTrack->isSegment() )
            m_labelMimeType->setPixmap( SmallIcon( "image-x-generic", TDEIcon::SizeMedium ) );
        else
            m_labelMimeType->setPixmap( SmallIcon( "video-x-generic", TDEIcon::SizeMedium ) );

        fillGui();
    }
}

K3bVcdTrackDialog::~K3bVcdTrackDialog()
{}

void K3bVcdTrackDialog::slotOk()
{
    slotApply();
    done( 0 );
}

void K3bVcdTrackDialog::setPbcTrack( K3bVcdTrack* selected, K3bCutComboBox* box, int which )
{
    // TODO: Unset Userdefined on default settings
    kdDebug() << TQString( "K3bVcdTrackDialog::setPbcTrack: currentItem = %1, count = %2" ).arg( box->currentItem() ).arg( m_tracks.count() ) << endl;

    int count = m_tracks.count();

    if ( selected->getPbcTrack( which ) == m_tracks.at( box->currentItem() ) ) {
        if ( selected->getNonPbcTrack( which ) == ( int ) ( box->currentItem() - count ) ) {
            kdDebug() << "K3bVcdTrackDialog::setPbcTrack: not changed, return" << endl;
            return ;
        }
    }

    if ( selected->getPbcTrack( which ) )
        selected->getPbcTrack( which ) ->delFromRevRefList( selected );

    if ( box->currentItem() > count - 1 ) {
        selected->setPbcTrack( which );
        selected->setPbcNonTrack( which, box->currentItem() - count );
    } else {
        selected->setPbcTrack( which, m_tracks.at( box->currentItem() ) );
        m_tracks.at( box->currentItem() ) ->addToRevRefList( selected );
    }

    selected->setUserDefined( which, true );
}

void K3bVcdTrackDialog::slotApply()
{
    // track set
    K3bVcdTrack * selectedTrack = m_selectedTracks.first();

    setPbcTrack( selectedTrack, m_pbc_previous, K3bVcdTrack::PREVIOUS );
    setPbcTrack( selectedTrack, m_pbc_next, K3bVcdTrack::NEXT );
    setPbcTrack( selectedTrack, m_pbc_return, K3bVcdTrack::RETURN );
    setPbcTrack( selectedTrack, m_pbc_default, K3bVcdTrack::DEFAULT );
    setPbcTrack( selectedTrack, m_comboAfterTimeout, K3bVcdTrack::AFTERTIMEOUT );

    selectedTrack->setPlayTime( m_spin_times->value() );
    selectedTrack->setWaitTime( m_spin_waittime->value() );
    selectedTrack->setReactivity( m_check_reactivity->isChecked() );
    selectedTrack->setPbcNumKeys( m_check_usekeys->isChecked() );
    selectedTrack->setPbcNumKeysUserdefined( m_check_overwritekeys->isChecked() );

    // global set
    VcdOptions() ->setPbcEnabled( m_check_pbc->isChecked() );

    // define numeric keys
    selectedTrack->delDefinedNumKey();

    if ( m_check_overwritekeys->isChecked() ) {
        TQListViewItemIterator it( m_list_keys );
        int skiped = 0;
        int startkey = 0;
        while ( it.current() ) {
            if ( it.current() ->text( 1 ).isEmpty() ) {
                skiped++;
            } else {

                if ( startkey > 0 )
                    for ( ; skiped > 0; skiped-- )
                        kdDebug() << "Key " << it.current() ->text( 0 ).toInt() - skiped << " Playing: " << displayName( selectedTrack ) << " (normaly none)" << endl;
                else {
                    skiped = 0;
                    startkey = it.current() ->text( 0 ).toInt();
                }

                TQMap<TQString, K3bVcdTrack*>::Iterator mit;
                mit = m_numkeysmap.find( it.current() ->text( 1 ) );
                if ( mit != m_numkeysmap.end() )
                    if ( mit.data() ) {
                        selectedTrack->setDefinedNumKey( it.current() ->text( 0 ).toInt(), mit.data() );
                        kdDebug() << "Key " << it.current() ->text( 0 ).toInt() << " Playing: " << it.current() ->text( 1 ) << "Track: " << mit.data() << endl;
                    } else {
                        selectedTrack->setDefinedNumKey( it.current() ->text( 0 ).toInt(), 0L );
                        kdDebug() << "Key " << it.current() ->text( 0 ).toInt() << " Playing: " << it.current() ->text( 1 ) << endl;
                    }
            }
            ++it;
        }
    } else {
        selectedTrack->setDefinedNumKey( 1, selectedTrack );
        kdDebug() << "Key 1" << " Playing: (default) " << displayName( selectedTrack ) << "Track: " << selectedTrack << endl;
    }
}

void K3bVcdTrackDialog::fillGui()
{
    K3bVcdTrack * selectedTrack = m_selectedTracks.first();

    m_mpegver_video->setText( selectedTrack->mpegTypeS() );
    m_rate_video->setText( selectedTrack->video_bitrate() );
    m_chromaformat_video->setText( selectedTrack->video_chroma() );
    m_format_video->setText( selectedTrack->video_format() );
    m_highresolution_video->setText( selectedTrack->highresolution() );
    m_resolution_video->setText( selectedTrack->resolution() );

    m_mpegver_audio->setText( selectedTrack->mpegTypeS( true ) );
    m_rate_audio->setText( selectedTrack->audio_bitrate() );

    m_sampling_frequency_audio->setText( selectedTrack->audio_sampfreq() );
    m_mode_audio->setText( selectedTrack->audio_mode() );
    m_copyright_audio->setText( selectedTrack->audio_copyright() );

    fillPbcGui();


    TQToolTip::add
        ( m_pbc_previous, i18n( "May also look like | << on the remote control. " ) );
    TQToolTip::add
        ( m_pbc_next, i18n( "May also look like >> | on the remote control." ) );
    TQToolTip::add
        ( m_pbc_return, i18n( "This key may be mapped to the STOP key." ) );
    TQToolTip::add
        ( m_pbc_default, i18n( "This key is usually mapped to the > or PLAY key." ) );
    TQToolTip::add
        ( m_comboAfterTimeout, i18n( "Target to be jumped to on time-out of <wait>." ) );
    TQToolTip::add
        ( m_check_reactivity, i18n( "Delay reactivity of keys." ) );
    TQToolTip::add
        ( m_check_pbc, i18n( "Playback control, PBC, is available for Video CD 2.0 and Super Video CD 1.0 disc formats." ) );
    TQToolTip::add
        ( m_check_usekeys, i18n( "Activate the use of numeric keys." ) );
    TQToolTip::add
        ( m_check_overwritekeys, i18n( "Overwrite default numeric keys." ) );
    TQToolTip::add
        ( m_list_keys, i18n( "Numeric keys." ) );
    TQToolTip::add
        ( m_spin_times, i18n( "Times to repeat the playback of 'play track'." ) );
    TQToolTip::add
        ( m_spin_waittime, i18n( "Time in seconds to wait after playback of 'play track'." ) );

    TQWhatsThis::add
        ( m_comboAfterTimeout, i18n( "<p>Target to be jumped to on time-out of <wait>."
                                     "<p>If omitted (and <wait> is not set to an infinite time) one of the targets is selected at random." ) );
    TQWhatsThis::add
        ( m_check_reactivity, i18n( "<p>When reactivity is set to delayed, it is recommended that the length of the referenced 'play track' is not more than 5 seconds."
                                    "<p>The recommended setting for a play item consisting of one still picture and no audio is to loop once and have a delayed reactivity." ) );
    TQWhatsThis::add
        ( m_check_pbc, i18n( "<p>Playback control, PBC, is available for Video CD 2.0 and Super Video CD 1.0 disc formats."
                             "<p>PBC allows control of the playback of play items and the possibility of interaction with the user through the remote control or some other input device available." ) );
    TQWhatsThis::add
        ( m_check_usekeys, i18n( "These are actually pseudo keys, representing the numeric keys 0, 1, ..., 9." ) );
    TQWhatsThis::add
        ( m_check_overwritekeys, i18n( "<p>If numeric keys enabled, you can overwrite the default settings." ) );
    TQWhatsThis::add
        ( m_spin_times, i18n( "<p>Times to repeat the playback of 'play track'."
                              "<p>The reactivity attribute controls whether the playback of 'play track' is finished, thus delayed, before executing user triggered action or an immediate jump is performed."
                              "<p>After the specified number of repetitions have completed, the <wait> time begins to count down, unless set to an infinite wait time."
                              "<p>If this element is omitted, a default of `1' is used, i.e. the 'play track' will be displayed once." ) );
    TQWhatsThis::add
        ( m_spin_waittime, i18n( "Time in seconds to wait after playback of 'play track' before triggering the <timeout> action (unless the user triggers some action before time ran up)." ) );

}

void K3bVcdTrackDialog::fillPbcGui()
{
    K3bVcdTrack * selectedTrack = m_selectedTracks.first();
    // add tracktitles to combobox
    int iPrevious = -1;
    int iNext = -1;
    int iReturn = -1;
    int iDefault = -1;
    int iAfterTimeOut = -1;

    K3bVcdTrack* track;
    K3bListViewItem* item;

    m_numkeysmap.insert( "", 0L );
    m_numkeysmap.insert( displayName( selectedTrack ) , selectedTrack );

    for ( track = m_tracks.first(); track; track = m_tracks.next() ) {
        TQPixmap pm;
        if ( track->isSegment() )
            pm = SmallIcon( "image-x-generic" );
        else
            pm = SmallIcon( "video-x-generic" );

        TQString s = displayName( track );
        if ( track != selectedTrack )              // donot insert selectedTrack, it was as "ItSelf" inserted to the begin of map
            m_numkeysmap.insert( s, track );

        m_pbc_previous->insertItem( pm, s );
        if ( track == selectedTrack->getPbcTrack( K3bVcdTrack::PREVIOUS ) )
            iPrevious = m_pbc_previous->count() - 1;

        m_pbc_next->insertItem( pm, s );
        if ( track == selectedTrack->getPbcTrack( K3bVcdTrack::NEXT ) )
            iNext = m_pbc_next->count() - 1;

        m_pbc_return->insertItem( pm, s );
        if ( track == selectedTrack->getPbcTrack( K3bVcdTrack::RETURN ) )
            iReturn = m_pbc_return->count() - 1;

        m_pbc_default->insertItem( pm, s );
        if ( track == selectedTrack->getPbcTrack( K3bVcdTrack::DEFAULT ) )
            iDefault = m_pbc_default->count() - 1;

        m_comboAfterTimeout->insertItem( pm, s );
        if ( track == selectedTrack->getPbcTrack( K3bVcdTrack::AFTERTIMEOUT ) )
            iAfterTimeOut = m_comboAfterTimeout->count() - 1;

    }

    // add Event Disabled
    TQPixmap pmDisabled = SmallIcon( "process-stop" );
    TQString txtDisabled = i18n( "Event Disabled" );
    m_pbc_previous->insertItem( pmDisabled, txtDisabled );
    m_pbc_next->insertItem( pmDisabled, txtDisabled );
    m_pbc_return->insertItem( pmDisabled, txtDisabled );
    m_pbc_default->insertItem( pmDisabled, txtDisabled );
    m_comboAfterTimeout->insertItem( pmDisabled, txtDisabled );

    // add VideoCD End
    TQPixmap pmEnd = SmallIcon( "media-optical-cdrom-unmounted" );
    TQString txtEnd = i18n( "VideoCD END" );
    m_pbc_previous->insertItem( pmEnd, txtEnd );
    m_pbc_next->insertItem( pmEnd, txtEnd );
    m_pbc_return->insertItem( pmEnd, txtEnd );
    m_pbc_default->insertItem( pmEnd, txtEnd );
    m_comboAfterTimeout->insertItem( pmEnd, txtEnd );
    m_numkeysmap.insert( txtEnd, 0L );

    for ( int i = 99; i > 0; i-- ) {
        item = new K3bListViewItem( m_list_keys, TQString::number( i ) + " ", "" );
        item->setEditor( 1, K3bListViewItem::COMBO , m_numkeysmap.keys() );
    }

    int count = m_tracks.count();

    if ( iPrevious < 0 )
        m_pbc_previous->setCurrentItem( count + selectedTrack->getNonPbcTrack( K3bVcdTrack::PREVIOUS ) );
    else
        m_pbc_previous->setCurrentItem( iPrevious );

    if ( iNext < 0 )
        m_pbc_next->setCurrentItem( count + selectedTrack->getNonPbcTrack( K3bVcdTrack::NEXT ) );
    else
        m_pbc_next->setCurrentItem( iNext );

    if ( iReturn < 0 )
        m_pbc_return->setCurrentItem( count + selectedTrack->getNonPbcTrack( K3bVcdTrack::RETURN ) );
    else
        m_pbc_return->setCurrentItem( iReturn );

    if ( iDefault < 0 )
        m_pbc_default->setCurrentItem( count + selectedTrack->getNonPbcTrack( K3bVcdTrack::DEFAULT ) );
    else
        m_pbc_default->setCurrentItem( iDefault );

    if ( iAfterTimeOut < 0 )
        m_comboAfterTimeout->setCurrentItem( count + selectedTrack->getNonPbcTrack( K3bVcdTrack::AFTERTIMEOUT ) );
    else
        m_comboAfterTimeout->setCurrentItem( iAfterTimeOut );


    m_spin_waittime->setValue( selectedTrack->getWaitTime() );
    m_spin_times->setValue( selectedTrack->getPlayTime() );

    m_check_reactivity->setChecked( selectedTrack->Reactivity() );
    m_check_pbc->setChecked( VcdOptions() ->PbcEnabled() );

    m_check_usekeys->setChecked( selectedTrack->PbcNumKeys() );
    m_check_overwritekeys->setChecked( selectedTrack->PbcNumKeysUserdefined() );

    m_mainTabbed->setTabEnabled( m_widgetnumkeys, m_check_usekeys->isChecked() && m_check_pbc->isChecked() );

    setDefinedNumKeys();
}

void K3bVcdTrackDialog::prepareGui()
{
    TQFrame * frame = plainPage();

    TQGridLayout* mainLayout = new TQGridLayout( frame );
    mainLayout->setSpacing( spacingHint() );
    mainLayout->setMargin( 0 );

    m_mainTabbed = new TQTabWidget( frame );

    ///////////////////////////////////////////////////
    // FILE-INFO BOX
    ///////////////////////////////////////////////////
    TQGroupBox* groupFileInfo = new TQGroupBox( 0, TQt::Vertical, i18n( "File Info" ), frame, "groupFileInfo" );
    groupFileInfo->layout() ->setSpacing( 0 );
    groupFileInfo->layout() ->setMargin( 0 );

    TQGridLayout* groupFileInfoLayout = new TQGridLayout( groupFileInfo->layout() );
    groupFileInfoLayout->setAlignment( TQt::AlignTop );
    groupFileInfoLayout->setSpacing( spacingHint() );
    groupFileInfoLayout->setMargin( marginHint() );

    m_labelMimeType = new TQLabel( groupFileInfo, "m_labelMimeType" );

    m_displayFileName = new KCutLabel( groupFileInfo );
    m_displayFileName->setText( i18n( "Filename" ) );
    m_displayFileName->setAlignment( int( TQLabel::AlignTop | TQLabel::AlignLeft ) );

    TQLabel* labelSize = new TQLabel( i18n( "Size:" ), groupFileInfo, "labelSize" );
    TQLabel* labelLength = new TQLabel( i18n( "Length:" ), groupFileInfo, "labelLength" );
    TQLabel* labelMuxrate = new TQLabel( i18n( "Muxrate:" ), groupFileInfo, "labelMuxrate" );

    m_displaySize = new TQLabel( groupFileInfo, "m_displaySize" );
    m_displaySize->setText( "0.0 MB" );
    m_displaySize->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

    m_displayLength = new TQLabel( groupFileInfo, "m_displayLength" );
    m_displayLength->setText( "0:0:0" );
    m_displayLength->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

    m_muxrate = new TQLabel( groupFileInfo, "m_muxrate" );
    m_muxrate->setText( i18n( "%1 bit/s" ).arg( 0 ) );
    m_muxrate->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

    TQFrame* fileInfoLine = new TQFrame( groupFileInfo );
    fileInfoLine->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );

    groupFileInfoLayout->addWidget( m_labelMimeType, 0, 0 );
    groupFileInfoLayout->addMultiCellWidget( m_displayFileName, 0, 1, 1, 1 );
    groupFileInfoLayout->addMultiCellWidget( fileInfoLine, 2, 2, 0, 1 );
    groupFileInfoLayout->addWidget( labelLength, 3, 0 );
    groupFileInfoLayout->addWidget( labelSize, 4, 0 );
    groupFileInfoLayout->addWidget( labelMuxrate, 5, 0 );
    groupFileInfoLayout->addWidget( m_displayLength, 3, 1 );
    groupFileInfoLayout->addWidget( m_displaySize, 4, 1 );
    groupFileInfoLayout->addWidget( m_muxrate, 5, 1 );

    groupFileInfoLayout->setRowStretch( 6, 1 );
    groupFileInfoLayout->setColStretch( 1, 1 );

    TQFont f( m_displayLength->font() );
    f.setBold( true );
    m_displayLength->setFont( f );
    m_displaySize->setFont( f );
    m_muxrate->setFont( f );
    ///////////////////////////////////////////////////

    mainLayout->addWidget( groupFileInfo, 0, 0 );
    mainLayout->addWidget( m_mainTabbed, 0, 1 );

    //  mainLayout->setColStretch( 0, 1 );

}

void K3bVcdTrackDialog::setupPbcTab()
{
    // /////////////////////////////////////////////////
    // Playback Control TAB
    // /////////////////////////////////////////////////
    TQWidget * w = new TQWidget( m_mainTabbed );

    TQGridLayout* grid = new TQGridLayout( w );
    grid->setAlignment( TQt::AlignTop );
    grid->setSpacing( spacingHint() );
    grid->setMargin( marginHint() );


    //////////////////////////////////////////////////////////////////////////////////////////
    TQGroupBox* groupOptions = new TQGroupBox( 3, TQt::Vertical, i18n( "Settings" ), w );
    groupOptions->layout() ->setSpacing( spacingHint() );
    groupOptions->layout() ->setMargin( marginHint() );

    m_check_pbc = new TQCheckBox( i18n( "Enable playback control (for the whole CD)" ), groupOptions, "m_check_pbc" );

    m_check_usekeys = new TQCheckBox( i18n( "Use numeric keys" ), groupOptions, "m_check_usekeys" );
    m_check_usekeys->setEnabled( false );

    m_check_reactivity = new TQCheckBox( i18n( "Reactivity delayed to the end of playing track" ), groupOptions, "m_check_reactivity" );
    m_check_reactivity->setEnabled( false );

    //////////////////////////////////////////////////////////////////////////////////////////
    m_groupPlay = new TQGroupBox( 0, TQt::Vertical, i18n( "Playing" ), w );
    m_groupPlay->layout() ->setSpacing( spacingHint() );
    m_groupPlay->layout() ->setMargin( marginHint() );

    TQGridLayout* groupPlayLayout = new TQGridLayout( m_groupPlay->layout() );
    groupPlayLayout->setAlignment( TQt::AlignTop );

    TQLabel* labelPlaying = new TQLabel( i18n( "Playing track" ) , m_groupPlay, "labelPlaying" );

    m_spin_times = new TQSpinBox( m_groupPlay, "m_spin_times" );
    m_spin_times->setValue( 1 );
    m_spin_times->setSuffix( i18n( " time(s)" ) );
    m_spin_times->setSpecialValueText( i18n( "forever" ) );

    //////////////////////////////////////////////////////////////////////////////////////////
    m_labelWait = new TQLabel( i18n( "then wait" ), m_groupPlay, "m_labelWait" );
    m_spin_waittime = new TQSpinBox( m_groupPlay, "m_spinSeconds" );
    m_spin_waittime->setMinValue( -1 );
    m_spin_waittime->setValue( 0 );
    // m_spin_waittime->setEnabled( false );
    m_spin_waittime->setSuffix( i18n( " seconds" ) );
    m_spin_waittime->setSpecialValueText( i18n( "infinite" ) );

    m_labelAfterTimeout = new TQLabel( i18n( "after timeout playing" ), m_groupPlay, "m_labelTimeout" );
    // m_labelAfterTimeout->setEnabled( false );
    m_comboAfterTimeout = new K3bCutComboBox( K3bCutComboBox::SQUEEZE, m_groupPlay, "m_comboAfterTimeout" );
    // m_comboAfterTimeout->setEnabled( false );

    groupPlayLayout->addWidget( labelPlaying, 1, 0 );
    groupPlayLayout->addWidget( m_spin_times, 1, 1 );
    groupPlayLayout->addWidget( m_labelWait, 1, 2 );
    groupPlayLayout->addWidget( m_spin_waittime, 1, 3 );
    groupPlayLayout->addMultiCellWidget( m_labelAfterTimeout, 2, 2, 1, 3 );
    groupPlayLayout->addMultiCellWidget( m_comboAfterTimeout, 3, 3, 1, 3 );

    //////////////////////////////////////////////////////////////////////////////////////////
    m_groupPbc = new TQGroupBox( 0, TQt::Vertical, i18n( "Key Pressed Interaction" ), w );
    m_groupPbc->layout() ->setSpacing( spacingHint() );
    m_groupPbc->layout() ->setMargin( marginHint() );

    TQGridLayout* groupPbcLayout = new TQGridLayout( m_groupPbc->layout() );
    groupPbcLayout->setAlignment( TQt::AlignTop );

    TQLabel* labelPbc_previous = new TQLabel( i18n( "Previous:" ), m_groupPbc, "labelPbc_previous" );
    TQLabel* labelPbc_next = new TQLabel( i18n( "Next:" ), m_groupPbc, "labelPbc_next" );
    TQLabel* labelPbc_return = new TQLabel( i18n( "Return:" ), m_groupPbc, "labelPbc_return" );
    TQLabel* labelPbc_default = new TQLabel( i18n( "Default:" ), m_groupPbc, "labelPbc_default" );

    m_pbc_previous = new K3bCutComboBox( K3bCutComboBox::SQUEEZE, m_groupPbc, "m_pbc_previous" );
    m_pbc_next = new K3bCutComboBox( K3bCutComboBox::SQUEEZE, m_groupPbc, "m_pbc_next" );
    m_pbc_return = new K3bCutComboBox( K3bCutComboBox::SQUEEZE, m_groupPbc, "m_pbc_return" );
    m_pbc_default = new K3bCutComboBox( K3bCutComboBox::SQUEEZE, m_groupPbc, "m_pbc_default" );

    groupPbcLayout->addWidget( labelPbc_previous, 1, 0 );
    groupPbcLayout->addMultiCellWidget( m_pbc_previous, 1, 1, 1, 3 );

    groupPbcLayout->addWidget( labelPbc_next, 2, 0 );
    groupPbcLayout->addMultiCellWidget( m_pbc_next, 2, 2, 1, 3 );

    groupPbcLayout->addWidget( labelPbc_return, 3, 0 );
    groupPbcLayout->addMultiCellWidget( m_pbc_return, 3, 3, 1, 3 );

    groupPbcLayout->addWidget( labelPbc_default, 4, 0 );
    groupPbcLayout->addMultiCellWidget( m_pbc_default, 4, 4, 1, 3 );


    grid->addWidget( groupOptions, 0, 0 );
    grid->addWidget( m_groupPlay, 1, 0 );
    grid->addWidget( m_groupPbc, 2, 0 );

    grid->setRowStretch( 9, 1 );

    m_mainTabbed->addTab( w, i18n( "Playback Control" ) );

    m_groupPlay->setEnabled( false );
    m_groupPbc->setEnabled( false );

    connect( m_check_pbc, TQ_SIGNAL( toggled( bool ) ), this, TQ_SLOT( slotPbcToggled( bool ) ) );
    connect( m_spin_times, TQ_SIGNAL( valueChanged( int ) ), this, TQ_SLOT( slotPlayTimeChanged( int ) ) );
    connect( m_spin_waittime, TQ_SIGNAL( valueChanged( int ) ), this, TQ_SLOT( slotWaitTimeChanged( int ) ) );
    connect( m_check_usekeys, TQ_SIGNAL( toggled( bool ) ), this, TQ_SLOT( slotUseKeysToggled( bool ) ) );
}

void K3bVcdTrackDialog::setupPbcKeyTab()
{
    // /////////////////////////////////////////////////
    // Playback Control Numeric Key's TAB
    // /////////////////////////////////////////////////
    m_widgetnumkeys = new TQWidget( m_mainTabbed );

    TQGridLayout* grid = new TQGridLayout( m_widgetnumkeys );
    grid->setAlignment( TQt::AlignTop );
    grid->setSpacing( spacingHint() );
    grid->setMargin( marginHint() );

    m_groupKey = new TQGroupBox( 3, TQt::Vertical, i18n( "Numeric Keys" ), m_widgetnumkeys );
    m_groupKey->setEnabled( false );
    m_groupKey->layout() ->setSpacing( spacingHint() );
    m_groupKey->layout() ->setMargin( marginHint() );

    m_list_keys = new K3bListView( m_groupKey, "m_list_keys" );
    m_list_keys->setAllColumnsShowFocus( true );
    m_list_keys->setDoubleClickForEdit( false );
    m_list_keys->setColumnAlignment( 0, TQt::AlignRight );
    m_list_keys->setSelectionMode( TQListView::NoSelection );
    m_list_keys->setSorting( -1 );
    m_list_keys->addColumn( i18n( "Key" ) );
    m_list_keys->addColumn( i18n( "Playing" ) );
    m_list_keys->setResizeMode( TQListView::LastColumn );
    m_check_overwritekeys = new TQCheckBox( i18n( "Overwrite default assignment" ), m_widgetnumkeys, "m_check_overwritekeys" );

    grid->addWidget( m_groupKey, 1, 0 );
    grid->addWidget( m_check_overwritekeys, 2, 0 );

    m_mainTabbed->addTab( m_widgetnumkeys, i18n( "Numeric Keys" ) );

    connect( m_check_overwritekeys, TQ_SIGNAL( toggled( bool ) ), this, TQ_SLOT( slotGroupkeyToggled( bool ) ) );

}

void K3bVcdTrackDialog::setupAudioTab()
{
    // /////////////////////////////////////////////////
    // AUDIO TAB
    // /////////////////////////////////////////////////
    TQWidget * w = new TQWidget( m_mainTabbed );

    TQGridLayout* grid = new TQGridLayout( w );
    grid->setAlignment( TQt::AlignTop );
    grid->setSpacing( spacingHint() );
    grid->setMargin( marginHint() );

    TQLabel* labelMpegVer_Audio = new TQLabel( i18n( "Type:" ), w, "labelMpegVer_Audio" );
    TQLabel* labelRate_Audio = new TQLabel( i18n( "Rate:" ), w, "labelRate_Audio" );
    TQLabel* labelSampling_Frequency_Audio = new TQLabel( i18n( "Sampling frequency:" ), w, "labelSampling_Frequency_Audio" );
    TQLabel* labelMode_Audio = new TQLabel( i18n( "Mode:" ), w, "labelMode_Audio" );
    TQLabel* labelCopyright_Audio = new TQLabel( i18n( "Copyright:" ), w, "labelCopyright_Audio" );

    m_mpegver_audio = new TQLabel( w, "m_mpegver_audio" );
    m_rate_audio = new TQLabel( w, "m_rate_audio" );
    m_sampling_frequency_audio = new TQLabel( w, "m_sampling_frequency_audio" );
    m_mode_audio = new TQLabel( w, "m_mode_audio" );
    m_copyright_audio = new TQLabel( w, "m_copyright_audio" );

    m_mpegver_audio->setFrameShape( TQLabel::LineEditPanel );
    m_rate_audio->setFrameShape( TQLabel::LineEditPanel );
    m_sampling_frequency_audio->setFrameShape( TQLabel::LineEditPanel );
    m_mode_audio->setFrameShape( TQLabel::LineEditPanel );
    m_copyright_audio->setFrameShape( TQLabel::LineEditPanel );

    m_mpegver_audio->setFrameShadow( TQLabel::Sunken );
    m_rate_audio->setFrameShadow( TQLabel::Sunken );
    m_sampling_frequency_audio->setFrameShadow( TQLabel::Sunken );
    m_mode_audio->setFrameShadow( TQLabel::Sunken );
    m_copyright_audio->setFrameShadow( TQLabel::Sunken );

    grid->addWidget( labelMpegVer_Audio, 1, 0 );
    grid->addMultiCellWidget( m_mpegver_audio, 1, 1, 1, 4 );

    grid->addWidget( labelRate_Audio, 2, 0 );
    grid->addMultiCellWidget( m_rate_audio, 2, 2, 1, 4 );

    grid->addWidget( labelSampling_Frequency_Audio, 3, 0 );
    grid->addMultiCellWidget( m_sampling_frequency_audio, 3, 3, 1, 4 );

    grid->addWidget( labelMode_Audio, 4, 0 );
    grid->addMultiCellWidget( m_mode_audio, 4, 4, 1, 4 );

    grid->addWidget( labelCopyright_Audio, 5, 0 );
    grid->addMultiCellWidget( m_copyright_audio, 5, 5, 1, 4 );

    grid->setRowStretch( 9, 4 );

    m_mainTabbed->addTab( w, i18n( "Audio" ) );

}

void K3bVcdTrackDialog::setupVideoTab()
{
    // /////////////////////////////////////////////////
    // VIDEO TAB
    // /////////////////////////////////////////////////
    TQWidget * w = new TQWidget( m_mainTabbed );

    TQGridLayout* grid = new TQGridLayout( w );
    grid->setAlignment( TQt::AlignTop );
    grid->setSpacing( spacingHint() );
    grid->setMargin( marginHint() );

    TQLabel* labelMpegVer_Video = new TQLabel( i18n( "Type:" ), w, "labelMpegVer_Video" );
    TQLabel* labelRate_Video = new TQLabel( i18n( "Rate:" ), w, "labelRate_Video" );
    TQLabel* labelChromaFormat_Video = new TQLabel( i18n( "Chroma format:" ), w, "labelChromaFormat_Video" );
    TQLabel* labelFormat_Video = new TQLabel( i18n( "Video format:" ), w, "labelFormat_Video" );
    TQLabel* labelResolution_Video = new TQLabel( i18n( "Resolution:" ), w, "labelSize_Video" );
    TQLabel* labelHighResolution_Video = new TQLabel( i18n( "High resolution:" ), w, "labelHighResolution_Video" );

    m_mpegver_video = new TQLabel( w, "m_mpegver_video" );
    m_rate_video = new TQLabel( w, "m_rate_video" );
    m_chromaformat_video = new TQLabel( w, "m_chromaformat_video" );
    m_format_video = new TQLabel( w, "m_format_video" );
    m_resolution_video = new TQLabel( w, "m_resolution_video" );
    m_highresolution_video = new TQLabel( w, "m_highresolution_video" );

    m_mpegver_video->setFrameShape( TQLabel::LineEditPanel );
    m_rate_video->setFrameShape( TQLabel::LineEditPanel );
    m_chromaformat_video->setFrameShape( TQLabel::LineEditPanel );
    m_format_video->setFrameShape( TQLabel::LineEditPanel );
    m_resolution_video->setFrameShape( TQLabel::LineEditPanel );
    m_highresolution_video->setFrameShape( TQLabel::LineEditPanel );

    m_mpegver_video->setFrameShadow( TQLabel::Sunken );
    m_rate_video->setFrameShadow( TQLabel::Sunken );
    m_chromaformat_video->setFrameShadow( TQLabel::Sunken );
    m_format_video->setFrameShadow( TQLabel::Sunken );
    m_resolution_video->setFrameShadow( TQLabel::Sunken );
    m_highresolution_video->setFrameShadow( TQLabel::Sunken );

    grid->addWidget( labelMpegVer_Video, 1, 0 );
    grid->addMultiCellWidget( m_mpegver_video, 1, 1, 1, 4 );

    grid->addWidget( labelRate_Video, 2, 0 );
    grid->addMultiCellWidget( m_rate_video, 2, 2, 1, 4 );

    grid->addWidget( labelChromaFormat_Video, 3, 0 );
    grid->addMultiCellWidget( m_chromaformat_video, 3, 3, 1, 4 );

    grid->addWidget( labelFormat_Video, 4, 0 );
    grid->addMultiCellWidget( m_format_video, 4, 4, 1, 4 );

    grid->addWidget( labelResolution_Video, 5, 0 );
    grid->addMultiCellWidget( m_resolution_video, 5, 5, 1, 4 );

    grid->addWidget( labelHighResolution_Video, 6, 0 );
    grid->addMultiCellWidget( m_highresolution_video, 6, 6, 1, 4 );

    grid->setRowStretch( 9, 4 );

    m_mainTabbed->addTab( w, i18n( "Video" ) );
}

void K3bVcdTrackDialog::setDefinedNumKeys( )
{
    K3bVcdTrack * selectedTrack = m_selectedTracks.first();
    if ( !m_check_overwritekeys->isChecked() ) {

        selectedTrack->delDefinedNumKey();
        selectedTrack->setDefinedNumKey( 1, selectedTrack );

    }

    TQListViewItemIterator it( m_list_keys );
    TQMap<int, K3bVcdTrack*> definedkeysmap = selectedTrack->DefinedNumKey();

    while ( it.current() ) {
        int itemId = it.current() ->text( 0 ).toInt();

        TQMap<int, K3bVcdTrack*>::const_iterator keyit = definedkeysmap.find( itemId );

        if ( keyit != definedkeysmap.end() ) {
            if ( keyit.data() ) {
                if ( m_tracks.findRef( keyit.data() ) >= 0 ) {
                    it.current() ->setText( 1 , displayName( keyit.data() ) ) ;
                } else {
                    it.current() ->setText( 1 , "" ) ;
                    selectedTrack->delDefinedNumKey( keyit.key() );
                }
            } else {
                it.current() ->setText( 1 , i18n( "VideoCD END" ) ) ;
            }
        } else {
            it.current() ->setText( 1 , "" ) ;
        }
        ++it;
    }
}

TQString K3bVcdTrackDialog::displayName( K3bVcdTrack * track )
{
    if ( track == m_selectedTracks.first() )
        return i18n( "ItSelf" );

    if ( track->isSegment() )
        return i18n( "Segment-%1 - %2" ).arg( TQString::number( track->index() + 1 ).rightJustify( 3, '0' ) ).arg( track->title() );

    return i18n( "Sequence-%1 - %2" ).arg( TQString::number( track->index() + 1 ).rightJustify( 3, '0' ) ).arg( track->title() );
}

void K3bVcdTrackDialog::slotPlayTimeChanged( int value )
{
    if ( value == 0 ) {
        m_labelWait->setEnabled( false );
        m_spin_waittime->setEnabled( false );
        m_labelAfterTimeout->setEnabled( false );
        m_comboAfterTimeout->setEnabled( false );
    } else {
        m_labelWait->setEnabled( true );
        m_spin_waittime->setEnabled( true );
        if ( m_spin_waittime->value() > -1 ) {
            m_labelAfterTimeout->setEnabled( true );
            m_comboAfterTimeout->setEnabled( true );
        }
    }
}

void K3bVcdTrackDialog::slotWaitTimeChanged( int value )
{
    if ( value < 0 || !m_labelWait->isEnabled() ) {
        m_labelAfterTimeout->setEnabled( false );
        m_comboAfterTimeout->setEnabled( false );
    } else {
        m_labelAfterTimeout->setEnabled( true );
        m_comboAfterTimeout->setEnabled( true );
    }
}

void K3bVcdTrackDialog::slotPbcToggled( bool b )
{
    m_groupPlay->setEnabled( b );
    m_groupPbc->setEnabled( b );
    m_check_usekeys->setEnabled( b );
    slotUseKeysToggled( b && m_check_usekeys->isChecked() );
    m_check_reactivity->setEnabled( b );
    if ( b )
        slotWaitTimeChanged( m_spin_waittime->value() );
}

void K3bVcdTrackDialog::slotUseKeysToggled( bool b )
{
    m_mainTabbed->setTabEnabled( m_widgetnumkeys, b );
}

void K3bVcdTrackDialog::slotGroupkeyToggled( bool b )
{
    m_groupKey->setEnabled( b );
    setDefinedNumKeys();
}

#include "k3bvcdtrackdialog.moc"
