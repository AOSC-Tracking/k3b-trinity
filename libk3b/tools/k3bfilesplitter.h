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

#ifndef _K3B_FILE_SPLITTER_H_
#define _K3B_FILE_SPLITTER_H_

#include <tqiodevice.h>
#include <tqstring.h>

#include <kio/global.h>

#include <k3b_export.h>


/**
 * TQFile replacement which splits
 * big files according to the underlying file system's
 * maximum file size.
 *
 * The filename will be changed to include a counter
 * if the file has to be splitted like so:
 *
 * <pre>
 * filename.iso
 * filename.iso.001
 * filename.iso.002
 * ...
 * </pre>
 */
class LIBK3B_EXPORT K3bFileSplitter : public TQIODevice
{
 public:
  K3bFileSplitter();
  K3bFileSplitter( const TQString& filename );
  ~K3bFileSplitter();

  /**
   * Set the maximum file size. If this is set to 0
   * (the default) the max filesize is determined based on 
   * the filesystem type.
   *
   * Be aware that setName will reset the max file size.
   */
  void setMaxFileSize( TDEIO::filesize_t size );

  const TQString& name() const;

  void setName( const TQString& filename );

  virtual bool open( int mode );

  virtual void close();

#ifdef USE_QT4
	inline qint64 readData ( char * data, qint64 maxSize ) { return readBlock(data, maxSize); }
	inline qint64 writeData ( const char * data, qint64 maxSize ) { return writeBlock(data, maxSize); }
#endif // USE_QT4

  /**
   * File descriptor to read from and write to.
   * Not implemented yet!
   */
  int handle() const;

  virtual void flush();

  /**
   * Not implemented
   */
#ifdef USE_QT4
  virtual qint64 size() const;
#else // USE_QT4
  virtual Offset size() const;
#endif // USE_QT4

  /**
   * Not implemented
   */
  virtual Offset at() const;

  /**
   * Not implemented
   */
  virtual bool at( Offset );

  virtual bool atEnd() const;
  virtual TQ_LONG readBlock( char *data, TQ_ULONG maxlen );
  virtual TQ_LONG writeBlock( const char *data, TQ_ULONG len );
  virtual int getch();
  virtual int putch( int );
  virtual int ungetch( int );

  /**
   * Deletes all the splitted files.
   * Caution: Does remove all files that fit the naming scheme without any 
   * additional checks.
   */
  void remove();

 private:
  class Private;
  Private* d;
};

#endif
