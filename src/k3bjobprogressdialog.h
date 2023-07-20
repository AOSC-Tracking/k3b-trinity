/* 
 *
 * $Id: k3bjobprogressdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_JOB_PROGRESSDIALOG_H_
#define _K3B_JOB_PROGRESSDIALOG_H_

#include <kdialog.h>

#include "k3bdebuggingoutputfile.h"

#include <k3bjobhandler.h>

#include <tqdatetime.h>
#include <tqfile.h>

class TQVBoxLayout;
class TQHBoxLayout;
class TQGridLayout;
class TDEListView;
class TQFrame;
class TQGroupBox;
class TQLabel;
class TQListViewItem;
class KProgress;
class TQPushButton;
class TQTimer;
class K3bJob;
class KCutLabel;
class TQCloseEvent;
class TQGridLayout;
class TQKeyEvent;
class K3bJobProgressOSD;
class K3bThemedLabel;


class K3bJobProgressDialog : public KDialog, public K3bJobHandler
{
  TQ_OBJECT
  

 public:
  K3bJobProgressDialog( TQWidget* parent = 0, 
			const char* name = 0, 
			bool showSubProgress = true, 
			bool modal = FALSE, 
			WFlags fl = 0 );
  virtual ~K3bJobProgressDialog();

  virtual void setJob( K3bJob* job );
  void setExtraInfo( TQWidget *extra );

  /**
   * reimplemented for internal reasons
   */
  void show();

  /**
   * reimplemented for internal reasons
   */
  void hide();

  /**
   * This will show the dialog and then start the given job or
   * if job == 0 the job set with setJob
   * Use instead of exec()
   */
  int startJob( K3bJob* job = 0 );

  TQSize sizeHint() const;

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
  
 protected slots:
  virtual void slotProcessedSize( int processed, int size );
  virtual void slotProcessedSubSize( int processed, int size );
  virtual void slotInfoMessage( const TQString& infoString, int type );
  virtual void slotDebuggingOutput( const TQString&, const TQString& );
  virtual void slotNewSubTask(const TQString& name);
  virtual void slotNewTask(const TQString& name);
  virtual void slotFinished(bool);
  virtual void slotCanceled();
  virtual void slotStarted();


  void slotCancelButtonPressed();
  void slotUpdateTime();
  void slotShowDebuggingOutput();

  void slotProgress( int );

  virtual void slotThemeChanged();

 protected:
  void closeEvent( TQCloseEvent* );
  void keyPressEvent( TQKeyEvent* e );

  void setupGUI();
  void setupConnections();
	
  K3bThemedLabel* m_labelJob;
  K3bThemedLabel* m_labelJobDetails;
  TDEListView* m_viewInfo;
  K3bThemedLabel* m_labelTask;
  K3bThemedLabel* m_labelElapsedTime;
  KCutLabel* m_labelSubTask;
  TQLabel* m_labelSubProcessedSize;
  KProgress* m_progressSubPercent;
  TQLabel* m_labelProcessedSize;
  KProgress* m_progressPercent;
  TQFrame* m_frameExtraInfo;
  TQPushButton* m_buttonCancel;
  TQPushButton* m_buttonClose;
  TQPushButton* m_buttonShowDebug;
  K3bThemedLabel* m_pixLabel;

  TQGridLayout* m_frameExtraInfoLayout;

 private:
  class Private;
  Private* d;

  K3bJob* m_job;
  TQTimer* m_timer;
  TQTime m_startTime;
  TQTime m_lastProgressUpdateTime;

  K3bDebuggingOutputFile m_logFile;

  TQMap<TQString, TQStringList> m_debugOutputMap;

  bool m_bCanceled;

  TQString m_plainCaption;

  bool in_loop;

  K3bJobProgressOSD* m_osd;
};


#endif
