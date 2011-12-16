/* 
 *
 * $Id: kcutlabel.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef KCUTLABEL_H
#define KCUTLABEL_H

#include <tqlabel.h>
#include "k3b_export.h"


/*
 * @ref TQLabel
 */
class LIBK3B_EXPORT KCutLabel : public TQLabel 
{
  Q_OBJECT
  TQ_OBJECT

 public:
  /**
   * Default constructor.
   */
  KCutLabel( TQWidget *parent = 0, const char *name = 0);
  KCutLabel( const TQString &text, TQWidget *parent = 0, const char *name = 0 );

  virtual TQSize tqminimumSizeHint() const;

  /**
   * \return the full text while text() returns the cut text
   */
  const TQString& fullText() const;

 public slots:
  void setText( const TQString & );

  /**
   * \param i the number of characters that have to be visible. Default is 1.
   */
  void setMinimumVisibleText( int i );

 protected:
  /**
   * used when widget is resized
   */
  void resizeEvent( TQResizeEvent * );
  /**
   * does the dirty work
   */
  void cutTextToLabel();

 private:
  TQString m_fullText;
  int m_minChars;
};

#endif // KCUTLABEL_H
