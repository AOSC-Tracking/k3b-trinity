/*
*
* $Id: k3bvcdburndialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqspinbox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqgrid.h>
#include <tqtoolbutton.h>
#include <tqfileinfo.h>

#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kapplication.h>

#include "k3bvcdburndialog.h"
#include "k3bvcddoc.h"
#include "k3bvcdoptions.h"
#include <k3bdevice.h>
#include <k3bcore.h>
#include <k3bwriterselectionwidget.h>
#include <k3btempdirselectionwidget.h>
#include <k3bstdguiitems.h>
#include <k3bglobals.h>
#include <k3bwritingmodewidget.h>
#include <k3bexternalbinmanager.h>
#include <k3bvalidators.h>

K3bVcdBurnDialog::K3bVcdBurnDialog( K3bVcdDoc* _doc, TQWidget *parent, const char *name, bool modal )
        : K3bProjectBurnDialog( _doc, parent, name, modal )
{
    m_vcdDoc = _doc;

    prepareGui();

    TQString vcdType;
    switch ( m_vcdDoc->vcdType() ) {
        case K3bVcdDoc::VCD11:
            vcdType = i18n( "Video CD (Version 1.1)" );
        case K3bVcdDoc::VCD20:
            vcdType = i18n( "Video CD (Version 2.0)" );
        case K3bVcdDoc::SVCD10:
            vcdType = i18n( "Super Video CD" );
        case K3bVcdDoc::HTQVCD:
            vcdType = i18n( "High-Quality Video CD" );
        default:
            vcdType = i18n( "Video CD" );
    }

    setTitle( vcdType, i18n( "1 MPEG (%1)", "%n MPEGs (%1)",
                             m_vcdDoc->tracks() ->count() ).arg( KIO::convertSize( m_vcdDoc->size() ) ) );

    const K3bExternalBin* cdrecordBin = k3bcore->externalBinManager() ->binObject( "cdrecord" );
    if ( cdrecordBin && cdrecordBin->hasFeature( "cuefile" ) )
        m_writerSelectionWidget->setSupportedWritingApps( K3b::CDRDAO | K3b::CDRECORD );
    else
        m_writerSelectionWidget->setSupportedWritingApps( K3b::CDRDAO );

    m_checkCacheImage->hide();

    TQSpacerItem* spacer = new TQSpacerItem( 20, 20, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    m_optionGroupLayout->addItem( spacer );

    setupVideoCdTab();
    setupLabelTab();
    setupAdvancedTab();

    connect( m_spinVolumeCount, TQT_SIGNAL( valueChanged( int ) ), this, TQT_SLOT( slotSpinVolumeCount() ) );
    connect( m_groupVcdFormat, TQT_SIGNAL( clicked( int ) ), this, TQT_SLOT( slotVcdTypeClicked( int ) ) );
    connect( m_checkCdiSupport, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotCdiSupportChecked( bool ) ) );
    connect( m_checkAutoDetect, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotAutoDetect( bool ) ) );
    connect( m_checkGaps, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotGapsChecked( bool ) ) );

    // ToolTips
    // -------------------------------------------------------------------------
    TQToolTip::add
        ( m_radioVcd11, i18n( "Select Video CD type %1" ).arg( "(VCD 1.1)" ) );
    TQToolTip::add
        ( m_radioVcd20, i18n( "Select Video CD type %1" ).arg( "(VCD 2.0)" ) );
    TQToolTip::add
        ( m_radioSvcd10, i18n( "Select Video CD type %1" ).arg( "(SVCD 1.0)" ) );
    TQToolTip::add
        ( m_radioHqVcd10, i18n( "Select Video CD type %1" ).arg( "(HQ-VCD 1.0)" ) );
    TQToolTip::add
        ( m_checkAutoDetect, i18n( "Automatic video type recognition." ) );
    TQToolTip::add
        ( m_checkNonCompliant, i18n( "Non-compliant compatibility mode for broken devices" ) );
    TQToolTip::add
        ( m_checkVCD30interpretation, i18n( "Chinese VCD3.0 track interpretation" ) );
    TQToolTip::add
        ( m_check2336, i18n( "Use 2336 byte sectors for output" ) );

    TQToolTip::add
        ( m_editVolumeId, i18n( "Specify ISO volume label for Video CD" ) );
    TQToolTip::add
        ( m_editAlbumId, i18n( "Specify album id for VideoCD set" ) );
    TQToolTip::add
        ( m_spinVolumeNumber, i18n( "Specify album set sequence number ( <= volume-count )" ) );
    TQToolTip::add
        ( m_spinVolumeCount, i18n( "Specify number of volumes in album set" ) );

    TQToolTip::add
        ( m_checkCdiSupport, i18n( "Enable CD-i Application Support for VideoCD Type 1.1 & 2.0" ) );
    TQToolTip::add
        ( m_editCdiCfg, i18n( "Configuration parameters (only for VCD 2.0)" ) );

    TQToolTip::add
        ( m_checkPbc, i18n( "Playback control, PBC, is available for Video CD 2.0 and Super Video CD 1.0 disc formats." ) );
    TQToolTip::add
        ( m_checkSegmentFolder, i18n( "Add always an empty `/SEGMENT' directory" ) );
    TQToolTip::add
        ( m_checkRelaxedAps, i18n( "This controls whether APS constraints are strict or relaxed. " ) );
    TQToolTip::add
        ( m_checkUpdateScanOffsets, i18n( "This controls whether to update the scan data information contained in the MPEG-2 video streams." ) );
    TQToolTip::add
        ( m_labelRestriction, i18n( "This element allows to set viewing restrictions which may be interpreted by the playing device." ) );

    TQToolTip::add
        ( m_checkGaps, i18n( "This option allows customization of Gaps and Margins." ) );
    TQToolTip::add
        ( m_labelPreGapLeadout, i18n( "Used to set the number of empty sectors added before the lead-out area begins." ) );
    TQToolTip::add
        ( m_labelPreGapTrack, i18n( "Used to set the track pre-gap for all tracks in sectors globally." ) );
    TQToolTip::add
        ( m_labelFrontMarginTrack, i18n( "Sets the front margin for sequence items." ) );
    TQToolTip::add
        ( m_labelRearMarginTrack, i18n( "Sets the rear margin for sequence items." ) );

    // What's This info
    // -------------------------------------------------------------------------
    TQWhatsThis::add
        ( m_radioVcd11, i18n( "<p>This is the most basic <b>Video CD</b> specification dating back to 1993, which has the following characteristics:"
                              "<ul><li>One mode2 mixed form ISO-9660 track containing file pointers to the information areas.</li>"
                              "<li>Up to 98 multiplex-ed MPEG-1 audio/video streams or CD-DA audio tracks.</li>"
                              "<li>Up to 500 MPEG sequence entry points used as chapter divisions.</li></ul>"
                              "<p>The Video CD specification requires the multiplex-ed MPEG-1 stream to have a CBR of less than 174300 bytes (1394400 bits) per second in order to accommodate single speed CD-ROM drives.<br>"
                              "The specification allows for the following two resolutions:"
                              "<ul><li>352 x 240 @ 29.97 Hz (NTSC SIF).</li>"
                              "<li>352 x 240 @ 23.976 Hz (FILM SIF).</li></ul>"
                              "<p>The CBR MPEG-1, layer II audio stream is fixed at 224 kbps with 1 stereo or 2 mono channels."
                              "<p><b>It is recommended to keep the video bit-rate under 1151929.1 bps.</b>" ) );

    TQWhatsThis::add
        ( m_radioVcd20, i18n( "<p>About two years after the Video CD 1.1 specification came out, an improved <b>Video CD 2.0</b> standard was published in 1995."
                              "<p>This one added the following items to the features already available in the Video CD 1.1 specification:"
                              "<ul><li>Support for MPEG segment play items (<b>\"SPI\"</b>), consisting of still pictures, motion pictures and/or audio (only) streams was added.</li>"
                              "<li>Note Segment Items::.</li>"
                              "<li>Support for interactive playback control (<b>\"PBC\"</b>) was added.</li>"
                              "<li>Support for playing related access by providing a scan point index file was added. (<b>\"/EXT/SCANDATA.DAT\"</b>)</li>"
                              "<li>Support for closed captions.</li>"
                              "<li>Support for mixing NTSC and PAL content.</li></ul>"
                              "<p>By adding PAL support to the Video CD 1.1 specification, the following resolutions became available:"
                              "<ul><li>352 x 240 @ 29.97 Hz (NTSC SIF).</li>"
                              "<li>352 x 240 @ 23.976 Hz (FILM SIF).</li>"
                              "<li>352 x 288 @ 25 Hz (PAL SIF).</li></ul>"
                              "<p>For segment play items the following audio encodings became available:"
                              "<ul><li>Joint stereo, stereo or dual channel audio streams at 128, 192, 224 or 384 kbit/sec bit-rate.</li>"
                              "<li>Mono audio streams at 64, 96 or 192 kbit/sec bit-rate.</li></ul>"
                              "<p>Also the possibility to have audio only streams and still pictures was provided."
                              "<p><b>The bit-rate of multiplex-ed streams should be kept under 174300 bytes/sec (except for single still picture items) in order to accommodate single speed drives.</b>" ) );

    TQWhatsThis::add
        ( m_radioSvcd10, i18n( "<p>With the upcoming of the DVD-V media, a new VCD standard had to be published in order to be able to keep up with technology, so the Super Video CD specification was called into life 1999."
                               "<p>In the midst of 2000 a full subset of this <b>Super Video CD</b> specification was published as <b>IEC-62107</b>."
                               "<p>As the most notable change over Video CD 2.0 is a switch from MPEG-1 CBR to MPEG-2 VBR encoding for the video stream was performed."
                               "<p>The following new features--building upon the Video CD 2.0 specification--are:"
                               "<ul><li>Use of MPEG-2 encoding instead of MPEG-1 for the video stream.</li>"
                               "<li>Allowed VBR encoding of MPEG-1 audio stream.</li>"
                               "<li>Higher resolutions (see below) for video stream resolution.</li>"
                               "<li>Up to 4 overlay graphics and text (<b>\"OGT\"</b>) sub-channels for user switchable subtitle displaying in addition to the already existing closed caption facility.</li>"
                               "<li>Command lists for controlling the SVCD virtual machine.</li></ul>"
                               "<p>For the <b>Super Video CD</b>, only the following two resolutions are supported for motion video and (low resolution) still pictures:"
                               "<ul><li>480 x 480 @ 29.97 Hz (NTSC 2/3 D-2).</li>"
                               "<li>480 x 576 @ 25 Hz (PAL 2/3 D-2).</li></ul>" ) );

    TQWhatsThis::add
        ( m_radioHqVcd10, i18n( "<p>This is actually just a minor variation defined in IEC-62107 on the Super Video CD 1.0 format for compatibility with current products in the market."
                                "<p>It differs from the Super Video CD 1.0 format in the following items:"
                                "<ul><li>The system profile tag field in <b>/SVCD/INFO.SVD</b> is set to <b>1</b> instead of <b>0</b>.</li>"
                                "<li>The system identification field value in <b>/SVCD/INFO.SVD</b> is set to <b>HQ-VCD</b> instead of <b>SUPERVCD</b>.</li>"
                                "<li><b>/EXT/SCANDATA.DAT</b> is mandatory instead of being optional.</li>"
                                "<li><b>/SVCD/SEARCH.DAT</b> is optional instead of being mandatory.</li></ul>" ) );

    TQWhatsThis::add
        ( m_checkAutoDetect, i18n( "<p>If Autodetect is:</p>"
                                   "<ul><li>ON then K3b will set the correct VideoCD type.</li>"
                                   "<li>OFF then the correct VideoCD type needs to be set by the user.</li></ul>"
                                   "<p>If you are not sure about the correct VideoCD type, it is best to turn Autodetect ON.</p>"
                                   "<p>If you want to force the VideoCD type, you must turn Autodetect OFF. This is useful for some standalone DVD players without SVCD support.</p>" ) );

    TQWhatsThis::add
        ( m_checkNonCompliant, i18n( "<ul><li>Rename <b>\"/MPEG2\"</b> folder on SVCDs to (non-compliant) \"/MPEGAV\".</li>"
                                     "<li>Enables the use of the (deprecated) signature <b>\"ENTRYSVD\"</b> instead of <b>\"ENTRYVCD\"</b> for the file <b>\"/SVCD/ENTRY.SVD\"</b>.</li></ul>" ) );
    TQWhatsThis::add
        ( m_checkVCD30interpretation, i18n( "<ul><li>Enables the use of the (deprecated) Chinese <b>\"/SVCD/TRACKS.SVD\"</b> format which differs from the format defined in the <b>IEC-62107</b> specification.</li></ul>"
                                            "<p><b>The differences are most exposed on SVCDs containing more than one video track.</b>" ) );

    TQWhatsThis::add
        ( m_check2336, i18n( "<p>though most devices will have problems with such an out-of-specification media."
                             "<p><b>You may want use this option for images longer than 80 minutes</b>" ) );

    TQWhatsThis::add
        ( m_checkCdiSupport, i18n( "<p>To allow the play of Video-CDs on a CD-i player, the Video-CD standard requires that a CD-i application program must be present."
                                   "<p>This program is designed to:"
                                   "<ul><li>provide full play back control as defined in the PSD of the standard</l>"
                                   "<li>be extremely simple to use and easy-to-learn for the end-user</li></ul>"
                                   "<p>The program runs on CD-i players equipped with the CDRTOS 1.1(.1) operating system and a Digital Video extension cartridge." ) );

    TQWhatsThis::add
        ( m_editCdiCfg, i18n( "<p>Configuration parameters only available for VideoCD 2.0"
                              "<p>The engine works perfectly well when used as-is."
                              "<p>You have the option to configure the VCD application."
                              "<p>You can adapt the color and/or the shape of the cursor and lots more." ) );


    TQWhatsThis::add
        ( m_checkPbc, i18n( "<p>Playback control, PBC, is available for Video CD 2.0 and Super Video CD 1.0 disc formats."
                            "<p>PBC allows control of the playback of play items and the possibility of interaction with the user through the remote control or some other input device available." ) );

    TQWhatsThis::add
        ( m_checkSegmentFolder, i18n( "<p>Here you can specify that the folder <b>SEGMENT</b> should always be present."
                                      "<p>Some DVD players need the folder to give a faultless rendition." ) );

    TQWhatsThis::add
        ( m_checkRelaxedAps, i18n( "<p>An Access Point Sector, APS, is an MPEG video sector on the VCD/SVCD which is suitable to be jumped to directly."
                                   "<p>APS are required for entry points and scantables. APS have to fulfil the requirement to precede every I-frame by a GOP header which shall be preceded by a sequence header in its turn."
                                   "<p>The start codes of these 3 items are required to be contained all in the same MPEG pack/sector, thus forming a so-called access point sector."
                                   "<p>This requirement can be relaxed by enabling the relaxed aps option, i.e. every sector containing an I-frame will be regarded as an APS."
                                   "<p><b>Warning:</b> The sequence header is needed for a playing device to figure out display parameters, such as display resolution and frame rate, relaxing the aps requirement may lead to non-working entry points." ) );

    TQWhatsThis::add
        ( m_checkUpdateScanOffsets, i18n( "<p>According to the specification, it is mandatory for Super Video CDs to encode scan information data into user data blocks in the picture layer of all intra coded picture."
                                          "<p>It can be used by playing devices for implementing fast forward & fast reverse scanning."
                                          "<p>The already existing scan information data can be updated by enabling the update scan offsets option." ) );

    TQWhatsThis::add
        ( m_labelRestriction, i18n( "<p>Viewing Restriction may be interpreted by the playing device."
                                    "<p>The allowed range goes from 0 to 3."
                                    "<ul><li>0 = unrestricted, free to view for all</li>"
                                    "<li>3 = restricted, content not suitable for ages under 18</li></ul>"
                                    "<p>Actually, the exact meaning is not defined and is player dependant."
                                    "<p><b>Most players ignore that value.<b>" ) );

    TQWhatsThis::add
        ( m_checkGaps, i18n( "<p>This option allows customization of Gaps and Margins." ) );
    TQWhatsThis::add
        ( m_labelPreGapLeadout, i18n( "<p>This option allows to set the number of empty sectors added before the lead-out area begins, i.e. the number of post-gap sectors."
                                      "<p>The ECMA-130 specification requires the last data track before the lead-out to carry a post-gap of at least 150 sectors, which is used as default for this parameter."
                                      "<p>Some operating systems may encounter I/O errors due to read-ahead issues when reading the last MPEG track if this parameter is set too low."
                                      "<p>Allowed value content: [0..300]. Default: 150." ) );

    TQWhatsThis::add
        ( m_labelPreGapTrack, i18n( "<p>Used to set the track pre-gap for all tracks in sectors globally."
                                    "<p>The specification requires the pre-gaps to be at least 150 sectors long."
                                    "<p>Allowed value content: [0..300]. Default: 150." ) );

    TQWhatsThis::add
        ( m_labelFrontMarginTrack, i18n( "Margins are used to compensate for inaccurate sector-addressing issues on CD-ROM media. Interestingly, they have been abandoned for Super Video CDs."
                                         "<p>For Video CD 1.0/1.1/2.0 this margin should be at least 15 sectors long."
                                         "<p>Allowed value content: [0..150]. Default: 30 for Video CD 1.0/1.1/2.0, otherwise (i.e. Super Video CD 1.0 and HQ-VCD 1.0) 0." ) );

    TQWhatsThis::add
        ( m_labelRearMarginTrack, i18n( "<p>Margins are used to compensate for inaccurate sector-addressing issues on CD-ROM media. Interestingly, they have been abandoned for Super Video CDs."
                                        "<p>For Video CD 1.0/1.1/2.0 this margin should be at least 15 sectors long."
                                        "<p>Allowed value content: [0..150]. Default: 45 for Video CD 1.0/1.1/2.0, otherwise 0." ) );

}


K3bVcdBurnDialog::~K3bVcdBurnDialog()
{}

void K3bVcdBurnDialog::setupAdvancedTab()
{
    TQWidget * w = new TQWidget( this );

    // ---------------------------------------------------- generic group ----
    m_groupGeneric = new TQGroupBox( 5, Qt::Vertical, i18n( "Generic" ), w );

    m_checkPbc = new TQCheckBox( i18n( "Playback Control (PBC)" ), m_groupGeneric );
    m_checkSegmentFolder = new TQCheckBox( i18n( "SEGMENT Folder must always be present" ), m_groupGeneric );
    m_checkRelaxedAps = new TQCheckBox( i18n( "Relaxed aps" ), m_groupGeneric );
    m_checkUpdateScanOffsets = new TQCheckBox( i18n( "Update scan offsets" ), m_groupGeneric );
    m_checkUpdateScanOffsets->setEnabled( false );


    // -------------------------------------------- gaps & margins group ----
    m_groupGaps = new TQGroupBox( 0, Qt::Vertical, i18n( "Gaps" ), w );
    m_groupGaps->tqlayout() ->setSpacing( spacingHint() );
    m_groupGaps->tqlayout() ->setMargin( marginHint() );

    TQGridLayout* groupGapsLayout = new TQGridLayout( m_groupGaps->tqlayout() );
    groupGapsLayout->setAlignment( TQt::AlignTop );

    m_checkGaps = new TQCheckBox( i18n( "Customize gaps and margins" ), m_groupGaps );

    m_labelPreGapLeadout = new TQLabel( i18n( "Leadout pre gap (0..300):" ), m_groupGaps, "labelPreGapLeadout" );
    m_spinPreGapLeadout = new TQSpinBox( m_groupGaps, "m_spinPreGapLeadout" );
    m_spinPreGapLeadout->setMinValue( 0 );
    m_spinPreGapLeadout->setMaxValue( 300 );

    m_labelPreGapTrack = new TQLabel( i18n( "Track pre gap (0..300):" ), m_groupGaps, "labelPreGapTrack" );
    m_spinPreGapTrack = new TQSpinBox( m_groupGaps, "m_spinPreGapTrack" );
    m_spinPreGapTrack->setMinValue( 0 );
    m_spinPreGapTrack->setMaxValue( 300 );

    m_labelFrontMarginTrack = new TQLabel( i18n( "Track front margin (0..150):" ), m_groupGaps, "labelFrontMarginTrack" );
    m_spinFrontMarginTrack = new TQSpinBox( m_groupGaps, "m_spinFrontMarginTrack" );
    m_spinFrontMarginTrack->setMinValue( 0 );
    m_spinFrontMarginTrack->setMaxValue( 150 );
    m_spinFrontMarginTrackSVCD = new TQSpinBox( m_groupGaps, "m_spinFrontMarginTrackSVCD" );
    m_spinFrontMarginTrackSVCD->setMinValue( 0 );
    m_spinFrontMarginTrackSVCD->setMaxValue( 150 );
    m_spinFrontMarginTrackSVCD->setHidden( true );

    m_labelRearMarginTrack = new TQLabel( i18n( "Track rear margin (0..150):" ), m_groupGaps, "labelRearMarginTrack" );
    m_spinRearMarginTrack = new TQSpinBox( m_groupGaps, "m_spinRearMarginTrack" );
    m_spinRearMarginTrack->setMinValue( 0 );
    m_spinRearMarginTrack->setMaxValue( 150 );
    m_spinRearMarginTrackSVCD = new TQSpinBox( m_groupGaps, "m_spinRearMarginTrackSVCD" );
    m_spinRearMarginTrackSVCD->setMinValue( 0 );
    m_spinRearMarginTrackSVCD->setMaxValue( 150 );
    m_spinRearMarginTrackSVCD->setHidden( true );

    groupGapsLayout->addMultiCellWidget( m_checkGaps, 1, 1, 0, 4 );
    groupGapsLayout->addWidget( m_labelPreGapLeadout, 2, 0 );
    groupGapsLayout->addWidget( m_spinPreGapLeadout, 2, 1 );
    groupGapsLayout->addWidget( m_labelPreGapTrack, 2, 3 );
    groupGapsLayout->addWidget( m_spinPreGapTrack, 2, 4 );

    groupGapsLayout->addWidget( m_labelFrontMarginTrack, 3, 0 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrack, 3, 1 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrackSVCD, 3, 1 );
    groupGapsLayout->addWidget( m_labelRearMarginTrack, 3, 3 );
    groupGapsLayout->addWidget( m_spinRearMarginTrack, 3, 4 );
    groupGapsLayout->addWidget( m_spinRearMarginTrackSVCD, 3, 4 );

    groupGapsLayout->setRowStretch( 4, 0 );

    groupGapsLayout->addMultiCellWidget( m_checkGaps, 1, 1, 0, 4 );
    groupGapsLayout->addWidget( m_labelPreGapLeadout, 2, 0 );
    groupGapsLayout->addWidget( m_spinPreGapLeadout, 2, 1 );
    groupGapsLayout->addWidget( m_labelPreGapTrack, 2, 3 );
    groupGapsLayout->addWidget( m_spinPreGapTrack, 2, 4 );

    groupGapsLayout->addWidget( m_labelFrontMarginTrack, 3, 0 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrack, 3, 1 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrackSVCD, 3, 1 );
    groupGapsLayout->addWidget( m_labelRearMarginTrack, 3, 3 );
    groupGapsLayout->addWidget( m_spinRearMarginTrack, 3, 4 );
    groupGapsLayout->addWidget( m_spinRearMarginTrackSVCD, 3, 4 );

    groupGapsLayout->setRowStretch( 4, 0 );
    groupGapsLayout->addMultiCellWidget( m_checkGaps, 1, 1, 0, 4 );
    groupGapsLayout->addWidget( m_labelPreGapLeadout, 2, 0 );
    groupGapsLayout->addWidget( m_spinPreGapLeadout, 2, 1 );
    groupGapsLayout->addWidget( m_labelPreGapTrack, 2, 3 );
    groupGapsLayout->addWidget( m_spinPreGapTrack, 2, 4 );

    groupGapsLayout->addWidget( m_labelFrontMarginTrack, 3, 0 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrack, 3, 1 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrackSVCD, 3, 1 );
    groupGapsLayout->addWidget( m_labelRearMarginTrack, 3, 3 );
    groupGapsLayout->addWidget( m_spinRearMarginTrack, 3, 4 );
    groupGapsLayout->addWidget( m_spinRearMarginTrackSVCD, 3, 4 );

    groupGapsLayout->setRowStretch( 4, 0 );
    groupGapsLayout->addMultiCellWidget( m_checkGaps, 1, 1, 0, 4 );
    groupGapsLayout->addWidget( m_labelPreGapLeadout, 2, 0 );
    groupGapsLayout->addWidget( m_spinPreGapLeadout, 2, 1 );
    groupGapsLayout->addWidget( m_labelPreGapTrack, 2, 3 );
    groupGapsLayout->addWidget( m_spinPreGapTrack, 2, 4 );

    groupGapsLayout->addWidget( m_labelFrontMarginTrack, 3, 0 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrack, 3, 1 );
    groupGapsLayout->addWidget( m_spinFrontMarginTrackSVCD, 3, 1 );
    groupGapsLayout->addWidget( m_labelRearMarginTrack, 3, 3 );
    groupGapsLayout->addWidget( m_spinRearMarginTrack, 3, 4 );
    groupGapsLayout->addWidget( m_spinRearMarginTrackSVCD, 3, 4 );

    groupGapsLayout->setRowStretch( 4, 0 );

    // ------------------------------------------------------- misc group ----
    m_groupMisc = new TQGroupBox( 0, Qt::Vertical, i18n( "Misc" ), w );
    m_groupMisc->tqlayout() ->setSpacing( spacingHint() );
    m_groupMisc->tqlayout() ->setMargin( marginHint() );

    TQGridLayout* groupMiscLayout = new TQGridLayout( m_groupMisc->tqlayout() );
    groupMiscLayout->setAlignment( TQt::AlignTop );

    m_labelRestriction = new TQLabel( i18n( "Restriction category (0..3):" ), m_groupMisc, "m_labelRestriction" );
    m_spinRestriction = new TQSpinBox( m_groupMisc, "m_spinRestriction" );
    m_spinRestriction->setMinValue( 0 );
    m_spinRestriction->setMaxValue( 3 );

    groupMiscLayout->addWidget( m_labelRestriction, 1, 0 );
    groupMiscLayout->addMultiCellWidget( m_spinRestriction, 1, 1, 1, 4 );
    groupMiscLayout->setRowStretch( 2, 0 );

    // ----------------------------------------------------------------------
    TQGridLayout* grid = new TQGridLayout( w );
    grid->setMargin( marginHint() );
    grid->setSpacing( spacingHint() );
    grid->addWidget( m_groupGeneric, 0, 0 );
    grid->addWidget( m_groupGaps, 1, 0 );
    grid->addWidget( m_groupMisc, 2, 0 );

    addPage( w, i18n( "Advanced" ) );
}

void K3bVcdBurnDialog::setupVideoCdTab()
{
    TQWidget * w = new TQWidget( this );

    // ---------------------------------------------------- Format group ----
    m_groupVcdFormat = new TQButtonGroup( 4, Qt::Vertical, i18n( "Type" ), w );
    m_radioVcd11 = new TQRadioButton( i18n( "VideoCD 1.1" ), m_groupVcdFormat );
    m_radioVcd20 = new TQRadioButton( i18n( "VideoCD 2.0" ), m_groupVcdFormat );
    m_radioSvcd10 = new TQRadioButton( i18n( "Super-VideoCD" ), m_groupVcdFormat );
    m_radioHqVcd10 = new TQRadioButton( i18n( "HQ-VideoCD" ), m_groupVcdFormat );
    m_groupVcdFormat->setExclusive( true );

    // ---------------------------------------------------- Options group ---

    m_groupOptions = new TQGroupBox( 5, Qt::Vertical, i18n( "Settings" ), w );
    m_checkAutoDetect = new TQCheckBox( i18n( "Autodetect VideoCD type" ), m_groupOptions );

    m_checkNonCompliant = new TQCheckBox( i18n( "Enable broken SVCD mode" ), m_groupOptions );
    // Only available on SVCD Type
    m_checkNonCompliant->setEnabled( false );
    m_checkNonCompliant->setChecked( false );

    m_checkVCD30interpretation = new TQCheckBox( i18n( "Enable %1 track interpretation" ).arg( "VCD 3.0" ), m_groupOptions );
    // Only available on SVCD Type
    m_checkVCD30interpretation->setEnabled( false );
    m_checkVCD30interpretation->setChecked( false );

    m_check2336 = new TQCheckBox( i18n( "Use 2336 byte sectors" ), m_groupOptions );

    m_checkCdiSupport = new TQCheckBox( i18n( "Enable CD-i support" ), m_groupOptions );

    // ------------------------------------------------- CD-i Application ---
    m_groupCdi = new TQGroupBox( 4, Qt::Vertical, i18n( "VideoCD on CD-i" ), w );
    m_editCdiCfg = new TQMultiLineEdit( m_groupCdi, "m_editCdiCfg" );
    m_editCdiCfg->setFrameShape( TQTextEdit::NoFrame );

    // ----------------------------------------------------------------------
    TQGridLayout* grid = new TQGridLayout( w );
    grid->setMargin( marginHint() );
    grid->setSpacing( spacingHint() );
    grid->addMultiCellWidget( m_groupVcdFormat, 0, 1, 0, 0 );
    grid->addWidget( m_groupOptions, 0, 1 );
    grid->addWidget( m_groupCdi, 1, 1 );

    addPage( w, i18n( "Settings" ) );
}

void K3bVcdBurnDialog::setupLabelTab()
{
    TQWidget * w = new TQWidget( this );

    // ----------------------------------------------------------------------
    // noEdit
    TQLabel* labelSystemId = new TQLabel( i18n( "System:" ), w, "labelSystemId" );
    TQLabel* labelApplicationId = new TQLabel( i18n( "Application:" ), w, "labelApplicationId" );
    TQLabel* labelInfoSystemId = new TQLabel( vcdDoc() ->vcdOptions() ->systemId(), w, "labelInfoSystemId" );
    TQLabel* labelInfoApplicationId = new TQLabel( vcdDoc() ->vcdOptions() ->applicationId(), w, "labelInfoApplicationId" );

    labelInfoSystemId->setFrameShape( TQLabel::LineEditPanel );
    labelInfoSystemId->setFrameShadow( TQLabel::Sunken );

    labelInfoApplicationId->setFrameShape( TQLabel::LineEditPanel );
    labelInfoApplicationId->setFrameShadow( TQLabel::Sunken );
    TQToolTip::add
        ( labelInfoApplicationId, i18n( "ISO application id for VideoCD" ) );

    // ----------------------------------------------------------------------

    TQLabel* labelVolumeId = new TQLabel( i18n( "&Volume name:" ), w, "labelVolumeId" );
    TQLabel* labelAlbumId = new TQLabel( i18n( "Volume &set name:" ), w, "labelAlbumId" );
    TQLabel* labelVolumeCount = new TQLabel( i18n( "Volume set s&ize:" ), w, "labelVolumeCount" );
    TQLabel* labelVolumeNumber = new TQLabel( i18n( "Volume set &number:" ), w, "labelVolumeNumber" );
    TQLabel* labelPublisher = new TQLabel( i18n( "&Publisher:" ), w, "labelPublisher" );


    m_editVolumeId = new TQLineEdit( w, "m_editVolumeId" );
    m_editAlbumId = new TQLineEdit( w, "m_editAlbumId" );
    m_spinVolumeNumber = new TQSpinBox( w, "m_editVolumeNumber" );
    m_spinVolumeCount = new TQSpinBox( w, "m_editVolumeCount" );
    m_editPublisher = new TQLineEdit( w, "m_editPublisher" );

    // only ISO646 d-Characters
    m_editVolumeId->setValidator( K3bValidators::iso646Validator( K3bValidators::Iso646_d, true, TQT_TQOBJECT(m_editVolumeId) ) );
    m_editAlbumId->setValidator( K3bValidators::iso646Validator( K3bValidators::Iso646_d, true, TQT_TQOBJECT(m_editVolumeId) ) );

    m_editVolumeId->setMaxLength( 32 );
    m_editAlbumId->setMaxLength( 16 );
    // only ISO646 a-Characters
    m_editPublisher->setValidator( K3bValidators::iso646Validator( K3bValidators::Iso646_a, true, TQT_TQOBJECT(m_editVolumeId) ) );
    m_editPublisher->setMaxLength( 128 );

    m_spinVolumeNumber->setMinValue( 1 );
    m_spinVolumeNumber->setMaxValue( 1 );
    m_spinVolumeCount->setMinValue( 1 );

    TQFrame* line = new TQFrame( w );
    line->setFrameShape( TQFrame::HLine );
    line->setFrameShadow( TQFrame::Sunken );
    line->setFrameShape( TQFrame::HLine );


    // ----------------------------------------------------------------------
    TQGridLayout* grid = new TQGridLayout( w );
    grid->setMargin( marginHint() );
    grid->setSpacing( spacingHint() );

    grid->addWidget( labelVolumeId, 1, 0 );
    grid->addMultiCellWidget( m_editVolumeId, 1, 1, 1, 3 );
    grid->addWidget( labelAlbumId, 2, 0 );
    grid->addMultiCellWidget( m_editAlbumId, 2, 2, 1, 3 );

    grid->addWidget( labelVolumeCount, 3, 0 );
    grid->addWidget( m_spinVolumeCount, 3, 1 );
    grid->addWidget( labelVolumeNumber, 3, 2 );
    grid->addWidget( m_spinVolumeNumber, 3, 3 );

    grid->addWidget( labelPublisher, 4, 0 );
    grid->addMultiCellWidget( m_editPublisher, 4, 4, 1, 3 );

    grid->addMultiCellWidget( line, 5, 5, 0, 3 );

    grid->addWidget( labelSystemId, 6, 0 );
    grid->addMultiCellWidget( labelInfoSystemId, 6, 6, 1, 3 );
    grid->addWidget( labelApplicationId, 7, 0 );
    grid->addMultiCellWidget( labelInfoApplicationId, 7, 7, 1, 3 );

    //  grid->addRowSpacing( 5, 15 );
    grid->setRowStretch( 8, 1 );

    // buddies
    labelVolumeId->setBuddy( m_editVolumeId );
    labelPublisher->setBuddy( m_editPublisher );
    labelAlbumId->setBuddy( m_editAlbumId );

    labelVolumeCount->setBuddy( m_spinVolumeCount );
    labelVolumeNumber->setBuddy( m_spinVolumeNumber );

    // tab order
    setTabOrder( m_editVolumeId, m_editAlbumId );
    setTabOrder( m_editAlbumId, m_spinVolumeCount );
    setTabOrder( m_spinVolumeCount, m_spinVolumeNumber );
    setTabOrder( m_spinVolumeNumber, m_editPublisher );

    addPage( w, i18n( "Volume Descriptor" ) );
}


void K3bVcdBurnDialog::slotStartClicked()
{

    if ( TQFile::exists( vcdDoc() ->vcdImage() ) ) {
        if ( KMessageBox::warningContinueCancel( this, i18n( "Do you want to overwrite %1" ).arg( vcdDoc() ->vcdImage() ), i18n( "File Exists" ), i18n("Overwrite") )
                != KMessageBox::Continue )
            return ;
    }

    K3bProjectBurnDialog::slotStartClicked();
}


void K3bVcdBurnDialog::loadK3bDefaults()
{
    K3bVcdOptions o = K3bVcdOptions::defaults();

    m_writingModeWidget->setWritingMode( K3b::WRITING_MODE_AUTO );
    m_checkSimulate->setChecked( false );
    m_checkRemoveBufferFiles->setChecked( true );
    m_checkOnlyCreateImage->setChecked( false );

    m_checkAutoDetect->setChecked( o.AutoDetect() );
    m_groupVcdFormat->setDisabled( o.AutoDetect() );

    m_check2336->setChecked( o.Sector2336() );
    m_checkNonCompliant->setChecked( o.NonCompliantMode() );
    m_checkVCD30interpretation->setChecked( o.VCD30interpretation() );
    m_spinVolumeCount->setValue( o.volumeCount() );
    m_spinVolumeNumber->setMaxValue( o.volumeCount() );
    m_spinVolumeNumber->setValue( o.volumeNumber() );


    // to not use i18n for this, so user can not use unsupported letters in her language pack
    if ( m_radioSvcd10->isChecked() ) {
        m_checkCdiSupport->setEnabled( false );
        m_checkCdiSupport->setChecked( false );
        m_checkUpdateScanOffsets->setEnabled( true );
        m_editVolumeId->setText( "SUPER_VIDEOCD" );
    } else if ( m_radioHqVcd10->isChecked() ) {
        m_checkCdiSupport->setEnabled( false );
        m_checkCdiSupport->setChecked( false );
        m_checkUpdateScanOffsets->setEnabled( true );
        m_editVolumeId->setText( "HTQ_VIDEOCD" );
    } else {
        m_checkCdiSupport->setEnabled( true );
        m_checkCdiSupport->setChecked( o.CdiSupport() );
        m_groupCdi->setEnabled( o.CdiSupport() );
        m_checkUpdateScanOffsets->setEnabled( false );
        m_editVolumeId->setText( "VIDEOCD" );
    }

    m_editPublisher->setText( o.publisher() );
    m_editAlbumId->setText( o.albumId() );

    m_checkPbc->setChecked( o.PbcEnabled() );
    m_checkSegmentFolder->setChecked( o.SegmentFolder() );
    m_checkRelaxedAps->setChecked( o.RelaxedAps() );
    m_checkUpdateScanOffsets->setChecked( o.UpdateScanOffsets() );
    m_spinRestriction->setValue( o.Restriction() );

    m_checkGaps->setChecked( o.UseGaps() );
    m_spinPreGapLeadout->setValue( o.PreGapLeadout() );
    m_spinPreGapTrack->setValue( o.PreGapTrack() );
    m_spinFrontMarginTrack->setValue( o.FrontMarginTrack() );
    m_spinRearMarginTrack->setValue( o.RearMarginTrack() );
    m_spinFrontMarginTrackSVCD->setValue( o.FrontMarginTrackSVCD() );
    m_spinRearMarginTrackSVCD->setValue( o.RearMarginTrackSVCD() );

    loadDefaultCdiConfig();
}

void K3bVcdBurnDialog::saveSettings()
{
    K3bProjectBurnDialog::saveSettings();

    // set VolumeID if empty
    setVolumeID();

    doc() ->setTempDir( m_tempDirSelectionWidget->tempPath() );
    doc() ->setOnTheFly( false );

    // save image file & path (.bin)
    vcdDoc() ->setVcdImage( m_tempDirSelectionWidget->tempPath() + "/" + m_editVolumeId->text() + ".bin" );

    vcdDoc() ->setVcdType( m_groupVcdFormat->id( m_groupVcdFormat->selected() ) );

    vcdDoc() ->vcdOptions() ->setVolumeId( m_editVolumeId->text() );
    vcdDoc() ->vcdOptions() ->setPublisher( m_editPublisher->text() );
    vcdDoc() ->vcdOptions() ->setAlbumId( m_editAlbumId->text() );

    vcdDoc() ->vcdOptions() ->setAutoDetect( m_checkAutoDetect->isChecked() );
    vcdDoc() ->vcdOptions() ->setNonCompliantMode( m_checkNonCompliant->isChecked() );
    vcdDoc() ->vcdOptions() ->setVCD30interpretation( m_checkVCD30interpretation->isChecked() );
    vcdDoc() ->vcdOptions() ->setSector2336( m_check2336->isChecked() );

    vcdDoc() ->vcdOptions() ->setCdiSupport( m_checkCdiSupport->isChecked() );
    //    vcdDoc() ->setOnlyCreateImages( m_checkOnlyCreateImage->isChecked() );

    vcdDoc() ->vcdOptions() ->setVolumeNumber( m_spinVolumeNumber->value() );
    vcdDoc() ->vcdOptions() ->setVolumeCount( m_spinVolumeCount->value() );

    vcdDoc() ->vcdOptions() ->setPbcEnabled( m_checkPbc->isChecked() );
    if ( m_checkPbc->isChecked() )
        vcdDoc() -> setPbcTracks();

    vcdDoc() ->vcdOptions() ->setSegmentFolder( m_checkSegmentFolder->isChecked() );
    vcdDoc() ->vcdOptions() ->setRelaxedAps( m_checkRelaxedAps->isChecked() );
    vcdDoc() ->vcdOptions() ->setUpdateScanOffsets( m_checkUpdateScanOffsets->isChecked() );
    vcdDoc() ->vcdOptions() ->setRestriction( m_spinRestriction->value() );

    vcdDoc() ->vcdOptions() ->setUseGaps( m_checkGaps->isChecked() );
    vcdDoc() ->vcdOptions() ->setPreGapLeadout( m_spinPreGapLeadout->value() );
    vcdDoc() ->vcdOptions() ->setPreGapTrack( m_spinPreGapTrack->value() );
    vcdDoc() ->vcdOptions() ->setFrontMarginTrack( m_spinFrontMarginTrack->value() );
    vcdDoc() ->vcdOptions() ->setRearMarginTrack( m_spinRearMarginTrack->value() );
    vcdDoc() ->vcdOptions() ->setFrontMarginTrackSVCD( m_spinFrontMarginTrackSVCD->value() );
    vcdDoc() ->vcdOptions() ->setRearMarginTrackSVCD( m_spinRearMarginTrackSVCD->value() );

    if ( m_editCdiCfg->edited() )
        saveCdiConfig();
}


void K3bVcdBurnDialog::readSettings()
{
    K3bProjectBurnDialog::readSettings();

    m_checkNonCompliant->setEnabled( false );
    m_checkVCD30interpretation->setEnabled( false );

    // read vcdType
    switch ( ( ( K3bVcdDoc* ) doc() ) ->vcdType() ) {
        case K3bVcdDoc::VCD11:
            m_radioVcd11->setChecked( true );
            break;
        case K3bVcdDoc::VCD20:
            m_radioVcd20->setChecked( true );
            break;
        case K3bVcdDoc::SVCD10:
            m_radioSvcd10->setChecked( true );
            m_checkNonCompliant->setEnabled( true );
            m_checkVCD30interpretation->setEnabled( true );
            break;
        case K3bVcdDoc::HTQVCD:
            m_radioHqVcd10->setChecked( true );
            break;
        default:
            m_radioVcd20->setChecked( true );
            break;
    }

    m_spinVolumeCount->setValue( vcdDoc() ->vcdOptions() ->volumeCount() );
    m_spinVolumeNumber->setMaxValue( vcdDoc() ->vcdOptions() ->volumeCount() );
    m_spinVolumeNumber->setValue( vcdDoc() ->vcdOptions() ->volumeNumber() );

    m_checkAutoDetect->setChecked( vcdDoc() ->vcdOptions() ->AutoDetect() );
    m_groupVcdFormat->setDisabled( vcdDoc() ->vcdOptions() ->AutoDetect() );

    m_check2336->setChecked( vcdDoc() ->vcdOptions() ->Sector2336() );

    m_checkCdiSupport->setEnabled( false );
    m_checkCdiSupport->setChecked( false );
    m_groupCdi->setEnabled( false );

    if ( m_radioSvcd10->isChecked() ) {
        m_checkNonCompliant->setChecked( vcdDoc() ->vcdOptions() ->NonCompliantMode() );
        m_checkUpdateScanOffsets->setEnabled( true );
        m_checkVCD30interpretation->setChecked( vcdDoc() ->vcdOptions() ->VCD30interpretation() );
    } else if ( m_radioHqVcd10->isChecked() ) {
        // NonCompliant only for SVCD
        m_checkNonCompliant->setChecked( false );
        m_checkNonCompliant->setEnabled( false );
        m_checkUpdateScanOffsets->setEnabled( false );
        m_checkVCD30interpretation->setChecked( false );
        m_checkVCD30interpretation->setEnabled( false );
    } else {
        // NonCompliant only for SVCD
        m_checkNonCompliant->setChecked( false );
        m_checkNonCompliant->setEnabled( false );
        m_checkVCD30interpretation->setChecked( false );
        m_checkUpdateScanOffsets->setEnabled( false );
        m_checkVCD30interpretation->setEnabled( false );
        // CD-I only for VCD and CD-i application was found :)
        if ( vcdDoc() ->vcdOptions() ->checkCdiFiles() ) {
            m_checkCdiSupport->setEnabled( true );
            m_checkCdiSupport->setChecked( vcdDoc() ->vcdOptions() ->CdiSupport() );
        }
    }

    // set VolumeID if empty
    setVolumeID();
    // m_editVolumeId->setText( vcdDoc() ->vcdOptions() ->volumeId() );
    m_editPublisher->setText( vcdDoc() ->vcdOptions() ->publisher() );
    m_editAlbumId->setText( vcdDoc() ->vcdOptions() ->albumId() );

    m_checkPbc->setChecked( vcdDoc() ->vcdOptions() ->PbcEnabled() );
    m_checkSegmentFolder->setChecked( vcdDoc() ->vcdOptions() ->SegmentFolder() );
    m_checkRelaxedAps->setChecked( vcdDoc() ->vcdOptions() ->RelaxedAps() );
    m_checkUpdateScanOffsets->setChecked( vcdDoc() ->vcdOptions() ->UpdateScanOffsets() );
    m_spinRestriction->setValue( vcdDoc() ->vcdOptions() ->Restriction() );

    m_checkGaps->setChecked( vcdDoc() ->vcdOptions() ->UseGaps() );
    slotGapsChecked( m_checkGaps->isChecked() ) ;
    m_spinPreGapLeadout->setValue( vcdDoc() ->vcdOptions() ->PreGapLeadout() );
    m_spinPreGapTrack->setValue( vcdDoc() ->vcdOptions() ->PreGapTrack() );
    m_spinFrontMarginTrack->setValue( vcdDoc() ->vcdOptions() ->FrontMarginTrack() );
    m_spinRearMarginTrack->setValue( vcdDoc() ->vcdOptions() ->RearMarginTrack() );
    m_spinFrontMarginTrackSVCD->setValue( vcdDoc() ->vcdOptions() ->FrontMarginTrackSVCD() );
    m_spinRearMarginTrackSVCD->setValue( vcdDoc() ->vcdOptions() ->RearMarginTrackSVCD() );

    if ( !doc() ->tempDir().isEmpty() )
        m_tempDirSelectionWidget->setTempPath( doc() ->tempDir() );

    loadCdiConfig();
}

void K3bVcdBurnDialog::loadUserDefaults( KConfigBase* c )
{
    K3bProjectBurnDialog::loadUserDefaults( c );

    K3bVcdOptions o = K3bVcdOptions::load( c );

    m_checkAutoDetect->setChecked( o.AutoDetect() );
    m_check2336->setChecked( o.Sector2336() );

    m_checkCdiSupport->setChecked( false );
    m_checkCdiSupport->setEnabled( false );
    m_groupCdi->setEnabled( false );

    if ( m_radioSvcd10->isChecked() ) {
        m_checkNonCompliant->setChecked( o.NonCompliantMode() );
        m_checkVCD30interpretation->setChecked( o.VCD30interpretation() );
    } else {
        m_checkNonCompliant->setChecked( false );
        m_checkNonCompliant->setEnabled( false );
        m_checkVCD30interpretation->setChecked( false );
        m_checkVCD30interpretation->setEnabled( false );
        if ( vcdDoc() ->vcdOptions() ->checkCdiFiles() ) {
            m_checkCdiSupport->setEnabled( true );
            m_checkCdiSupport->setChecked( o.CdiSupport() );
        }
    }

    m_spinVolumeCount->setValue( o.volumeCount() );
    m_spinVolumeNumber->setMaxValue( o.volumeCount() );
    m_spinVolumeNumber->setValue( o.volumeNumber() );

    m_editVolumeId->setText( o.volumeId() );
    m_editPublisher->setText( o.publisher() );
    m_editAlbumId->setText( o.albumId() );
    m_checkPbc->setChecked( o.PbcEnabled() );
    m_checkSegmentFolder->setChecked( o.SegmentFolder() );
    m_checkRelaxedAps->setChecked( o.RelaxedAps() );
    m_checkUpdateScanOffsets->setChecked( o.UpdateScanOffsets() );
    m_spinRestriction->setValue( o.Restriction() );

    m_checkGaps->setChecked( o.UseGaps() );
    m_spinPreGapLeadout->setValue( o.PreGapLeadout() );
    m_spinPreGapTrack->setValue( o.PreGapTrack() );
    m_spinFrontMarginTrack->setValue( o.FrontMarginTrack() );
    m_spinRearMarginTrack->setValue( o.RearMarginTrack() );
    m_spinFrontMarginTrackSVCD->setValue( o.FrontMarginTrackSVCD() );
    m_spinRearMarginTrackSVCD->setValue( o.RearMarginTrackSVCD() );

    loadCdiConfig();
}


void K3bVcdBurnDialog::saveUserDefaults( KConfigBase* c )
{
    K3bProjectBurnDialog::saveUserDefaults( c );

    K3bVcdOptions o;

    o.setVolumeId( m_editVolumeId->text() );
    o.setPublisher( m_editPublisher->text() );
    o.setAlbumId( m_editAlbumId->text() );
    o.setAutoDetect( m_checkAutoDetect->isChecked() );
    o.setNonCompliantMode( m_checkNonCompliant->isChecked() );
    o.setVCD30interpretation( m_checkVCD30interpretation->isChecked() );
    o.setSector2336( m_check2336->isChecked() );
    o.setVolumeCount( m_spinVolumeCount->value() );
    o.setVolumeNumber( m_spinVolumeNumber->value() );
    o.setCdiSupport( m_checkCdiSupport->isChecked() );
    o.setPbcEnabled( m_checkPbc->isChecked() );
    o.setSegmentFolder( m_checkSegmentFolder->isChecked() );
    o.setRelaxedAps( m_checkRelaxedAps->isChecked() );
    o.setUpdateScanOffsets( m_checkUpdateScanOffsets->isChecked() );
    o.setRestriction( m_spinRestriction->value() );
    o.setUseGaps( m_checkGaps->isChecked() );
    o.setPreGapLeadout( m_spinPreGapLeadout->value() );
    o.setPreGapTrack( m_spinPreGapTrack->value() );
    o.setFrontMarginTrack( m_spinFrontMarginTrack->value() );
    o.setRearMarginTrack( m_spinRearMarginTrack->value() );
    o.setFrontMarginTrackSVCD( m_spinFrontMarginTrackSVCD->value() );
    o.setRearMarginTrackSVCD( m_spinRearMarginTrackSVCD->value() );

    o.save( c );

    saveCdiConfig();
}

void K3bVcdBurnDialog::saveCdiConfig()
{

    TQString filename = locateLocal( "appdata", "cdi/cdi_vcd.cfg" );
    if ( TQFile::exists( filename ) )
        TQFile::remove
            ( filename );

    TQFile cdi( filename );
    if ( !cdi.open( IO_WriteOnly ) )
        return ;

    TQTextStream s( &cdi );
    int i = m_editCdiCfg->numLines();

    for ( int j = 0; j < i; j++ )
        s << TQString( "%1" ).arg( m_editCdiCfg->textLine( j ) ) << "\n";

    cdi.close();

    m_editCdiCfg->setEdited( false );
}

void K3bVcdBurnDialog::loadCdiConfig()
{
    TQString filename = locateLocal( "appdata", "cdi/cdi_vcd.cfg" );
    if ( TQFile::exists( filename ) ) {
        TQFile cdi( filename );
        if ( !cdi.open( IO_ReadOnly ) ) {
            loadDefaultCdiConfig();
            return ;
        }

        TQTextStream s( &cdi );

        m_editCdiCfg->clear();

        while ( !s.atEnd() )
            m_editCdiCfg->insertLine( s.readLine() );

        cdi.close();
        m_editCdiCfg->setEdited( false );
        m_editCdiCfg->setCursorPosition( 0, 0, false );
        m_groupCdi->setEnabled( m_checkCdiSupport->isChecked() );
    } else
        loadDefaultCdiConfig();

}

void K3bVcdBurnDialog::loadDefaultCdiConfig()
{
    TQString filename = locate( "data", "k3b/cdi/cdi_vcd.cfg" );
    if ( TQFile::exists( filename ) ) {
        TQFile cdi( filename );
        if ( !cdi.open( IO_ReadOnly ) ) {
            m_checkCdiSupport->setChecked( false );
            m_checkCdiSupport->setEnabled( false );
            return ;
        }

        TQTextStream s( &cdi );

        m_editCdiCfg->clear();

        while ( !s.atEnd() )
            m_editCdiCfg->insertLine( s.readLine() );

        cdi.close();
        m_editCdiCfg->setEdited( false );
        m_editCdiCfg->setCursorPosition( 0, 0, false );
        m_groupCdi->setEnabled( m_checkCdiSupport->isChecked() );
    }
}

void K3bVcdBurnDialog::setVolumeID()
{
    if ( m_editVolumeId->text().length() < 1 ) {
        if ( m_radioSvcd10->isChecked() )
            m_editVolumeId->setText( "SUPER_VIDEOCD" );
        else if ( m_radioHqVcd10->isChecked() )
            m_editVolumeId->setText( "HTQ_VIDEOCD" );
        else
            m_editVolumeId->setText( "VIDEOCD" );
    }
}

void K3bVcdBurnDialog::slotSpinVolumeCount()
{
    m_spinVolumeNumber->setMaxValue( m_spinVolumeCount->value() );
}

void K3bVcdBurnDialog::slotVcdTypeClicked( int i )
{

    switch ( i ) {
        case 0:
            // vcd 1.1 no support for version 3.x.
            // v4 work also for vcd 1.1 but without CD-i menus.
            // Do anybody use vcd 1.1 with cd-i????
            m_checkCdiSupport->setEnabled( vcdDoc() ->vcdOptions() ->checkCdiFiles() );
            m_checkCdiSupport->setChecked( false );

            m_checkNonCompliant->setEnabled( false );
            m_checkNonCompliant->setChecked( false );
            m_checkVCD30interpretation->setEnabled( false );
            m_checkVCD30interpretation->setChecked( false );
            m_checkUpdateScanOffsets->setEnabled( false );
            m_checkUpdateScanOffsets->setChecked( false );
            break;
        case 1:
            //vcd 2.0
            m_checkCdiSupport->setEnabled( vcdDoc() ->vcdOptions() ->checkCdiFiles() );
            m_groupCdi->setEnabled( m_checkCdiSupport->isChecked() );

            m_checkNonCompliant->setEnabled( false );
            m_checkNonCompliant->setChecked( false );
            m_checkVCD30interpretation->setEnabled( false );
            m_checkVCD30interpretation->setChecked( false );
            m_checkUpdateScanOffsets->setEnabled( false );
            m_checkUpdateScanOffsets->setChecked( false );
            break;
        case 2:
            //svcd 1.0
            m_checkCdiSupport->setEnabled( false );
            m_checkCdiSupport->setChecked( false );
            m_groupCdi->setEnabled( false );

            m_checkNonCompliant->setEnabled( true );
            m_checkVCD30interpretation->setEnabled( true );
            m_checkUpdateScanOffsets->setEnabled( true );
            break;
        case 3:
            //hqvcd 1.0
            m_checkCdiSupport->setEnabled( false );
            m_checkCdiSupport->setChecked( false );
            m_groupCdi->setEnabled( false );

            m_checkNonCompliant->setEnabled( false );
            m_checkNonCompliant->setChecked( false );
            m_checkVCD30interpretation->setEnabled( false );
            m_checkVCD30interpretation->setChecked( false );
            m_checkUpdateScanOffsets->setEnabled( true );
            break;
    }

    MarginChecked( m_checkGaps->isChecked() );

}

void K3bVcdBurnDialog::slotGapsChecked( bool b )
{
    m_labelPreGapLeadout->setEnabled( b );
    m_spinPreGapLeadout->setEnabled( b );
    m_labelPreGapTrack->setEnabled( b );
    m_spinPreGapTrack->setEnabled( b );
    MarginChecked( b );
}

void K3bVcdBurnDialog::MarginChecked( bool b )
{
    if ( m_radioSvcd10->isChecked() || m_radioHqVcd10->isChecked() ) {
        m_spinFrontMarginTrack->setHidden( true );
        m_spinFrontMarginTrackSVCD->setHidden( false );
        m_spinRearMarginTrack->setHidden( true );
        m_spinRearMarginTrackSVCD->setHidden( false );
    } else {
        m_spinFrontMarginTrack->setHidden( false );
        m_spinFrontMarginTrackSVCD->setHidden( true );
        m_spinRearMarginTrack->setHidden( false );
        m_spinRearMarginTrackSVCD->setHidden( true );
    }

    m_labelFrontMarginTrack->setEnabled( b );
    m_spinFrontMarginTrack->setEnabled( b );
    m_spinFrontMarginTrackSVCD->setEnabled( b );

    m_labelRearMarginTrack->setEnabled( b );
    m_spinRearMarginTrack->setEnabled( b );
    m_spinRearMarginTrackSVCD->setEnabled( b );

}

void K3bVcdBurnDialog::slotCdiSupportChecked( bool b )
{
    m_groupCdi->setEnabled( b );
}

void K3bVcdBurnDialog::slotAutoDetect( bool b )
{
    if ( b ) {
        m_groupVcdFormat->setButton( vcdDoc() ->vcdOptions() ->mpegVersion() );
        slotVcdTypeClicked( vcdDoc() ->vcdOptions() ->mpegVersion() );
    }

    m_groupVcdFormat->setDisabled( b );

}

void K3bVcdBurnDialog::toggleAll()
{
    K3bProjectBurnDialog::toggleAll();

    m_writingModeWidget->setSupportedModes( K3b::DAO );
    m_checkRemoveBufferFiles->setDisabled( m_checkOnlyCreateImage->isChecked() );
}

#include "k3bvcdburndialog.moc"
