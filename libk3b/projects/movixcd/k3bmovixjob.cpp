/* 
 *
 * $Id: k3bmovixjob.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bmovixjob.h"
#include "k3bmovixdoc.h"
#include "k3bmovixfileitem.h"
#include "k3bmovixdocpreparer.h"

#include <k3bcore.h>
#include <k3bdatajob.h>
#include <k3bdevice.h>

#include <klocale.h>
#include <kdebug.h>


K3bMovixJob::K3bMovixJob( K3bMovixDoc* doc, K3bJobHandler* jh, TQObject* tqparent )
  : K3bBurnJob( jh, tqparent ),
    m_doc(doc)
{
  m_dataJob = new K3bDataJob( doc, this, this );
  m_movixDocPreparer = new K3bMovixDocPreparer( doc, this, this );

  // pipe signals
  connect( m_dataJob, TQT_SIGNAL(percent(int)), this, TQT_SIGNAL(percent(int)) );
  connect( m_dataJob, TQT_SIGNAL(subPercent(int)), this, TQT_SIGNAL(subPercent(int)) );
  connect( m_dataJob, TQT_SIGNAL(processedSubSize(int, int)), this, TQT_SIGNAL(processedSubSize(int, int)) );
  connect( m_dataJob, TQT_SIGNAL(processedSize(int, int)), this, TQT_SIGNAL(processedSize(int, int)) );
  connect( m_dataJob, TQT_SIGNAL(buffertqStatus(int)), this, TQT_SIGNAL(buffertqStatus(int)) );
  connect( m_dataJob, TQT_SIGNAL(deviceBuffer(int)), this, TQT_SIGNAL(deviceBuffer(int)) );
  connect( m_dataJob, TQT_SIGNAL(writeSpeed(int, int)), this, TQT_SIGNAL(writeSpeed(int, int)) );
  connect( m_dataJob, TQT_SIGNAL(newTask(const TQString&)), this, TQT_SIGNAL(newTask(const TQString&)) );
  connect( m_dataJob, TQT_SIGNAL(newSubTask(const TQString&)), this, TQT_SIGNAL(newSubTask(const TQString&)) );
  connect( m_dataJob, TQT_SIGNAL(debuggingOutput(const TQString&, const TQString&)),
	   this, TQT_SIGNAL(debuggingOutput(const TQString&, const TQString&)) );
  connect( m_dataJob, TQT_SIGNAL(infoMessage(const TQString&, int)),
	   this, TQT_SIGNAL(infoMessage(const TQString&, int)) );
  connect( m_dataJob, TQT_SIGNAL(burning(bool)), this, TQT_SIGNAL(burning(bool)) );

  // we need to clean up here
  connect( m_dataJob, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotDataJobFinished(bool)) );

  connect( m_movixDocPreparer, TQT_SIGNAL(infoMessage(const TQString&, int)),
	   this, TQT_SIGNAL(infoMessage(const TQString&, int)) );
}


K3bMovixJob::~K3bMovixJob()
{
}


K3bDevice::Device* K3bMovixJob::writer() const
{
  return m_dataJob->writer();
}


K3bDoc* K3bMovixJob::doc() const
{
  return m_doc; 
}


void K3bMovixJob::start()
{
  jobStarted();

  m_canceled = false;
  m_dataJob->setWritingApp( writingApp() );

  if( m_movixDocPreparer->createMovixStructures() ) {
    m_dataJob->start();
  }
  else {
    m_movixDocPreparer->removeMovixStructures();
    jobFinished(false);
  }
}


void K3bMovixJob::cancel()
{
  m_canceled = true;
  m_dataJob->cancel();
}


void K3bMovixJob::slotDataJobFinished( bool success )
{
  m_movixDocPreparer->removeMovixStructures();

  if( m_canceled || m_dataJob->hasBeenCanceled() )
    emit canceled();

  jobFinished( success );
}


TQString K3bMovixJob::jobDescription() const
{
  if( m_doc->isoOptions().volumeID().isEmpty() )
    return i18n("Writing eMovix CD");
  else
    return i18n("Writing eMovix CD (%1)").tqarg(m_doc->isoOptions().volumeID());
}


TQString K3bMovixJob::jobDetails() const
{
  return ( i18n("1 file (%1) and about 8 MB eMovix data", 
		"%n files (%1) and about 8 MB eMovix data", 
		m_doc->movixFileItems().count()).tqarg(KIO::convertSize(m_doc->size()))
	   + ( m_doc->copies() > 1 
	       ? i18n(" - %n copy", " - %n copies", m_doc->copies()) 
	       : TQString() ) );
}

#include "k3bmovixjob.moc"
