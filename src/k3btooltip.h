/* 
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
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

#ifndef _K3B_TOOLTIP_H_
#define _K3B_TOOLTIP_H_

#include <tqobject.h>
#include <tqpixmap.h>

#include "k3bwidgetshoweffect.h"

class TQTimer;

/**
 * More beautiful tooltip
 */
class K3bToolTip : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bToolTip( TQWidget* widget );
  ~K3bToolTip();

  TQWidget* parentWidget() const { return m_parentWidget; }

 public slots:
  /**
   * default is 700 mseconds (same as TQToolTip)
   */
  void setTipTimeout( int msec ) { m_tipTimeout = msec; }

 protected:
  /**
   * \see TQToolTip::maybeTip
   */
  virtual void maybeTip( const TQPoint& ) = 0;

  /**
   * Show a tooltip.
   */
  void tip( const TQRect&, const TQString&, int effect = K3bWidgetShowEffect::Dissolve );
  void tip( const TQRect& rect, const TQPixmap& pix, int effect = K3bWidgetShowEffect::Dissolve );

  /**
   * Use some arbitrary widget as the tooltip
   * \param effect Use 0 for no effect
   */
  void tip( const TQRect&, TQWidget* w, int effect = K3bWidgetShowEffect::Dissolve );

  bool eventFilter( TQObject* o, TQEvent* e );

 private slots:
  void slotCheckShowTip();

 private:
  void hideTip();

  TQWidget* m_parentWidget;
  TQWidget* m_currentTip;
  TQRect m_currentTipRect;

  TQTimer* m_tipTimer;
  TQPoint m_lastMousePos;

  int m_tipTimeout;
};

#endif
