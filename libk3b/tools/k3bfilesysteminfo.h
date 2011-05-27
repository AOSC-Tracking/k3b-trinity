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

#ifndef _K3B_FILE_SYSTEM_INFO_H_
#define _K3B_FILE_SYSTEM_INFO_H_

#include <k3b_export.h>

#include <tqstring.h>

class LIBK3B_EXPORT K3bFileSystemInfo
{
 public:
  K3bFileSystemInfo();
  K3bFileSystemInfo( const TQString& path );
  K3bFileSystemInfo( const K3bFileSystemInfo& );
  ~K3bFileSystemInfo();

  TQString path() const;
  void setPath( const TQString& path );

  enum FileSystemType {
    FS_UNKNOWN,
    FS_FAT
    // FIXME: add way more file system types
  };

  FileSystemType type() const;

  /**
   * Ensures that the file path does not contain
   * any invalid chars.
   *
   * For now it only replaces characters like * or [
   * on FAT file systems.
   */
  TQString fixupPath( const TQString& );

 private:
  class Private;
  Private* d;
};

#endif
