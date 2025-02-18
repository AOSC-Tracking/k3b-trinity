/* This file is part of the KDE project
   Copyright (C) 1998, 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __koStore_h_
#define __koStore_h_

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqiodevice.h>
#include <tqvaluestack.h>
//#include <koffice_export.h>
#include <tdemacros.h>

#define KOSTORE_EXPORT TDE_EXPORT

class TQWidget;

class KURL;

/**
 * Saves and loads KOffice documents using various backends. Currently supported
 * backends are ZIP, tar and directory.
 * We call a "store" the file on the hard disk (the one the users sees)
 * and call a "file" a file inside the store.
 */
class KoStore
{
public:

  enum Mode { Read, Write };
  enum Backend { Auto, Tar, Zip, Directory };

  /**
   * Open a store (i.e. the representation on disk of a KOffice document).
   *
   * @param fileName the name of the file to open
   * @param mode if KoStore::Read, open an existing store to read it.
   *             if KoStore::Write, create or replace a store.
   * @param backend the backend to use for the data storage.
   * Auto means automatically-determined for reading,
   * and the current format (now Zip) for writing.
   *
   * @param appIdentification the application's mimetype,
   * to be written in the file for "mime-magic" identification.
   * Only meaningful if mode is Write, and if backend!=Directory.
   */
  static KoStore* createStore( const TQString& fileName, Mode mode, const TQCString & appIdentification = "", Backend backend = Auto );

  /**
   * Create a store for any kind of TQIODevice: file, memory buffer...
   * KoStore will take care of opening the TQIODevice.
   * This method doesn't support the Directory store!
   */
  static KoStore* createStore( TQIODevice *device, Mode mode, const TQCString & appIdentification = "", Backend backend = Auto );

  /**
   * Open a store (i.e. the representation on disk of a KOffice document).
   *
   * @param window associated window (for the progress bar dialog and authentication)
   * @param url URL of the file to open
   * @param mode if KoStore::Read, open an existing store to read it.
   *             if KoStore::Write, create or replace a store.
   * @param backend the backend to use for the data storage.
   * Auto means automatically-determined for reading,
   * and the current format (now Zip) for writing.
   *
   * @param appIdentification the application's mimetype,
   * to be written in the file for "mime-magic" identification.
   * Only meaningful if mode is Write, and if backend!=Directory.
   *
   * If the file is remote, the backend Directory cannot be used!
   *
   * @since 1.4
   * @bug saving not completely implemented (fixed temporary file)
   */
  static KoStore* createStore( TQWidget* window, const KURL& url, Mode mode, const TQCString & appIdentification = "", Backend backend = Auto );

  /**
   * Destroys the store (i.e. closes the file on the hard disk)
   */
  virtual ~KoStore();

  /**
   * Open a new file inside the store
   * @param name The filename, internal representation ("root", "tar:/0"... ).
   *        If the tar:/ prefix is missing it's assumed to be a relative URI.
   * @return true on success.
   */
  bool open( const TQString & name );

  /**
   * Check whether a file inside the store is currently opened with open(),
   * ready to be read or written.
   * @return true if a file is currently opened.
   */
  bool isOpen() const;

  /**
   * Close the file inside the store
   * @return true on success.
   */
  bool close();

  /**
   * Get a device for reading a file from the store directly
   * (slightly faster than read() calls)
   * You need to call @ref open first, and @ref close afterwards.
   */
  TQIODevice* device() const;

  /**
   * Read data from the currently opened file. You can also use the streams
   * for this.
   */
  TQByteArray read( unsigned long int max );

  /**
   * Write data into the currently opened file. You can also use the streams
   * for this.
   */
  TQ_LONG write( const TQByteArray& _data );

  /**
   * Read data from the currently opened file. You can also use the streams
   * for this.
   * @return size of data read, -1 on error
   */
  TQ_LONG read( char *_buffer, TQ_ULONG _len );

  /**
   * Write data into the currently opened file. You can also use the streams
   * for this.
   */
  virtual TQ_LONG write( const char* _data, TQ_ULONG _len );

  /**
   * @return the size of the currently opened file, -1 on error.
   * Can be used as an argument for the read methods, for instance
   */
  TQIODevice::Offset size() const;

  /**
   * @return true if an error occurred
   */
  bool bad() const { return !m_bGood; } // :)

  /**
   * @return the mode used when opening, read or write
   */
  Mode mode() const { return m_mode; }

  /**
   * Enters one or multiple directories. In Read mode this actually
   * checks whether the specified directories exist and returns false
   * if they don't. In Write mode we don't create the directory, we
   * just use the "current directory" to generate the absolute path
   * if you pass a relative path (one not starting with tar:/) when
   * opening a stream.
   * Note: Operates on internal names
   */
  bool enterDirectory( const TQString& directory );

  /**
   * Leaves a directory. Equivalent to "cd .."
   * @return true on success, false if we were at the root already to
   * make it possible to "loop to the root"
   */
  bool leaveDirectory();

  /**
   * Returns the current path including a trailing slash.
   * Note: Returns a path in "internal name" style
   */
  TQString currentPath() const;

  /**
   * Returns the current directory.
   * Note: Returns a path in "internal name" style
   */
  TQString currentDirectory() const;


  /**
   * Stacks the current directory. Restore the current path using
   * @ref popDirectory .
   */
  void pushDirectory();

  /**
   * Restores the previously pushed directory. No-op if the stack is
   * empty.
   */
  void popDirectory();

  /**
   * @return true if the given file exists in the current directory,
   * i.e. if open(fileName) will work.
   */
  bool hasFile( const TQString& fileName ) const;

  /**
   * Imports a local file into a store
   * @param fileName file on hard disk
   * @param destName file in the store
   */
  bool addLocalFile( const TQString &fileName, const TQString &destName );

  /**
   * Imports a local directory
   * @param dirPath path to the directory on a disk
   * @param dest path in the store where the directory should get saved
   * @return the directory index
   */
  TQStringList addLocalDirectory( const TQString &dirPath, const TQString &dest );


  /**
   * Extracts a file out of the store
   * @param srcName file in the store
   * @param fileName file on a disk
   */
  bool extractFile( const TQString &srcName, const TQString &fileName );

  //@{
  /// See TQIODevice
  bool at( TQIODevice::Offset pos );
  TQIODevice::Offset at() const;
  bool atEnd() const;
  //@}

  /**
   * Do not expand file and directory names
   * Useful when using KoStore on non-KOffice files.
   * (This method should be called just after the constructor)
   */
  void disallowNameExpansion( void );

protected:

  KoStore() {}

  /**
   * Init store - called by constructor.
   * @return true on success
   */
  virtual bool init( Mode mode );
  /**
   * Open the file @p name in the store, for writing
   * On success, this method must set m_stream to a stream in which we can write.
   * @param name "absolute path" (in the archive) to the file to open
   * @return true on success
   */
  virtual bool openWrite( const TQString& name ) = 0;
  /**
   * Open the file @p name in the store, for reading.
   * On success, this method must set m_stream to a stream from which we can read,
   * as well as setting m_iSize to the size of the file.
   * @param name "absolute path" (in the archive) to the file to open
   * @return true on success
   */
  virtual bool openRead( const TQString& name ) = 0;

  /**
   * @return true on success
   */
  virtual bool closeRead() = 0;
  /**
   * @return true on success
   */
  virtual bool closeWrite() = 0;

  /**
   * Enter a subdirectory of the current directory.
   * The directory might not exist yet in Write mode.
   */
  virtual bool enterRelativeDirectory( const TQString& dirName ) = 0;
  /**
   * Enter a directory where we've been before.
   * It is guaranteed to always exist.
   */
  virtual bool enterAbsoluteDirectory( const TQString& path ) = 0;

  /**
   * Check if a file exists inside the store.
   * @param absPath the absolute path inside the store, i.e. not relative to the current directory
   */
  virtual bool fileExists( const TQString& absPath ) const = 0;

private:
  static Backend determineBackend( TQIODevice* dev );

  /**
   * Conversion routine
   * @param _internalNaming name used internally : "root", "tar:/0", ...
   * @return the name used in the file, more user-friendly ("maindoc.xml",
   *         "part0/maindoc.xml", ...)
   * Examples:
   *
   * tar:/0 is saved as part0/maindoc.xml
   * tar:/0/1 is saved as part0/part1/maindoc.xml
   * tar:/0/1/pictures/picture0.png is saved as part0/part1/pictures/picture0.png
   *
   * see specification (koffice/lib/store/SPEC) for details.
   */
  TQString toExternalNaming( const TQString & _internalNaming ) const;

  /**
   *  Expands a full path name for a stream (directories+filename)
   */
  TQString expandEncodedPath( TQString intern ) const;

  /**
   * Expands only directory names(!)
   * Needed for the path handling code, as we only operate on internal names
   */
  TQString expandEncodedDirectory( TQString intern ) const;

  mutable enum
  {
      NAMING_VERSION_2_1,
      NAMING_VERSION_2_2,
      NAMING_VERSION_RAW  ///< Never expand file and directory names
  } m_namingVersion;

  /**
   * Enter *one* single directory. Nothing like foo/bar/bleh allowed.
   * Performs some checking when in Read mode
   */
  bool enterDirectoryInternal( const TQString& directory );

protected:

  Mode m_mode;

  /// Store the filenames (with full path inside the archive) when writing, to avoid duplicates
  TQStringList m_strFiles;

  /// The "current directory" (path)
  TQStringList m_currentPath;

  /// Used to push/pop directories to make it easy to save/restore the state
  TQValueStack<TQString> m_directoryStack;

  /// Current filename (between an open() and a close())
  TQString m_sName;
  /// Current size of the file named m_sName
  TQIODevice::Offset m_iSize;

  /// The stream for the current read or write operation
  TQIODevice * m_stream;

  bool m_bIsOpen;
  /// Must be set by the constructor.
  bool m_bGood;

  static const int s_area;

private:
  KoStore( const KoStore& store );  ///< don't copy
  KoStore& operator=( const KoStore& store );  ///< don't assign

  class Private;
  Private * d;

};

#endif
