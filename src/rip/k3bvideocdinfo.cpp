/*
*
* $Id: k3bvideocdinfo.cpp 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003 Christian Kvasny <chris@k3b.org>
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



#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqstringlist.h>
#include <tqtimer.h>
#include <tqdom.h>

#include <tdelocale.h>
#include <tdeconfig.h>
#include <kdebug.h>

#include "k3bvideocdinfo.h"

#include <k3bprocess.h>
#include <k3bexternalbinmanager.h>


K3bVideoCdInfo::K3bVideoCdInfo( TQObject* parent, const char* name )
        : TQObject( parent, name )
{
    m_process = 0L;
    m_isXml = false;
}


K3bVideoCdInfo::~K3bVideoCdInfo()
{
    delete m_process;
}

void K3bVideoCdInfo::cancelAll()
{
    if ( m_process->isRunning() ) {
        m_process->disconnect( this );
        m_process->kill();
    }
}

void K3bVideoCdInfo::info( const TQString& device )
{
    if ( !k3bcore ->externalBinManager() ->foundBin( "vcdxrip" ) ) {
        kdDebug() << "(K3bVideoCdInfo::info) could not find vcdxrip executable" << endl;
        emit infoFinished( false );
        return ;
    }

    delete m_process;
    m_process = new K3bProcess();

    *m_process << k3bcore ->externalBinManager() ->binPath( "vcdxrip" );

    *m_process << "-q" << "--norip" << "-i" << device << "-o" << "-";

    connect( m_process, TQ_SIGNAL( receivedStderr( TDEProcess*, char*, int ) ),
             this, TQ_SLOT( slotParseOutput( TDEProcess*, char*, int ) ) );
    connect( m_process, TQ_SIGNAL( receivedStdout( TDEProcess*, char*, int ) ),
             this, TQ_SLOT( slotParseOutput( TDEProcess*, char*, int ) ) );
    connect( m_process, TQ_SIGNAL( processExited( TDEProcess* ) ),
             this, TQ_SLOT( slotInfoFinished() ) );

    if ( !m_process->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) ) {
        kdDebug() << "(K3bVideoCdInfo::info) could not start vcdxrip" << endl;
        cancelAll();
        emit infoFinished( false );
    }
}

void K3bVideoCdInfo::slotParseOutput( TDEProcess*, char* output, int len )
{
    TQString buffer = TQString::fromLocal8Bit( output, len );

    // split to lines
    TQStringList lines = TQStringList::split( "\n", buffer );
    TQStringList::Iterator end( lines.end());
    for ( TQStringList::Iterator str = lines.begin(); str != end; ++str ) {

        if ( ( *str ).contains( "<?xml" ) )
            m_isXml = true;

        if ( m_isXml )
            m_xmlData += *str;
        else
            kdDebug() << "(K3bVideoCdInfo::slotParseOutput) " << *str << endl;

        if ( ( *str ).contains( "</videocd>" ) )
            m_isXml = false;
    }
}

void K3bVideoCdInfo::slotInfoFinished()
{
    if ( m_process->normalExit() ) {
        // TODO: check the process' exitStatus()
        switch ( m_process->exitStatus() ) {
            case 0:
                break;
            default:
                cancelAll();
                emit infoFinished( false );
                return ;
        }
    } else {
        cancelAll();
        emit infoFinished( false );
        return ;
    }

    if ( m_xmlData.isEmpty() ) {
        emit infoFinished( false );
        return ;
    }

    parseXmlData();
    emit infoFinished( true );
}

void K3bVideoCdInfo::parseXmlData()
{
    TQDomDocument xml_doc;
    TQDomElement xml_root;

    m_Result.xmlData = m_xmlData;

    xml_doc.setContent( m_xmlData );
    xml_root = xml_doc.documentElement();

    m_Result.type = xml_root.attribute( "class" );
    m_Result.version = xml_root.attribute( "version" );

    for ( TQDomNode node = xml_root.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        TQDomElement el = node.toElement();
        TQString tagName = el.tagName().lower();

        if ( tagName == "pvd" ) {
            for ( TQDomNode snode = node.firstChild(); !snode.isNull(); snode = snode.nextSibling() ) {
                TQDomElement sel = snode.toElement();
                TQString pvdElement = sel.tagName().lower();
                TQString pvdElementText = sel.text();
                if ( pvdElement == "volume-id" )
                    m_Result.volumeId = pvdElementText;
            }

        } else if ( tagName == "sequence-items" ) {
            for ( TQDomNode snode = node.firstChild(); !snode.isNull(); snode = snode.nextSibling() ) {
                TQDomElement sel = snode.toElement();
                TQString seqElement = sel.tagName().lower();
                m_Result.addEntry( K3bVideoCdInfoResultEntry(
                                       sel.attribute( "src" ),
                                       sel.attribute( "id" ) ),
                                   K3bVideoCdInfoResult::SEQUENCE
                                 );
            }
        } else if ( tagName == "segment-items" ) {
            for ( TQDomNode snode = node.firstChild(); !snode.isNull(); snode = snode.nextSibling() ) {
                TQDomElement sel = snode.toElement();
                TQString seqElement = sel.tagName().lower();
                m_Result.addEntry( K3bVideoCdInfoResultEntry(
                                       sel.attribute( "src" ),
                                       sel.attribute( "id" ) ),
                                   K3bVideoCdInfoResult::SEGMENT
                                 );
            }
        } else {
            kdDebug() << TQString( "(K3bVideoCdInfo::parseXmlData) tagName '%1' not used" ).arg( tagName ) << endl;
        }
    }
}

const K3bVideoCdInfoResult& K3bVideoCdInfo::result() const
{
    return m_Result;
}

const K3bVideoCdInfoResultEntry& K3bVideoCdInfoResult::entry( unsigned int number, int type ) const
{
    switch ( type ) {
        case K3bVideoCdInfoResult::FILE:
            if ( number >= m_fileEntry.count() )
                return m_emptyEntry;
            return m_fileEntry[ number ];
        case K3bVideoCdInfoResult::SEGMENT:
            if ( number >= m_segmentEntry.count() )
                return m_emptyEntry;
            return m_segmentEntry[ number ];
        case K3bVideoCdInfoResult::SEQUENCE:
            if ( number >= m_sequenceEntry.count() )
                return m_emptyEntry;
            return m_sequenceEntry[ number ];
        default:
            kdDebug() << "(K3bVideoCdInfoResult::entry) not supported entrytype." << endl;
    }

    return m_emptyEntry;

}


void K3bVideoCdInfoResult::addEntry( const K3bVideoCdInfoResultEntry& entry, int type )
{
    switch ( type ) {
        case K3bVideoCdInfoResult::FILE:
            m_fileEntry.append( entry );
            break;
        case K3bVideoCdInfoResult::SEGMENT:
            m_segmentEntry.append( entry );
            break;
        case K3bVideoCdInfoResult::SEQUENCE:
            m_sequenceEntry.append( entry );
            break;
        default:
            kdDebug() << "(K3bVideoCdInfoResult::addEntry) not supported entrytype." << endl;
    }
}

int K3bVideoCdInfoResult::foundEntries( int type ) const
{
    switch ( type ) {
        case K3bVideoCdInfoResult::FILE:
            return m_fileEntry.count();
        case K3bVideoCdInfoResult::SEGMENT:
            return m_segmentEntry.count();
        case K3bVideoCdInfoResult::SEQUENCE:
            return m_sequenceEntry.count();
        default:
            kdDebug() << "(K3bVideoCdInfoResult::addEntry) not supported entrytype." << endl;
    }
    return 0;
}

#include "k3bvideocdinfo.moc"

