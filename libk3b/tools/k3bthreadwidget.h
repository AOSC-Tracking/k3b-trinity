/* 
 *
 * $Id: k3bthreadwidget.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_THREAD_WIDGET_H_
#define _K3B_THREAD_WIDGET_H_

#include <tqobject.h>
#include <tqintdict.h>


class TQCustomEvent;
namespace K3bDevice {
  class Device;
}

/**
 * This class allows a thread other than the GUI thread to perform simple GUI
 * operations. Mainly creating some simple K3b Dialogs like Device selection.
 *
 * Since the calling thread cannot create the K3bThreadWidget by himself there exists
 * exactly one instance created by K3bCore which is used by all threads.
 */
class K3bThreadWidget : public TQObject
{
  Q_OBJECT
  

 public:
  ~K3bThreadWidget();

  static K3bThreadWidget* instance();

  /**
   * Call this from a thread to show a device selection dialog.
   */
  static K3bDevice::Device* selectDevice( TQWidget* parent, 
					  const TQString& text = TQString() );

 protected:
  /**
   * communication between the threads
   */
  void customEvent( TQCustomEvent* );

 private:
  /**
   * used internally
   */
  class DeviceSelectionEvent;
  class Data;

  K3bThreadWidget();

  /**
   * Get unique id
   */
  int getNewId();
  void clearId( int id );
  Data* data( int id );

  int m_idCounter;
  TQIntDict<Data> m_dataMap;

  static K3bThreadWidget* s_instance;
};

#endif
