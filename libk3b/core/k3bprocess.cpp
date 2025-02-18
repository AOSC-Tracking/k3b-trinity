/* 
 *
 * $Id: k3bprocess.cpp 619556 2007-01-03 17:38:12Z trueg $
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



#include "k3bprocess.h"
#include "k3bexternalbinmanager.h"

#include <tqstringlist.h>
#include <tqsocketnotifier.h>
#include <tqptrqueue.h>
#include <tqapplication.h>

#include <kdebug.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


class K3bProcess::Data
{
public:
  TQString unfinishedStdoutLine;
  TQString unfinishedStderrLine;

  int dupStdoutFd;
  int dupStdinFd;

  bool rawStdin;
  bool rawStdout;

  int in[2];
  int out[2];

  bool suppressEmptyLines;
};


K3bProcess::K3bProcess()
  : TDEProcess(),
    m_bSplitStdout(false)
{
  d = new Data();
  d->dupStdinFd = d->dupStdoutFd = -1;
  d->rawStdout = d->rawStdin = false;
  d->in[0] = d->in[1] = -1;
  d->out[0] = d->out[1] = -1;
  d->suppressEmptyLines = true;
}

K3bProcess::~K3bProcess()
{
  delete d;
}


K3bProcess& K3bProcess::operator<<( const K3bExternalBin* bin )
{
  return this->operator<<( bin->path );
}

K3bProcess& K3bProcess::operator<<( const TQString& arg )
{
  static_cast<TDEProcess*>(this)->operator<<( arg );
  return *this;
}

K3bProcess& K3bProcess::operator<<( const char* arg )
{
  static_cast<TDEProcess*>(this)->operator<<( arg );
  return *this;
}

K3bProcess& K3bProcess::operator<<( const TQCString& arg )
{
  static_cast<TDEProcess*>(this)->operator<<( arg );
  return *this;
}

K3bProcess& K3bProcess::operator<<( const TQStringList& args )
{
  static_cast<TDEProcess*>(this)->operator<<( args );
  return *this;
}


bool K3bProcess::start( RunMode run, Communication com )
{
  if( com & Stderr ) {
    connect( this, TQ_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
	     this, TQ_SLOT(slotSplitStderr(TDEProcess*, char*, int)) );
  }
  if( com & Stdout ) {
    connect( this, TQ_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
	     this, TQ_SLOT(slotSplitStdout(TDEProcess*, char*, int)) );
  }

  return TDEProcess::start( run, com );
}


void K3bProcess::slotSplitStdout( TDEProcess*, char* data, int len )
{
  if( m_bSplitStdout ) {
    TQStringList lines = splitOutput( data, len, d->unfinishedStdoutLine, d->suppressEmptyLines );

    for( TQStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
      TQString& str = *it;
      
      // just to be sure since something in splitOutput does not do this right
      if( d->suppressEmptyLines && str.isEmpty() )
	continue;

      emit stdoutLine( str );
    }
  }
}


void K3bProcess::slotSplitStderr( TDEProcess*, char* data, int len )
{
  TQStringList lines = splitOutput( data, len, d->unfinishedStderrLine, d->suppressEmptyLines );

  for( TQStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
    TQString& str = *it;

    // just to be sure since something in splitOutput does not do this right
    if( d->suppressEmptyLines && str.isEmpty() )
      continue;

    emit stderrLine( str );
  }
}


TQStringList K3bProcess::splitOutput( char* data, int len, 
				     TQString& unfinishedLine, bool suppressEmptyLines )
{
  //
  // The stderr splitting is mainly used for parsing of messages
  // That's why we simplify the data before proceeding
  //

  TQString buffer;
  for( int i = 0; i < len; i++ ) {
    if( data[i] == '\b' ) {
      while( data[i] == '\b' )  // we replace multiple backspaces with a single line feed
	i++;
      buffer += '\n';
    }
    if( data[i] == '\r' )
      buffer += '\n';
    else if( data[i] == '\t' )  // replace tabs with a single space
      buffer += " ";
    else
      buffer += data[i];
  }

  TQStringList lines = TQStringList::split( '\n', buffer, !suppressEmptyLines );

  // in case we suppress empty lines we need to handle the following separately
  // to make sure we join unfinished lines correctly
  if( suppressEmptyLines && buffer[0] == '\n' )
    lines.prepend( TQString() );

  if( !unfinishedLine.isEmpty() ) {
    lines.first().prepend( unfinishedLine );
    unfinishedLine.truncate(0);

    kdDebug() << "(K3bProcess)           joined line: '" << (lines.first()) << "'" << endl;
  }

  TQStringList::iterator it;

  // check if line ends with a newline
  // if not save the last line because it is not finished
  TQChar c = buffer.right(1).at(0);
  bool hasUnfinishedLine = ( c != '\n' && c != '\r' && c != TQChar(46) );  // What is unicode 46?? It is printed as a point
  if( hasUnfinishedLine ) {
    kdDebug() << "(K3bProcess) found unfinished line: '" << lines.last() << "'" << endl;
    kdDebug() << "(K3bProcess)             last char: '" << buffer.right(1) << "'" << endl;
    unfinishedLine = lines.last();
    it = lines.end();
    --it;
    lines.remove(it);
  }

  return lines;
}


int K3bProcess::setupCommunication( Communication comm )
{
  if( TDEProcess::setupCommunication( comm ) ) {

    //
    // Setup our own socketpair
    //

    if( d->rawStdin ) {
      if( socketpair(AF_UNIX, SOCK_STREAM, 0, d->in) == 0 ) {
	fcntl(d->in[0], F_SETFD, FD_CLOEXEC);
	fcntl(d->in[1], F_SETFD, FD_CLOEXEC);
      }
      else
	return 0;
    }

    if( d->rawStdout ) {
      if( socketpair(AF_UNIX, SOCK_STREAM, 0, d->out) == 0 ) {
	fcntl(d->out[0], F_SETFD, FD_CLOEXEC);
	fcntl(d->out[1], F_SETFD, FD_CLOEXEC);
      }
      else {
	if( d->rawStdin || d->dupStdinFd ) {
	  close(d->in[0]);
	  close(d->in[1]);
	}
	return 0;
      }
    }

    return 1;
  }
  else
    return 0;
}


void K3bProcess::commClose()
{
  if( d->rawStdin ) {
    close(d->in[1]);
    d->in[1] = -1;
  }
  if( d->rawStdout ) {
    close(d->out[0]);
    d->out[0] = -1;
  }

  TDEProcess::commClose();
}


int K3bProcess::commSetupDoneP()
{
  int ok = TDEProcess::commSetupDoneP();

  if( d->rawStdin )
    close(d->in[0]);
  if( d->rawStdout )
    close(d->out[1]);

  d->in[0] = d->out[1] = -1;

  return ok;
}


int K3bProcess::commSetupDoneC()
{
  int ok = TDEProcess::commSetupDoneC();

  if( d->dupStdoutFd != -1 ) {
    //
    // make STDOUT_FILENO a duplicate of d->dupStdoutFd such that writes to STDOUT_FILENO are "redirected"
    // to d->dupStdoutFd
    //
    if( ::dup2( d->dupStdoutFd, STDOUT_FILENO ) < 0 ) {
      kdDebug() << "(K3bProcess) Error while dup( " << d->dupStdoutFd << ", " << STDOUT_FILENO << endl;
      ok = 0;
    }
  }
  else if( d->rawStdout ) {
    if( ::dup2( d->out[1], STDOUT_FILENO ) < 0 ) {
      kdDebug() << "(K3bProcess) Error while dup( " << d->out[1] << ", " << STDOUT_FILENO << endl;
      ok = 0;
    }
  }

  if( d->dupStdinFd != -1 ) {
    if( ::dup2( d->dupStdinFd, STDIN_FILENO ) < 0 ) {
      kdDebug() << "(K3bProcess) Error while dup( " << d->dupStdinFd << ", " << STDIN_FILENO << endl;
      ok = 0;
    }
  }
  else if( d->rawStdin ) {
    if( ::dup2( d->in[0], STDIN_FILENO ) < 0 ) {
      kdDebug() << "(K3bProcess) Error while dup( " << d->in[0] << ", " << STDIN_FILENO << endl;
      ok = 0;
    }
  }

  return ok;
}



int K3bProcess::stdinFd() const
{
  if( d->rawStdin )
    return d->in[1];
  else if( d->dupStdinFd != -1 )
    return d->dupStdinFd;
  else
    return -1;
}

int K3bProcess::stdoutFd() const
{
  if( d->rawStdout )
    return d->out[0];
  else if( d->dupStdoutFd != -1 )
    return d->dupStdoutFd;
  else
    return -1;
}


void K3bProcess::dupStdout( int fd )
{
  writeToFd( fd );
}

void K3bProcess::dupStdin( int fd )
{
  readFromFd( fd );
}


void K3bProcess::writeToFd( int fd )
{
  d->dupStdoutFd = fd;
  if( fd != -1 )
    d->rawStdout = false;
}

void K3bProcess::readFromFd( int fd )
{
  d->dupStdinFd = fd;
  if( fd != -1 )
    d->rawStdin = false;
}


void K3bProcess::setRawStdin(bool b)
{
  if( b ) {
    d->rawStdin = true;
    d->dupStdinFd = -1;
  }
  else
    d->rawStdin = false;
}


void K3bProcess::setRawStdout(bool b)
{
  if( b ) {
    d->rawStdout = true;
    d->dupStdoutFd = -1;
  }
  else
    d->rawStdout = false;
}


void K3bProcess::setSuppressEmptyLines( bool b )
{
  d->suppressEmptyLines = b;
}


bool K3bProcess::closeStdin()
{
  if( d->rawStdin ) {
    close(d->in[1]);
    d->in[1] = -1;
    return true;
  }
  else
    return TDEProcess::closeStdin();
}


bool K3bProcess::closeStdout()
{
  if( d->rawStdout ) {
    close(d->out[0]);
    d->out[0] = -1;
    return true;
  }
  else
    return TDEProcess::closeStdout();
}


K3bProcessOutputCollector::K3bProcessOutputCollector( TDEProcess* p )
  : m_process(0)
{
  setProcess( p );
}

void K3bProcessOutputCollector::setProcess( TDEProcess* p )
{
  if( m_process )
    m_process->disconnect( this );

  m_process = p;
  if( p ) {
    connect( p, TQ_SIGNAL(receivedStdout(TDEProcess*, char*, int)), 
	     this, TQ_SLOT(slotGatherStdout(TDEProcess*, char*, int)) );
    connect( p, TQ_SIGNAL(receivedStderr(TDEProcess*, char*, int)), 
	     this, TQ_SLOT(slotGatherStderr(TDEProcess*, char*, int)) );
  }

  m_gatheredOutput.truncate( 0 );
  m_stderrOutput.truncate( 0 );
  m_stdoutOutput.truncate( 0 );
}

void K3bProcessOutputCollector::slotGatherStderr( TDEProcess*, char* data, int len )
{
  m_gatheredOutput.append( TQString::fromLocal8Bit( data, len ) );
  m_stderrOutput.append( TQString::fromLocal8Bit( data, len ) );
}

void K3bProcessOutputCollector::slotGatherStdout( TDEProcess*, char* data, int len )
{
  m_gatheredOutput.append( TQString::fromLocal8Bit( data, len ) );
  m_stdoutOutput.append( TQString::fromLocal8Bit( data, len ) );
}


#include "k3bprocess.moc"
