/*
 *
 * $Id$
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


#include "k3bburnprogressdialog.h"

#include "k3bapplication.h"
#include "k3bjob.h"
#include <k3bdevice.h>
#include "k3bstdguiitems.h"
#include "k3bthemedlabel.h"
#include <k3bthememanager.h>

#include <tdeglobal.h>
#include <kprogress.h>
#include <tdelocale.h>

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqframe.h>


K3bBurnProgressDialog::K3bBurnProgressDialog( TQWidget *parent, const char *name, bool showSubProgress,
					      bool modal, WFlags wf )
  : K3bJobProgressDialog(parent,name, showSubProgress, modal, wf)
{
  m_labelWritingSpeed = new TQLabel( m_frameExtraInfo, "m_labelWritingSpeed" );
  //  m_labelWritingSpeed->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );

  m_frameExtraInfoLayout->addWidget( m_labelWritingSpeed, 2, 0 );
  m_frameExtraInfoLayout->addWidget( new TQLabel( i18n("Estimated writing speed:"), m_frameExtraInfo ), 1, 0 );

  m_labelWriter = new K3bThemedLabel( m_frameExtraInfo );
  m_labelWriter->setFrameShape( TQFrame::StyledPanel );
  m_labelWriter->setFrameShadow( TQFrame::Sunken );
  m_labelWriter->setLineWidth( 1 );
  m_labelWriter->setMargin( 5 );
  TQFont textLabel14_font( m_labelWriter->font() );
  textLabel14_font.setBold( true );
  m_labelWriter->setFont( textLabel14_font );

  m_frameExtraInfoLayout->addMultiCellWidget( m_labelWriter, 0, 0, 0, 3 );
  m_frameExtraInfoLayout->addWidget( new TQLabel( i18n("Software buffer:"), m_frameExtraInfo ), 1, 2 );
  m_frameExtraInfoLayout->addWidget( new TQLabel( i18n("Device buffer:"), m_frameExtraInfo ), 2, 2 );

  m_progressWritingBuffer = new KProgress( m_frameExtraInfo, "m_progressWritingBuffer" );
  m_frameExtraInfoLayout->addWidget( m_progressWritingBuffer, 1, 3 );

  m_progressDeviceBuffer = new KProgress( m_frameExtraInfo );
  m_frameExtraInfoLayout->addWidget( m_progressDeviceBuffer, 2, 3 );
  m_frameExtraInfoLayout->addMultiCellWidget( K3bStdGuiItems::verticalLine( m_frameExtraInfo ), 1, 2, 1, 1 );
}

K3bBurnProgressDialog::~K3bBurnProgressDialog()
{
}


void K3bBurnProgressDialog::setJob( K3bJob* job )
{
  if( K3bBurnJob* burnJob = dynamic_cast<K3bBurnJob*>(job) )
    setBurnJob(burnJob);
  else
    K3bJobProgressDialog::setJob(job);
}


void K3bBurnProgressDialog::setBurnJob( K3bBurnJob* burnJob )
{
  K3bJobProgressDialog::setJob(burnJob);

  if( burnJob ) {
    connect( burnJob, TQT_SIGNAL(bufferStatus(int)), this, TQT_SLOT(slotBufferStatus(int)) );
    connect( burnJob, TQT_SIGNAL(deviceBuffer(int)), this, TQT_SLOT(slotDeviceBuffer(int)) );
    connect( burnJob, TQT_SIGNAL(writeSpeed(int, int)), this, TQT_SLOT(slotWriteSpeed(int, int)) );
    connect( burnJob, TQT_SIGNAL(burning(bool)), m_progressWritingBuffer, TQT_SLOT(setEnabled(bool)) );
    connect( burnJob, TQT_SIGNAL(burning(bool)), m_progressDeviceBuffer, TQT_SLOT(setEnabled(bool)) );
    connect( burnJob, TQT_SIGNAL(burning(bool)), m_labelWritingSpeed, TQT_SLOT(setEnabled(bool)) );

    if( burnJob->writer() )
      m_labelWriter->setText( i18n("Writer: %1 %2").arg(burnJob->writer()->vendor()).
			      arg(burnJob->writer()->description()) );

    m_labelWritingSpeed->setText( i18n("no info") );
    m_progressWritingBuffer->setFormat( i18n("no info") );
    m_progressDeviceBuffer->setFormat( i18n("no info") );
  }
}


void K3bBurnProgressDialog::slotFinished( bool success )
{
  K3bJobProgressDialog::slotFinished( success );
  if( success ) {
    m_labelWritingSpeed->setEnabled( false );
    m_progressWritingBuffer->setEnabled( false );
    m_progressDeviceBuffer->setEnabled( false );
  }
}


void K3bBurnProgressDialog::slotBufferStatus( int b )
{
  m_progressWritingBuffer->setFormat( "%p%" );
  m_progressWritingBuffer->setValue( b );
}


void K3bBurnProgressDialog::slotDeviceBuffer( int b )
{
  m_progressDeviceBuffer->setFormat( "%p%" );
  m_progressDeviceBuffer->setValue( b );
}


void K3bBurnProgressDialog::slotWriteSpeed( int s, int multiplicator )
{
  m_labelWritingSpeed->setText( TQString("%1 KB/s (%2x)").arg(s).arg(TDEGlobal::locale()->formatNumber((double)s/(double)multiplicator,2)) );
}

#include "k3bburnprogressdialog.moc"
