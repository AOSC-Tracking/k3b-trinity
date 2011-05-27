/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

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

#ifndef koZipStore_h
#define koZipStore_h

#include "koStoreBase.h"

class KZip;
class KArchiveDirectory;
class KURL;

class KoZipStore : public KoStoreBase
{
public:
    KoZipStore( const TQString & _filename, Mode _mode, const TQCString & appIdentification );
    KoZipStore( TQIODevice *dev, Mode mode, const TQCString & appIdentification );
    /**
     * KURL-constructor
     * @todo saving not completely implemented (fixed temporary file)
     * @since 1.4
     */
    KoZipStore( TQWidget* window, const KURL& _url, const TQString & _filename, Mode _mode, const TQCString & appIdentification );
    ~KoZipStore();

    virtual TQ_LONG write( const char* _data, TQ_ULONG _len );
protected:
    virtual bool init( Mode _mode, const TQCString& appIdentification );
    virtual bool openWrite( const TQString& name );
    virtual bool openRead( const TQString& name );
    virtual bool closeWrite();
    virtual bool closeRead() { return true; }
    virtual bool enterRelativeDirectory( const TQString& dirName );
    virtual bool enterAbsoluteDirectory( const TQString& path );
    virtual bool fileExists( const TQString& absPath ) const;

    /// The archive
    KZip * m_pZip;

    /** In "Read" mode this pointer is pointing to the
    current directory in the archive to speed up the verification process */
    const KArchiveDirectory* m_currentDir;
};

#endif
