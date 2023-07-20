/*
 *
 * $Id: k3bblankingdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3B_BLANKING_DIALOG_H
#define K3B_BLANKING_DIALOG_H

#include "k3binteractiondialog.h"
#include <k3bjobhandler.h>

class TQGroupBox;
class TQComboBox;
class TQCloseEvent;
class TDEListView;
class K3bWriterSelectionWidget;
namespace K3bDevice {
  class Device;
}


class K3bBlankingDialog : public K3bInteractionDialog, public K3bJobHandler
{
TQ_OBJECT
  

 public:
  K3bBlankingDialog( TQWidget*, const char* );
  ~K3bBlankingDialog();

  /**
   * @reimplemented from K3bJobHandler
   */
  int waitForMedia( K3bDevice::Device*,
		    int mediaState = K3bDevice::STATE_EMPTY,
		    int mediaType = K3bDevice::MEDIA_WRITABLE_CD,
		    const TQString& message = TQString() );
  
  /**
   * @reimplemented from K3bJobHandler
   */
  bool questionYesNo( const TQString& text,
		      const TQString& caption = TQString(),
		      const TQString& yesText = TQString(),
		      const TQString& noText = TQString() );

  /**
   * reimplemented from K3bJobHandler
   */
  void blockingInformation( const TQString& text,
			    const TQString& caption = TQString() );

 public slots:
  void setDevice( K3bDevice::Device* );

 protected slots:
  void slotStartClicked();
  void slotWriterChanged();
  void slotWritingAppChanged( int );
  void slotJobFinished( bool );

 private:
  void setupGui();
  void loadK3bDefaults();
  void loadUserDefaults( TDEConfigBase* );
  void saveUserDefaults( TDEConfigBase* );

  K3bWriterSelectionWidget* m_writerSelectionWidget;

  TQComboBox* m_comboEraseMode;

  class Private;
  Private* d;
};

#endif
