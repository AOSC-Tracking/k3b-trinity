/*
 *
 * $Id: k3bsetup2.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3BSETUP2_H_
#define _K3BSETUP2_H_

#include <tdecmodule.h>
#include <tdeaboutdata.h>


class base_K3bSetup2;
class TQCheckListItem;


class K3bSetup2: public TDECModule
{
  TQ_OBJECT
  

 public:
  K3bSetup2( TQWidget* parent = 0, const char* name = 0, const TQStringList& args = TQStringList() );
  ~K3bSetup2();

  TQString quickHelp() const;
  const TDEAboutData* aboutData() { return m_aboutData; };

  void load();
  void save();
  void defaults();

 public slots:
  void updateViews();

 private slots:
  void slotSearchPrograms();
  void slotAddDevice();

 private:
  void updatePrograms();
  void updateDevices();
  TQString burningGroup() const;
  void makeReadOnly();
  TQCheckListItem* createDeviceItem( const TQString& deviceNode );

  class Private;
  Private* d;

  base_K3bSetup2* w;

  TDEAboutData* m_aboutData;
};

#endif
