/*
 *
 * $Id: k3bprogressinfoevent.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_PROGRESS_INFO_EVENT_H
#define K3B_PROGRESS_INFO_EVENT_H

#include <tqevent.h>
#include <tqstring.h>


/**
 * Custom event class for posting events corresponding to the
 * K3bJob signals. This is useful for a threaded job since
 * in that case it's not possible to emit signals that directly
 * change the GUI (see TQThread docu).
 */
class K3bProgressInfoEvent : public TQCustomEvent
{
 public:
  K3bProgressInfoEvent( int type )
    : TQCustomEvent( type ),
    m_type(type)
    {}

  K3bProgressInfoEvent( int type, const TQString& v1, const TQString& v2 = TQString(), 
			int value1 = 0, int value2 = 0 )
    : TQCustomEvent( type ),
    m_type( type),
    m_firstValue(value1),
    m_secondValue(value2),
    m_firstString(v1),
    m_secondString(v2)
    {}

  K3bProgressInfoEvent( int type, int value1, int value2 = 0 )
    : TQCustomEvent( type ),
    m_type( type),
    m_firstValue(value1),
    m_secondValue(value2)
    {}

  int type() const { return m_type; }
  const TQString& firstString() const { return m_firstString; }
  const TQString& secondString() const { return m_secondString; }
  int firstValue() const { return m_firstValue; }
  int secondValue() const { return m_secondValue; }

  enum K3bProgressInfoEventType {
    Progress = TQEvent::User + 1,
    SubProgress,
    ProcessedSize,
    ProcessedSubSize,
    InfoMessage,
    Started,
    Canceled,
    Finished,
    NewTask,
    NewSubTask,
    DebuggingOutput,
    BufferStatus,
    WriteSpeed,
    NextTrack
  };

 private:
  int m_type;
  int m_firstValue;
  int m_secondValue;
  TQString m_firstString;
  TQString m_secondString;
};

#endif
