/*
*
* $Id: k3bvcdtrackdialog.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2004 Christian Kvasny <chris@k3b.org>
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

#ifndef K3BVCDTRACKDIALOG_H
#define K3BVCDTRACKDIALOG_H

#include <kdialogbase.h>
#include <tqptrlist.h>
#include <tqtabwidget.h>

#include <k3bvcddoc.h>
#include <k3blistview.h>

class K3bVcdTrack;
class TQLabel;
class TQCheckBox;
class TQComboBox;
class TQGroupBox;
class TQRadioButton;
class TQButtonGroup;
class KCutLabel;
class K3bCutComboBox;


class K3bVcdTrackDialog : public KDialogBase
{
        TQ_OBJECT
  

    public:
        K3bVcdTrackDialog( K3bVcdDoc*, TQPtrList<K3bVcdTrack>& tracks, TQPtrList<K3bVcdTrack>& selectedTracks, TQWidget* parent = 0, const char* name = 0 );
        ~K3bVcdTrackDialog();

    protected slots:
        void slotOk();
        void slotApply();

    private slots:
        void slotPlayTimeChanged( int );
        void slotWaitTimeChanged( int );
        void slotPbcToggled( bool );
        void slotUseKeysToggled( bool );
        void slotGroupkeyToggled( bool );


    private:
        K3bVcdDoc* m_vcdDoc;
        TQPtrList<K3bVcdTrack> m_tracks;
        TQPtrList<K3bVcdTrack> m_selectedTracks;
        TQMap<TQString, K3bVcdTrack*> m_numkeysmap;
        TQTabWidget* m_mainTabbed;

        KCutLabel* m_displayFileName;
        TQLabel* m_labelMimeType;
        TQLabel* m_displaySize;
        TQLabel* m_displayLength;
        TQLabel* m_muxrate;

        TQLabel* m_mpegver_audio;
        TQLabel* m_rate_audio;
        TQLabel* m_sampling_frequency_audio;
        TQLabel* m_mode_audio;
        TQLabel* m_copyright_audio;

        TQLabel* m_mpegver_video;
        TQLabel* m_rate_video;
        TQLabel* m_chromaformat_video;
        TQLabel* m_format_video;
        TQLabel* m_resolution_video;
        TQLabel* m_highresolution_video;

        TQLabel* m_labelAfterTimeout;
        TQLabel* m_labelWait;

        TQGroupBox* m_groupPlay;
        TQGroupBox* m_groupPbc;
        TQGroupBox* m_groupKey;
        TQWidget* m_widgetnumkeys;

        K3bCutComboBox* m_pbc_previous;
        K3bCutComboBox* m_pbc_next;
        K3bCutComboBox* m_pbc_return;
        K3bCutComboBox* m_pbc_default;
        K3bCutComboBox* m_comboAfterTimeout;

        TQCheckBox* m_check_reactivity;
        TQCheckBox* m_check_pbc;
        TQCheckBox* m_check_usekeys;
        TQCheckBox* m_check_overwritekeys;
        K3bListView* m_list_keys;

        TQSpinBox* m_spin_times;
        TQSpinBox* m_spin_waittime;

        void prepareGui();
        void setupPbcTab();
        void setupPbcKeyTab();
        void setupAudioTab();
        void setupVideoTab();
        void fillGui();
        void fillPbcGui();

        void setPbcTrack( K3bVcdTrack*, K3bCutComboBox*, int );
        void setDefinedNumKeys( );
        TQString displayName( K3bVcdTrack* );
        K3bVcdOptions* VcdOptions()
        {
            return m_vcdDoc->vcdOptions();
        }
};

#endif
