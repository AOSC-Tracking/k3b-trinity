/*
 *
 * $Id: k3bfillstatusdisplay.cpp 768504 2008-01-30 08:53:22Z trueg $
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


#include "k3bfillstatusdisplay.h"
#include "k3bdoc.h"

#include <k3bapplication.h>
#include <k3bmediaselectiondialog.h>
#include <k3bdevice.h>
#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bmsf.h>
#include <k3bradioaction.h>
#include <k3bmediacache.h>

#include <tqevent.h>
#include <tqpainter.h>
#include <tqcolor.h>
#include <tqrect.h>
#include <tqfont.h>
#include <tqfontmetrics.h>
#include <tqvalidator.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqtimer.h>

#include <tdeaction.h>
#include <tdepopupmenu.h>
#include <tdelocale.h>
#include <kinputdialog.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <tdeio/global.h>
#include <tdemessagebox.h>
#include <tdeglobal.h>
#include <kpixmapeffect.h>


static const int DEFAULT_CD_SIZE_74 = 74*60*75;
static const int DEFAULT_CD_SIZE_80 = 80*60*75;
static const int DEFAULT_CD_SIZE_100 = 100*60*75;
static const int DEFAULT_DVD_SIZE_4_4 = 2295104;
static const int DEFAULT_DVD_SIZE_8_0 = 4173824;

class K3bFillStatusDisplayWidget::Private
{
public:
  K3b::Msf cdSize;
  bool showTime;
  K3bDoc* doc;
};


K3bFillStatusDisplayWidget::K3bFillStatusDisplayWidget( K3bDoc* doc, TQWidget* parent )
  : TQWidget( parent, 0, WRepaintNoErase )
{
  d = new Private();
  d->doc = doc;
  setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Preferred ) );
}


K3bFillStatusDisplayWidget::~K3bFillStatusDisplayWidget()
{
  delete d;
}


const K3b::Msf& K3bFillStatusDisplayWidget::cdSize() const
{
  return d->cdSize;
}


void K3bFillStatusDisplayWidget::setShowTime( bool b )
{
  d->showTime = b;
  update();
}


void K3bFillStatusDisplayWidget::setCdSize( const K3b::Msf& size )
{
  d->cdSize = size;
  update();
}


TQSize K3bFillStatusDisplayWidget::sizeHint() const
{
  return minimumSizeHint();
}


TQSize K3bFillStatusDisplayWidget::minimumSizeHint() const
{
  int margin = 2;
  TQFontMetrics fm( font() );
  return TQSize( -1, fm.height() + 2 * margin );
}


void K3bFillStatusDisplayWidget::mousePressEvent( TQMouseEvent* e )
{
  if( e->button() == Qt::RightButton )
    emit contextMenu( e->globalPos() );
}


void K3bFillStatusDisplayWidget::paintEvent( TQPaintEvent* )
{
  // double buffer
  TQPixmap buffer( size() );
  buffer.fill( colorGroup().base() );
  TQPainter p;
  p.begin( &buffer, TQT_TQOBJECT(this) );
  p.setPen( TQt::black ); // we use a fixed bar color (which is not very nice btw, so we also fix the text color)

  long long docSize;
  long long cdSize;
  long long maxValue;
  long tolerance;

  if( d->showTime ) {
    docSize = d->doc->length().totalFrames() / 75 / 60;
    cdSize = d->cdSize.totalFrames() / 75 / 60;
    maxValue = (cdSize > docSize ? cdSize : docSize) + 10;
    tolerance = 1;
  }
  else {
    docSize = d->doc->size()/1024/1024;
    cdSize = d->cdSize.mode1Bytes()/1024/1024;
    maxValue = (cdSize > docSize ? cdSize : docSize) + 100;
    tolerance = 10;
  }

  // so split width() in maxValue pieces
  double one = (double)rect().width() / (double)maxValue;
  TQRect crect( rect() );
  crect.setWidth( (int)(one*(double)docSize) );

  p.setClipping(true);
  p.setClipRect(crect);

  p.fillRect( crect, TQt::green );

  TQRect oversizeRect(crect);
  // draw yellow if cdSize - tolerance < docSize
  if( docSize > cdSize - tolerance ) {
    oversizeRect.setLeft( oversizeRect.left() + (int)(one * (cdSize - tolerance)) );
    p.fillRect( oversizeRect, TQt::yellow );
    KPixmap pix;
    pix.resize( rect().height()*2, rect().height() );
    KPixmapEffect::gradient( pix, green, yellow, KPixmapEffect::HorizontalGradient, 0 );
    p.drawPixmap( oversizeRect.left() - pix.width()/2, 0, pix );
  }

  // draw red if docSize > cdSize + tolerance
  if( docSize > cdSize + tolerance ) {
    oversizeRect.setLeft( oversizeRect.left() + (int)(one * tolerance*2) );
    p.fillRect( oversizeRect, TQt::red );
    KPixmap pix;
    pix.resize( rect().height()*2, rect().height() );
    KPixmapEffect::gradient( pix, yellow, red, KPixmapEffect::HorizontalGradient, 0 );
    p.drawPixmap( oversizeRect.left() - pix.width()/2, 0, pix );
  }

  p.setClipping(false);

  // ====================================================================================
  // Now the colored bar is painted
  // Continue with the texts
  // ====================================================================================

  // first we determine the text to display
  // ====================================================================================
  TQString docSizeText;
  if( d->showTime )
    docSizeText = d->doc->length().toString(false) + " " + i18n("min");
  else
    docSizeText = TDEIO::convertSize( d->doc->size() );

  TQString overSizeText;
  if( d->cdSize.mode1Bytes() >= d->doc->size() )
    overSizeText = i18n("Available: %1 of %2")
      .arg( d->showTime
	    ? i18n("%1 min").arg((K3b::Msf( cdSize*60*75 ) - d->doc->length()).toString(false))
	    : TDEIO::convertSize( TQMAX( (cdSize * 1024LL * 1024LL) - (long long)d->doc->size(), 0LL ) ) )
      .arg( d->showTime
	    ? i18n("%1 min").arg(K3b::Msf( cdSize*60*75 ).toString(false))
	    : TDEIO::convertSizeFromKB( cdSize * 1024 ) );
  else
    overSizeText = i18n("Capacity exceeded by %1")
      .arg( d->showTime
	    ? i18n("%1 min").arg( (d->doc->length() - K3b::Msf( cdSize*60*75 ) ).toString(false))
	    : TDEIO::convertSize( (long long)d->doc->size() - (cdSize * 1024LL * 1024LL) ) );
  // ====================================================================================

  // draw the medium size marker
  // ====================================================================================
  int mediumSizeMarkerPos = rect().left() + (int)(one*cdSize);
  p.drawLine( mediumSizeMarkerPos, rect().bottom(),
	      mediumSizeMarkerPos, rect().top() + ((rect().bottom()-rect().top())/2) );
  // ====================================================================================



  // we want to draw the docSizeText centered in the filled area
  // if there is not enough space we just align it left
  // ====================================================================================
  int docSizeTextPos = 0;
  int docSizeTextLength = fontMetrics().width(docSizeText);
  if( docSizeTextLength + 5 > crect.width() ) {
    docSizeTextPos = crect.left() + 5; // a little margin
  }
  else {
    docSizeTextPos = ( crect.width() - docSizeTextLength ) / 2;

    // make sure the text does not cross the medium size marker
    if( docSizeTextPos <= mediumSizeMarkerPos && mediumSizeMarkerPos <= docSizeTextPos + docSizeTextLength )
      docSizeTextPos = TQMAX( crect.left() + 5, mediumSizeMarkerPos - docSizeTextLength - 5 );
  }
  // ====================================================================================

  // draw the over size text
  // ====================================================================================
  TQFont fnt(font());
  fnt.setPointSize( TQMAX( 8, fnt.pointSize()-4 ) );
  fnt.setBold(false);

  TQRect overSizeTextRect( rect() );
  int overSizeTextLength = TQFontMetrics(fnt).width(overSizeText);
  if( overSizeTextLength + 5 > overSizeTextRect.width() - (int)(one*cdSize) ) {
    // we don't have enough space on the right, so we paint to the left of the line
    overSizeTextRect.setLeft( (int)(one*cdSize) - overSizeTextLength - 5 );
  }
  else {
    overSizeTextRect.setLeft( mediumSizeMarkerPos + 5 );
  }

  // make sure the two text do not overlap (this does not cover all cases though)
  if( overSizeTextRect.left() < docSizeTextPos + docSizeTextLength )
    docSizeTextPos = TQMAX( crect.left() + 5, TQMIN( overSizeTextRect.left() - docSizeTextLength - 5, mediumSizeMarkerPos - docSizeTextLength - 5 ) );

  TQRect docTextRect( rect() );
  docTextRect.setLeft( docSizeTextPos );
  p.drawText( docTextRect, TQt::AlignLeft | TQt::AlignVCenter, docSizeText );

  p.setFont(fnt);
  p.drawText( overSizeTextRect, TQt::AlignLeft | TQt::AlignVCenter, overSizeText );
  // ====================================================================================

  p.end();

  bitBlt( this, 0, 0, &buffer );
}



// ----------------------------------------------------------------------------------------------------


class K3bFillStatusDisplay::ToolTip : public TQToolTip
{
public:
  ToolTip( K3bDoc* doc, TQWidget* parent )
    : TQToolTip( parent, 0 ),
      m_doc(doc) {
  }

  void maybeTip( const TQPoint& ) {
    tip( parentWidget()->rect(),
	 TDEIO::convertSize( m_doc->size() ) +
	 " (" + TDEGlobal::locale()->formatNumber( m_doc->size(), 0 ) + "), " +
	 m_doc->length().toString(false) + " " + i18n("min") +
	 " (" + i18n("Right click for media sizes") + ")");
  }

private:
  K3bDoc* m_doc;
};

class K3bFillStatusDisplay::Private
{
public:
  TDEActionCollection* actionCollection;
  TDERadioAction* actionShowMinutes;
  TDERadioAction* actionShowMegs;
  TDERadioAction* actionAuto;
  TDERadioAction* action74Min;
  TDERadioAction* action80Min;
  TDERadioAction* action100Min;
  TDERadioAction* actionDvd4_7GB;
  TDERadioAction* actionDvdDoubleLayer;
  K3bRadioAction* actionCustomSize;
  K3bRadioAction* actionDetermineSize;
  TDEAction* actionSaveUserDefaults;
  TDEAction* actionLoadUserDefaults;

  TDEPopupMenu* popup;
  TDEPopupMenu* dvdPopup;

  TQToolButton* buttonMenu;

  K3bFillStatusDisplayWidget* displayWidget;

  bool showDvdSizes;
  bool showTime;

  K3bDoc* doc;

  TQTimer updateTimer;
};


K3bFillStatusDisplay::K3bFillStatusDisplay( K3bDoc* doc, TQWidget *parent, const char *name )
  : TQFrame(parent,name)
{
  d = new Private;
  d->doc = doc;

  m_toolTip = new ToolTip( doc, this );

  setFrameStyle( Panel | Sunken );

  d->displayWidget = new K3bFillStatusDisplayWidget( doc, this );
//   d->buttonMenu = new TQToolButton( this );
//   d->buttonMenu->setIconSet( SmallIconSet("media-optical-cdrom-unmounted") );
//   d->buttonMenu->setAutoRaise(true);
//   TQToolTip::add( d->buttonMenu, i18n("Fill display properties") );
//   connect( d->buttonMenu, TQT_SIGNAL(clicked()), TQT_TQOBJECT(this), TQT_SLOT(slotMenuButtonClicked()) );

  TQGridLayout* layout = new TQGridLayout( this );
  layout->setSpacing(5);
  layout->setMargin(frameWidth());
  layout->addWidget( d->displayWidget, 0, 0 );
  //  layout->addWidget( d->buttonMenu, 0, 1 );
  layout->setColStretch( 0, 1 );

  setupPopupMenu();

  showDvdSizes( false );

  connect( d->doc, TQT_SIGNAL(changed()), TQT_TQOBJECT(this), TQT_SLOT(slotDocChanged()) );
  connect( &d->updateTimer, TQT_SIGNAL(timeout()), TQT_TQOBJECT(this), TQT_SLOT(slotUpdateDisplay()) );
  connect( k3bappcore->mediaCache(), TQT_SIGNAL(mediumChanged(K3bDevice::Device*)),
	   this, TQT_SLOT(slotMediumChanged(K3bDevice::Device*)) );
}

K3bFillStatusDisplay::~K3bFillStatusDisplay()
{
  delete d;
  delete m_toolTip;
}


void K3bFillStatusDisplay::setupPopupMenu()
{
  d->actionCollection = new TDEActionCollection( this );

  // we use a nother popup for the dvd sizes
  d->popup = new TDEPopupMenu( this, "popup" );
  d->dvdPopup = new TDEPopupMenu( this, "dvdpopup" );

  d->actionShowMinutes = new TDERadioAction( i18n("Minutes"), 0, TQT_TQOBJECT(this), TQT_SLOT(showTime()),
					   d->actionCollection, "fillstatus_show_minutes" );
  d->actionShowMegs = new TDERadioAction( i18n("Megabytes"), 0, TQT_TQOBJECT(this), TQT_SLOT(showSize()),
					d->actionCollection, "fillstatus_show_megabytes" );

  d->actionShowMegs->setExclusiveGroup( "show_size_in" );
  d->actionShowMinutes->setExclusiveGroup( "show_size_in" );

  d->actionAuto = new TDERadioAction( i18n("Auto"), 0, TQT_TQOBJECT(this), TQT_SLOT(slotAutoSize()),
				    d->actionCollection, "fillstatus_auto" );
  d->action74Min = new TDERadioAction( i18n("%1 MB").arg(650), 0, TQT_TQOBJECT(this), TQT_SLOT(slot74Minutes()),
				     d->actionCollection, "fillstatus_74minutes" );
  d->action80Min = new TDERadioAction( i18n("%1 MB").arg(700), 0, TQT_TQOBJECT(this), TQT_SLOT(slot80Minutes()),
				     d->actionCollection, "fillstatus_80minutes" );
  d->action100Min = new TDERadioAction( i18n("%1 MB").arg(880), 0, TQT_TQOBJECT(this), TQT_SLOT(slot100Minutes()),
				      d->actionCollection, "fillstatus_100minutes" );
  d->actionDvd4_7GB = new TDERadioAction( TDEIO::convertSizeFromKB((int)(4.4*1024.0*1024.0)), 0, TQT_TQOBJECT(this), TQT_SLOT(slotDvd4_7GB()),
					d->actionCollection, "fillstatus_dvd_4_7gb" );
  d->actionDvdDoubleLayer = new TDERadioAction( TDEIO::convertSizeFromKB((int)(8.0*1024.0*1024.0)),
					      0, TQT_TQOBJECT(this), TQT_SLOT(slotDvdDoubleLayer()),
					      d->actionCollection, "fillstatus_dvd_double_layer" );
  d->actionCustomSize = new K3bRadioAction( i18n("Custom..."), 0, TQT_TQOBJECT(this), TQT_SLOT(slotCustomSize()),
					    d->actionCollection, "fillstatus_custom_size" );
  d->actionCustomSize->setAlwaysEmitActivated(true);
  d->actionDetermineSize = new K3bRadioAction( i18n("From Medium..."), "media-optical-cdrom-unmounted", 0,
					       TQT_TQOBJECT(this), TQT_SLOT(slotDetermineSize()),
					       d->actionCollection, "fillstatus_size_from_disk" );
  d->actionDetermineSize->setAlwaysEmitActivated(true);

  d->actionAuto->setExclusiveGroup( "cd_size" );
  d->action74Min->setExclusiveGroup( "cd_size" );
  d->action80Min->setExclusiveGroup( "cd_size" );
  d->action100Min->setExclusiveGroup( "cd_size" );
  d->actionDvd4_7GB->setExclusiveGroup( "cd_size" );
  d->actionDvdDoubleLayer->setExclusiveGroup( "cd_size" );
  d->actionCustomSize->setExclusiveGroup( "cd_size" );
  d->actionDetermineSize->setExclusiveGroup( "cd_size" );

  d->actionLoadUserDefaults = new TDEAction( i18n("User Defaults"), "", 0,
					   TQT_TQOBJECT(this), TQT_SLOT(slotLoadUserDefaults()),
					   d->actionCollection, "load_user_defaults" );
  d->actionSaveUserDefaults = new TDEAction( i18n("Save User Defaults"), "", 0,
					   TQT_TQOBJECT(this), TQT_SLOT(slotSaveUserDefaults()),
					   d->actionCollection, "save_user_defaults" );

  TDEAction* dvdSizeInfoAction = new TDEAction( i18n("Why 4.4 instead of 4.7?"), "", 0,
					    TQT_TQOBJECT(this), TQT_SLOT(slotWhy44()),
					    d->actionCollection, "why_44_gb" );

  d->popup->insertTitle( i18n("Show Size In") );
  d->actionShowMinutes->plug( d->popup );
  d->actionShowMegs->plug( d->popup );
  d->popup->insertTitle( i18n("CD Size") );
  d->actionAuto->plug( d->popup );
  d->action74Min->plug( d->popup );
  d->action80Min->plug( d->popup );
  d->action100Min->plug( d->popup );
  d->actionCustomSize->plug( d->popup );
  d->actionDetermineSize->plug( d->popup );
  d->popup->insertSeparator();
  d->actionLoadUserDefaults->plug( d->popup );
  d->actionSaveUserDefaults->plug( d->popup );

  d->dvdPopup->insertTitle( i18n("DVD Size") );
  dvdSizeInfoAction->plug( d->dvdPopup );
  d->actionAuto->plug( d->dvdPopup );
  d->actionDvd4_7GB->plug( d->dvdPopup );
  d->actionDvdDoubleLayer->plug( d->dvdPopup );
  d->actionCustomSize->plug( d->dvdPopup );
  d->actionDetermineSize->plug( d->dvdPopup );
  d->dvdPopup->insertSeparator();
  d->actionLoadUserDefaults->plug( d->dvdPopup );
  d->actionSaveUserDefaults->plug( d->dvdPopup );

  connect( d->displayWidget, TQT_SIGNAL(contextMenu(const TQPoint&)), TQT_TQOBJECT(this), TQT_SLOT(slotPopupMenu(const TQPoint&)) );
}


void K3bFillStatusDisplay::showSize()
{
  d->actionShowMegs->setChecked( true );

  d->action74Min->setText( i18n("%1 MB").arg(650) );
  d->action80Min->setText( i18n("%1 MB").arg(700) );
  d->action100Min->setText( i18n("%1 MB").arg(880) );

  d->showTime = false;
  d->displayWidget->setShowTime(false);
}

void K3bFillStatusDisplay::showTime()
{
  d->actionShowMinutes->setChecked( true );

  d->action74Min->setText( i18n("unused", "%n minutes", 74) );
  d->action80Min->setText( i18n("unused", "%n minutes", 80) );
  d->action100Min->setText( i18n("unused", "%n minutes", 100) );

  d->showTime = true;
  d->displayWidget->setShowTime(true);
}


void K3bFillStatusDisplay::showDvdSizes( bool b )
{
  d->showDvdSizes = b;
  slotLoadUserDefaults();
}


void K3bFillStatusDisplay::slotAutoSize()
{
  slotMediumChanged( 0 );
}


void K3bFillStatusDisplay::slot74Minutes()
{
  d->displayWidget->setCdSize( DEFAULT_CD_SIZE_74 );
}


void K3bFillStatusDisplay::slot80Minutes()
{
  d->displayWidget->setCdSize( DEFAULT_CD_SIZE_80 );
}


void K3bFillStatusDisplay::slot100Minutes()
{
  d->displayWidget->setCdSize( DEFAULT_CD_SIZE_100 );
}


void K3bFillStatusDisplay::slotDvd4_7GB()
{
  d->displayWidget->setCdSize( DEFAULT_DVD_SIZE_4_4 );
}


void K3bFillStatusDisplay::slotDvdDoubleLayer()
{
  d->displayWidget->setCdSize( DEFAULT_DVD_SIZE_8_0 );
}


void K3bFillStatusDisplay::slotWhy44()
{
  TQWhatsThis::display( i18n("<p><b>Why does K3b offer 4.4 GB and 8.0 GB instead of 4.7 and 8.5 like "
			    "it says on the media?</b>"
			    "<p>A single layer DVD media has a capacity of approximately "
			    "4.4 GB which equals 4.4*1024<sup>3</sup> bytes. Media producers just "
			    "calculate with 1000 instead of 1024 for advertising reasons.<br>"
			    "This results in 4.4*1024<sup>3</sup>/1000<sup>3</sup> = 4.7 GB.") );
}


void K3bFillStatusDisplay::slotCustomSize()
{
  // allow the units to be translated
  TQString gbS = i18n("gb");
  TQString mbS = i18n("mb");
  TQString minS = i18n("min");

  TQRegExp rx( "(\\d+\\" + TDEGlobal::locale()->decimalSymbol() + "?\\d*)(" + gbS + "|" + mbS + "|" + minS + ")?" );
  bool ok;
  TQString size = KInputDialog::getText( i18n("Custom Size"),
					i18n("<p>Please specify the size of the media. Use suffixes <b>gb</b>,<b>mb</b>, "
					     "and <b>min</b> for <em>gigabytes</em>, <em>megabytes</em>, and <em>minutes</em>"
					     " respectively."),
					d->showDvdSizes ? TQString("4%14%2").arg(TDEGlobal::locale()->decimalSymbol()).arg(gbS) :
					(d->showTime ? TQString("74")+minS : TQString("650")+mbS),
					&ok, this, (const char*)0,
					new TQRegExpValidator( rx, TQT_TQOBJECT(this) ) );
  if( ok ) {
    // determine size
    if( rx.exactMatch( size ) ) {
      TQString valStr = rx.cap(1);
      if( valStr.endsWith( TDEGlobal::locale()->decimalSymbol() ) )
	valStr += "0";
      double val = TDEGlobal::locale()->readNumber( valStr, &ok );
      if( ok ) {
	TQString s = rx.cap(2);
	if( s == gbS || (s.isEmpty() && d->showDvdSizes) )
	  val *= 1024*512;
	else if( s == mbS || (s.isEmpty() && !d->showTime) )
	  val *= 512;
	else
	  val *= 60*75;
	d->displayWidget->setCdSize( (int)val );
	update();
      }
    }
  }
}


void K3bFillStatusDisplay::slotMenuButtonClicked()
{
  TQSize size = d->showDvdSizes ? d->dvdPopup->sizeHint() : d->popup->sizeHint();
  slotPopupMenu( d->buttonMenu->mapToGlobal(TQPoint(d->buttonMenu->width(), 0)) +
		 TQPoint(-1*size.width(), -1*size.height()) );
}


void K3bFillStatusDisplay::slotPopupMenu( const TQPoint& p )
{
  if( d->showDvdSizes )
    d->dvdPopup->popup(p);
  else
    d->popup->popup(p);
}


void K3bFillStatusDisplay::slotDetermineSize()
{
  bool canceled = false;
  K3bDevice::Device* dev = K3bMediaSelectionDialog::selectMedium( d->showDvdSizes ? K3bDevice::MEDIA_WRITABLE_DVD : K3bDevice::MEDIA_WRITABLE_CD,
								  K3bDevice::STATE_EMPTY|K3bDevice::STATE_INCOMPLETE,
								  parentWidget(),
								  TQString(), TQString(), &canceled );

  if( dev ) {
    K3b::Msf size = k3bappcore->mediaCache()->diskInfo( dev ).capacity();
    if( size > 0 ) {
      d->displayWidget->setCdSize( size );
      d->actionCustomSize->setChecked(true);
      update();
    }
    else
      KMessageBox::error( parentWidget(), i18n("Medium is not empty.") );
  }
  else if( !canceled )
    KMessageBox::error( parentWidget(), i18n("No usable medium found.") );
}


void K3bFillStatusDisplay::slotLoadUserDefaults()
{
  // load project specific values
  TDEConfig* c = k3bcore->config();
  c->setGroup( "default " + d->doc->typeString() + " settings" );

  // defaults to megabytes
  d->showTime = c->readBoolEntry( "show minutes", false );
  d->displayWidget->setShowTime(d->showTime);
  d->actionShowMegs->setChecked( !d->showTime );
  d->actionShowMinutes->setChecked( d->showTime );


  long size = c->readNumEntry( "default media size", 0 );

  switch( size ) {
  case 0:
    // automatic mode
    d->actionAuto->setChecked( true );
    break;
  case 74:
    d->action74Min->setChecked( true );
    break;
  case 80:
    d->action80Min->setChecked( true );
    break;
  case 100:
    d->action100Min->setChecked( true );
    break;
  case 510:
    d->actionDvd4_7GB->setChecked( true );
    break;
  default:
    d->actionCustomSize->setChecked( true );
    break;
  }

  if( size == 0 ) {
    slotMediumChanged( 0 );
  }
  else {
    d->displayWidget->setCdSize( size*60*75 );
  }
}


void K3bFillStatusDisplay::slotMediumChanged( K3bDevice::Device* )
{
  if( d->actionAuto->isChecked() ) {
    //
    // now search for a usable medium
    // if we find exactly one usable or multiple with the same size
    // we use that size
    //

    // TODO: once we have only one data project we need to change this to handle both

    K3bDevice::Device* dev = 0;
    TQPtrList<K3bDevice::Device> devs;
    if( d->showDvdSizes )
      devs = k3bcore->deviceManager()->dvdWriter();
    else
      devs = k3bcore->deviceManager()->cdWriter();

    for( TQPtrListIterator<K3bDevice::Device> it( devs ); *it; ++it ) {
      const K3bMedium& medium = k3bappcore->mediaCache()->medium( *it );

      if( ( medium.diskInfo().empty() ||
	    medium.diskInfo().appendable() ||
	    medium.diskInfo().rewritable() ) &&
	  ( medium.diskInfo().isDvdMedia() == d->showDvdSizes ) &&
	  d->doc->length() <= medium.diskInfo().capacity() ) {

	// first usable medium
	if( !dev ) {
	  dev = medium.device();
	}

	// roughly compare the sizes of the two usable media. If they match, carry on.
	else if( k3bappcore->mediaCache()->diskInfo( dev ).capacity().lba()/75/60
		 != medium.diskInfo().capacity().lba()/75/60 ) {
	  // different usable media -> fallback
	  dev = 0;
	  break;
	}
	// else continue;
      }
    }

    if( dev ) {
      d->displayWidget->setCdSize( k3bappcore->mediaCache()->diskInfo( dev ).capacity().lba() );
    }
    else {
      // default fallback
      if( d->showDvdSizes ) {
	if( d->doc->length().lba() > DEFAULT_DVD_SIZE_4_4 )
	  d->displayWidget->setCdSize( DEFAULT_DVD_SIZE_8_0 );
	else
	  d->displayWidget->setCdSize( DEFAULT_DVD_SIZE_4_4 );
      }
      else
	d->displayWidget->setCdSize( DEFAULT_CD_SIZE_80 );
    }
  }
}


void K3bFillStatusDisplay::slotSaveUserDefaults()
{
  // save project specific values
  TDEConfig* c = k3bcore->config();
  c->setGroup( "default " + d->doc->typeString() + " settings" );

  c->writeEntry( "show minutes", d->showTime );
  c->writeEntry( "default media size", d->actionAuto->isChecked() ? 0 : d->displayWidget->cdSize().totalFrames() );
}


void K3bFillStatusDisplay::slotUpdateDisplay()
{
  if( d->actionAuto->isChecked() ) {
      //
      // also update the medium list in case the docs size exceeds the capacity
      //
      slotMediumChanged( 0 );
  }
  else {
      d->displayWidget->update();
  }
}


void K3bFillStatusDisplay::slotDocChanged()
{
  // cache updates
  if( !d->updateTimer.isActive() ) {
    slotUpdateDisplay();
    d->updateTimer.start( 500, false );
  }
}

#include "k3bfillstatusdisplay.moc"
