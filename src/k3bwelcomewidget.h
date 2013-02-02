/* 
 *
 * $Id: k3bwelcomewidget.h 642063 2007-03-13 09:40:13Z trueg $
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


#ifndef _K3B_WELCOME_WIDGET_H_
#define _K3B_WELCOME_WIDGET_H_

#include <tqscrollview.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqimage.h>

#include <kurl.h>
#include <tdeaction.h>

class K3bMainWindow;
class TQDropEvent;
class TQDragEnterEvent;
class K3bFlatButton;
class TQPaintEvent;
class TQResizeEvent;
class TQSimpleRichText;
class TDEConfigBase;
class TQMouseEvent;
class TQShowEvent;


class K3bWelcomeWidget : public TQScrollView
{
  Q_OBJECT
  

 public:
  K3bWelcomeWidget( K3bMainWindow*, TQWidget* parent = 0, const char* name = 0 );
  ~K3bWelcomeWidget();

  void loadConfig( TDEConfigBase* c );
  void saveConfig( TDEConfigBase* c );

  class Display;

 public slots:
  void slotMoreActions();

 protected:
  void resizeEvent( TQResizeEvent* );
  void showEvent( TQShowEvent* );
  void contentsMousePressEvent( TQMouseEvent* e );

 private:
  void fixSize();

  K3bMainWindow* m_mainWindow;
  Display* main;
};


class K3bWelcomeWidget::Display : public TQWidget
{
  Q_OBJECT
  

 public:
  Display( K3bWelcomeWidget* parent );
  ~Display();

  TQSize minimumSizeHint() const;
  TQSizePolicy sizePolicy () const;
  int heightForWidth ( int w ) const;

  void addAction( TDEAction* );
  void removeAction( TDEAction* );
  void removeButton( K3bFlatButton* );
  void rebuildGui();
  void rebuildGui( const TQPtrList<TDEAction>& );

 signals:
  void dropped( const KURL::List& );

 protected:
  void resizeEvent( TQResizeEvent* );
  void paintEvent( TQPaintEvent* );
  void dropEvent( TQDropEvent* event );
  void dragEnterEvent( TQDragEnterEvent* event );

 private slots:
  void slotThemeChanged();

 private:
  void repositionButtons();
  void updateBgPix();

  TQSimpleRichText* m_header;
  TQSimpleRichText* m_infoText;

  TQSize m_buttonSize;
  int m_cols;
  int m_rows;

  TQPtrList<TDEAction> m_actions;
  TQPtrList<K3bFlatButton> m_buttons;
  TQMap<K3bFlatButton*, TDEAction*> m_buttonMap;

  K3bFlatButton* m_buttonMore;

  bool m_infoTextVisible;

  TQPixmap m_bgPixmap;
  TQImage m_bgImage;

  friend class K3bWelcomeWidget;
};

#endif
