/*
*
* $Id: k3bvcdtrack.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2004 Christian Kvasny <chris@k3b.org>
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

#ifndef K3BVCDTRACK_H
#define K3BVCDTRACK_H

// TQt Includes
#include <tqstring.h>
#include <tqfileinfo.h>
#include <tqfile.h>
#include <tqptrlist.h>

// Kde Includes
#include <kio/global.h>

// K3b Includes
#include "mpeginfo/k3bmpeginfo.h"
#include "k3b_export.h"
class LIBK3B_EXPORT K3bVcdTrack
{
    public:
        K3bVcdTrack( TQPtrList<K3bVcdTrack>* parent, const TQString& filename );
        ~K3bVcdTrack();

        TQString fileName() const
        {
            return TQFileInfo( m_file ).fileName();
        }
        TQString absPath() const
        {
            return TQFileInfo( m_file ).absFilePath();
        }
        KIO::filesize_t size() const;
        int index() const;

        const TQString& title() const
        {
            return m_title;
        }
        void setTitle( const TQString& t )
        {
            m_title = t;
        }
        bool isSegment()
        {
            return mpegType() == 1;
        };



        // PBC
        enum PbcTracks { PREVIOUS, NEXT, RETURN, DEFAULT, AFTERTIMEOUT, _maxPbcTracks };
        enum PbcTypes { DISABLED, VIDEOEND };

        void addToRevRefList( K3bVcdTrack* revreftrack );
        void delFromRevRefList( K3bVcdTrack* revreftrack );
        bool hasRevRef();
        void delRefToUs();
        void delRefFromUs();

        void setPbcTrack( int, K3bVcdTrack* pbctrack = 0L );
        void setPbcNonTrack( int, int );
        void setUserDefined( int, bool );
        void setPlayTime( int t )
        {
            m_pbcplaytime = t;
        }
        void setWaitTime( int t )
        {
            m_pbcwaittime = t;
        }
        void setReactivity( bool b )
        {
            m_reactivity = b;
        }
        void setPbcNumKeys( const bool& b )
        {
            m_pbcnumkeys = b;
        }
        bool PbcNumKeys() const
        {
            return m_pbcnumkeys;
        };
        void setPbcNumKeysUserdefined( const bool& b )
        {
            m_pbcnumkeysuserdefined = b;
        };
        bool PbcNumKeysUserdefined() const
        {
            return m_pbcnumkeysuserdefined;
        };

        K3bVcdTrack* getPbcTrack( const int& );
        int getNonPbcTrack( const int& );
        bool isPbcUserDefined( int );
        int getPlayTime()
        {
            return m_pbcplaytime;
        }
        int getWaitTime()
        {
            return m_pbcwaittime;
        }
        bool Reactivity()
        {
            return m_reactivity;
        }

        // Numeric keys
        void setDefinedNumKey( int key, K3bVcdTrack* track )
        {
            m_definedkeysmap.insert( key, track );
        }
        void delDefinedNumKey( int key )
        {
            m_definedkeysmap.remove( key );
        }
        void delDefinedNumKey()
        {
            m_definedkeysmap.clear();
        }
        TQMap<int, K3bVcdTrack*> DefinedNumKey()
        {
            return m_definedkeysmap;
        }

        // Mpeg Infos
        const TQString resolution();
        const TQString highresolution();
        const TQString video_frate();
        const TQString video_bitrate();
        const TQString audio_layer();
        const TQString audio_bitrate();
        const TQString audio_sampfreq();

        const TQString duration()
        {
            return SecsToHMS( mpeg_info->playing_time );
        };
        const int version()
        {
            return mpeg_info->version;
        };
        const unsigned long muxrate()
        {
            return mpeg_info->muxrate;
        };
        const TQString video_format( );
        const TQString video_chroma( );
        const TQString audio_mode( );
        const TQString audio_copyright( );
        const TQString mpegTypeS( bool audio = false );
        const int mpegType();

        void PrintInfo();

        Mpeginfo* mpeg_info;

    protected:

        const TQString audio_type2str( unsigned int , unsigned int, unsigned int );
        TQString SecsToHMS( double );

        TQPtrList<K3bVcdTrack>* m_parent;

        // PBC
        TQPtrList<K3bVcdTrack>* m_revreflist;          // List of Tracks which points to us
        TQMap<int, K3bVcdTrack*> m_pbctrackmap;        // Pbc Tracks (Previous, Next, ...)
        TQMap<int, int> m_pbcnontrackmap;              // Pbc NON Track types (Previous, Next, ...)
        TQMap<int, bool> m_pbcusrdefmap;               // Pbc is userdefined or defaults (Previous, Next, ...)
        TQMap<int, K3bVcdTrack*> m_definedkeysmap;

        bool m_pbcnumkeys;
        bool m_pbcnumkeysuserdefined;

        int m_pbcplaytime;
        int m_pbcwaittime;
        /********************************************************************************/

        bool m_reactivity;
        int m_filetype;
        TQFile m_file;
        TQString m_title;
};

#endif
