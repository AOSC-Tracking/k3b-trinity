/*
*
* $Id: k3bvcdview.cpp 619556 2007-01-03 17:38:12Z trueg $
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

// TQt includes
#include <tqlayout.h>
#include <tqstring.h>


// KDE-includes
#include <tdelocale.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdemessagebox.h>

// K3b Includes
#include "k3bvcdview.h"
#include "k3bvcddoc.h"
#include "k3bvcdlistview.h"
#include "k3bvcdburndialog.h"
#include <k3bfillstatusdisplay.h>
#include <k3bexternalbinmanager.h>
#include <k3bcore.h>


K3bVcdView::K3bVcdView( K3bVcdDoc* pDoc, TQWidget* parent, const char *name )
        : K3bView( pDoc, parent, name )
{
    m_doc = pDoc;

    // --- setup GUI ---------------------------------------------------

    m_vcdlist = new K3bVcdListView( this, pDoc, this );
    setMainWidget( m_vcdlist );
    fillStatusDisplay() ->showSize();

    connect( m_vcdlist, TQ_SIGNAL( lengthReady() ), fillStatusDisplay(), TQ_SLOT( update() ) );
    connect( m_doc, TQ_SIGNAL( newTracks() ), fillStatusDisplay(), TQ_SLOT( update() ) );
}

K3bVcdView::~K3bVcdView()
{}


K3bProjectBurnDialog* K3bVcdView::newBurnDialog( TQWidget * parent, const char * name )
{
  return new K3bVcdBurnDialog( m_doc, parent, name, true );
}


void K3bVcdView::init()
{
  if( !k3bcore->externalBinManager()->foundBin( "vcdxbuild" ) ) {
    kdDebug() << "(K3bVcdView) could not find vcdxbuild executable" << endl;
    KMessageBox::information( this,
			      i18n( "Could not find VcdImager executable. "
				    "To create VideoCD's you must install VcdImager >= 0.7.12. "
				    "You can find this on your distribution disks or download "
				    "it from http://www.vcdimager.org" ) );
  }
}

#include "k3bvcdview.moc"
