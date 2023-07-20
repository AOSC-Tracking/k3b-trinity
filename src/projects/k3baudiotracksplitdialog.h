/* 
 *
 * $Id: k3baudiotracksplitdialog.h 620140 2007-01-05 12:02:29Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_AUDIO_TRACK_SPLIT_DIALOG_H_
#define _K3B_AUDIO_TRACK_SPLIT_DIALOG_H_

#include <kdialogbase.h>

namespace K3b {
  class Msf;
}
class K3bAudioTrack;
class K3bAudioEditorWidget;
class K3bMsfEdit;
class TDEActionCollection;
class TDEPopupMenu;


/**
 * Internally used by K3bAudioTrackView to get an msf value from the user.
 */
class K3bAudioTrackSplitDialog : public KDialogBase
{
  TQ_OBJECT
  

 public:
  K3bAudioTrackSplitDialog( K3bAudioTrack*, TQWidget* parent = 0, const char* name = 0 );
  ~K3bAudioTrackSplitDialog();

  bool eventFilter( TQObject* o, TQEvent* e );

  TDEActionCollection* actionCollection() const { return m_actionCollection; }

  /**
   * if this method returns true val is filled with the user selected value.
   */
  static void splitTrack( K3bAudioTrack* track, TQWidget* parent = 0, const char* name = 0 );

 private slots:
  void slotRangeModified( int, const K3b::Msf& start, const K3b::Msf& );
  void slotMsfEditChanged( const K3b::Msf& msf );
  void slotRangeSelectionChanged( int );
  void slotSplitHere();
  void slotRemoveRange();
  void splitAt( const TQPoint& p );

 private:
  void setupActions();

  K3bAudioEditorWidget* m_editorWidget;
  K3bMsfEdit* m_msfEditStart;
  K3bMsfEdit* m_msfEditEnd;
  K3bAudioTrack* m_track;
  TDEActionCollection* m_actionCollection;
  TDEPopupMenu* m_popupMenu;
  TQPoint m_lastClickPosition;
};

#endif
