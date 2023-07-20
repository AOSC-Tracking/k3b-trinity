/* 
 *
 * $Id: k3bsystemproblemdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_SYSTEM_DIALOG_H_
#define _K3B_SYSTEM_DIALOG_H_

#include <tqstring.h>
#include <tqvaluelist.h>

#include <kdialog.h>

class TQPushButton;
class TQCheckBox;
class TQCloseEvent;
namespace K3bDevice {
  class Device;
}


class K3bSystemProblem
{
 public:
  K3bSystemProblem( int type = NON_CRITICAL,
		    const TQString& problem = TQString(),
		    const TQString& details = TQString(),
		    const TQString& solution = TQString(),
		    bool k3bsetup = false );

  enum {
    CRITICAL,
    NON_CRITICAL,
    WARNING
  };

  int type;
  TQString problem;
  TQString details;
  TQString solution;
  bool solvableByK3bSetup;
};


/**
 * The K3bSystemProblem checks for problems with the system setup
 * that could prevent K3b from funcioning properly. Examples are
 * missing external appplications like cdrecord or versions of 
 * external applications that are too old.
 *
 * Usage:
 * <pre>
 * if( K3bSystemProblemDialog::readCheckSystemConfig() )
 *    K3bSystemProblemDialog::checkSystem( this );
 * </pre>
 */
class K3bSystemProblemDialog : public KDialog
{
  TQ_OBJECT
  

 public:
  /**
   * Determines if the system problem dialog should be shown or not.
   * It basicly reads a config entry. But in addition it
   * always forces the system check if a new version has been installed
   * or K3b is started for the first time.
   */
  static bool readCheckSystemConfig();
  static void checkSystem( TQWidget* parent = 0, 
			   const char* name = 0 );

 protected:
  void closeEvent( TQCloseEvent* );

 private slots:
  void slotK3bSetup();

 private:
  K3bSystemProblemDialog( const TQValueList<K3bSystemProblem>&,
			  TQWidget* parent = 0, 
			  const char* name = 0 );
  static int dmaActivated( K3bDevice::Device* );
  static TQPtrList<K3bDevice::Device> checkForAutomounting();

  TQPushButton* m_closeButton;
  TQPushButton* m_k3bsetupButton;
  TQCheckBox* m_checkDontShowAgain;
};

#endif
