//
/* This file is part of the KDE project
   Copyright 2004 Nicolas GOUTTE <goutte@kde.org>

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

#ifndef KOSTORE_BASE_H
#define KOSTORE_BASE_H

#include <kurl.h>

#include "koStore.h"

/**
 * Helper class for KoStore (mainly for remote file support)
 * @since 1.4
 */
class KoStoreBase : public KoStore
{
public:
    KoStoreBase(void);
    virtual ~KoStoreBase(void);
public:
    enum FileMode { /*Bad=0,*/ Local=1, RemoteRead, RemoteWrite };

protected:
    /**
     * original URL of the remote file
     * (undefined for a local file)
     */
    KURL m_url;
    FileMode m_fileMode;
    TQString m_localFileName;
    TQWidget* m_window;
};

#endif //KOSTORE_BASE_H
