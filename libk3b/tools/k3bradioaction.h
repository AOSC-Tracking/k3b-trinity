/* 
 *
 * $Id: k3bradioaction.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_RADIO_ACTION_H_
#define _K3B_RADIO_ACTION_H_

#include <kactionclasses.h>
#include "k3b_export.h"

/**
 * This differs from KRadioAction only in the boolean 
 * flag which says if it should always emit the signals 
 * even if it was checked twice.
 *
 * Docu copied from kdelibs
 */
class LIBK3B_EXPORT K3bRadioAction : public KToggleAction
{
  Q_OBJECT
  TQ_OBJECT

 public:
  /**
   * Constructs a radio action with text and potential keyboard
   * accelerator but nothing else. Use this only if you really
   * know what you are doing.
   *
   * @param text The text that will be displayed.
   * @param cut The corresponding keyboard accelerator (shortcut).
   * @param tqparent This action's tqparent.
   * @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const KShortcut& cut = KShortcut(), TQObject* tqparent = 0, const char* name = 0 );

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's tqparent.
   *  @param slot The TQT_SLOT to invoke to execute this action.
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const KShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* tqparent, const char* name = 0 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const TQIconSet& pix, const KShortcut& cut = KShortcut(),
                  TQObject* tqparent = 0, const char* name = 0 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const TQString& pix, const KShortcut& cut = KShortcut(),
                  TQObject* tqparent = 0, const char* name = 0 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's tqparent.
   *  @param slot The TQT_SLOT to invoke to execute this action.
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const TQIconSet& pix, const KShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* tqparent, const char* name = 0 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's tqparent.
   *  @param slot The TQT_SLOT to invoke to execute this action.
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( const TQString& text, const TQString& pix, const KShortcut& cut,
                  const TQObject* receiver, const char* slot,
                  TQObject* tqparent, const char* name = 0 );

  /**
   *  @param tqparent This action's tqparent.
   *  @param name An internal name for this action.
   */
  K3bRadioAction( TQObject* tqparent = 0, const char* name = 0 );

  /**
   * @param b if true the action will always emit the activated signal
   *          even if the toggled state did not change. The default is false.
   *          which is the same behaviour as KRadioAction
   */
  void setAlwaysEmitActivated( bool b ) { m_alwaysEmit = b; }

 protected:
  virtual void slotActivated();

 private:
  bool m_alwaysEmit;
};

#endif
