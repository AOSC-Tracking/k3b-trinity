/* 
 *
 * $Id: k3bplugin.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_PLUGIN_H_
#define _K3B_PLUGIN_H_

#include <tqobject.h>
#include <kgenericfactory.h>
#include "k3b_export.h"

#define K3B_PLUGIN_SYSTEM_VERSION 3


class K3bPluginConfigWidget;
class TQWidget;



class K3bPluginInfo
{
  friend class K3bPluginManager;

 public:
  K3bPluginInfo() {
  }

  K3bPluginInfo( TQString libraryName,
		 TQString name,
		 TQString author,
		 TQString email,
		 TQString comment,
		 TQString version,
		 TQString licence )
    : m_libraryName(libraryName),
    m_name(name),
    m_author(author),
    m_email(email),
    m_comment(comment),
    m_version(version),
    m_licence(licence) {
  }

  const TQString& name() const { return m_name; }
  const TQString& author() const { return m_author; }
  const TQString& email() const { return m_email; }
  const TQString& comment() const { return m_comment; }
  const TQString& version() const { return m_version; }
  const TQString& licence() const { return m_licence; }

  const TQString& libraryName() const { return m_libraryName; }

 private:
  TQString m_libraryName;

  TQString m_name;
  TQString m_author;
  TQString m_email;
  TQString m_comment;
  TQString m_version;
  TQString m_licence;
};


/**
 * Base class for all plugins. You may use the K3bPluginFactory to make your plugin available.
 */
class LIBK3B_EXPORT K3bPlugin : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

    friend class K3bPluginManager;

 public:
  K3bPlugin( TQObject* parent = 0, const char* name = 0 );
  virtual ~K3bPlugin();

  const K3bPluginInfo& pluginInfo() const { return m_pluginInfo; }

  /**
   * Version of the plugin system this plugin was written for.
   */
  virtual int pluginSystemVersion() const = 0;

  /**
   * The plugin group.
   */
  virtual TQString group() const = 0;

  /**
   * Returns a widget which configures the plugin.
   *
   * The caller has to destroy the widget
   */
  virtual K3bPluginConfigWidget* createConfigWidget( TQWidget* parent = 0, const char* name = 0 ) const;

 private:
  K3bPluginInfo m_pluginInfo;
};

#endif
