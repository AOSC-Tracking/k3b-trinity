/* 
 *
 * $Id: k3bmsfedit.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3B_MSF_EDIT_H
#define K3B_MSF_EDIT_H


#include <tqspinbox.h>
#include <tqstring.h>
#include <tqvalidator.h>

#include <k3bmsf.h>
#include "k3b_export.h"

class K3bMsfValidator : public TQRegExpValidator
{
 public:
  K3bMsfValidator( TQObject* parent = 0, const char* name = 0 );
};


class LIBK3B_EXPORT K3bMsfEdit : public TQSpinBox
{
  TQ_OBJECT
  

 public:
  K3bMsfEdit( TQWidget* parent = 0, const char* name = 0 );
  ~K3bMsfEdit();

  TQSize sizeHint() const;

  void setFrameStyle( int style );
  void setLineWidth(int);

  K3b::Msf msfValue() const;

 signals:
  void valueChanged( const K3b::Msf& );

 public slots:
  void setValue( int v );
  void setText( const TQString& );
  void setMsfValue( const K3b::Msf& );
  void stepUp();
  void stepDown();

 protected:
  TQString mapValueToText( int );
  int mapTextToValue( bool* ok );
  int currentStepValue() const;

 private slots:
  void slotValueChanged( int );
};


#endif
