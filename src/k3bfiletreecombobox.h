/*
 *
 * $Id: k3bfiletreecombobox.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_FILETREE_COMBOBOX_H_
#define _K3B_FILETREE_COMBOBOX_H_

#include <kcombobox.h>

class K3bFileTreeView;
class TQEvent;
class TQKeyEvent;
class TQMouseEvent;
class TQPaintEvent;

namespace K3bDevice {
  class Device;
}

class K3bFileTreeComboBox : public KComboBox
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bFileTreeComboBox( TQWidget* tqparent = 0, const char* name = 0 );
  ~K3bFileTreeComboBox();

  void popup();
  void popdown();

  void setCurrentItem( int );
  void setCurrentText( const TQString& );

 public slots:
  void setDevice( K3bDevice::Device* );
  void setUrl( const KURL& url );
  void slotGoUrl();

 signals:
  void urlExecuted( const KURL& url );
  void deviceExecuted( K3bDevice::Device* dev );

 private slots:
  void slotDeviceExecuted( K3bDevice::Device* );
  void slotUrlExecuted( const KURL& url );

 protected:
  bool eventFilter( TQObject*, TQEvent* );
  void keyPressEvent( TQKeyEvent* );
  void mousePressEvent( TQMouseEvent* );
  void paintEvent( TQPaintEvent* );

 private:
  void setEditText( const TQPixmap& pix, const TQString& t );

  class Private;
  Private* d;
  K3bFileTreeView* m_fileTreeView;
};

#endif
