/* 
 *
 * $Id: k3blistviewitemanimator.h 689561 2007-07-18 15:19:38Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_LISTVIEWITEM_ANIMATOR_H_
#define _K3B_LISTVIEWITEM_ANIMATOR_H_

#include <tqobject.h>
#include <tqpixmap.h>
#include "k3b_export.h"

class TQListViewItem;
class TQTimer;


/**
 * Fades an icon on a listview item in and out.
 */
class LIBK3B_EXPORT K3bListViewItemAnimator : public TQObject
{
  TQ_OBJECT
  

 public:
  K3bListViewItemAnimator( TQObject* parent = 0, const char* name = 0 );
  /**
   * Will use the items pixmap.
   */
  K3bListViewItemAnimator( TQListViewItem* item, int col, TQObject* parent = 0, const char* name = 0 );
  ~K3bListViewItemAnimator();

  TQListViewItem* item() const;

 public slots:
  void start();
  void stop();

  void setItem( TQListViewItem*, int col );

  /**
   * Default is the pixmap from the item.
   */
  void setPixmap( const TQPixmap& );

  void setColumn( int col );

  /**
   * Default is the base color of the listview.
   */
  void setFadeColor( const TQColor& );

 private slots:
  void slotAnimate();

 private:
 void init();

  int m_animationStep;
  bool m_animationBack;
  TQPixmap m_pixmap;
  TQColor m_fadeColor;
  TQListViewItem* m_item;
  int m_column;

  TQTimer* m_timer;
};

#endif
