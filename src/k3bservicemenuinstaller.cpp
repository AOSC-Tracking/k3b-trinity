/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bservicemenuinstaller.h"

#include <tdeglobal.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <tdemessagebox.h>
#include <tdeio/netaccess.h>
#include <kdebug.h>


class K3bServiceInstaller::Private
{
public:
  TQString app;
  TQStringList allServiceMenus;
  TQStringList allServiceMenuFiles;
  TQString konqiServicemenusFolder;

  void update() {
    // the list of installable servicemenus the application provides
    allServiceMenus = TDEGlobal::dirs()->findAllResources( "data",
							 "k3b/servicemenus/*.desktop",
							 false,
							 true );

    // just the filenames
    allServiceMenuFiles.clear();
    for( unsigned int i = 0; i < allServiceMenus.count(); ++i )
      allServiceMenuFiles.append( allServiceMenus[i].section( '/', -1 ) );

    // the local konqueror servicemenu folder (we just create it here to be on the safe side)
    konqiServicemenusFolder = TDEGlobal::dirs()->saveLocation( "data", "konqueror/servicemenus/", true );
  }
};


K3bServiceInstaller::K3bServiceInstaller( const TQString& appname )
{
  d = new Private;
  d->app = appname;
}


K3bServiceInstaller::~K3bServiceInstaller()
{
  delete d;
}


bool K3bServiceInstaller::allInstalled() const
{
  d->update();

  for( unsigned int i = 0; i < d->allServiceMenuFiles.count(); ++i )
    if( !TDEIO::NetAccess::exists( d->konqiServicemenusFolder + d->allServiceMenuFiles[i], true, 0 ) ) {
      kdDebug() << "(K3bServiceInstaller) service menu " << d->konqiServicemenusFolder << d->allServiceMenuFiles[i]
		<< " does not exist." << endl;
      return false;
    }
  
  return true;
}


bool K3bServiceInstaller::install( TQWidget* parent )
{
  d->update();

  bool success = true;

  // simply link all the globally installed K3b service menus to the local konqi service menu folder
  for( unsigned int i = 0; i < d->allServiceMenus.count(); ++i )
    if( !TDEIO::NetAccess::file_copy( KURL::fromPathOrURL( d->allServiceMenus[i] ), 
				    KURL::fromPathOrURL( d->konqiServicemenusFolder + d->allServiceMenuFiles[i] ), -1, 
				    true, false, parent ) )
      success = false;

  if( !success && parent )
    KMessageBox::error( parent,
			TDEIO::NetAccess::lastErrorString(),
			i18n("Failed to copy service menu files") );

  return success;
}


bool K3bServiceInstaller::remove( TQWidget* parent )
{
  d->update();

  bool success = true;

  for( unsigned int i = 0; i < d->allServiceMenuFiles.count(); ++i )
    if( TDEIO::NetAccess::exists( d->konqiServicemenusFolder + d->allServiceMenuFiles[i], true, parent ) )
      if( !TDEIO::NetAccess::del( d->konqiServicemenusFolder + d->allServiceMenuFiles[i], parent ) )
	success = false;

  if( !success && parent )
    KMessageBox::error( parent,
			TDEIO::NetAccess::lastErrorString(),
			i18n("Failed to remove service menu files") );

  return success;
}
