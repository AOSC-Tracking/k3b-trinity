/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_TIMEOUT_WIDGET_H_
#define _K3B_TIMEOUT_WIDGET_H_

#include <tqwidget.h>

class TQPaintEvent;
class TQResizeEvent;


class K3bTimeoutWidget : public TQWidget
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bTimeoutWidget( TQWidget* parent );
  ~K3bTimeoutWidget();

  TQSize sizeHint() const;
  TQSize minimumSizeHint() const;

 public slots:
  void setTimeout( int msecs );
  void start();
  void stop();
  void pause();
  void resume();

 signals:
  void timeout();

 protected:
  void paintEvent( TQPaintEvent* );
  void resizeEvent( TQResizeEvent* );

 private slots:
  void timeStep();
  void startTimer();

 private:
  class Private;
  Private* d;
};

#endif
