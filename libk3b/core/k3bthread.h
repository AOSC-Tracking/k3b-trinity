/*
 *
 * $Id: k3bthread.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_THREAD_H_
#define _K3B_THREAD_H_

#include <tqthread.h>
#include "k3b_export.h"

class TQObject;

/**
 * The threaded couterpart to K3bJob
 * instead of emitting the information signals
 * one has to use the emitXXX methods which will post
 * K3bProgressInfoEvents to the eventhandler.
 *
 * K3bThreadJob can be used to automatically wrap the thread in a K3bJob.
 *
 * As in K3bJob it is important to call emitStarted and emitFinished.
 *
 * See K3bThreadJob for more information.
 */
class LIBK3B_EXPORT K3bThread : public TQThread
{
 public:
  K3bThread( TQObject* eventHandler = 0 );
  K3bThread( unsigned int stackSize, TQObject* eventHandler = 0  );
  virtual ~K3bThread();

  void setProgressInfoEventHandler( TQObject* eventHandler );

  /**
   * Initialize the thread before starting it in the GUi thread.
   * K3bThreadJob automatically calls this.
   *
   * The default implementation does nothing.
   */
  virtual void init();

  /**
   * to provide the same api like K3bJob
   * the default implementation calls terminate and
   * emitCancled() and emitFinished(false)
   */
  virtual void cancel();

  virtual TQString jobDescription() const;
  virtual TQString jobDetails() const;

  /**
   * waits until all running K3bThread have finished.
   * This is used by K3bApplication.
   */
  static void waitUntilFinished();

 protected:
  virtual void run() = 0;

  /**
   * uses the K3bJob::MessageType enum
   */
  void emitInfoMessage( const TQString& msg, int type );
  void emitPercent( int p );
  void emitSubPercent( int p );
  void emitStarted();
  void emitCanceled();
  void emitFinished( bool success );
  void emitProcessedSize( int processed, int size );
  void emitProcessedSubSize( int processed, int size );
  void emitNewTask( const TQString& job );
  void emitNewSubTask( const TQString& job );
  void emitDebuggingOutput(const TQString&, const TQString&);
  void emitData( const char* data, int len );
  void emitNextTrack( int track, int trackNum );

 private:
  class Private;
  Private* d;
};

#endif
