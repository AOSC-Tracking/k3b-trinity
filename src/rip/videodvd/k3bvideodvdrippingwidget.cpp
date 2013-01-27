/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bvideodvdrippingwidget.h"

#include <k3bvideodvdtitletranscodingjob.h>
#include <k3bglobals.h>
#include <k3brichtextlabel.h>
#include <k3bintmapcombobox.h>

#include <klistview.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <tdeio/global.h>
#include <kurllabel.h>
#include <kdialogbase.h>
#include <klineedit.h>

#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqlayout.h>


static const int s_mp3Bitrates[] = {
  32,
  40,
  48,
  56,
  64,
  80,
  96,
  112,
  128,
  160,
  192,
  224,
  256,
  320,
  0 // just used for the loops below
};


static const int PICTURE_SIZE_ORIGINAL = 0;
static const int PICTURE_SIZE_640 = 1;
static const int PICTURE_SIZE_320 = 2;
static const int PICTURE_SIZE_CUSTOM = 3;
static const int PICTURE_SIZE_MAX = 4;

static const char* s_pictureSizeNames[] = {
  I18N_NOOP("Keep original dimensions"),
  I18N_NOOP("640x? (automatic height)"),
  I18N_NOOP("320x? (automatic height)"),
  I18N_NOOP("Custom")
};


K3bVideoDVDRippingWidget::K3bVideoDVDRippingWidget( TQWidget* parent )
  : base_K3bVideoDVDRippingWidget( parent )
{
  m_editBaseDir->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

  m_titleView->addColumn( i18n("Title") );
  m_titleView->addColumn( i18n("Video Size") );
  m_titleView->addColumn( i18n("File Size") );
  m_titleView->addColumn( i18n("Filename") );
  m_titleView->setSorting( -1 );

  //
  // Example filename pattern
  //
  m_comboFilenamePattern->insertItem( TQString( "%b - %1 %t (%n %a %c)" ).arg(i18n("Title") ) );
  m_comboFilenamePattern->insertItem( TQString( "%{volumeid} (%{title})" ) );


  //
  // Add the Audio bitrates
  //
  for( int i = 0; s_mp3Bitrates[i]; ++i )
    m_comboAudioBitrate->insertItem( i18n("%1 kbps" ).arg(s_mp3Bitrates[i]) );


  for( int i = 0; i < K3bVideoDVDTitleTranscodingJob::VIDEO_CODEC_NUM_ENTRIES; ++i ) {
    K3bVideoDVDTitleTranscodingJob::VideoCodec codec( (K3bVideoDVDTitleTranscodingJob::VideoCodec)i );
    if( K3bVideoDVDTitleTranscodingJob::transcodeBinaryHasSupportFor( codec ) )
      m_comboVideoCodec->insertItem( i,
				     K3bVideoDVDTitleTranscodingJob::videoCodecString( codec ),
				     K3bVideoDVDTitleTranscodingJob::videoCodecDescription( codec ) );
  }
  for( int i = 0; i < K3bVideoDVDTitleTranscodingJob::AUDIO_CODEC_NUM_ENTRIES; ++i ) {
    K3bVideoDVDTitleTranscodingJob::AudioCodec codec( (K3bVideoDVDTitleTranscodingJob::AudioCodec)i );
    if( K3bVideoDVDTitleTranscodingJob::transcodeBinaryHasSupportFor( codec ) )
      m_comboAudioCodec->insertItem( i,
				     K3bVideoDVDTitleTranscodingJob::audioCodecString( codec ), 
				     K3bVideoDVDTitleTranscodingJob::audioCodecDescription( codec ) );
  }

  for( int i = 0; i < PICTURE_SIZE_MAX; ++i ) {
    m_comboVideoSize->insertItem( i18n( s_pictureSizeNames[i] ) );
  }

  slotAudioCodecChanged( m_comboAudioCodec->selectedValue() );

  connect( m_comboAudioBitrate, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SIGNAL(changed()) );
  connect( m_spinVideoBitrate, TQT_SIGNAL(valueChanged(int)),
	   this, TQT_SIGNAL(changed()) );
  connect( m_checkBlankReplace, TQT_SIGNAL(toggled(bool)),
	   this, TQT_SIGNAL(changed()) );
  connect( m_editBlankReplace, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SIGNAL(changed()) );
  connect( m_comboFilenamePattern, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SIGNAL(changed()) );
  connect( m_editBaseDir, TQT_SIGNAL(textChanged(const TQString&)), 
	   this, TQT_SIGNAL(changed()) );

  connect( m_comboAudioCodec, TQT_SIGNAL(valueChanged(int)),
	   this, TQT_SLOT(slotAudioCodecChanged(int)) );
  connect( m_specialStringsLabel, TQT_SIGNAL(leftClickedURL()),
	   this, TQT_SLOT(slotSeeSpecialStrings()) );
  connect( m_buttonCustomPictureSize, TQT_SIGNAL(clicked()),
	   this, TQT_SLOT(slotCustomPictureSize()) );
  connect( m_comboVideoSize, TQT_SIGNAL(activated(int)),
	   this, TQT_SLOT(slotVideoSizeChanged(int)) );

  // refresh every 2 seconds
  m_freeSpaceUpdateTimer = new TQTimer( this );
  connect( m_freeSpaceUpdateTimer, TQT_SIGNAL(timeout()),
	   this, TQT_SLOT(slotUpdateFreeTempSpace()) );
  m_freeSpaceUpdateTimer->start(2000);
  slotUpdateFreeTempSpace();
}


K3bVideoDVDRippingWidget::~K3bVideoDVDRippingWidget()
{
}


K3bVideoDVDTitleTranscodingJob::VideoCodec K3bVideoDVDRippingWidget::selectedVideoCodec() const
{
  return (K3bVideoDVDTitleTranscodingJob::VideoCodec)m_comboVideoCodec->selectedValue();
}


TQSize K3bVideoDVDRippingWidget::selectedPictureSize() const
{
  switch( m_comboVideoSize->currentItem() ) {
  case PICTURE_SIZE_ORIGINAL:
    return TQSize(0,0);
  case PICTURE_SIZE_640:
    return TQSize(640,0);
  case PICTURE_SIZE_320:
    return TQSize(320,0);
  default:
    return m_customVideoSize;
  }
}


void K3bVideoDVDRippingWidget::setSelectedPictureSize( const TQSize& size )
{
  m_customVideoSize = size;
  if( size == TQSize(0,0) )
    m_comboVideoSize->setCurrentItem( PICTURE_SIZE_ORIGINAL );
  else if( size == TQSize(640,0) )
    m_comboVideoSize->setCurrentItem( PICTURE_SIZE_640 );
  else if( size == TQSize(320,0) )
    m_comboVideoSize->setCurrentItem( PICTURE_SIZE_320 );
  else {
    m_comboVideoSize->changeItem( i18n(s_pictureSizeNames[PICTURE_SIZE_CUSTOM])
				  + TQString(" (%1x%2)")
				  .arg(size.width() == 0 ? i18n("auto") : TQString::number(size.width()))
				  .arg(size.height() == 0 ? i18n("auto") : TQString::number(size.height())),
				  PICTURE_SIZE_CUSTOM );
    m_comboVideoSize->setCurrentItem( PICTURE_SIZE_CUSTOM );
  }
}


void K3bVideoDVDRippingWidget::setSelectedVideoCodec( K3bVideoDVDTitleTranscodingJob::VideoCodec codec )
{
  m_comboVideoCodec->setSelectedValue( (int)codec );
}


K3bVideoDVDTitleTranscodingJob::AudioCodec K3bVideoDVDRippingWidget::selectedAudioCodec() const
{
  return (K3bVideoDVDTitleTranscodingJob::AudioCodec)m_comboAudioCodec->selectedValue();
}


void K3bVideoDVDRippingWidget::setSelectedAudioCodec( K3bVideoDVDTitleTranscodingJob::AudioCodec codec )
{
  m_comboAudioCodec->setSelectedValue( (int)codec );
  slotAudioCodecChanged( (int)codec );
}


int K3bVideoDVDRippingWidget::selectedAudioBitrate() const
{
  if( selectedAudioCodec() == K3bVideoDVDTitleTranscodingJob::AUDIO_CODEC_MP3 )
    return s_mp3Bitrates[m_comboAudioBitrate->currentItem()];
  else
    return m_spinAudioBitrate->value();
}


void K3bVideoDVDRippingWidget::setSelectedAudioBitrate( int bitrate )
{
  m_spinAudioBitrate->setValue( bitrate );

  // select the bitrate closest to "bitrate"
  int bi = 0;
  int diff = 1000;
  for( int i = 0; s_mp3Bitrates[i]; ++i ) {
    int newDiff = s_mp3Bitrates[i] - bitrate;
    if( newDiff < 0 )
      newDiff = -1 * newDiff;
    if( newDiff < diff ) {
      diff = newDiff;
      bi = i;
    }
  }

  m_comboAudioBitrate->setCurrentItem( bi );
}


void K3bVideoDVDRippingWidget::slotUpdateFreeTempSpace()
{
  TQString path = m_editBaseDir->url();

  if( !TQFile::exists( path ) )
    path.truncate( path.findRev('/') );

  unsigned long size, avail;
  if( K3b::kbFreeOnFs( path, size, avail ) ) {
    m_labelFreeSpace->setText( TDEIO::convertSizeFromKB(avail) );
    if( avail < m_neededSize/1024 )
      m_labelNeededSpace->setPaletteForegroundColor( TQt::red );
    else
      m_labelNeededSpace->setPaletteForegroundColor( paletteForegroundColor() );
  }
  else {
    m_labelFreeSpace->setText("-");
    m_labelNeededSpace->setPaletteForegroundColor( paletteForegroundColor() );
  }
}


void K3bVideoDVDRippingWidget::setNeededSize( TDEIO::filesize_t size )
{
  m_neededSize = size;
  if( size > 0 )
    m_labelNeededSpace->setText( TDEIO::convertSize( size ) );
  else
    m_labelNeededSpace->setText( i18n("unknown") );

  slotUpdateFreeTempSpace();
}


void K3bVideoDVDRippingWidget::slotSeeSpecialStrings()
{
  TQWhatsThis::display( i18n( "<p><b>Pattern special strings:</b>"
			     "<p>The following strings will be replaced with their respective meaning in every "
			     "track name.<br>"
                             "<p><table border=\"0\">"
			     "<tr><td></td><td><em>Meaning</em></td><td><em>Alternatives</em></td></tr>"
                             "<tr><td>%t</td><td>title number</td><td>%{t} or %{title_number}</td></tr>"
                             "<tr><td>%i</td><td>volume id (mostly the name of the Video DVD)</td><td>%{i} or %{volume_id}</td></tr>"
                             "<tr><td>%b</td><td>beautified volume id</td><td>%{b} or %{beautified_volume_id}</td></tr>"
                             "<tr><td>%l</td><td>two chars language code</td><td>%{l} or %{lang_code}</td></tr>"
                             "<tr><td>%n</td><td>language name</td><td>%{n} or %{lang_name}</td></tr>"
                             "<tr><td>%a</td><td>audio format (on the Video DVD)</td><td>%{a} or %{audio_format}</td></tr>"
                             "<tr><td>%c</td><td>number of audio channels (on the Video DVD)</td><td>%{c} or %{channels}</td></tr>"
                             "<tr><td>%v</td><td>size of the original video</td><td>%{v} or %{orig_video_size}</td></tr>"
                             "<tr><td>%s</td><td>size of the resulting video (<em>Caution: auto-clipping values are not taken into account!</em>)</td><td>%{s} or %{video_size}</td></tr>"
                             "<tr><td>%r</td><td>aspect ratio of the original video</td><td>%{r} or %{aspect_ratio}</td></tr>"
                             "<tr><td>%d</td><td>current date</td><td>%{d} or %{date}</td></tr>"
                             "</table>"
			     "<p><em>Hint: K3b also accepts slight variations of the long special strings. "
			     "One can, for example, leave out the underscores.</em>") );
}


void K3bVideoDVDRippingWidget::slotAudioCodecChanged( int codec )
{
  switch( codec ) {
  case K3bVideoDVDTitleTranscodingJob::AUDIO_CODEC_MP3:
    m_stackAudioQuality->raiseWidget( m_stackPageAudioQualityMp3 );
    break;
  case K3bVideoDVDTitleTranscodingJob::AUDIO_CODEC_AC3_STEREO:
    m_stackAudioQuality->raiseWidget( m_stackPageAudioQualityAC3 );
    break;
  case K3bVideoDVDTitleTranscodingJob::AUDIO_CODEC_AC3_PASSTHROUGH:
    m_stackAudioQuality->raiseWidget( m_stackPageAudioQualityAC3Pt );
    break;
  }

  emit changed();
}


void K3bVideoDVDRippingWidget::slotVideoSizeChanged( int sizeIndex )
{
  if( sizeIndex == PICTURE_SIZE_CUSTOM )
    slotCustomPictureSize();
  else
    emit changed();
}


void K3bVideoDVDRippingWidget::slotCustomPictureSize()
{
  KDialogBase dlg( KDialogBase::Plain,
		   i18n("Video Picture Size"),
		   KDialogBase::Ok|KDialogBase::Cancel,
		   KDialogBase::Ok,
		   this,
		   0,
		   true,
		   true );
  K3bRichTextLabel* label = new K3bRichTextLabel( i18n("<p>Please choose the width and height of the resulting video. "
						       "If one value is set to <em>Auto</em> K3b will choose this value "
						       "depending on the aspect ratio of the video picture.<br>"
						       "Be aware that setting both the width and the height to fixed values "
						       "will result in no aspect ratio correction to be performed."), 
					  dlg.plainPage() );
  TQSpinBox* spinWidth = new TQSpinBox( 0, 20000, 16, dlg.plainPage() );
  TQSpinBox* spinHeight = new TQSpinBox( 0, 20000, 16, dlg.plainPage() );
  spinWidth->setSpecialValueText( i18n("Auto") );
  spinHeight->setSpecialValueText( i18n("Auto") );
  TQLabel* labelW = new TQLabel( spinWidth, i18n("Width") + ':', dlg.plainPage() );
  TQLabel* labelH = new TQLabel( spinHeight, i18n("Height") + ':', dlg.plainPage() );
  labelW->setAlignment( TQt::AlignRight|TQt::AlignVCenter );
  labelH->setAlignment( TQt::AlignRight|TQt::AlignVCenter );

  TQGridLayout* grid = new TQGridLayout( dlg.plainPage() );
  grid->setMargin( 0 );
  grid->setSpacing( KDialog::spacingHint() );
  grid->addMultiCellWidget( label, 0, 0, 0, 3 );
  grid->addWidget( labelW, 1, 0 );
  grid->addWidget( spinWidth, 1, 1 );
  grid->addWidget( labelH, 1, 2 );
  grid->addWidget( spinHeight, 1, 3 );

  spinWidth->setValue( m_customVideoSize.width() );
  spinHeight->setValue( m_customVideoSize.height() );

  if( dlg.exec() ) {
    setSelectedPictureSize( TQSize( spinWidth->value(), spinHeight->value() ) );
    emit changed();
  }
}

#include "k3bvideodvdrippingwidget.moc"
