/* 
 *
 * $Id: k3btitlelabel.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_TITLE_LABEL_H_
#define _K3B_TITLE_LABEL_H_

#include <tqframe.h>
#include "k3b_export.h"
class TQPainter;
class TQResizeEvent;


class LIBK3B_EXPORT K3bTitleLabel : public TQFrame
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bTitleLabel( TQWidget* parent = 0, const char* name = 0 );
  ~K3bTitleLabel();

  TQSize sizeHint() const;
  TQSize minimumSizeHint() const;

 public slots:
   /**
    * default: 2
    */
  void setMargin( int );

  void setTitle( const TQString& title, const TQString& subTitle = TQString() );
  void setSubTitle( const TQString& subTitle );

  /**
   * The title label only supports alignments left, hcenter, and right
   *
   * Default tqalignment is left.
   */
  // FIXME: honor right-to-left languages
  void setAlignment( int align );

 protected:
  void resizeEvent( TQResizeEvent* );
  void drawContents( TQPainter* p );

 private:
  void updatePositioning();

  class ToolTip;
  ToolTip* m_toolTip;

  class Private;
  Private* d;
};

#endif
