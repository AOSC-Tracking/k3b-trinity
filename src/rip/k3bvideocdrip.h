/*
*
* $Id: k3bvideocdrip.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003 Christian Kvasny <chris@k3b.org>
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

#ifndef K3BVIDEOCDRIP_H
#define K3BVIDEOCDRIP_H

#include <k3bjob.h>
#include <k3bdiskinfo.h>
#include "k3bvideocdrippingoptions.h"

class TQString;
class TDEProcess;
class TQDataStream;

class K3bVideoCdRip : public K3bJob
{
        TQ_OBJECT
  

    public:
        K3bVideoCdRip( K3bJobHandler*, K3bVideoCdRippingOptions* options, TQObject* parent = 0, const char* name = 0 );
        ~K3bVideoCdRip();

        enum { CDROM, BIN_IMAGE, NRG_IMAGE };

        TQString jobDescription() const;
        TQString jobDetails() const;

    public slots:
        void start();
        void cancel();

    private slots:
        void cancelAll();

    protected slots:
        void slotVcdXRipFinished();
        void slotParseVcdXRipOutput( TDEProcess*, char* output, int len );

    private:
        void vcdxRip();
        void parseInformation( TQString );

        enum { stageUnknown, stageScan, stageFinished, _stage_max };

        int m_stage;
        int m_bytesFinished;
        int m_ripsourceType;
        int m_oldpercent;

        long m_subPosition;

        TQString m_collectedOutput;

        K3bVideoCdRippingOptions * m_videooptions;

        bool m_canceled;

        TDEProcess* m_process;

};

#endif
