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


#ifndef _K3B_CDIMAGEWRITINGDIALOG_H_
#define _K3B_CDIMAGEWRITINGDIALOG_H_

#include <k3binteractiondialog.h>


class TQCheckBox;
class K3bWriterSelectionWidget;
class TQLabel;
class KURL;
class KActiveLabel;
class KProgress;
class K3bDataModeWidget;
class K3bWritingModeWidget;
class K3bTempDirSelectionWidget;
class KURLRequester;
class K3bListView;
class TQSpinBox;
class TQComboBox;
class K3bIso9660;
class K3bCueFileParser;
class TQDragEnterEvent;
class TQDropEvent;
class TDEListView;
class TQListViewItem;
class TQPoint;


/**
  *@author Sebastian Trueg
  */
class K3bCdImageWritingDialog : public K3bInteractionDialog
{
  Q_OBJECT
  

 public: 
  K3bCdImageWritingDialog( TQWidget* = 0, const char* = 0, bool = true );
  ~K3bCdImageWritingDialog();

  void setImage( const KURL& url );

 protected slots:
  void slotStartClicked();

  void slotMd5JobPercent( int );
  void slotMd5JobFinished( bool );
  void slotContextMenu( TDEListView*, TQListViewItem*, const TQPoint& pos );

  void slotUpdateImage( const TQString& );

 protected:
  void loadUserDefaults( TDEConfigBase* );
  void saveUserDefaults( TDEConfigBase* );
  void loadK3bDefaults();

  void calculateMd5Sum( const TQString& );
  void dragEnterEvent( TQDragEnterEvent* );
  void dropEvent( TQDropEvent* );

  void init();

  void toggleAll();

 private:
  enum {
    IMAGE_UNKNOWN,
    IMAGE_ISO,
    IMAGE_CUE_BIN,
    IMAGE_AUDIO_CUE,
    IMAGE_CDRDAO_TOC,
    IMAGE_CDRECORD_CLONE };

  void setupGui();
  void createIso9660InfoItems( K3bIso9660* );
  void createCdrecordCloneItems( const TQString&, const TQString& );
  void createCueBinItems( const TQString&, const TQString& );
  void createAudioCueItems( const K3bCueFileParser& cp );
  int currentImageType();
  TQString imagePath() const;

  K3bWriterSelectionWidget* m_writerSelectionWidget;
  TQCheckBox* m_checkDummy;
  TQCheckBox* m_checkNoFix;
  TQCheckBox* m_checkCacheImage;
  TQCheckBox* m_checkVerify;
  K3bDataModeWidget* m_dataModeWidget;
  K3bWritingModeWidget* m_writingModeWidget;
  TQSpinBox* m_spinCopies;

  KURLRequester* m_editImagePath;
  TQComboBox* m_comboImageType;

  K3bListView* m_infoView;
  K3bTempDirSelectionWidget* m_tempDirSelectionWidget;

  class Private;
  Private* d;
};

#endif
