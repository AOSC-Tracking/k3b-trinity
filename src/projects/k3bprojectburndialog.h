/* 
 *
 * $Id: k3bprojectburndialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BPROJECTBURNDIALOG_H
#define K3BPROJECTBURNDIALOG_H

#include <k3binteractiondialog.h>


class K3bDoc;
class K3bBurnJob;
class K3bWriterSelectionWidget;
class K3bTempDirSelectionWidget;
class TQGroupBox;
class TQCheckBox;
class TQTabWidget;
class TQSpinBox;
class TQVBoxLayout;
class K3bWritingModeWidget;
class TDEConfigBase;


/**
  *@author Sebastian Trueg
  */
class K3bProjectBurnDialog : public K3bInteractionDialog
{
   TQ_OBJECT
  

 public:
   K3bProjectBurnDialog( K3bDoc* doc, TQWidget *parent=0, const char *name=0, 
			 bool modal = true, bool dvd = false );
   ~K3bProjectBurnDialog();

   enum resultCode { Canceled = 0, Saved = 1, Burn = 2 };

   /**
    * shows the dialog with exec().
    * Use this instead of K3bInteractionDialog::exec
    * \param burn If true the dialog shows the Burn-button
    */
   int execBurnDialog( bool burn );

   K3bDoc* doc() const { return m_doc; }
	
 protected slots:
   /** burn */
   virtual void slotStartClicked();
   /** save */
   virtual void slotSaveClicked();
   virtual void slotCancelClicked();

   /**
    * gets called if the user changed the writer
    * default implementation just calls 
    * toggleAllOptions()
    */
   virtual void slotWriterChanged();

   /**
    * gets called if the user changed the writing app
    * default implementation just calls 
    * toggleAllOptions()
    */
   virtual void slotWritingAppChanged( int );

 signals:
   void writerChanged();

 protected:
   /**
    * The default implementation loads the following defaults:
    * <ul>
    *   <li>Writing mode</li>
    *   <li>Simulate</li>
    *   <li>on the fly</li>
    *   <li>remove images</li>
    *   <li>only create images</li>
    * </ul>
    */
   virtual void loadK3bDefaults();

   /**
    * The default implementation loads the following settings from the TDEConfig.
    * May be used in subclasses.
    * <ul>
    *   <li>Writing mode</li>
    *   <li>Simulate</li>
    *   <li>on the fly</li>
    *   <li>remove images</li>
    *   <li>only create images</li>
    *   <li>writer</li>
    *   <li>writing speed</li>
    * </ul>
    */
   virtual void loadUserDefaults( TDEConfigBase* );

   /**
    * The default implementation saves the following settings to the TDEConfig.
    * May be used in subclasses.
    * <ul>
    *   <li>Writing mode</li>
    *   <li>Simulate</li>
    *   <li>on the fly</li>
    *   <li>remove images</li>
    *   <li>only create images</li>
    *   <li>writer</li>
    *   <li>writing speed</li>
    * </ul>
    */
   virtual void saveUserDefaults( TDEConfigBase* );

   /**
    * The default implementation saves the following settings to the doc and may be called 
    * in subclasses:
    * <ul>
    *   <li>Writing mode</li>
    *   <li>Simulate</li>
    *   <li>on the fly</li>
    *   <li>remove images</li>
    *   <li>only create images</li>
    *   <li>writer</li>
    *   <li>writing speed</li>
    * </ul>
    */
   virtual void saveSettings();

   /**
    * The default implementation reads the following settings from the doc and may be called 
    * in subclasses:
    * <ul>
    *   <li>Writing mode</li>
    *   <li>Simulate</li>
    *   <li>on the fly</li>
    *   <li>remove images</li>
    *   <li>only create images</li>
    *   <li>writer</li>
    *   <li>writing speed</li>
    * </ul>
    */
   virtual void readSettings();

   virtual void toggleAll();

   /**
    * use this to set additionell stuff in the job
    */
   virtual void prepareJob( K3bBurnJob* ) {};

   void prepareGui();

   void addPage( TQWidget*, const TQString& title );

   /**
    * Call this if you must reimplement it.
    * \reimplemented from K3bInteractionDialog
    */
   virtual void init();

   K3bWriterSelectionWidget* m_writerSelectionWidget;
   K3bTempDirSelectionWidget* m_tempDirSelectionWidget;
   K3bWritingModeWidget* m_writingModeWidget;
   TQGroupBox* m_optionGroup;
   TQVBoxLayout* m_optionGroupLayout;
   TQCheckBox* m_checkCacheImage;
   TQCheckBox* m_checkSimulate;
   TQCheckBox* m_checkRemoveBufferFiles;
   TQCheckBox* m_checkOnlyCreateImage;
   TQSpinBox* m_spinCopies;

 private:
   K3bDoc* m_doc;
   K3bBurnJob* m_job;
   TQTabWidget* m_tabWidget;
   bool m_dvd;
};

#endif
