/* 
 *
 * $Id: k3bcutcombobox.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_CUT_COMBOBOX_H_
#define _K3B_CUT_COMBOBOX_H_

#include <kcombobox.h>
#include "k3b_export.h"
class TQResizeEvent;


/**
 * Cuts it's text.
 * Since it rebuilds the complete list of strings every time
 * a new string is added or one gets removed it is not a good
 * idea to use this for dynamic lists.
 *
 * Be aware that currently only insertItem works.
 * none of the insertStrList or insertStringList methods are implemeted
 * yet and also the removeItem methos does not work.
 */ 
class LIBK3B_EXPORT K3bCutComboBox : public KComboBox
{
  Q_OBJECT
  

 public:
  K3bCutComboBox( TQWidget* parent = 0, const char* name = 0 );
  K3bCutComboBox( int method, TQWidget* parent = 0, const char* name = 0 );
  virtual ~K3bCutComboBox();

  enum Method {
    CUT,
    STQUEEZE
  };

  /**
   * The method to shorten the text
   * defaut: CUT
   */
  void setMethod( int );

  /** reimplemeted */
  TQSize sizeHint() const;

  /** reimplemeted */
  TQSize minimumSizeHint() const;

  /** reimplemeted */
  virtual void setCurrentText( const TQString& );

  void	insertStringList( const TQStringList &, int index=-1 );
  void	insertStrList( const TQStrList &, int index=-1 );
  void	insertStrList( const TQStrList *, int index=-1 );
  void	insertStrList( const char **, int numStrings=-1, int index=-1);

  void	insertItem( const TQString &text, int index=-1 );
  void	insertItem( const TQPixmap &pixmap, int index=-1 );
  void	insertItem( const TQPixmap &pixmap, const TQString &text, int index=-1 );

  void	removeItem( int index );

  void	changeItem( const TQString &text, int index );
  void	changeItem( const TQPixmap &pixmap, const TQString &text, int index );

  TQString text( int ) const;
  TQString currentText() const;

  void clear();

 protected:
  void resizeEvent( TQResizeEvent* e );
  void cutText();

 private:
  class Private;
  Private* d;
};

#endif
