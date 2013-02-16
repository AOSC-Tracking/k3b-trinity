/*
 *
 * $Id: k3bpluginoptiontab.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bpluginoptiontab.h"


#include <k3bpluginmanager.h>
#include <k3bplugin.h>
#include <k3bpluginconfigwidget.h>
#include <k3blistview.h>
#include <k3bcore.h>

#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <tdeconfig.h>
#include <tdeglobalsettings.h>
#include <tdeversion.h>

#include <tqstringlist.h>
#include <tqpushbutton.h>


class K3bPluginOptionTab::PluginViewItem : public K3bListViewItem
{
public:
  PluginViewItem( K3bPlugin* p, TDEListViewItem* parent )
    : K3bListViewItem( parent ),
      plugin(p) {
    const K3bPluginInfo& info = p->pluginInfo();
    setText( 0, info.name() );
    if( !info.author().isEmpty() ) {
      if( info.email().isEmpty() )
	setText( 1, info.author() );
      else
	setText( 1, info.author() + " <" + info.email() + ">" );
    }
    setText( 2, info.version() );
    setText( 3, info.comment() );
    setText( 4, info.licence() );
  }

  K3bPlugin* plugin;
};



K3bPluginOptionTab::K3bPluginOptionTab( TQWidget* parent, const char* name )
  : base_K3bPluginOptionTab( parent, name )
{
#if KDE_IS_VERSION(3,4,0)
  m_viewPlugins->setShadeSortColumn( false );
#endif
  m_viewPlugins->addColumn( i18n("Name") );
  m_viewPlugins->addColumn( i18n("Author") );
  m_viewPlugins->addColumn( i18n("Version") );
  m_viewPlugins->addColumn( i18n("Description") );
  m_viewPlugins->addColumn( i18n("License") );
  m_viewPlugins->setAlternateBackground( TQColor() );
  m_viewPlugins->setAllColumnsShowFocus(true);

  connect( m_viewPlugins, TQT_SIGNAL(doubleClicked(TQListViewItem*, const TQPoint&, int)), this, TQT_SLOT(slotConfigureButtonClicked()) );
  connect( m_buttonConfigure, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotConfigureButtonClicked()) );
  connect( m_viewPlugins, TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(slotSelectionChanged()) );
}


K3bPluginOptionTab::~K3bPluginOptionTab()
{
}


void K3bPluginOptionTab::readSettings()
{
  m_viewPlugins->clear();
  TQStringList groups = k3bcore->pluginManager()->groups();
  for( TQStringList::const_iterator it = groups.begin();
       it != groups.end(); ++it ) {
    const TQString& group = *it;

    K3bListViewItem* groupViewItem = new K3bListViewItem( m_viewPlugins,
							  m_viewPlugins->lastChild(),
							  group );
    TQFont f( font() );
    f.setBold(true);
    groupViewItem->setFont( 0, f );
    groupViewItem->setBackgroundColor( 0, TDEGlobalSettings::alternateBackgroundColor() );
    groupViewItem->setBackgroundColor( 1, TDEGlobalSettings::alternateBackgroundColor() );
    groupViewItem->setBackgroundColor( 2, TDEGlobalSettings::alternateBackgroundColor() );
    groupViewItem->setBackgroundColor( 3, TDEGlobalSettings::alternateBackgroundColor() );
    groupViewItem->setBackgroundColor( 4, TDEGlobalSettings::alternateBackgroundColor() );
    groupViewItem->setSelectable( false );

    TQPtrList<K3bPlugin> fl = k3bcore->pluginManager()->plugins( group );
    for( TQPtrListIterator<K3bPlugin> fit( fl ); fit.current(); ++fit )
      (void)new PluginViewItem( fit.current(), groupViewItem );

    groupViewItem->setOpen(true);
  }

  slotSelectionChanged();
}


bool K3bPluginOptionTab::saveSettings()
{
  return true;
}


void K3bPluginOptionTab::slotConfigureButtonClicked()
{
  TQListViewItem* item = m_viewPlugins->selectedItem();
  if( PluginViewItem* pi = dynamic_cast<PluginViewItem*>( item ) )
    k3bcore->pluginManager()->execPluginDialog( pi->plugin, this );
}


void K3bPluginOptionTab::slotSelectionChanged()
{
  m_buttonConfigure->setEnabled( dynamic_cast<PluginViewItem*>( m_viewPlugins->selectedItem() ) != 0 );
}

#include "k3bpluginoptiontab.moc"
