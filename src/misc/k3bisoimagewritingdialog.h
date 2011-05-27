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


#ifndef K3BISOIMAGEWRITINGDIALOG_H
#define K3BISOIMAGEWRITINGDIALOG_H

#include <k3binteractiondialog.h>


class TQCheckBox;
class K3bWriterSelectionWidget;
class TQLabel;
class KURL;
class K3bMd5Job;
class K3bWritingModeWidget;
class KURLRequester;
class K3bListView;
class TQSpinBox;
class TQDragEnterEvent;
class TQDropEvent;
class KListView;
class TQListViewItem;
class TQPoint;


/**
  *@author Sebastian Trueg
  */
class K3bIsoImageWritingDialog : public K3bInteractionDialog
{
  Q_OBJECT
  TQ_OBJECT

 public: 
  K3bIsoImageWritingDialog( TQWidget* = 0, const char* = 0, bool = true );
  ~K3bIsoImageWritingDialog();

  void setImage( const KURL& url );

 protected slots:
  void slotStartClicked();
  void updateImageSize( const TQString& );
  void slotWriterChanged();
  void slotMd5JobPercent( int );
  void slotMd5JobFinished( bool );
  void slotContextMenu( KListView*, TQListViewItem*, const TQPoint& pos );

 protected:
  void loadUserDefaults( KConfigBase* );
  void saveUserDefaults( KConfigBase* );
  void loadK3bDefaults();

  void calculateMd5Sum( const TQString& );
  void dragEnterEvent( TQDragEnterEvent* );
  void dropEvent( TQDropEvent* );

  void init();

 private:
  void setupGui();
  TQString imagePath() const;

  K3bMd5Job* m_md5Job;

  K3bWriterSelectionWidget* m_writerSelectionWidget;
  TQCheckBox* m_checkDummy;
  TQCheckBox* m_checkVerify;
  TQSpinBox* m_spinCopies;
  K3bWritingModeWidget* m_writingModeWidget;

  KURLRequester* m_editImagePath;
  K3bListView* m_infoView;

  class Private;
  Private* d;
};

#endif
