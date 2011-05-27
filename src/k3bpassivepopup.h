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

#ifndef _K3B_PASSIVE_POPUP_H_
#define _K3B_PASSIVE_POPUP_H_

#include <tqframe.h>

class K3bTimeoutWidget;
class KActiveLabel;
class KPushButton;
class TQLabel;


/**
 * A message box which is closed using a timer or a close button
 * It will delete itself once it has been closed.
 */
class K3bPassivePopup : public TQFrame
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bPassivePopup( TQWidget* tqparent );
  ~K3bPassivePopup();

  enum MessageType {
   Information,
   Warning,
   Error
  };

  /**
   * This will show the popup using K3bWidgetShowEffect::Slide and close it the 
   * same way.
   */
  void slideIn();

  static void showPopup( const TQString& message, 
			 const TQString& title = TQString(), 
			 MessageType messageType = Information,
			 bool countdown = true,
			 bool button = true );

 public slots:
  void setShowCloseButton( bool b );
  void setShowCountdown( bool b );
  void setMessage( const TQString& m );
  void setTitle( const TQString& t );
  void setTimeout( int msecs );
  void setMessageType( MessageType m );

 private slots:
  void slotShown();
  void slotHidden();
  void slotClose();
  void slotSticky( bool );

 private:
  class Private;
  Private* d;
};

#endif
