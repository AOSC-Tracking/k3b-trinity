/* 
 *
 * $Id: k3bburnprogressdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BBURNPROGRESSDIALOG_H
#define K3BBURNPROGRESSDIALOG_H

#include <k3bjobprogressdialog.h>

class K3bBurnJob;
class KProgress;
class TQLabel;


/**
  *@author Sebastian Trueg
  */
class K3bBurnProgressDialog : public K3bJobProgressDialog  {

  TQ_OBJECT
  

 public:
  K3bBurnProgressDialog( TQWidget* parent = 0, const char* name = 0, bool showSubProgress = true, 
			 bool modal = true, WFlags = 0 );
  ~K3bBurnProgressDialog();

  void setJob( K3bJob* );
  void setBurnJob( K3bBurnJob* );

 protected slots:
  void slotWriteSpeed( int, int );
  void slotBufferStatus( int );
  void slotDeviceBuffer( int );
  void slotFinished(bool);

 protected:
  TQLabel* m_labelWriter;
  KProgress* m_progressWritingBuffer;
  KProgress* m_progressDeviceBuffer;
  TQLabel* m_labelWritingSpeed;
};

#endif
