/* 
 *
 * $Id$
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

#ifndef K3B_TOOLBOX_H
#define K3B_TOOLBOX_H

#include <tqframe.h>
#include <tqstring.h>
#include <tqtoolbutton.h>
#include <tqptrlist.h>
#include "k3b_export.h"

class KAction;
class KToggleAction;
class KWidgetAction;
class TQGridLayout;
class TQPopupMenu;
class TQResizeEvent;


/**
 * internal class. Do not use!
 */
class LIBK3B_EXPORT K3bToolBoxButton : public TQToolButton
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bToolBoxButton( KAction*, TQWidget* tqparent );
  K3bToolBoxButton( const TQString& text, const TQString& icon, 
		    const TQString& tooltip, const TQString& whatsthis,
		    TQObject* receiver, const char* slot,
		    TQWidget* tqparent );

 private slots:
  void slotPopupActivated();

 protected:
  void resizeEvent( TQResizeEvent* );

 private:
  TQPopupMenu* m_popupMenu;
};


class LIBK3B_EXPORT K3bToolBox : public TQFrame
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bToolBox( TQWidget* tqparent = 0, const char* name = 0 );
  ~K3bToolBox();

  K3bToolBoxButton* addButton( const TQString& text, const TQString& icon, 
			       const TQString& tooltip = TQString(), const TQString& whatsthis = TQString(),
			       TQObject* receiver = 0, const char* slot = 0,
			       bool forceTextLabel = false );
  K3bToolBoxButton* addButton( KAction*, bool forceTextLabel = false );
  K3bToolBoxButton* addToggleButton( KToggleAction* );
  void addWidgetAction( KWidgetAction* );

  /**
   * Be aware that the toolbox will take ownership of the widget
   * and destroy it on destruction. Becasue of this it is not fitted
   * for WidgetActions.
   */
  void addWidget( TQWidget* );
  void addLabel( const TQString& );
  void addSpacing();
  void addSeparator();
  void addStretch();

  void clear();

 protected:
  TQGridLayout* m_mainLayout;
  TQPtrList<TQWidget> m_doNotDeleteWidgets;
};


#endif
