/* 
 *
 * $Id: k3bdiritem.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDIRITEM_H
#define K3BDIRITEM_H


#include <tqstring.h>
#include <tqptrlist.h>

#include <tdeio/global.h>

#include "k3bdataitem.h"
#include "k3b_export.h"
class K3bDataDoc;

/**
 *@author Sebastian Trueg
 */

class LIBK3B_EXPORT K3bDirItem : public K3bDataItem
{
 public: 
  K3bDirItem( const TQString& name, K3bDataDoc*, K3bDirItem* parentDir = 0 );

  /**
   * Default copy constructor. Copies the dir including all children. However, none of the
   * children will have set a doc and the copy dir will not have set a parent dir.
   */
  K3bDirItem( const K3bDirItem& );

  virtual ~K3bDirItem();

  K3bDataItem* copy() const;
	
  K3bDirItem* getDirItem() const;

  const TQPtrList<K3bDataItem>& children() const { return m_children; }
  K3bDirItem* addDataItem( K3bDataItem* item );
  K3bDataItem* takeDataItem( K3bDataItem* item );
	
  K3bDataItem* nextSibling() const;
  K3bDataItem* nextChild( K3bDataItem* ) const;

  bool alreadyInDirectory( const TQString& fileName ) const;
  K3bDataItem* find( const TQString& filename ) const;
  K3bDataItem* findByPath( const TQString& );

  long numFiles() const;
  long numDirs() const;

  bool isEmpty() const { return ( numDirs() + numFiles() == 0 ); }

  /**
   * returns true if item is a subItem of 
   * this dir item 
   * (returns also true if item == this
   */
  bool isSubItem( K3bDataItem* item ) const;

  bool isDir() const { return true; }

  virtual bool isRemoveable() const;

  /**
   * \return true if some child is from an old session.
   */
  virtual bool isFromOldSession() const;

  /**
   * Recursively creates a directory.
   */
  bool mkdir( const TQString& dir );

  void setLocalPath( const TQString& p ) { m_localPath = p; }
  TQString localPath() const { return m_localPath; }

  /**
   * \reimplemented
   */
  bool writeToCd() const;

 protected:
  /**
   * Normally one does not use this method but K3bDataItem::size()
   *
   * This method does not take into account the possibility to share the data
   * between files with the same inode in an iso9660 filesystem.
   * For that one has to use K3bFileCompilationSizeHandler.
   */
  TDEIO::filesize_t itemSize( bool followSymlinks ) const;

  /*
   * Normally one does not use this method but K3bDataItem::blocks()
   */
  K3b::Msf itemBlocks( bool followSymlinks ) const;

 private:
  /**
   * this recursivly updates the size of the directories.
   * The size of this dir and the parent dir is updated.
   * These values are just used for user information.
   */
  void updateSize( K3bDataItem*, bool removed = false );
  /**
   * Updates the number of files and directories. These values are
   * just used for user information.
   */
  void updateFiles( long files, long dirs );

  mutable TQPtrList<K3bDataItem> m_children;

  // size of the items simply added
  TDEIO::filesize_t m_size;
  TDEIO::filesize_t m_followSymlinksSize;

  // number of blocks (2048 bytes) used by all the items
  long m_blocks;
  long m_followSymlinksBlocks;

  long m_files;
  long m_dirs;

  // HACK: store the original path to be able to use it's permissions
  //        �remove this once we have a backup project
  TQString m_localPath;
};


class K3bRootItem : public K3bDirItem
{
 public:
  K3bRootItem( K3bDataDoc* );
  ~K3bRootItem();

  const TQString& k3bName() const;
  void setK3bName( const TQString& );

  bool isMoveable() const { return false; }
  bool isRemoveable() const { return false; }
};
#endif
