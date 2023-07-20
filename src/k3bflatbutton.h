/*
 *
 * $Id: k3bflatbutton.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef FLATBUTTON_H
#define FLATBUTTON_H

#include <tqframe.h>
#include <tqcolor.h>
#include <tqpixmap.h>

class TQEvent;
class TQMouseEvent;
class TQPainter;
class TDEAction;


/**
@author Sebastian Trueg
*/
class K3bFlatButton : public TQFrame
{
  TQ_OBJECT
  

 public:
  K3bFlatButton( TQWidget *parent = 0, const char *name = 0 );
  K3bFlatButton( const TQString& text, TQWidget *parent = 0, const char *name = 0 );
  K3bFlatButton( TDEAction*, TQWidget *parent = 0, const char *name = 0 );
  
  ~K3bFlatButton();

  TQSize sizeHint() const;

 public slots:
  void setColors( const TQColor& fore, const TQColor& back );
  void setText( const TQString& );
  void setPixmap( const TQPixmap& );

 signals:
  void pressed();
  void clicked();

 private slots:
  void slotThemeChanged();

 private:
  void init();

  void mousePressEvent(TQMouseEvent* e);
  void mouseReleaseEvent(TQMouseEvent* e);
  void enterEvent( TQEvent* );
  void leaveEvent( TQEvent* );
  void drawContents( TQPainter* );

  void setHover( bool );

  bool m_pressed;
  TQColor m_backColor;
  TQColor m_foreColor;
  TQString m_text;
  TQPixmap m_pixmap;

  bool m_hover;
};

#endif
