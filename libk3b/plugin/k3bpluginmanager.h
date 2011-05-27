/* 
 *
 * $Id: k3bpluginmanager.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_PLUGIN_MANAGER_H_
#define _K3B_PLUGIN_MANAGER_H_

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include "k3b_export.h"


class K3bPlugin;
class TQWidget;


/**
 * Use this class to access all K3b plugins (this does not include the
 * KParts Plugins!).
 * Like the K3bCore the single instance (which has to be created manually)
 * can be obtained with the k3bpluginmanager macro.
 */
class LIBK3B_EXPORT K3bPluginManager : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bPluginManager( TQObject* tqparent = 0, const char* name = 0 );
  ~K3bPluginManager();

  /**
   * if group is empty all plugins are returned
   */
  TQPtrList<K3bPlugin> plugins( const TQString& group = TQString() ) const;

  /**
   * Returnes a list of the available groups.
   */
  TQStringList groups() const;

  int pluginSystemVersion() const;

 public slots:
  /**
   * Loads all plugins from the ressource directories.
   */
  void loadAll();

  void loadPlugin( const TQString& fileName );

  int execPluginDialog( K3bPlugin*, TQWidget* tqparent = 0, const char* name = 0 );

 private:
  class Private;
  Private* d;
};

#endif
