/*
 *
 * $Id: k3bthread.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bthread.h"
#include "k3bprogressinfoevent.h"
#include "k3bdataevent.h"

#include <kdebug.h>

#include <tqapplication.h>


static TQPtrList<K3bThread> s_threads;


void K3bThread::waitUntilFinished()
{
  TQPtrListIterator<K3bThread> it( s_threads );
  while( it.current() ) {
    kdDebug() << "Waiting for thread " << it.current() << endl;
    it.current()->wait();
    ++it;
  }

  kdDebug() << "Thread waiting done." << endl;
}


class K3bThread::Private
{
public:
  Private()
    : eventHandler( 0 ) {
  }

  TQObject* eventHandler;
};


K3bThread::K3bThread( TQObject* eventHandler )
  : TQThread()
{
  d = new Private;
  d->eventHandler = eventHandler;

  s_threads.append(this);
}


K3bThread::K3bThread( unsigned int stackSize, TQObject* eventHandler )
  : TQThread( stackSize )
{
  d = new Private;
  d->eventHandler = eventHandler;

  s_threads.append(this);
}


K3bThread::~K3bThread()
{
  s_threads.removeRef(this);
  delete d;
}


void K3bThread::setProgressInfoEventHandler( TQObject* eventHandler )
{ 
  d->eventHandler = eventHandler; 
}

TQString K3bThread::jobDescription() const
{
  return TQString();
}


TQString K3bThread::jobDetails() const
{
  return TQString();
}


void K3bThread::init()
{
  // do nothing...
}


void K3bThread::cancel()
{
  if( running() ) {
    terminate();
    if( d->eventHandler ) {
      emitCanceled();
      emitFinished(false);
    }
  }
}


void K3bThread::emitInfoMessage( const TQString& msg, int type )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler,
			     new K3bProgressInfoEvent( K3bProgressInfoEvent::InfoMessage, msg, TQString(), type ) );
  else
    kdWarning() << "(K3bThread) call to emitInfoMessage() without eventHandler." << endl;
}

void K3bThread::emitPercent( int p )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler,
			     new K3bProgressInfoEvent( K3bProgressInfoEvent::Progress, p ) );
  else
    kdWarning() << "(K3bThread) call to emitPercent() without eventHandler." << endl;
}

void K3bThread::emitSubPercent( int p )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler,
			     new K3bProgressInfoEvent( K3bProgressInfoEvent::SubProgress, p ) );
  else
    kdWarning() << "(K3bThread) call to emitSubPercent() without eventHandler." << endl;
}

void K3bThread::emitStarted()
{
  if( d->eventHandler )
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::Started ) );
  else
    kdWarning() << "(K3bThread) call to emitStarted() without eventHandler." << endl;
}

void K3bThread::emitCanceled()
{
  if( d->eventHandler )
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::Canceled ) );
  else
    kdWarning() << "(K3bThread) call to emitCanceled() without eventHandler." << endl;
}

void K3bThread::emitFinished( bool success )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::Finished, success ) );
  else
    kdWarning() << "(K3bThread) call to emitFinished() without eventHandler." << endl;
}

void K3bThread::emitProcessedSize( int p, int size )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::ProcessedSize, p, size ) );
  else
    kdWarning() << "(K3bThread) call to emitProcessedSize() without eventHandler." << endl;
}

void K3bThread::emitProcessedSubSize( int p, int size )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::ProcessedSubSize, p, size ) );
  else
    kdWarning() << "(K3bThread) call to emitProcessedSubSize() without eventHandler." << endl;
}

void K3bThread::emitNewTask( const TQString& job )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::NewTask, job ) );
  else
    kdWarning() << "(K3bThread) call to emitNewTask() without eventHandler." << endl;
}

void K3bThread::emitNewSubTask( const TQString& job )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::NewSubTask, job ) );
  else
    kdWarning() << "(K3bThread) call to emitNewSubTask() without eventHandler." << endl;
}

void K3bThread::emitDebuggingOutput(const TQString& group, const TQString& text)
{
  if( d->eventHandler )
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::DebuggingOutput, group, text ) );
  else
    kdWarning() << "(K3bThread) call to emitDebuggingOutput() without eventHandler." << endl;
}

void K3bThread::emitData( const char* data, int len )
{
  if( d->eventHandler )
    TQApplication::postEvent( d->eventHandler, new K3bDataEvent( data, len ) );
  else
    kdWarning() << "(K3bThread) call to emitData() without eventHandler." << endl;
}

void K3bThread::emitNextTrack( int t, int n )
{
  if( d->eventHandler ) 
    TQApplication::postEvent( d->eventHandler, new K3bProgressInfoEvent( K3bProgressInfoEvent::NextTrack, t, n ) );
  else
    kdWarning() << "(K3bThread) call to emitNextTrack() without eventHandler." << endl;
}

