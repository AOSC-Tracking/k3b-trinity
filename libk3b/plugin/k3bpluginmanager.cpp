/* 
 *
 * $Id: k3bpluginmanager.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bpluginmanager.h"
#include "k3bplugin.h"
#include "k3bpluginconfigwidget.h"
#include <k3bversion.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kdialogbase.h>
#include <tdemessagebox.h>
#include <klibloader.h>

#include <tqptrlist.h>
#include <tqmap.h>
#include <tqdir.h>



class K3bPluginManager::Private
{
public:
  TQPtrList<K3bPlugin> plugins;
};




K3bPluginManager::K3bPluginManager( TQObject* parent, const char* name )
  : TQObject( parent, name )
{
  d = new Private();
}


K3bPluginManager::~K3bPluginManager()
{
  delete d;
}



TQStringList K3bPluginManager::groups() const
{
  TQStringList grps;

  TQPtrList<K3bPlugin> fl;
  for( TQPtrListIterator<K3bPlugin> it( d->plugins );
       it.current(); ++it ) {
    if( !grps.contains( it.current()->group() ) )
	grps.append( it.current()->group() );
  }

  return grps;
}


TQPtrList<K3bPlugin> K3bPluginManager::plugins( const TQString& group ) const
{
  TQPtrList<K3bPlugin> fl;
  for( TQPtrListIterator<K3bPlugin> it( d->plugins );
       it.current(); ++it ) {
    if( it.current()->group() == group || group.isEmpty() )
      fl.append( it.current() );
  }
  return fl;
}


void K3bPluginManager::loadPlugin( const TQString& fileName )
{
  KSimpleConfig c( fileName, true );
  c.setGroup( "K3b Plugin" );

  TQString libName = c.readEntry( "Lib" );
  if( libName.isEmpty() ) {
    kdDebug() << "(K3bPluginManager) no Lib specified in " << fileName << endl;
    return;
  }

  // read the lib
  KLibFactory* factory = KLibLoader::self()->factory( libName.latin1() );
  if( factory ) {
    K3bPlugin* plugin = dynamic_cast<K3bPlugin*>( factory->create( this ) );
    if( plugin ) {
      // FIXME: improve this versioning stuff
      if( plugin->pluginSystemVersion() != K3B_PLUGIN_SYSTEM_VERSION ) {
	delete plugin;
	kdDebug() << "(K3bPluginManager) plugin system does not fit lib " << libName << endl;
      }
      else {
	plugin->m_pluginInfo = K3bPluginInfo( libName,
					      c.readEntry( "Name" ),
					      c.readEntry( "Author" ),
					      c.readEntry( "Email" ),
					      c.readEntry( "Comment" ),
					      c.readEntry( "Version" ),
					      c.readEntry( "License" ) );

	// make sure to only use the latest version of one plugin
	bool addPlugin = true;
	for( TQPtrListIterator<K3bPlugin> it( d->plugins ); *it; ++it ) {
	  if( it.current()->pluginInfo().name() == plugin->pluginInfo().name() ) {
	    if( K3bVersion(it.current()->pluginInfo().version()) < K3bVersion(plugin->pluginInfo().version()) ) {
	      K3bPlugin* p = it.current();
	      d->plugins.removeRef( p );
	      delete p;
	    }
	    else {
	      addPlugin = false;
	    }
	    break;
	  }
	}
	if( addPlugin )
	  d->plugins.append( plugin );
	else
	  delete plugin;
      }
    }
    else
      kdDebug() << "(K3bPluginManager) lib " << libName << " not a K3b plugin" << endl;
  }
  else
    kdDebug() << "(K3bPluginManager) lib " << libName << " not found" << endl;
}


void K3bPluginManager::loadAll()
{
  // we simply search the K3b plugin dir for now
  TQStringList dirs = TDEGlobal::dirs()->findDirs( "data", "k3b/plugins/" );

  for( TQStringList::const_iterator it = dirs.begin();
       it != dirs.end(); ++it ) {
    TQStringList entries = TQDir(*it).entryList( "*.plugin", TQDir::Files );
    for( TQStringList::const_iterator it2 = entries.begin();
	 it2 != entries.end(); ++it2 ) {
      loadPlugin( *it + *it2 );
    }
  }
}

int K3bPluginManager::pluginSystemVersion() const
{
  return K3B_PLUGIN_SYSTEM_VERSION;
}


int K3bPluginManager::execPluginDialog( K3bPlugin* plugin, TQWidget* parent, const char* name )
{
  KDialogBase dlg( parent, 
		   name, 
		   true,
		   i18n("Configure plugin %1").arg( plugin->pluginInfo().name() ) );
  
  K3bPluginConfigWidget* configWidget = plugin->createConfigWidget( &dlg );
  if( configWidget ) {
    dlg.setMainWidget( configWidget );
    connect( &dlg, TQ_SIGNAL(applyClicked()), configWidget, TQ_SLOT(saveConfig()) );
    connect( &dlg, TQ_SIGNAL(okClicked()), configWidget, TQ_SLOT(saveConfig()) );
    configWidget->loadConfig();
    int r = dlg.exec();
    delete configWidget;
    return r;
  }
  else {
    KMessageBox::sorry( parent, i18n("No settings available for plugin %1.").arg( plugin->pluginInfo().name() ) );
    return 0;
  }
}

#include "k3bpluginmanager.moc"
