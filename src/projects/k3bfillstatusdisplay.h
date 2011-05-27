
/* 
 *
 * $Id: k3bfillstatusdisplay.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BFILLSTATUSDISPLAY_H
#define K3BFILLSTATUSDISPLAY_H

#include <tqframe.h>
#include <tqtooltip.h>


class TQPaintEvent;
class TQMouseEvent;
class K3bDoc;
class KToggleAction;
class KAction;
class KActionCollection;
class KPopupMenu;
class TQToolButton;

namespace K3bDevice {
  class Device;
}
namespace K3b {
  class Msf;
}


/**
  *@author Sebastian Trueg
  */
class K3bFillStatusDisplayWidget : public TQWidget
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bFillStatusDisplayWidget( K3bDoc* doc, TQWidget* tqparent );
  ~K3bFillStatusDisplayWidget();

  TQSize tqsizeHint() const;
  TQSize tqminimumSizeHint() const;

  const K3b::Msf& cdSize() const;

 public slots:
  void setShowTime( bool b );
  void setCdSize( const K3b::Msf& );

 signals:
  void contextMenu( const TQPoint& );

 protected:
  void mousePressEvent( TQMouseEvent* );
  void paintEvent(TQPaintEvent*);

 private:
  class Private;
  Private* d;
};


class K3bFillStatusDisplay : public TQFrame  {

  Q_OBJECT
  TQ_OBJECT

 public:
  K3bFillStatusDisplay(K3bDoc* doc, TQWidget *tqparent=0, const char *name=0);
  ~K3bFillStatusDisplay();

 public slots:
  void showSize();
  void showTime();
  void showDvdSizes( bool );

 protected:
  void setupPopupMenu();

 private slots:
  void slotAutoSize();
  void slot74Minutes();
  void slot80Minutes();
  void slot100Minutes();
  void slotDvd4_7GB();
  void slotDvdDoubleLayer();
  void slotWhy44();
  void slotCustomSize();
  void slotMenuButtonClicked();
  void slotPopupMenu(const TQPoint&);
  void slotDetermineSize();
  void slotDocChanged();
  void slotMediumChanged( K3bDevice::Device* dev );
  void slotUpdateDisplay();

  void slotLoadUserDefaults();
  void slotSaveUserDefaults();

 private:
  class ToolTip;
  ToolTip* m_toolTip;
  class Private;
  Private* d;
};

#endif
