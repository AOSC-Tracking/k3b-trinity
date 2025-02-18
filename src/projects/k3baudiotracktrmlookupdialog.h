/* 
 *
 * $Id: k3baudiotracktrmlookupdialog.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_AUDIOTRACK_TRM_LOOKUP_DIALOG_H_
#define _K3B_AUDIOTRACK_TRM_LOOKUP_DIALOG_H_

#include <kdialogbase.h>
#include <tqptrlist.h>

class TQLabel;
class K3bAudioTrack;
class K3bMusicBrainzJob;
class K3bBusyWidget;


class K3bAudioTrackTRMLookupDialog : public KDialogBase
{
  TQ_OBJECT
  

 public:
  K3bAudioTrackTRMLookupDialog( TQWidget* parent = 0, const char* name = 0 );
  ~K3bAudioTrackTRMLookupDialog();

  /**
   * This will show the dialog and start the lookup
   */
  int lookup( const TQPtrList<K3bAudioTrack>& tracks );

 private slots:
  void slotMbJobFinished( bool );
  void slotMbJobInfoMessage( const TQString&, int );
  void slotTrackFinished( K3bAudioTrack* track, bool success );
  void slotCancel();

 private:
  TQLabel* m_infoLabel;
  K3bBusyWidget* m_busyWidget;
  K3bMusicBrainzJob* m_mbJob;
  bool m_inLoop;
};

#endif
