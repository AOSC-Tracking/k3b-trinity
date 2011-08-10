/*
*
* $Id: k3bvcdburndialog.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3BVCDBURNDIALOG_H
#define K3BVCDBURNDIALOG_H

#include "k3bprojectburndialog.h"
#include <tqmultilineedit.h>

class TQCheckBox;
class TQGroupBox;
class TQButtonGroup;
class TQSpinBox;
class TQRadioButton;
class TQLabel;
class TQLineEdit;
class TQMultiLineEdit;
class TQToolButton;
class K3bWriterSelectionWidget;
class K3bTempDirSelectionWidget;
class K3bVcdDoc;
class K3bVcdOptions;

class K3bVcdBurnDialog : public K3bProjectBurnDialog
{
        Q_OBJECT
  TQ_OBJECT

    public:
        K3bVcdBurnDialog( K3bVcdDoc* doc, TQWidget *parent = 0, const char *name = 0, bool modal = true );
        ~K3bVcdBurnDialog();

        K3bVcdDoc* vcdDoc() const
        {
            return m_vcdDoc;
        }

    protected:
        void setupAdvancedTab();
        void setupVideoCdTab();
        void setupLabelTab();
        void saveSettings();
        void readSettings();

        void loadK3bDefaults();
        void loadUserDefaults( KConfigBase* );
        void saveUserDefaults( KConfigBase* );

        // -----------------------------------------------------------
        // the video-cd-tab
        // -----------------------------------------------------------

        TQButtonGroup* m_groupVcdFormat;
        TQRadioButton* m_radioVcd11;
        TQRadioButton* m_radioVcd20;
        TQRadioButton* m_radioSvcd10;
        TQRadioButton* m_radioHqVcd10;

        TQGroupBox* m_groupOptions;
        TQCheckBox* m_checkAutoDetect;
        TQCheckBox* m_checkNonCompliant;
        TQCheckBox* m_checkVCD30interpretation;
        TQCheckBox* m_check2336;

        // CD-i
        TQGroupBox* m_groupCdi;
        TQCheckBox* m_checkCdiSupport;
        TQMultiLineEdit* m_editCdiCfg;


        // -----------------------------------------------------------
        // the video-label-tab
        // -----------------------------------------------------------

        TQLineEdit* m_editVolumeId;
        TQLineEdit* m_editPublisher;
        TQLineEdit* m_editAlbumId;

        TQSpinBox* m_spinVolumeCount;
        TQSpinBox* m_spinVolumeNumber;

        // -----------------------------------------------------------
        // the advanced-tab
        // -----------------------------------------------------------

        TQGroupBox* m_groupGeneric;
        TQGroupBox* m_groupGaps;
        TQGroupBox* m_groupMisc;

        TQCheckBox* m_checkPbc;
        TQCheckBox* m_checkSegmentFolder;
        TQCheckBox* m_checkRelaxedAps;
        TQCheckBox* m_checkUpdateScanOffsets;
        TQCheckBox* m_checkGaps;

        TQSpinBox* m_spinRestriction;
        TQSpinBox* m_spinPreGapLeadout;
        TQSpinBox* m_spinPreGapTrack;
        TQSpinBox* m_spinFrontMarginTrack;
        TQSpinBox* m_spinRearMarginTrack;
        TQSpinBox* m_spinFrontMarginTrackSVCD;
        TQSpinBox* m_spinRearMarginTrackSVCD;

        TQLabel* m_labelRestriction;
        TQLabel* m_labelPreGapLeadout;
        TQLabel* m_labelPreGapTrack;
        TQLabel* m_labelFrontMarginTrack;
        TQLabel* m_labelRearMarginTrack;

        // -----------------------------------------------------------

    private:
        K3bVcdDoc* m_vcdDoc;
        void setVolumeID( );
        void MarginChecked( bool );
        void saveCdiConfig();
        void loadCdiConfig();
        void loadDefaultCdiConfig();
        void toggleAll();

    protected slots:
        void slotStartClicked();

        void slotGapsChecked( bool );
        void slotSpinVolumeCount();
        void slotVcdTypeClicked( int );
        void slotCdiSupportChecked( bool );
        void slotAutoDetect( bool );
};

#endif
