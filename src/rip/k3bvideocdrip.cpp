/*
*
* $Id: k3bvideocdrip.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <kconfig.h>
#include <kdebug.h>
#include <kio/global.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>

#include <tqdatetime.h>
#include <tqdom.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqurl.h>

// K3b Includes
#include "k3bvideocdrip.h"
#include <k3bcore.h>
#include <k3bexternalbinmanager.h>
#include <k3bglobals.h>
#include <k3bprocess.h>

K3bVideoCdRip::K3bVideoCdRip( K3bJobHandler* hdl, K3bVideoCdRippingOptions* options, TQObject* tqparent, const char* name )
  : K3bJob( hdl, tqparent, name ),
    m_ripsourceType( 0 ),
    m_subPosition ( 0 ),
    m_videooptions( options ),
    m_canceled( false ),
    m_process( 0 )
{}


K3bVideoCdRip::~K3bVideoCdRip()
{
    if ( m_process )
        delete m_process;

}


void K3bVideoCdRip::cancel()
{
    cancelAll();

    emit infoMessage( i18n( "Job canceled by user." ), K3bJob::ERROR );
    emit canceled();
    jobFinished( false );
}


void K3bVideoCdRip::cancelAll()
{
    m_canceled = true;

    if ( m_process->isRunning() ) {
        m_process->disconnect( this );
        m_process->kill();
    }
}


void K3bVideoCdRip::start()
{
    kdDebug() << "(K3bVideoCdRip) starting job" << endl;

    jobStarted();
    m_canceled = false;

    vcdxRip();
}

void K3bVideoCdRip::vcdxRip()
{
    emit newTask( i18n( "Check files" ) );

    m_stage = stageUnknown;
    delete m_process;
    m_process = new K3bProcess();

    const K3bExternalBin* bin = k3bcore ->externalBinManager() ->binObject( "vcdxrip" );

    if ( !bin ) {
        kdDebug() << "(K3bVideoCdRip) could not find vcdxrip executable" << endl;
        emit infoMessage( i18n( "Could not tqfind %1 executable." ).tqarg( "vcdxrip" ), K3bJob::ERROR );
        emit infoMessage( i18n( "To rip VideoCD's you must install VcdImager Version %1." ).tqarg( ">= 0.7.12" ), K3bJob::INFO );
        emit infoMessage( i18n( "You can find this on your distribution disks or download it from http://www.vcdimager.org" ), K3bJob::INFO );
        cancelAll();
        jobFinished( false );
        return ;
    }

    if( bin->version < K3bVersion("0.7.12") ) {
        kdDebug() << "(K3bVideoCdRip) vcdxrip executable too old!" << endl;
        emit infoMessage( i18n( "%1 executable too old! Need version %2 or greater" ).tqarg( "Vcdxrip" ).tqarg( "0.7.12" ), K3bJob::ERROR );
        emit infoMessage( i18n( "You can find this on your distribution disks or download it from http://www.vcdimager.org" ), K3bJob::INFO );
        cancelAll();
        jobFinished( false );
        return ;
    }
        
    if ( !bin->copyright.isEmpty() )
        emit infoMessage( i18n( "Using %1 %2 - Copyright (C) %3" ).tqarg( bin->name() ).tqarg( bin->version ).tqarg( bin->copyright ), INFO );


    *m_process << k3bcore ->externalBinManager() ->binPath( "vcdxrip" );

    // additional user parameters from config
    const TQStringList& params = k3bcore->externalBinManager() ->program( "vcdxrip" ) ->userParameters();
    for ( TQStringList::const_iterator it = params.begin(); it != params.end(); ++it )
        *m_process << *it;

    *m_process << "--gui" << "--progress";

     if ( !m_videooptions ->getVideoCdRipFiles() )
        *m_process << "--nofiles";

     if ( !m_videooptions ->getVideoCdRipSegments() )
        *m_process << "--nosegments";
        
     if ( !m_videooptions ->getVideoCdRipSequences() )
        *m_process << "--nosequences";

     if ( m_videooptions ->getVideoCdIgnoreExt() )
        *m_process << "--no-ext-psd";

     if ( m_videooptions ->getVideoCdSector2336() )
        *m_process << "--sector-2336";
        
     *m_process << "-i" << TQString( "%1" ).tqarg( TQFile::encodeName( m_videooptions ->getVideoCdSource() ).data() );

     if ( m_videooptions ->getVideoCdExtractXml() )
        *m_process << "-o" << TQString( "%1" ).tqarg( TQFile::encodeName( m_videooptions ->getVideoCdDescription() + ".xml" ).data() );
     else
        *m_process << "-o" << "/dev/null";
      

    connect( m_process, TQT_SIGNAL( receivedStderr( KProcess*, char*, int ) ),
             this, TQT_SLOT( slotParseVcdXRipOutput( KProcess*, char*, int ) ) );
    connect( m_process, TQT_SIGNAL( receivedStdout( KProcess*, char*, int ) ),
             this, TQT_SLOT( slotParseVcdXRipOutput( KProcess*, char*, int ) ) );
    connect( m_process, TQT_SIGNAL( processExited( KProcess* ) ),
             this, TQT_SLOT( slotVcdXRipFinished() ) );

    m_process->setWorkingDirectory( TQUrl( m_videooptions ->getVideoCdDestination() ).dirPath() );

    // vcdxrip commandline parameters
    kdDebug() << "***** vcdxrip parameters:" << endl;
    ;
    const TQValueList<TQCString>& args = m_process->args();
    TQString s;
    for ( TQValueList<TQCString>::const_iterator it = args.begin(); it != args.end(); ++it ) {
        s += *it + " ";
    }
    kdDebug() << s << flush << endl;
    emit debuggingOutput( "vcdxrip command:", s );

    emit newTask( i18n( "Extracting" ) );
    emit infoMessage( i18n( "Start extracting." ), K3bJob::INFO );
    emit infoMessage( i18n( "Extract files from %1 to %2." ).tqarg( m_videooptions ->getVideoCdSource() ).tqarg( m_videooptions ->getVideoCdDestination() ), K3bJob::INFO );
            
    if ( !m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput ) ) {
        kdDebug() << "(K3bVideoCdRip) could not start vcdxrip" << endl;
        emit infoMessage( i18n( "Could not start %1." ).tqarg( "vcdxrip" ), K3bJob::ERROR );
        cancelAll();
        jobFinished( false );
    }
}

void K3bVideoCdRip::slotParseVcdXRipOutput( KProcess*, char* output, int len )
{
    TQString buffer = TQString::fromLocal8Bit( output, len );

    // split to lines
    TQStringList lines = TQStringList::split( "\n", buffer );

    TQDomDocument xml_doc;
    TQDomElement xml_root;

    // do every line
    TQStringList::Iterator end( lines.end());
    for ( TQStringList::Iterator str = lines.begin(); str != end; ++str ) {
        *str = ( *str ).stripWhiteSpace();

        emit debuggingOutput( "vcdxrip", *str );

        xml_doc.setContent( TQString( "<?xml version='1.0'?><vcdxrip>" ) + *str + "</vcdxrip>" );

        xml_root = xml_doc.documentElement();

        for ( TQDomNode node = xml_root.firstChild(); !node.isNull(); node = node.nextSibling() ) {
            TQDomElement el = node.toElement();
            if ( el.isNull() )
                continue;

            const TQString tagName = el.tagName().lower();

            if ( tagName == "progress" ) {
                const TQString oper = el.attribute( "operation" ).lower();
                const unsigned long long overallPos = el.attribute( "position" ).toLong();
                const unsigned long long pos = overallPos - m_subPosition;
                const unsigned long long size = el.attribute( "size" ).toLong() - m_subPosition;

                if ( oper == "extract" ) {
                    emit subPercent( ( int ) ( 100.0 * ( double ) pos / ( double ) size ) );
                    emit processedSubSize( ( pos * 2352 ) / 1024 / 1024 , ( size * 2352 ) / 1024 / 1024 );

                    m_bytesFinished = pos;

                    kdDebug() << "(slotParseVcdXRipOutput) overall: " << ((long)overallPos  * 2352)
			      << ", videocdsize: " << m_videooptions->getVideoCdSize() << endl;
                    double relOverallWritten = ( ( double ) overallPos  * 2352 ) / ( double ) m_videooptions ->getVideoCdSize() ;
                    int newpercent =  ( int ) ( 100 * relOverallWritten );
                    if ( newpercent > m_oldpercent ) {
                        emit percent(  newpercent );
                        m_oldpercent = newpercent;
                    }

                } else {
                    return ;
                }

            } else if ( tagName == "log" ) {
                TQDomText tel = el.firstChild().toText();
                const TQString level = el.attribute( "level" ).lower();
                if ( tel.isText() ) {
                    const TQString text = tel.data();
                    if ( level == "information" ) {
                        kdDebug() << TQString( "(K3bVideoCdRip) vcdxrip information, %1" ).tqarg( text ) << endl;
                        parseInformation( text );
                    } else {
                        if ( level != "error" ) {
                            kdDebug() << TQString( "(K3bVideoCdRip) vcdxrip warning, %1" ).tqarg( text ) << endl;
                            emit debuggingOutput( "vcdxrip", text );
                            parseInformation( text );
                        } else {
                            kdDebug() << TQString( "(K3bVideoCdRip) vcdxrip error, %1" ).tqarg( text ) << endl;
                            emit infoMessage( text, K3bJob::ERROR );
                        }
                    }
                }
            }
        }
    }
}


void K3bVideoCdRip::slotVcdXRipFinished()
{
    if ( m_process->normalExit() ) {
        // TODO: check the process' exitStatus()
        switch ( m_process->exitStatus() ) {
            case 0:
                emit infoMessage( i18n( "Files successfully extracted." ), K3bJob::SUCCESS );
                break;
            default:
                emit infoMessage( i18n( "%1 returned an unknown error (code %2)." ).tqarg( "vcdxrip" ).tqarg( m_process->exitStatus() ), K3bJob::ERROR );
                emit infoMessage( i18n( "Please send me an email with the last output..." ), K3bJob::ERROR );
                cancelAll();
                jobFinished( false );
                return ;
        }
    } else {
        emit infoMessage( i18n( "%1 did not exit cleanly." ).tqarg( "Vcdxrip" ), K3bJob::ERROR );
        cancelAll();
        jobFinished( false );
        return ;
    }

    jobFinished( true );
}

void K3bVideoCdRip::parseInformation( TQString text )
{
    // parse warning
    if ( text.tqcontains( "encountered non-form2 sector" ) ) {
        // I think this is an error not a warning. Finish ripping with invalid mpegs.
        emit infoMessage( i18n( "%1 encountered non-form2 sector" ).tqarg("Vcdxrip"), K3bJob::ERROR );
        emit infoMessage( i18n( "leaving loop" ), K3bJob::ERROR );
        cancelAll();
        jobFinished( false );
        return;
    }
    
    // parse extra info
    else if ( text.tqcontains( "detected extended VCD2.0 PBC files" ) )
        emit infoMessage( i18n( "detected extended VCD2.0 PBC files" ), K3bJob::INFO );

    // parse startposition and extracting sequence info
    // extracting avseq05.mpg... (start lsn 32603 (+28514))
    else if ( text.startsWith( "extracting" ) ) {
        if ( text.tqcontains( "(start lsn" ) ) {
            int index = text.tqfind( "(start lsn" );
            int end = text.tqfind( " (+" );
            if ( end > 0) {
                m_subPosition = text.mid( index + 11, end - index - 11 ).stripWhiteSpace().toLong();
            }
            else {
                // found segment here we can get only the start lsn :)
                // extracting item0001.mpg... (start lsn 225, 1 segments)
                int end = text.tqfind(  ",", index );
                int overallPos = text.mid( index + 11, end - index - 11 ).stripWhiteSpace().toLong();
                double relOverallWritten = ( ( double ) overallPos  * 2352 ) / ( double ) m_videooptions ->getVideoCdSize()  ;
                int newpercent =  ( int ) ( 100 * relOverallWritten );
                if ( newpercent > m_oldpercent ) {
                    emit percent(  newpercent );
                    m_oldpercent = newpercent;
                }
            }


            index = 11;
            end = text.tqfind( "(start lsn" );
            emit newSubTask( i18n( "Extracting %1" ).tqarg( text.mid( index, end - index ).stripWhiteSpace() ) );
        }
        // parse extracting files info
        // extracting CDI/CDI_IMAG.RTF to _cdi_cdi_imag.rtf (lsn 258, size 1315168, raw 1)
        else if ( text.tqcontains( "(lsn" ) && text.tqcontains( "size" ) ) {
            int index = 11;
            int end = text.tqfind( "to" );
            TQString extractFileName = text.mid( index, end - index ).stripWhiteSpace();
            index = text.tqfind( " to " );
            end = text.tqfind( " (lsn" );
            TQString toFileName = text.mid( index + 4, end - index - 4 ).stripWhiteSpace();
            emit newSubTask( i18n( "Extracting %1 to %2" ).tqarg( extractFileName ).tqarg( toFileName ) );
        }
    }
}

TQString K3bVideoCdRip::jobDescription() const
{
    return i18n( "Extracting %1" ).tqarg( m_videooptions ->getVideoCdDescription() );
}

TQString K3bVideoCdRip::jobDetails() const
{
    return TQString( "(%1)" ).arg ( KIO::convertSize( m_videooptions ->getVideoCdSize() ) );
}

#include "k3bvideocdrip.moc"
