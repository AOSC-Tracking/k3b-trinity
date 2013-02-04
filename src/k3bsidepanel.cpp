/* 
 *
 * $Id: k3bsidepanel.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bsidepanel.h"
#include "k3b.h"
#include "k3bfiletreeview.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <tqtoolbutton.h>
#include <tqframe.h>
#include <tqlayout.h>



K3bSidePanel::K3bSidePanel( K3bMainWindow* m, TQWidget* parent, const char* name )
  : TQToolBox( parent, name ),
    m_mainWindow(m)
{
  // our first widget is the tree view
  m_fileTreeView = new K3bFileTreeView( this );
  addItem( m_fileTreeView, SmallIconSet( "folder_open" ), i18n("Folders") );

  // CD projects
  TQFrame* cdProjectsFrame = createPanel();
  addItem( cdProjectsFrame, SmallIconSet( "cdrom_unmount" ), i18n("CD Tasks") );
  addButton( cdProjectsFrame, m_mainWindow->action( "file_new_audio" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "file_new_data" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "file_new_mixed" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "file_new_vcd" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "file_new_movix" ) );
  TQGridLayout* grid = (TQGridLayout*)cdProjectsFrame->layout();
  grid->setRowSpacing( grid->numRows(), 15 );
  addButton( cdProjectsFrame, m_mainWindow->action( "tools_copy_cd" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "tools_write_cd_image" ) );
  addButton( cdProjectsFrame, m_mainWindow->action( "tools_blank_cdrw" ) );
  grid->setRowStretch( grid->numRows()+1, 1 );

  // DVD projects
  TQFrame* dvdProjectsFrame = createPanel();
  addItem( dvdProjectsFrame, SmallIconSet( "dvd_unmount" ), i18n("DVD Tasks") );
  addButton( dvdProjectsFrame, m_mainWindow->action( "file_new_dvd" ) );
  addButton( dvdProjectsFrame, m_mainWindow->action( "file_new_video_dvd" ) );
  addButton( dvdProjectsFrame, m_mainWindow->action( "file_new_movix_dvd" ) );
  grid = (TQGridLayout*)dvdProjectsFrame->layout();
  grid->setRowSpacing( grid->numRows(), 15 );
  addButton( dvdProjectsFrame, m_mainWindow->action( "tools_copy_dvd" ) );
  addButton( dvdProjectsFrame, m_mainWindow->action( "tools_write_dvd_iso" ) );
  addButton( dvdProjectsFrame, m_mainWindow->action( "tools_format_dvd" ) );
  grid->setRowStretch( grid->numRows()+1, 1 );


  // Tools
  // TODO sidepanel tools
}


K3bSidePanel::~K3bSidePanel()
{
}


TQFrame* K3bSidePanel::createPanel()
{
  TQFrame* frame = new TQFrame( this );
  frame->setPaletteBackgroundColor( TQt::white );
  TQGridLayout* grid = new TQGridLayout( frame );
  grid->setMargin( 5 );
  grid->setSpacing( 5 );
  return frame;
}


void K3bSidePanel::addButton( TQFrame* frame, TDEAction* a )
{
  if( a ) {
    TQToolButton* b = new TQToolButton( frame );
    b->setTextLabel( a->toolTip(), true );
    b->setTextLabel( a->text(), false );
    b->setIconSet( a->iconSet(TDEIcon::Small) );
    b->setUsesTextLabel( true );
    b->setAutoRaise( true );
    b->setTextPosition( TQToolButton::BesideIcon );

    connect( b, TQT_SIGNAL(clicked()), a, TQT_SLOT(activate()) );

    TQGridLayout* grid = (TQGridLayout*)(frame->layout());
    grid->addWidget( b, grid->numRows(), 0 );
  }
  else
    kdDebug() << "(K3bSidePanel) null action." << endl;
}

#include "k3bsidepanel.moc"
