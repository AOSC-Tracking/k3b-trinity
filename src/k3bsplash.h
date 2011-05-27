/* 
 *
 * $Id: k3bsplash.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BSPLASH_H
#define K3BSPLASH_H

#include <tqvbox.h>

class TQLabel;
class TQMouseEvent;
class TQPaintEvent;
class TQString;


class K3bSplash : public TQVBox
{
Q_OBJECT
  TQ_OBJECT

 public:
  K3bSplash( TQWidget* tqparent = 0, const char* name = 0 );
  ~K3bSplash();

 public slots:
  void show();
  void addInfo( const TQString& );

 protected:
  void mousePressEvent( TQMouseEvent* );
  //  void paintEvent( TQPaintEvent* );

 private:
  TQLabel* m_infoBox;
};

#endif
