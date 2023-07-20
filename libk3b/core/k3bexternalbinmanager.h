/* 
 *
 * $Id: k3bexternalbinmanager.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_EXTERNAL_BIN_MANAGER_H
#define K3B_EXTERNAL_BIN_MANAGER_H

#include <tqmap.h>
#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include "k3b_export.h"
#include "k3bversion.h"

class TDEConfig;
class TDEProcess;


class K3bExternalProgram;


/**
 * A K3bExternalBin represents an installed version of a program.
 * All K3bExternalBin objects are managed by K3bExternalPrograms.
 *
 * A bin may have certain features that are represented by a string.
 */
class LIBK3B_EXPORT K3bExternalBin
{
 public:
  K3bExternalBin( K3bExternalProgram* );
  virtual ~K3bExternalBin() {}

  K3bVersion version;
  TQString path;
  TQString copyright;

  const TQString& name() const;
  bool isEmpty() const;
  const TQStringList& userParameters() const;
  const TQStringList& features() const { return m_features; }

  bool hasFeature( const TQString& ) const;
  void addFeature( const TQString& );

  K3bExternalProgram* program() const { return m_program; }

 private:
  TQStringList m_features;
  K3bExternalProgram* m_program;
};


/**
 * This is the main class that represents a program
 * It's scan method has to be reimplemented for every program
 * It manages a list of K3bExternalBin-objects that each represent
 * one installed version of the program.
 */
class LIBK3B_EXPORT K3bExternalProgram
{
 public:
  K3bExternalProgram( const TQString& name );
  virtual ~K3bExternalProgram();

  const K3bExternalBin* defaultBin() const { return m_bins.getFirst(); }
  const K3bExternalBin* mostRecentBin() const;

  void addUserParameter( const TQString& );
  void setUserParameters( const TQStringList& list ) { m_userParameters = list; }

  const TQStringList& userParameters() const { return m_userParameters; }
  const TQString& name() const { return m_name; }

  void addBin( K3bExternalBin* );
  void clear() { m_bins.clear(); }
  void setDefault( const K3bExternalBin* );
  void setDefault( const TQString& path );

  const TQPtrList<K3bExternalBin>& bins() const { return m_bins; }

  /**
   * this scans for the program in the given path,
   * adds the found bin object to the list and returnes true.
   * if nothing could be found false is returned.
   */
  virtual bool scan( const TQString& ) {return false;}//= 0;

  /**
   * reimplement this if it does not make sense to have the user be able
   * to specify additional parameters
   */
  virtual bool supportsUserParameters() const { return true; }

 private:
  TQString m_name;
  TQStringList m_userParameters;
  TQPtrList<K3bExternalBin> m_bins;
};


class LIBK3B_EXPORT K3bExternalBinManager : public TQObject
{
  TQ_OBJECT
  

 public:
  K3bExternalBinManager( TQObject* parent = 0, const char* name = 0 );
  ~K3bExternalBinManager();

  void search();

  /**
   * read config and add changes to current map.
   * Takes care of setting the config group
   */
  bool readConfig( TDEConfig* );

  /**
   * Takes care of setting the config group
   */
  bool saveConfig( TDEConfig* );

  bool foundBin( const TQString& name );
  const TQString& binPath( const TQString& name );
  const K3bExternalBin* binObject( const TQString& name );
  const K3bExternalBin* mostRecentBinObject( const TQString& name );

  K3bExternalProgram* program( const TQString& ) const;
  const TQMap<TQString, K3bExternalProgram*>& programs() const { return m_programs; }

  /** always extends the default searchpath */
  void setSearchPath( const TQStringList& );
  void addSearchPath( const TQString& );
  void loadDefaultSearchPath();

  const TQStringList& searchPath() const { return m_searchPath; }

  void addProgram( K3bExternalProgram* );
  void clear();

 private:
  TQMap<TQString, K3bExternalProgram*> m_programs;
  TQStringList m_searchPath;

  static TQString m_noPath;  // used for binPath() to return const string

  TQString m_gatheredOutput;
};

#endif
