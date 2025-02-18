/* 
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bsignalwaiter.h"
#include "k3bjob.h"

#include <tqeventloop.h>
#include <tqapplication.h>


K3bSignalWaiter::K3bSignalWaiter()
  : TQObject(),
    m_inLoop(true)
{
}


K3bSignalWaiter::~K3bSignalWaiter()
{
}


void K3bSignalWaiter::waitForSignal( TQObject* o, const char* signal )
{
  K3bSignalWaiter w;
  connect( o, signal,
	   &w, TQ_SLOT(slotSignal()) );

  TQApplication::eventLoop()->enterLoop();
}


void K3bSignalWaiter::waitForJob( K3bJob* job )
{
  if( !job->active() )
    return;

  waitForSignal( job, TQ_SIGNAL(finished(bool)) );
}


void K3bSignalWaiter::slotSignal()
{
  if( m_inLoop ) {
    m_inLoop = false;
    TQApplication::eventLoop()->exitLoop();
  }
}

#include "k3bsignalwaiter.moc"
