/*
*
* $Id: k3bvcdxmlview.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <tqfile.h>

#include <kstandarddirs.h>
#include <kdebug.h>


#include "k3bvcdxmlview.h"
#include "k3bvcdtrack.h"
#include <k3bcore.h>
#include <k3bversion.h>

K3bVcdXmlView::K3bVcdXmlView( K3bVcdDoc* pDoc )
{

    m_doc = pDoc;

}

K3bVcdXmlView::~K3bVcdXmlView()
{}

bool K3bVcdXmlView::write( const TQString& fname )
{

    TQDomDocument xmlDoc( "videocd PUBLIC \"-//GNU//DTD VideoCD//EN\" \"http://www.gnu.org/software/vcdimager/videocd.dtd\"" );
    // xmlDoc.appendChild( xmlDoc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"iso-8859-1\"" ) );
    xmlDoc.appendChild( xmlDoc.createProcessingInstruction( "xml", "version=\"1.0\"" ) );

    // create root element
    TQDomElement root = xmlDoc.createElement( "videocd" );
    root.setAttribute( "xmlns", "http://www.gnu.org/software/vcdimager/1.0/" );
    root.setAttribute( "class", m_doc->vcdOptions() ->vcdClass() );
    root.setAttribute( "version", m_doc->vcdOptions() ->vcdVersion() );
    xmlDoc.appendChild( root );

    // create option elements

    // Broken SVCD mode - NonCompliantMode
    if ( m_doc->vcdOptions() ->NonCompliantMode() ) {
        TQDomElement elemOption;
        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "svcd vcd30 mpegav" );
        elemOption.setAttribute( "value", "true" );

        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "svcd vcd30 entrysvd" );
        elemOption.setAttribute( "value", "true" );
    }

    // VCD3.0 track interpretation
    if ( m_doc->vcdOptions() ->VCD30interpretation() ) {
        TQDomElement elemOption;
        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "svcd vcd30 tracksvd" );
        elemOption.setAttribute( "value", "true" );
    }

    // Relaxed aps
    if ( m_doc->vcdOptions() ->RelaxedAps() ) {
        TQDomElement elemOption;
        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "relaxed aps" );
        elemOption.setAttribute( "value", "true" );
    }

    // Update scan offsets
    if ( m_doc->vcdOptions() ->UpdateScanOffsets() ) {
        TQDomElement elemOption;
        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "update scan offsets" );
        elemOption.setAttribute( "value", "true" );

    }

    // Gaps & Margins
    if ( m_doc->vcdOptions() ->UseGaps() ) {
        TQDomElement elemOption;
        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "leadout pregap" );
        elemOption.setAttribute( "value", m_doc->vcdOptions() ->PreGapLeadout() );

        elemOption = addSubElement( xmlDoc, root, "option" );
        elemOption.setAttribute( "name", "track pregap" );
        elemOption.setAttribute( "value", m_doc->vcdOptions() ->PreGapTrack() );

        if ( m_doc->vcdOptions() ->vcdClass() == "vcd" ) {
            elemOption = addSubElement( xmlDoc, root, "option" );
            elemOption.setAttribute( "name", "track front margin" );
            elemOption.setAttribute( "value", m_doc->vcdOptions() ->FrontMarginTrack() );

            elemOption = addSubElement( xmlDoc, root, "option" );
            elemOption.setAttribute( "name", "track rear margin" );
            elemOption.setAttribute( "value", m_doc->vcdOptions() ->RearMarginTrack() );
        } else {
            elemOption = addSubElement( xmlDoc, root, "option" );
            elemOption.setAttribute( "name", "track front margin" );
            elemOption.setAttribute( "value", m_doc->vcdOptions() ->FrontMarginTrackSVCD() );

            elemOption = addSubElement( xmlDoc, root, "option" );
            elemOption.setAttribute( "name", "track rear margin" );
            elemOption.setAttribute( "value", m_doc->vcdOptions() ->RearMarginTrackSVCD() );
        }

    }

    // create info element
    TQDomElement elemInfo = addSubElement( xmlDoc, root, "info" );
    addSubElement( xmlDoc, elemInfo, "album-id", m_doc->vcdOptions() ->albumId().upper() );
    addSubElement( xmlDoc, elemInfo, "volume-count", m_doc->vcdOptions() ->volumeCount() );
    addSubElement( xmlDoc, elemInfo, "volume-number", m_doc->vcdOptions() ->volumeNumber() );
    addSubElement( xmlDoc, elemInfo, "restriction", m_doc->vcdOptions() ->Restriction() );

    // create pvd element
    TQDomElement elemPvd = addSubElement( xmlDoc, root, "pvd" );
    addSubElement( xmlDoc, elemPvd, "volume-id", m_doc->vcdOptions() ->volumeId().upper() );
    addSubElement( xmlDoc, elemPvd, "system-id", m_doc->vcdOptions() ->systemId() );
    addSubElement( xmlDoc, elemPvd, "application-id", m_doc->vcdOptions() ->applicationId() );
    addSubElement( xmlDoc, elemPvd, "preparer-id", TQString( "K3b - Version %1" ).tqarg( k3bcore->version() ).upper() );
    addSubElement( xmlDoc, elemPvd, "publisher-id", m_doc->vcdOptions() ->publisher().upper() );


    // create filesystem element
    TQDomElement elemFileSystem = addSubElement( xmlDoc, root, "filesystem" );

    // SEGMENT folder, some standalone DVD-Player need this
    if ( !m_doc->vcdOptions() ->haveSegments() && m_doc->vcdOptions() ->SegmentFolder() )
        addFolderElement( xmlDoc, elemFileSystem, "SEGMENT" );

    // create cdi element
    if ( m_doc->vcdOptions() ->CdiSupport() ) {
        TQDomElement elemFolder = addFolderElement( xmlDoc, elemFileSystem, "CDI" );

        addFileElement( xmlDoc, elemFolder, locate( "data", "k3b/cdi/cdi_imag.rtf" ), "CDI_IMAG.RTF", true );
        addFileElement( xmlDoc, elemFolder, locate( "data", "k3b/cdi/cdi_text.fnt" ), "CDI_TEXT.FNT" );
        addFileElement( xmlDoc, elemFolder, locate( "data", "k3b/cdi/cdi_vcd.app" ), "CDI_VCD.APP" );

        TQString usercdicfg = locateLocal( "appdata", "cdi/cdi_vcd.cfg" );
        if ( TQFile::exists( usercdicfg ) )
            addFileElement( xmlDoc, elemFolder, usercdicfg, "CDI_VCD.CFG" );
        else
            addFileElement( xmlDoc, elemFolder, locate( "data", "k3b/cdi/cdi_vcd.cfg" ), "CDI_VCD.CFG" );
    }

    // sequence-items element & segment-items element
    TQDomElement elemsequenceItems;
    TQDomElement elemsegmentItems;

    // sequence-item element & segment-item element
    TQDomElement elemsequenceItem;
    TQDomElement elemsegmentItem;

    // if we have segments, elemsegmentItems must be before any sequence in xml file order
    if ( m_doc->vcdOptions()->haveSegments()  )
        elemsegmentItems = addSubElement( xmlDoc, root, "segment-items" );

    // sequence must always available ...
    elemsequenceItems = addSubElement( xmlDoc, root, "sequence-items" );
    // if we have no sequence (photo (s)vcd) we must add a dummy sequence they inform the user to turn on pbc on there videoplayer
    if ( !m_doc->vcdOptions()->haveSequence() )  {
            TQString filename;
            if  ( m_doc->vcdOptions()->mpegVersion() == 1 )
                filename = locate( "data", "k3b/extra/k3bphotovcd.mpg" );
            else
                filename = locate( "data", "k3b/extra/k3bphotosvcd.mpg" );

            elemsequenceItem = addSubElement( xmlDoc, elemsequenceItems, "sequence-item" );
            elemsequenceItem.setAttribute( "src", TQString( "%1" ).tqarg( TQFile::encodeName(  filename ).data() ) );
            elemsequenceItem.setAttribute( "id", "sequence-000"  );
            // default entry
            TQDomElement elemdefaultEntry;
            elemdefaultEntry = addSubElement( xmlDoc, elemsequenceItem, "default-entry" );
            elemdefaultEntry.setAttribute( "id", "entry-000"  );
    }


    // pbc
    TQDomElement elemPbc;

    // Add Tracks to XML
    TQPtrListIterator<K3bVcdTrack> it( *m_doc->tracks() );
    for ( ; it.current(); ++it ) {
        if ( !it.current() ->isSegment() ) {
            TQString seqId = TQString::number( it.current() ->index() ).rightJustify( 3, '0' );

            elemsequenceItem = addSubElement( xmlDoc, elemsequenceItems, "sequence-item" );
            elemsequenceItem.setAttribute( "src", TQString( "%1" ).tqarg( TQFile::encodeName( it.current() ->absPath() ).data() ) );
            elemsequenceItem.setAttribute( "id", TQString( "sequence-%1" ).tqarg( seqId ) );

            // default entry
            TQDomElement elemdefaultEntry;
            elemdefaultEntry = addSubElement( xmlDoc, elemsequenceItem, "default-entry" );
            elemdefaultEntry.setAttribute( "id", TQString( "entry-%1" ).tqarg( seqId ) );

        } else {
            // sequence-items element needs at least one segment to fit the XML
            elemsegmentItem = addSubElement( xmlDoc, elemsegmentItems, "segment-item" );
            elemsegmentItem.setAttribute( "src", TQString( "%1" ).tqarg( TQFile::encodeName( it.current() ->absPath() ).data() ) );
            elemsegmentItem.setAttribute( "id", TQString( "segment-%1" ).tqarg( TQString::number( it.current() ->index() ).rightJustify( 3, '0' ) ) );

        }
    }
    for ( it.toFirst(); it.current(); ++it ) {

        if ( m_doc->vcdOptions() ->PbcEnabled() ) {
            if ( elemPbc.isNull() )
                elemPbc = addSubElement( xmlDoc, root, "pbc" );

            doPbc( xmlDoc, elemPbc, it.current() );
        }
    }

    if ( ! elemPbc.isNull() ) {
        TQDomElement elemEndlist = addSubElement( xmlDoc, elemPbc, "endlist" );
        elemEndlist.setAttribute( "id", "end" );
        elemEndlist.setAttribute( "rejected", "true" );
    }

    m_xmlstring = xmlDoc.toString();
    kdDebug() << TQString( "(K3bVcdXmlView) Write Data to %1:" ).tqarg( fname ) << endl;

    TQFile xmlFile( fname );
    if ( xmlFile.open( IO_WriteOnly ) ) {
        TQTextStream ts( & xmlFile );
        ts << m_xmlstring;
        xmlFile.close();
        return true;
    }

    return false;
}

void K3bVcdXmlView::addComment( TQDomDocument& doc, TQDomElement& parent, const TQString& text )
{
    TQDomComment comment = doc.createComment( text );
    parent.appendChild( comment );
}

TQDomElement K3bVcdXmlView::addSubElement( TQDomDocument& doc, TQDomElement& parent, const TQString& name, const TQString& value )
{
    TQDomElement element = doc.createElement( name );
    parent.appendChild( element );
    if ( !value.isNull() ) {
        TQDomText t = doc.createTextNode( value );
        element.appendChild( t );
    }
    return element;
}

TQDomElement K3bVcdXmlView::addSubElement( TQDomDocument& doc, TQDomElement& parent, const TQString& name, const int& value )
{
    TQDomElement element = doc.createElement( name );
    parent.appendChild( element );
    if ( value >= -1 ) {
        TQDomText t = doc.createTextNode( TQString( "%1" ).tqarg( value ) );
        element.appendChild( t );
    }
    return element;
}

TQDomElement K3bVcdXmlView::addFolderElement( TQDomDocument& doc, TQDomElement& parent, const TQString& name )
{
    TQDomElement elemFolder = addSubElement( doc, parent, "folder" );
    addSubElement( doc, elemFolder, "name", name );

    return elemFolder;
}

void K3bVcdXmlView::addFileElement( TQDomDocument& doc, TQDomElement& parent, const TQString& src, const TQString& name, bool mixed )
{
    TQDomElement elemFile = addSubElement( doc, parent, "file" );
    elemFile.setAttribute( "src", TQString( "%1" ).tqarg( src ) );
    if ( mixed )
        elemFile.setAttribute( "format", "mixed" );

    addSubElement( doc, elemFile, "name", name );
}

void K3bVcdXmlView::doPbc( TQDomDocument& doc, TQDomElement& parent, K3bVcdTrack* track )
{
    TQString ref = ( track->isSegment() ) ? "segment" : "sequence";

    TQDomElement elemSelection = addSubElement( doc, parent, "selection" );
    elemSelection.setAttribute( "id", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( track->index() ).rightJustify( 3, '0' ) ) );

    setNumkeyBSN( doc, elemSelection, track );

    for ( int i = 0; i < K3bVcdTrack::_maxPbcTracks; i++ ) {
        TQDomElement elemPbcSelectionPNRDT;

        if ( track->getPbcTrack( i ) ) {
            int index = track->getPbcTrack( i ) ->index();
            TQString ref = ( track->getPbcTrack( i ) ->isSegment() ) ? "segment" : "sequence";

            switch ( i ) {
                case K3bVcdTrack::PREVIOUS:
                    elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "prev" );
                    elemPbcSelectionPNRDT.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( index ).rightJustify( 3, '0' ) ) );
                    break;
                case K3bVcdTrack::NEXT:
                    elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "next" );
                    elemPbcSelectionPNRDT.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( index ).rightJustify( 3, '0' ) ) );
                    break;
                case K3bVcdTrack::RETURN:
                    elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "return" );
                    elemPbcSelectionPNRDT.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( index ).rightJustify( 3, '0' ) ) );
                    break;
                case K3bVcdTrack::DEFAULT:
                    elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "default" );
                    elemPbcSelectionPNRDT.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( index ).rightJustify( 3, '0' ) ) );
                    break;
                case K3bVcdTrack::AFTERTIMEOUT:
                    if ( track->getWaitTime() >= 0 ) {
                        elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "timeout" );
                        elemPbcSelectionPNRDT.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( index ).rightJustify( 3, '0' ) ) );
                    }
                    break;
            }
        } else {
            // jump to <endlist> otherwise do noop while disabled
            if ( track->getNonPbcTrack( i ) == K3bVcdTrack::VIDEOEND ) {
                switch ( i ) {
                    case K3bVcdTrack::PREVIOUS:
                        elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "prev" );
                        elemPbcSelectionPNRDT.setAttribute( "ref", "end" );
                        break;
                    case K3bVcdTrack::NEXT:
                        elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "next" );
                        elemPbcSelectionPNRDT.setAttribute( "ref", "end" );
                        break;
                    case K3bVcdTrack::RETURN:
                        elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "return" );
                        elemPbcSelectionPNRDT.setAttribute( "ref", "end" );
                        break;
                    case K3bVcdTrack::DEFAULT:
                        elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "default" );
                        elemPbcSelectionPNRDT.setAttribute( "ref", "end" );
                        break;
                    case K3bVcdTrack::AFTERTIMEOUT:
                        if ( track->getWaitTime() >= 0 ) {
                            elemPbcSelectionPNRDT = addSubElement( doc, elemSelection, "timeout" );
                            elemPbcSelectionPNRDT.setAttribute( "ref", "end" );
                        }
                        break;
                }
            }
        }
    }

    addSubElement( doc, elemSelection, "wait", track->getWaitTime() );
    TQDomElement loop = addSubElement( doc, elemSelection, "loop", track->getPlayTime() );
    if ( track->Reactivity() )
        loop.setAttribute( "jump-timing", "delayed" );
    else
        loop.setAttribute( "jump-timing", "immediate" );

    addSubElement( doc, elemSelection, "play-item" ).setAttribute( "ref", TQString( "%1-%2" ).tqarg( ref ).tqarg( TQString::number( track->index() ).rightJustify( 3, '0' ) ) );

    setNumkeySEL( doc, elemSelection, track );
}

void K3bVcdXmlView::setNumkeyBSN( TQDomDocument& doc, TQDomElement& parent, K3bVcdTrack* track )
{
    if ( track->PbcNumKeys() ) {
        if ( track->PbcNumKeysUserdefined() ) {
            TQMap<int, K3bVcdTrack*> numKeyMap = track->DefinedNumKey();
            TQMap<int, K3bVcdTrack*>::const_iterator trackIt;

            m_startkey = 0;
            trackIt = numKeyMap.begin();
            if ( trackIt != numKeyMap.end() )
                m_startkey = trackIt.key();

            if ( m_startkey > 0 )
                addSubElement( doc, parent, "bsn", m_startkey );
            else // user has no numKeys defined for this track
                track->setPbcNumKeys( false );

        } else {
            // default start with key #1
            addSubElement( doc, parent, "bsn", 1 );
        }
    }
}

void K3bVcdXmlView::setNumkeySEL( TQDomDocument& doc, TQDomElement& parent, K3bVcdTrack* track )
{
    if ( track->PbcNumKeys() ) {
        TQDomElement elemPbcSelectionNumKeySEL;
        TQString ref = ( track->isSegment() ) ? "segment" : "sequence";
        int none = m_startkey;
        if ( track->PbcNumKeysUserdefined() ) {
            TQMap<int, K3bVcdTrack*> numKeyMap = track->DefinedNumKey();
            TQMap<int, K3bVcdTrack*>::const_iterator trackIt;

            for ( trackIt = numKeyMap.begin(); trackIt != numKeyMap.end(); ++trackIt ) {

                kdDebug() << TQString( "trackIt key: %1 none: %2" ).tqarg( trackIt.key() ).tqarg( none ) << endl;
                while ( none < trackIt.key() ) {
                    elemPbcSelectionNumKeySEL = addSubElement( doc, parent, "select" );
                    elemPbcSelectionNumKeySEL.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( track->index() ).rightJustify( 3, '0' ) ) );
                    addComment( doc, parent, TQString( "key %1 -> %2 (normal none)" ).tqarg( none ).tqarg( TQFile::encodeName( track->absPath() ).data() ) );
                    none++;
                }

                if ( trackIt.data() ) {
                    TQString ref = ( trackIt.data() ->isSegment() ) ? "segment" : "sequence";
                    elemPbcSelectionNumKeySEL = addSubElement( doc, parent, "select" );
                    elemPbcSelectionNumKeySEL.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( trackIt.data() ->index() ).rightJustify( 3, '0' ) ) );
                    addComment( doc, parent, TQString( "key %1 -> %2" ).tqarg( trackIt.key() ).tqarg( TQFile::encodeName( trackIt.data() ->absPath() ).data() ) );
                } else {
                    elemPbcSelectionNumKeySEL = addSubElement( doc, parent, "select" );
                    elemPbcSelectionNumKeySEL.setAttribute( "ref", "end" );
                    addComment( doc, parent, TQString( "key %1 -> end" ).tqarg( trackIt.key() ) );
                }
                none++;
            }
        } else {
            // default reference to itSelf
            elemPbcSelectionNumKeySEL = addSubElement( doc, parent, "select" );
            elemPbcSelectionNumKeySEL.setAttribute( "ref", TQString( "select-%1-%2" ).tqarg( ref ).tqarg( TQString::number( track->index() ).rightJustify( 3, '0' ) ) );
        }
    }
}

