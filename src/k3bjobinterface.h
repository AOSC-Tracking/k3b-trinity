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

#ifndef _K3B_JOB_INTERFACE_H_
#define _K3B_JOB_INTERFACE_H_

#include <tqobject.h>
#include <dcopobject.h>

class K3bJob;


/**
 * A DCOP interface for K3b's currently running job.
 *
 * This may be used for example in a karamba theme with a non-volitile
 * DCOP connection.
 */
class K3bJobInterface : public TQObject, public DCOPObject
{
  Q_OBJECT
//  TQ_OBJECT
  K_DCOP

 public:
  K3bJobInterface( TQObject* tqparent );

  void setJob( K3bJob* );

 k_dcop:
  bool jobRunning() const;

  TQString jobDescription() const;
  TQString jobDetails() const;

 k_dcop_signals:
  void started();
  void canceled();
  void finished( bool );
  void infoMessage( const TQString&, int );
  void progress( int );
  void subProgress( int );
  void newTask( const TQString& );
  void newSubTask( const TQString& );
  void buffer( int );
  void deviceBuffer( int );
  void nextTrack( int track, int numTracks );

 private slots:
  void slotStarted();
  void slotCanceled();
  void slotFinished( bool );
  void slotInfoMessage( const TQString&, int );
  void slotProgress( int );
  void slotSubProgress( int );
  void slotNewTask( const TQString& );
  void slotNewSubTask( const TQString& );
  void slotBuffer( int );
  void slotDeviceBuffer( int );
  void slotNextTrack( int track, int numTracks );

  void slotDestroyed();

 private:
  K3bJob* m_job;

  int m_lastProgress;
  int m_lastSubProgress;
};

#endif
