/*
*
* $Id: k3bvcddoc.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3BVCDDOC_H
#define K3BVCDDOC_H

// TQt includes
#include <tqptrqueue.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdatetime.h>
#include <tqtextstream.h>
#include <tqimage.h>

// Kde Includes
#include <kurl.h>

// K3b Includes
#include "k3bvcdoptions.h"
#include "mpeginfo/k3bmpeginfo.h"
#include <k3bdoc.h>
#include "k3b_export.h"
class K3bApp;
class K3bVcdTrack;
class K3bVcdJob;
//class K3bView;
class TQWidget;
class TQTimer;
class TQDomDocument;
class TQDomElement;
class TDEConfig;



class LIBK3B_EXPORT K3bVcdDoc : public K3bDoc
{
        TQ_OBJECT
  

    public:
        K3bVcdDoc( TQObject* );
        ~K3bVcdDoc();

	int type() const { return VCD; }

	TQString name() const;

        enum vcdTypes { VCD11, VCD20, SVCD10, HTQVCD, NONE};

        bool newDocument();
        int numOfTracks() const
        {
            return m_tracks->count();
        }

        const TQString& vcdImage() const
        {
            return m_vcdImage;
        }
        void setVcdImage( const TQString& s )
        {
            m_vcdImage = s;
        }

        K3bVcdTrack* first()
        {
            return m_tracks->first();
        }
        K3bVcdTrack* current() const
        {
            return m_tracks->current();
        }
        K3bVcdTrack* next()
        {
            return m_tracks->next();
        }
        K3bVcdTrack* prev()
        {
            return m_tracks->prev();
        }
        K3bVcdTrack* at( uint i )
        {
            return m_tracks->at( i );
        }
        K3bVcdTrack* take( uint i )
        {
            return m_tracks->take( i );
        }

        const TQPtrList<K3bVcdTrack>* tracks() const
        {
            return m_tracks;
        }

        /** get the current size of the project */
        TDEIO::filesize_t size() const;
        K3b::Msf length() const;

        K3bBurnJob* newBurnJob( K3bJobHandler* hdl, TQObject* parent );
        K3bVcdOptions* vcdOptions() const
        {
            return m_vcdOptions;
        }

        int vcdType() const
        {
            return m_vcdType;
        }
        void setVcdType( int type );
        void setPbcTracks();

    public slots:
        /**
         * will test the file and add it to the project.
         * connect to at least result() to know when
         * the process is finished and check error()
         * to know about the result.
         **/
        void addUrls( const KURL::List& );
        void addTrack( const KURL&, uint );
        void addTracks( const KURL::List&, uint );
        /** adds a track without any testing */
        void addTrack( K3bVcdTrack* track, uint position = 0 );

        // --- TODO: this should read: removeTrack( K3bVcdTrack* )
        void removeTrack( K3bVcdTrack* );
        void moveTrack( const K3bVcdTrack* track, const K3bVcdTrack* after );

    protected slots:
        /** processes queue "urlsToAdd" **/
        void slotWorkUrlQueue();

    signals:
        void newTracks();

        void trackRemoved( K3bVcdTrack* );

    protected:
        /** reimplemented from K3bDoc */
        bool loadDocumentData( TQDomElement* root );
        /** reimplemented from K3bDoc */
        bool saveDocumentData( TQDomElement* );

        TQString typeString() const;

    private:
        K3bVcdTrack* createTrack( const KURL& url );
        void informAboutNotFoundFiles();

        TQStringList m_notFoundFiles;
        TQString m_vcdImage;

        class PrivateUrlToAdd
        {
            public:
                PrivateUrlToAdd( const KURL& u, int _pos )
                        : url( u ), position( _pos )
                {}
                KURL url;
                int position;
        };

        /** Holds all the urls that have to be added to the list of tracks. **/
        TQPtrQueue<PrivateUrlToAdd> urlsToAdd;
        TQTimer* m_urlAddingTimer;

        TQPtrList<K3bVcdTrack>* m_tracks;
        TDEIO::filesize_t calcTotalSize() const;
        TDEIO::filesize_t ISOsize() const;

        bool isImage( const KURL& url );

        K3bVcdTrack* m_lastAddedTrack;
        K3bVcdOptions* m_vcdOptions;

        int m_vcdType;
        uint lastAddedPosition;
};

#endif
