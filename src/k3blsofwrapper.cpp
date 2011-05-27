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

#include "k3blsofwrapper.h"

#include <k3bdevice.h>
#include <k3bprocess.h>
#include <k3bglobals.h>

#include <tqfile.h>
#include <tqfileinfo.h>

#include <sys/types.h>
#include <unistd.h>

static K3bLsofWrapper::Process createProcess( const TQString& name, int pid )
{
  K3bLsofWrapper::Process p;
  p.name = name;
  p.pid = pid;
  return p;
}


class K3bLsofWrapper::Private
{
public:
  TQValueList<Process> apps;
  TQString lsofBin;
};


K3bLsofWrapper::K3bLsofWrapper()
{
  d = new Private;
}


K3bLsofWrapper::~K3bLsofWrapper()
{
  delete d;
}


bool K3bLsofWrapper::checkDevice( K3bDevice::Device* dev )
{
  d->apps.clear();

  if( !findLsofExecutable() )
    return false;

  // run lsof
  KProcess p;
  K3bProcessOutputCollector out( &p );

  //
  // We use the following output form: 
  // p<PID>
  // c<COMMAND_NAME>
  //
  p << d->lsofBin << "-Fpc" << dev->blockDeviceName();

  if( !p.start( KProcess::Block, KProcess::Stdout ) )
    return false;

  //
  // now process its output
  TQStringList l = TQStringList::split( "\n", out.output() );
  for( TQStringList::iterator it = l.begin(); it != l.end(); ++it ) {
    int pid = (*it).mid(1).toInt();
    TQString app = (*(++it)).mid(1);

    kdDebug() << "(K3bLsofWrapper) matched: app: " << app << " pid: " << pid << endl;

    // we don't care about ourselves using the device ;)
    if( pid != (int)::getpid() )
      d->apps.append( createProcess( app, pid ) );
  }

  return true;
}


const TQValueList<K3bLsofWrapper::Process>& K3bLsofWrapper::usingApplications() const
{
  return d->apps;
}


bool K3bLsofWrapper::findLsofExecutable()
{
  if( d->lsofBin.isEmpty() )
    d->lsofBin = K3b::findExe( "lsof" );

  return !d->lsofBin.isEmpty();
}
