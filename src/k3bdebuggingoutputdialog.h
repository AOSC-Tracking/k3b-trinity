/* 
 *
 * $Id: k3bdebuggingoutputdialog.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_DEBUGGING_OUTPUT_DIALOG_H_
#define _K3B_DEBUGGING_OUTPUT_DIALOG_H_

#include <kdialogbase.h>
#include <tqmap.h>

class TQTextEdit;

class K3bDebuggingOutputDialog : public KDialogBase
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bDebuggingOutputDialog( TQWidget* tqparent );
  
 public slots:
  void setOutput( const TQMap<TQString, TQStringList>& );
  void addOutput( const TQString&, const TQString& );
  void clear();

 private:
  void slotUser1();
  void slotUser2();
  
  TQTextEdit* debugView;
  TQMap<TQString, int> m_paragraphMap;
};



#endif
