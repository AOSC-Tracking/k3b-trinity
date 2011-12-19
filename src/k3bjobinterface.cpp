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

#include "k3bjobinterface.h"

#include <k3bjob.h>

#include <tqcstring.h>
#include <tqdatastream.h>


K3bJobInterface::K3bJobInterface( TQObject* parent )
  : TQObject( parent ),
    DCOPObject( "K3bJobInterface" ),
    m_job( 0 )
{
}


void K3bJobInterface::setJob( K3bJob* job )
{
  if( m_job )
    m_job->disconnect( this );

  m_job = job;
  m_lastProgress = m_lastSubProgress = 0;

  if( m_job ) {
    connect( m_job, TQT_SIGNAL(newTask(const TQString&)), this, TQT_SLOT(slotNewTask(const TQString&)) );
    connect( m_job, TQT_SIGNAL(newSubTask(const TQString&)), this, TQT_SLOT(slotNewSubTask(const TQString&)) );
    connect( m_job, TQT_SIGNAL(infoMessage(const TQString&, int)), this, TQT_SLOT(slotInfoMessage(const TQString&, int)) );
    connect( m_job, TQT_SIGNAL(finished(bool)), this, TQT_SLOT(slotFinished(bool)) );
    connect( m_job, TQT_SIGNAL(started()), this, TQT_SLOT(slotStarted()) );
    connect( m_job, TQT_SIGNAL(canceled()), this, TQT_SLOT(slotCanceled()) );
    connect( m_job, TQT_SIGNAL(percent(int)), this, TQT_SLOT(slotProgress(int)) );
    connect( m_job, TQT_SIGNAL(subPercent(int)), this, TQT_SLOT(slotSubProgress(int)) );
    connect( m_job, TQT_SIGNAL(nextTrack(int, int)), this, TQT_SLOT(slotNextTrack(int, int)) );

    if( m_job->inherits( "K3bBurnJob" ) ) {
      connect( m_job, TQT_SIGNAL(bufferStatus(int)), this, TQT_SLOT(slotBuffer(int)) );
      connect( m_job, TQT_SIGNAL(deviceBuffer(int)), this, TQT_SLOT(slotDeviceBuffer(int)) );
    }

    connect( m_job, TQT_SIGNAL(destroyed()), this, TQT_SLOT(slotDestroyed()) );
  }
}


bool K3bJobInterface::jobRunning() const
{
  return ( m_job && m_job->active() );
}


TQString K3bJobInterface::jobDescription() const
{
  if( m_job )
    return m_job->jobDescription();
  else
    return TQString();
}


TQString K3bJobInterface::jobDetails() const
{
  if( m_job )
    return m_job->jobDetails();
  else
    return TQString();
}


void K3bJobInterface::slotStarted()
{
  m_lastProgress = m_lastSubProgress = 0;
  emitDCOPSignal( "started()", TQByteArray() );
}


void K3bJobInterface::slotCanceled()
{
  emitDCOPSignal( "canceled()", TQByteArray() );
}


void K3bJobInterface::slotFinished( bool success )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << success;
  emitDCOPSignal( "finished(bool)", params );
}


void K3bJobInterface::slotInfoMessage( const TQString& message, int type )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << message << type;
  emitDCOPSignal( "infoMessage(TQString)", params );
}


void K3bJobInterface::slotProgress( int val )
{
  if( m_lastProgress != val ) {
    m_lastProgress = val;
    TQByteArray params;
    TQDataStream stream(params, IO_WriteOnly);
    stream << val;
    emitDCOPSignal( "progress(int)", params );
  }
}


void K3bJobInterface::slotSubProgress( int val )
{
  if( m_lastSubProgress != val ) {
    m_lastSubProgress = val;
    TQByteArray params;
    TQDataStream stream(params, IO_WriteOnly);
    stream << val;
    emitDCOPSignal( "subProgress(int)", params );
  }
}


void K3bJobInterface::slotNewTask( const TQString& task )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << task;
  emitDCOPSignal( "newTask(TQString)", params );
}


void K3bJobInterface::slotNewSubTask( const TQString& task )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << task;
  emitDCOPSignal( "newSubTask(TQString)", params );
}


void K3bJobInterface::slotBuffer( int val )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << val;
  emitDCOPSignal( "buffer(int)", params );
}


void K3bJobInterface::slotDeviceBuffer( int val )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << val;
  emitDCOPSignal( "deviceBuffer(int)", params );
}


void K3bJobInterface::slotNextTrack( int track, int numTracks )
{
  TQByteArray params;
  TQDataStream stream(params, IO_WriteOnly);
  stream << track << numTracks;
  emitDCOPSignal( "nextTrack(int,int)", params );
}


void K3bJobInterface::slotDestroyed()
{
  m_job = 0;
}

#include "k3bjobinterface.moc"
