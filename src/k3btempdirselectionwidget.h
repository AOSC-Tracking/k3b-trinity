/* 
 *
 * $Id: k3btempdirselectionwidget.h 690207 2007-07-20 10:40:19Z trueg $
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


#ifndef K3BTEMPDIRSELECTIONWIDGET_H
#define K3BTEMPDIRSELECTIONWIDGET_H

#include <tqgroupbox.h>
#include <tdeio/global.h>

class TQTimer;
class TQLabel;
class KURLRequester;
class TDEConfigBase;


/**
  *@author Sebastian Trueg
  */
class K3bTempDirSelectionWidget : public TQGroupBox
{
  TQ_OBJECT
  

 public: 
  K3bTempDirSelectionWidget( TQWidget *parent = 0, const char *name = 0 );
  ~K3bTempDirSelectionWidget();

  /** determines if the selection dialog should ask for a dir or a file */
  enum mode { DIR, FILE };

  int selectionMode() const { return m_mode; }

  /**
   * \return Free space in KB
   * FIXME: use TDEIO::filesize_t and return the number of bytes
   */
  unsigned long freeTempSpace() const;
  TQString tempPath() const;
  TQString tempDirectory() const;

  /**
   * Use this if you don't want K3bTempDirSelectionWidget to modify the
   * user input based on the mode.
   */
  TQString plainTempPath() const;

 public slots:
  void setTempPath( const TQString& );
  void setSelectionMode( int mode );
  void setNeededSize( TDEIO::filesize_t bytes );

  /**
   * In file selection mode if the user enters a directory name it will
   * automatically be expended to this filename.
   * Default is k3b_image.iso
   */
  void setDefaultImageFileName( const TQString& name );

  /**
   * saves the current path as the global default tempd dir.
   */
  void saveConfig();

  void readConfig( TDEConfigBase* );
  void saveConfig( TDEConfigBase* );

 private slots:
  void slotUpdateFreeTempSpace();
  void slotTempDirButtonPressed( KURLRequester* );
  void slotFixTempPath();

 private:
  void fixTempPath( bool forceNewImageName );

  TQLabel* m_imageFileLabel;
  TQLabel* m_labelCdSize;
  TQLabel* m_labelFreeSpace;
  KURLRequester* m_editDirectory;

  mutable unsigned long m_freeTempSpace;
  unsigned long m_requestedSize;

  int m_mode;

  TQString m_defaultImageFileName;
};

#endif
