/*
 *
 * $Id: k3bemptydiscwaiter.cpp 642801 2007-03-15 12:39:04Z trueg $
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


#include "k3bemptydiscwaiter.h"
#include "k3bmediacache.h"
#include "k3bapplication.h"
#include <k3bdevice.h>
#include <k3bdeviceglobals.h>
#include <k3bdevicehandler.h>
#include <k3bglobals.h>
#include <k3bcore.h>
#include <k3biso9660.h>
#include "k3bblankingjob.h"
#include <k3bbusywidget.h>
#include <k3bprogressdialog.h>
#include <k3bdvdformattingjob.h>

#include <tqtimer.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqpushbutton.h>
#include <tqapplication.h>
#include <tqeventloop.h>
#include <tqfont.h>

#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kactivelabel.h>
#include <knotifyclient.h>


class K3bEmptyDiscWaiter::Private
{
public:
  Private()
    : erasingInfoDialog(0) {
    dialogVisible = false;
    inLoop = false;
    mediumChanged = 0;
    blockMediaChange = false;
  }

  K3bDevice::Device* device;

  int wantedMediaType;
  int wantedMediaState;

  TQString wantedMediaTypeString;

  int result;
  int dialogVisible;
  bool inLoop;

  bool blockMediaChange;
  int mediumChanged;

  bool forced;
  bool canceled;

  bool waitingDone;

  TQLabel* labelRequest;
  TQLabel* labelFoundMedia;
  TQLabel* pixLabel;

  K3bProgressDialog* erasingInfoDialog;
};



K3bEmptyDiscWaiter::K3bEmptyDiscWaiter( K3bDevice::Device* device, TQWidget* tqparent, const char* name )
  : KDialogBase( KDialogBase::Plain, i18n("Waiting for Disk"),
		 KDialogBase::Cancel|KDialogBase::User1|KDialogBase::User2|KDialogBase::User3,
		 KDialogBase::User3, tqparent, name, true, true, i18n("Force"), i18n("Eject"), i18n("Load") )
{
  d = new Private();
  d->device = device;

  // setup the gui
  // -----------------------------
  d->labelRequest = new TQLabel( plainPage() );
  d->labelRequest->tqsetAlignment( TQt::AlignLeft | TQt::AlignVCenter );
  d->labelFoundMedia = new TQLabel( plainPage() );
  d->pixLabel = new TQLabel( plainPage() );
  d->pixLabel->tqsetAlignment( TQt::AlignHCenter | TQt::AlignTop );

  TQFont f( d->labelFoundMedia->font() );
  f.setBold(true);
  d->labelFoundMedia->setFont( f );

  TQGridLayout* grid = new TQGridLayout( plainPage() );
  grid->setMargin( marginHint() );
  grid->setSpacing( spacingHint() );

  grid->addMultiCellWidget( d->pixLabel, 0, 2, 0, 0 );
  grid->addColSpacing( 1, 20 );
  grid->addWidget( new TQLabel( i18n("Found media:"), plainPage() ), 0, 2 );
  grid->addWidget( d->labelFoundMedia, 0, 3 );
  grid->addMultiCellWidget( d->labelRequest, 1, 1, 2, 3 );
  grid->setRowStretch( 2, 1 );
  grid->setColStretch( 3, 1 );
  // -----------------------------

  connect( k3bappcore->mediaCache(), TQT_SIGNAL(mediumChanged(K3bDevice::Device*)),
	   this, TQT_SLOT(slotMediumChanged(K3bDevice::Device*)) );

  TQToolTip::add( actionButton(KDialogBase::User1),
		 i18n("Force K3b to continue if it seems not to detect your empty CD/DVD.") );
}


K3bEmptyDiscWaiter::~K3bEmptyDiscWaiter()
{
  delete d;
}


int K3bEmptyDiscWaiter::waitForDisc( int mediaState, int mediaType, const TQString& message )
{
  if ( d->inLoop ) {
    kdError() << "(K3bEmptyDiscWaiter) Recursive call detected." << endl;
    return -1;
  }

  d->wantedMediaState = mediaState;
  d->wantedMediaType = mediaType;
  d->dialogVisible = false;
  d->forced = false;
  d->canceled = false;
  d->waitingDone = false;
  d->blockMediaChange = false;
  d->mediumChanged = 0;

  //
  // We do not cover every case here but just the ones that really make sense
  //
  if( (d->wantedMediaType & K3bDevice::MEDIA_WRITABLE_DVD) &&
	   (d->wantedMediaType & K3bDevice::MEDIA_WRITABLE_CD) )
    d->wantedMediaTypeString = i18n("CD-R(W) or DVD%1R(W)").tqarg("�");
  else if( d->wantedMediaType & K3bDevice::MEDIA_WRITABLE_DVD_SL )
    d->wantedMediaTypeString = i18n("DVD%1R(W)").tqarg("�");
  else if( d->wantedMediaType & K3bDevice::MEDIA_WRITABLE_DVD_DL )
    d->wantedMediaTypeString = i18n("Double Layer DVD%1R").tqarg("�");
  else
    d->wantedMediaTypeString = i18n("CD-R(W)");

  if( message.isEmpty() ) {
    if( (d->wantedMediaState & K3bDevice::STATE_COMPLETE) && (d->wantedMediaState & K3bDevice::STATE_INCOMPLETE) )
      d->labelRequest->setText( i18n("Please insert a complete or appendable %4 medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename())
				.tqarg( d->wantedMediaTypeString ) );
    else if( d->wantedMediaState & K3bDevice::STATE_COMPLETE )
      d->labelRequest->setText( i18n("Please insert a complete %4 medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename())
				.tqarg( d->wantedMediaTypeString ) );
    else if( (d->wantedMediaState & K3bDevice::STATE_INCOMPLETE) && (d->wantedMediaState & K3bDevice::STATE_EMPTY) )
      d->labelRequest->setText( i18n("Please insert an empty or appendable %4 medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename())
				.tqarg( d->wantedMediaTypeString ) );
    else if( d->wantedMediaState & K3bDevice::STATE_INCOMPLETE )
      d->labelRequest->setText( i18n("Please insert an appendable %4 medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename())
				.tqarg( d->wantedMediaTypeString ) );
    else if( d->wantedMediaState & K3bDevice::STATE_EMPTY )
      d->labelRequest->setText( i18n("Please insert an empty %4 medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename())
				.tqarg( d->wantedMediaTypeString ) );
    else // fallback case (this should not happen in K3b)
      d->labelRequest->setText( i18n("Please insert a suitable medium "
				     "into drive<p><b>%1 %2 (%3)</b>.")
				.tqarg(d->device->vendor())
				.tqarg(d->device->description())
				.tqarg(d->device->devicename()) );

  }
  else
    d->labelRequest->setText( message );

  if( d->wantedMediaType & K3bDevice::MEDIA_WRITABLE_DVD )
    d->pixLabel->setPixmap( KGlobal::instance()->iconLoader()->loadIcon( "dvd_unmount",
									 KIcon::NoGroup, KIcon::SizeMedium ) );
  else
    d->pixLabel->setPixmap( KGlobal::instance()->iconLoader()->loadIcon( "cdwriter_unmount",
									 KIcon::NoGroup, KIcon::SizeMedium ) );

  adjustSize();

  slotMediumChanged( d->device );

  //
  // in case we already found a medium and thus the dialog is not shown entering
  // the loop only causes problems (since there is no dialog yet the user could
  // not have forced or canceled yet
  //
  if( !d->waitingDone ) {
    d->inLoop = true;
    TQApplication::eventLoop()->enterLoop();
  }

  return d->result;
}


int K3bEmptyDiscWaiter::exec()
{
  return waitForDisc();
}


void K3bEmptyDiscWaiter::slotMediumChanged( K3bDevice::Device* dev )
{
  kdDebug() << "(K3bEmptyDiscWaiter) slotMediumChanged() " << endl;
  if( d->forced || d->canceled || d->device != dev )
    return;

  //
  // This slot may open dialogs which enter a new event loop and that
  // may result in another call to this slot if a medium changes while
  // a dialog is open
  //
  if( d->blockMediaChange ) {
    d->mediumChanged++;
    return;
  }

  d->blockMediaChange = true;

  KConfig* c = k3bcore->config();
  bool formatWithoutAsking = KConfigGroup( k3bcore->config(), "General Options" ).readBoolEntry( "auto rewritable erasing", false );

  K3bMedium medium = k3bappcore->mediaCache()->medium( dev );

  d->labelFoundMedia->setText( medium.shortString( false ) );

  if( medium.diskInfo().diskState() == K3bDevice::STATE_NO_MEDIA ) {
    continueWaiting();
    d->blockMediaChange = false;
    return;
  }

//   TQString mediaState;
//   if( medium.diskInfo().diskState() == K3bDevice::STATE_COMPLETE )
//     mediaState = i18n("complete");
//   else if( medium.diskInfo().diskState() == K3bDevice::STATE_INCOMPLETE )
//     mediaState = i18n("appendable");
//   else if( medium.diskInfo().diskState() == K3bDevice::STATE_EMPTY )
//     mediaState = i18n("empty");

//   if( !mediaState.isEmpty() )
//     mediaState = " (" + mediaState +")";


  // /////////////////////////////////////////////////////////////
  //
  // DVD+RW handling
  //
  // /////////////////////////////////////////////////////////////

  // DVD+RW: if empty we need to preformat. Although growisofs does it before writing doing it here
  //         allows better control and a progress bar. If it's not empty we should check if there is
  //         already a filesystem on the media.
  if( (d->wantedMediaType & K3bDevice::MEDIA_DVD_PLUS_RW) &&
      (medium.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_PLUS_RW) ) {

    kdDebug() << "(K3bEmptyDiscWaiter) ------ found DVD+RW as wanted." << endl;

    if( medium.diskInfo().diskState() == K3bDevice::STATE_EMPTY ) {
      if( d->wantedMediaState & K3bDevice::STATE_EMPTY ) {
	// special case for the formatting job which wants to preformat on it's own!
	if( d->wantedMediaState & K3bDevice::STATE_COMPLETE &&
	    d->wantedMediaState & K3bDevice::STATE_EMPTY ) {
	  kdDebug() << "(K3bEmptyDiscWaiter) special case: DVD+RW for the formatting job." << endl;
	  finishWaiting( K3bDevice::MEDIA_DVD_PLUS_RW );
	}
	else {
	  // empty - preformat without asking
	  prepareErasingDialog();

	  K3bDvdFormattingJob job( this );
	  job.setDevice( d->device );
	  job.setQuickFormat( true );
	  job.setForce( false );
	  job.setForceNoEject( true );

	  d->erasingInfoDialog->setText( i18n("Preformatting DVD+RW") );
	  connect( &job, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotErasingFinished(bool)) );
	  connect( &job, TQT_SIGNAL(percent(int)), d->erasingInfoDialog, TQT_SLOT(setProgress(int)) );
	  connect( d->erasingInfoDialog, TQT_SIGNAL(cancelClicked()), &job, TQT_SLOT(cancel()) );
	  job.start( medium.diskInfo() );
	  d->erasingInfoDialog->exec( true );
	}
      }
      else {
	kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: empty DVD+RW where a non-empty was requested." << endl;
	continueWaiting();
      }
    }
    else {
      //
      // We have a DVD+RW medium which is already preformatted
      //
      if( d->wantedMediaState == K3bDevice::STATE_EMPTY ) {
	// check if the media contains a filesystem
	K3bIso9660 isoF( d->device );
	bool hasIso = isoF.open();

	if( formatWithoutAsking ||
	    !hasIso ||
	    KMessageBox::warningContinueCancel( tqparentWidgetToUse(),
						i18n("Found %1 media in %2 - %3. "
						     "Should it be overwritten?")
						.tqarg("DVD+RW")
						.tqarg(d->device->vendor())
						.tqarg(d->device->description()),
						i18n("Found %1").tqarg("DVD+RW"),i18n("Overwrite") ) == KMessageBox::Continue ) {
	  finishWaiting( K3bDevice::MEDIA_DVD_PLUS_RW );
	}
	else {
	  kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: no DVD+RW overwrite" << endl;
	  K3b::unmount( d->device );
	  K3bDevice::eject( d->device );
	  continueWaiting();
	}
      }

      //
      // We want a DVD+RW not nessessarily empty. No problem, just use this one. Becasue incomplete and complete
      // are handled the same everywhere (isofs is grown).
      //
      else {
	finishWaiting( K3bDevice::MEDIA_DVD_PLUS_RW );
      }
    }
  } // --- DVD+RW --------


    // /////////////////////////////////////////////////////////////
    //
    // DVD-RW handling
    //
    // /////////////////////////////////////////////////////////////

    //
    // DVD-RW in sequential mode can be empty. DVD-RW in restricted overwrite mode is always complete.
    //
  else if( (d->wantedMediaType & (K3bDevice::MEDIA_DVD_RW|
				  K3bDevice::MEDIA_DVD_RW_SEQ|
				  K3bDevice::MEDIA_DVD_RW_OVWR) ) &&
	   (medium.diskInfo().mediaType() & (K3bDevice::MEDIA_DVD_RW|
					     K3bDevice::MEDIA_DVD_RW_SEQ|
					     K3bDevice::MEDIA_DVD_RW_OVWR) ) ) {

    kdDebug() << "(K3bEmptyDiscWaiter) ------ found DVD-R(W) as wanted." << endl;

    // we format in the following cases:
    // seq. incr. and not empty and empty requested
    // seq. incr. and restr. overwri. reqested
    // restr. ovw. and seq. incr. requested

    // we have exactly what was requested
    if( (d->wantedMediaType & medium.diskInfo().mediaType()) &&
	(d->wantedMediaState & medium.diskInfo().diskState()) ) {
      finishWaiting( medium.diskInfo().mediaType() );
    }

    // DVD-RW in restr. overwrite may just be overwritten
    else if( (medium.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_RW_OVWR) &&
	     (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_OVWR) ) {
      if( d->wantedMediaState == K3bDevice::STATE_EMPTY ) {

	kdDebug() << "(K3bEmptyDiscWaiter) ------ DVD-RW restricted overwrite." << endl;

	// check if the media contains a filesystem
	K3bIso9660 isoF( d->device );
	bool hasIso = isoF.open();

	if( formatWithoutAsking ||
	    !hasIso ||
	    KMessageBox::warningContinueCancel( tqparentWidgetToUse(),
						i18n("Found %1 media in %2 - %3. "
						     "Should it be overwritten?")
						.tqarg(K3bDevice::mediaTypeString(medium.diskInfo().mediaType()))
						.tqarg(d->device->vendor())
						.tqarg(d->device->description()),
						i18n("Found %1").tqarg("DVD-RW"),i18n("Overwrite") ) == KMessageBox::Continue ) {
	  finishWaiting( K3bDevice::MEDIA_DVD_RW_OVWR );
	}
	else {
	  kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: no DVD-RW overwrite." << endl;
	  K3b::unmount( d->device );
	  K3bDevice::eject( d->device );
	  continueWaiting();
	}
      }

      else if( !(d->wantedMediaState & K3bDevice::STATE_EMPTY ) ) {
	// check if the media contains a filesystem
	K3bIso9660 isoF( d->device );
	bool hasIso = isoF.open();

	if( hasIso ) {
	  finishWaiting( K3bDevice::MEDIA_DVD_RW_OVWR );
	}
	else {
	  kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: empty DVD-RW where a non-empty was requested." << endl;
	  continueWaiting();
	}
      }

      //
      // We want a DVD-RW overwrite not nessessarily empty. No problem, just use this one. Becasue incomplete and complete
      // are handled the same everywhere (isofs is grown).
      //
      else {
	finishWaiting( K3bDevice::MEDIA_DVD_RW_OVWR );
      }
    }

    // formatting
    else if( ( (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_OVWR) &&
	       (medium.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_RW_SEQ) &&
	       !(d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_SEQ) ) ||

	     ( (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_SEQ) &&
	       (medium.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_RW_OVWR) &&
	       !(d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_OVWR) ) ||

	     ( (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_SEQ) &&
	       (medium.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_RW_SEQ) &&
	       (d->wantedMediaState & K3bDevice::STATE_EMPTY) &&
	       (medium.diskInfo().diskState() != K3bDevice::STATE_EMPTY) ) ) {

      kdDebug() << "(K3bEmptyDiscWaiter) ------ DVD-RW needs to be formated." << endl;

      if( formatWithoutAsking ||
	  KMessageBox::warningContinueCancel( tqparentWidgetToUse(),
					      i18n("Found %1 media in %2 - %3. "
						   "Should it be formatted?")
					      .tqarg( K3bDevice::mediaTypeString(medium.diskInfo().mediaType()) )
					      .tqarg(d->device->vendor())
					      .tqarg(d->device->description()),
					      i18n("Found %1").tqarg("DVD-RW"), i18n("Format") ) == KMessageBox::Continue ) {

	kdDebug() << "(K3bEmptyDiscWaiter) ------ formatting DVD-RW." << endl;

	prepareErasingDialog();

	K3bDvdFormattingJob job( this );
	job.setDevice( d->device );
	// we prefere the current mode of the media if no special mode has been requested
	job.setMode( ( (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_SEQ) &&
		       (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_OVWR) )
		     ? ( medium.diskInfo().mediaType() == K3bDevice::MEDIA_DVD_RW_OVWR
			 ? K3b::WRITING_MODE_RES_OVWR
			 : K3b::WRITING_MODE_INCR_SEQ )
		     : ( (d->wantedMediaType & K3bDevice::MEDIA_DVD_RW_SEQ)
			 ? K3b::WRITING_MODE_INCR_SEQ
			 : K3b::WRITING_MODE_RES_OVWR ) );
	job.setQuickFormat( true );
	job.setForce( false );
	job.setForceNoEject(true);

	d->erasingInfoDialog->setText( i18n("Formatting DVD-RW") );
	connect( &job, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotErasingFinished(bool)) );
	connect( &job, TQT_SIGNAL(percent(int)), d->erasingInfoDialog, TQT_SLOT(setProgress(int)) );
	connect( d->erasingInfoDialog, TQT_SIGNAL(cancelClicked()), &job, TQT_SLOT(cancel()) );
	job.start( medium.diskInfo() );
	d->erasingInfoDialog->exec( true );
      }
      else {
	kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: no DVD-RW formatting." << endl;
	K3b::unmount( d->device );
	K3bDevice::eject( d->device );
	continueWaiting();
      }
    }
    else {
      kdDebug() << "(K3bEmptyDiscWaiter) ------ nothing useful found." << endl;
      continueWaiting();
    }
  } // --- DVD-RW ------


    // /////////////////////////////////////////////////////////////
    //
    // CD handling (and DVD-R and DVD+R)
    //
    // /////////////////////////////////////////////////////////////

    // we have exactly what was requested
  else if( (d->wantedMediaType & medium.diskInfo().mediaType()) &&
	   (d->wantedMediaState & medium.diskInfo().diskState()) )
    finishWaiting( medium.diskInfo().mediaType() );

  else if( (medium.diskInfo().mediaType() != K3bDevice::MEDIA_UNKNOWN) &&
	   (d->wantedMediaType & medium.diskInfo().mediaType()) &&
	   (d->wantedMediaState & medium.diskInfo().diskState()) )
    finishWaiting( medium.diskInfo().mediaType() );

  // this is for CD drives that are not able to determine the state of a disk
  else if( medium.diskInfo().diskState() == K3bDevice::STATE_UNKNOWN &&
	   medium.diskInfo().mediaType() == K3bDevice::MEDIA_CD_ROM &&
	   d->wantedMediaType & K3bDevice::MEDIA_CD_ROM )
    finishWaiting( medium.diskInfo().mediaType() );

  // format CD-RW
  else if( (d->wantedMediaType & medium.diskInfo().mediaType()) &&
	   (d->wantedMediaState & K3bDevice::STATE_EMPTY) &&
	   medium.diskInfo().rewritable() ) {

    if( formatWithoutAsking ||
	KMessageBox::questionYesNo( tqparentWidgetToUse(),
				    i18n("Found rewritable media in %1 - %2. "
					 "Should it be erased?").tqarg(d->device->vendor()).tqarg(d->device->description()),
				    i18n("Found Rewritable Disk"),
				    KGuiItem(i18n("&Erase"), "cdrwblank"),
				    KGuiItem(i18n("E&ject")) ) == KMessageBox::Yes ) {


      prepareErasingDialog();

      // start a k3bblankingjob
      d->erasingInfoDialog->setText( i18n("Erasing CD-RW") );

      // the user may be using cdrdao for erasing as cdrecord does not work
      int erasingApp = K3b::DEFAULT;
      if( KConfigGroup( c, "General Options" ).readBoolEntry( "Manual writing app selection", false ) ) {
	erasingApp = K3b::writingAppFromString( KConfigGroup( c, "CDRW Erasing" ).readEntry( "writing_app" ) );
      }

      K3bBlankingJob job( this );
      job.setDevice( d->device );
      job.setMode( K3bBlankingJob::Fast );
      job.setForce(true);
      job.setForceNoEject(true);
      job.setSpeed( 0 ); // Auto
      job.setWritingApp( erasingApp );
      connect( &job, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotErasingFinished(bool)) );
      connect( d->erasingInfoDialog, TQT_SIGNAL(cancelClicked()), &job, TQT_SLOT(cancel()) );
      job.start();
      d->erasingInfoDialog->exec( false );
    }
    else {
      kdDebug() << "(K3bEmptyDiscWaiter) starting devicehandler: no CD-RW overwrite." << endl;
      K3b::unmount( d->device );
      K3bDevice::eject( d->device );
      continueWaiting();
    }
  }
  else {
    kdDebug() << "(K3bEmptyDiscWaiter) ------ nothing useful found." << endl;
    continueWaiting();
  }

  // handle queued medium changes
  d->blockMediaChange = false;
  if( d->mediumChanged > 0 ) {
    d->mediumChanged--;
    slotMediumChanged( dev );
  }
}


void K3bEmptyDiscWaiter::showDialog()
{
  // we need to show the dialog if not done already
  if( !d->dialogVisible ) {

    KNotifyClient::event( 0, "WaitingForMedium", i18n("Waiting for Medium") );

    d->dialogVisible = true;
    clearWFlags( WDestructiveClose );
    setWFlags( WShowModal );
    setResult( 0 );
    show();
  }
}


void K3bEmptyDiscWaiter::continueWaiting()
{
  showDialog();
}


void K3bEmptyDiscWaiter::slotCancel()
{
  kdDebug() << "(K3bEmptyDiscWaiter) slotCancel() " << endl;
  d->canceled = true;
  finishWaiting( CANCELED );
}


void K3bEmptyDiscWaiter::slotUser1()
{
  d->forced = true;
  finishWaiting( DISK_READY );
}


void K3bEmptyDiscWaiter::slotUser2()
{
  K3b::unmount( d->device );
  K3bDevice::eject( d->device );
}


void K3bEmptyDiscWaiter::slotUser3()
{
  K3bDevice::load( d->device );
}


void K3bEmptyDiscWaiter::finishWaiting( int code )
{
  kdDebug() << "(K3bEmptyDiscWaiter) finishWaiting() " << endl;

  d->waitingDone = true;
  d->result = code;

  if( d->dialogVisible )
    hide();

  if( d->inLoop ) {
    d->inLoop = false;
    kdDebug() << "(K3bEmptyDiscWaiter) exitLoop " << endl;
    TQApplication::eventLoop()->exitLoop();
  }
}


void K3bEmptyDiscWaiter::slotErasingFinished( bool success )
{
  if( success ) {
    connect( K3bDevice::reload( d->device ),
	     TQT_SIGNAL(finished(K3bDevice::DeviceHandler*)),
	     this,
	     TQT_SLOT(slotReloadingAfterErasingFinished(K3bDevice::DeviceHandler*)) );
  }
  else {
    K3bDevice::eject( d->device );
    KMessageBox::error( d->erasingInfoDialog, i18n("Erasing failed.") );
    d->erasingInfoDialog->hide(); // close the dialog thus ending it's event loop -> back to slotMediumChanged
  }
}


void K3bEmptyDiscWaiter::slotReloadingAfterErasingFinished( K3bDevice::DeviceHandler* )
{
  // close the dialog thus ending it's event loop -> back to slotMediumChanged
  d->erasingInfoDialog->hide();
}


int K3bEmptyDiscWaiter::wait( K3bDevice::Device* device, bool appendable, int mediaType, TQWidget* tqparent )
{
  K3bEmptyDiscWaiter d( device, tqparent ? tqparent : TQT_TQWIDGET(tqApp->activeWindow()) );
  int mediaState = K3bDevice::STATE_EMPTY;
  if( appendable ) mediaState |= K3bDevice::STATE_INCOMPLETE;
  return d.waitForDisc( mediaState, mediaType );
}


int K3bEmptyDiscWaiter::wait( K3bDevice::Device* device,
			      int mediaState,
			      int mediaType,
			      const TQString& message,
			      TQWidget* tqparent )
{
  K3bEmptyDiscWaiter d( device, tqparent ? tqparent : TQT_TQWIDGET(tqApp->activeWindow()) );
  return d.waitForDisc( mediaState, mediaType, message );
}


void K3bEmptyDiscWaiter::prepareErasingDialog()
{
  // we hide the emptydiskwaiter so the info dialog needs to have the same tqparent
  if( !d->erasingInfoDialog )
    d->erasingInfoDialog = new K3bProgressDialog( TQString(), tqparentWidget() );

  //
  // hide the dialog
  //
  if( d->dialogVisible ) {
    hide();
    d->dialogVisible = false;
  }
}


TQWidget* K3bEmptyDiscWaiter::tqparentWidgetToUse()
{
  // we might also show dialogs if the discwaiter widget is not visible yet
  if( d->dialogVisible )
    return this;
  else
    return tqparentWidget();
}


int K3bEmptyDiscWaiter::waitForMedia( K3bDevice::Device* device,
				      int mediaState,
				      int mediaType,
				      const TQString& message )
{
  // this is only needed for the formatting
  return wait( device, mediaState, mediaType, message, d->erasingInfoDialog );
}


bool K3bEmptyDiscWaiter::questionYesNo( const TQString& text,
					const TQString& caption,
					const TQString& yesText,
					const TQString& noText )
{
  return ( KMessageBox::questionYesNo( tqparentWidgetToUse(),
				       text,
				       caption,
				       yesText.isEmpty() ? KStdGuiItem::yes() : KGuiItem(yesText),
				       noText.isEmpty() ? KStdGuiItem::no() : KGuiItem(noText) ) == KMessageBox::Yes );
}


void K3bEmptyDiscWaiter::blockingInformation( const TQString& text,
					      const TQString& caption )
{
  KMessageBox::information( this, text, caption );
}

#include "k3bemptydiscwaiter.moc"
