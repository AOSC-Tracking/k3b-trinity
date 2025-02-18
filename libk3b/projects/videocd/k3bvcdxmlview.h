/*
*
* $Id: k3bvcdxmlview.h 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2004 Christian Kvasny <chris@k3b.org>
*             THX to Manfred Odenstein <odix@chello.at>
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

#ifndef K3B_VCD_XMLVIEW_H
#define K3B_VCD_XMLVIEW_H

#include <tqstring.h>
#include <tqdom.h>
#include <tdetempfile.h>

#include <k3bvcddoc.h>

class K3bVcdOptions;
class K3bVcdTrack;


class K3bVcdXmlView
{

    public:
        K3bVcdXmlView( K3bVcdDoc* );
        ~K3bVcdXmlView();

        bool write( const TQString& );
        TQString xmlString()
        {
            return m_xmlstring;
        }

    private:
        TQString m_xmlstring;

        void addComment( TQDomDocument& doc, TQDomElement& parent, const TQString& text );
        TQDomElement addSubElement( TQDomDocument&, TQDomElement&, const TQString& name, const TQString& value = TQString() );
        TQDomElement addSubElement( TQDomDocument&, TQDomElement&, const TQString& name, const int& value );

        TQDomElement addFolderElement( TQDomDocument&, TQDomElement&, const TQString& name );
        void addFileElement( TQDomDocument&, TQDomElement&, const TQString& src, const TQString& name, bool mixed = false );
        void doPbc( TQDomDocument&, TQDomElement&, K3bVcdTrack* );
        void setNumkeyBSN( TQDomDocument& , TQDomElement&, K3bVcdTrack* );
        void setNumkeySEL( TQDomDocument& , TQDomElement&, K3bVcdTrack* );
        K3bVcdDoc* m_doc;
        int m_startkey;
};

#endif
