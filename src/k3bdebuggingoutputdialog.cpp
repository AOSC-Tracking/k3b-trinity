/* 
 *
 * $Id: k3bdebuggingoutputdialog.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bdebuggingoutputdialog.h"

#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bdeviceglobals.h>
#include <k3bcore.h>
#include <k3bversion.h>
#include <k3bglobals.h>

#include <tqtextedit.h>
#include <tqcursor.h>
#include <tqfile.h>
#include <tqclipboard.h>

#include <tdelocale.h>
#include <kstdguiitem.h>
#include <tdeglobalsettings.h>
#include <tdeapplication.h>
#include <tdefiledialog.h>
#include <tdemessagebox.h>


K3bDebuggingOutputDialog::K3bDebuggingOutputDialog( TQWidget* parent )
  : KDialogBase( parent, "debugViewDialog", true, i18n("Debugging Output"), Close|User1|User2, Close, 
		 false, 
		 KStdGuiItem::saveAs(), 
		 KGuiItem( i18n("Copy"), "edit-copy" ) )
{
  setButtonTip( User1, i18n("Save to file") );
  setButtonTip( User2, i18n("Copy to clipboard") );

  debugView = new TQTextEdit( this );
  debugView->setReadOnly(true);
  debugView->setTextFormat( TQTextEdit::PlainText );
  debugView->setCurrentFont( TDEGlobalSettings::fixedFont() );
  debugView->setWordWrap( TQTextEdit::NoWrap );

  setMainWidget( debugView );

  resize( 600, 300 );
}


void K3bDebuggingOutputDialog::setOutput( const TQMap<TQString, TQStringList>& map )
{
  // the following may take some time
  TQApplication::setOverrideCursor( TQCursor(TQt::WaitCursor) );

  clear();

  // add the debugging output
  for( TQMap<TQString, TQStringList>::ConstIterator itMap = map.begin(); itMap != map.end(); ++itMap ) {
    const TQStringList& list = itMap.data();
    debugView->append( itMap.key() + "\n" );
    debugView->append( "-----------------------\n" );
    for( TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
       TQStringList lines = TQStringList::split( "\n", *it );
       // do every line
       TQStringList::ConstIterator end( lines.end() );
       for( TQStringList::ConstIterator str = lines.begin(); str != end; ++str )
	 debugView->append( *str + "\n" );
    }
    m_paragraphMap[itMap.key()] = debugView->paragraphs();
    debugView->append( "\n" );
  }

  TQApplication::restoreOverrideCursor();
}


void K3bDebuggingOutputDialog::addOutput( const TQString& app, const TQString& msg )
{
  TQMap<TQString, int>::Iterator it = m_paragraphMap.find( app );

  if( it == m_paragraphMap.end() ) {
    // create new section
    debugView->append( app + "\n" );
    debugView->append( "-----------------------\n" );
    debugView->append( msg + "\n" );
    m_paragraphMap[app] = debugView->paragraphs();
    debugView->append( "\n" );
  }
  else {
    debugView->insertParagraph( msg, *it );
    // update the paragraphs map 
    // FIXME: we cannot count on the map to be sorted properly!
    while( it != m_paragraphMap.end() ) {
      it.data() += 1;
      ++it;
    }
  }
}


void K3bDebuggingOutputDialog::clear()
{
  debugView->clear();
  m_paragraphMap.clear();

  addOutput( "System", "K3b Version: " + k3bcore->version() );
  addOutput( "System", "KDE Version: " + TQString(KDE::versionString()) );
  addOutput( "System", "TQt Version:  " + TQString(tqVersion()) );
  addOutput( "System", "Kernel:      " + K3b::kernelVersion() );
  
  // devices in the logfile
  for( TQPtrListIterator<K3bDevice::Device> it( k3bcore->deviceManager()->allDevices() ); *it; ++it ) {
    K3bDevice::Device* dev = *it;
    addOutput( "Devices", 
	       TQString( "%1 (%2, %3) [%5] [%6] [%7]" )
	       .arg( dev->vendor() + " " + dev->description() + " " + dev->version() )
	       .arg( dev->blockDeviceName() )
	       .arg( dev->genericDevice() )
	       .arg( K3bDevice::deviceTypeString( dev->type() ) )
	       .arg( K3bDevice::mediaTypeString( dev->supportedProfiles() ) )
	       .arg( K3bDevice::writingModeString( dev->writingModes() ) ) );
  }
}


void K3bDebuggingOutputDialog::slotUser1()
{
  TQString filename = KFileDialog::getSaveFileName();
  if( !filename.isEmpty() ) {
    TQFile f( filename );
    if( !f.exists() || KMessageBox::warningContinueCancel( this,
						  i18n("Do you want to overwrite %1?").arg(filename),
						  i18n("File Exists"), i18n("Overwrite") )
	== KMessageBox::Continue ) {

      if( f.open( IO_WriteOnly ) ) {
	TQTextStream t( &f );
	t << debugView->text();
      }
      else {
	KMessageBox::error( this, i18n("Could not open file %1").arg(filename) );
      }
    }
  }
}


void K3bDebuggingOutputDialog::slotUser2()
{
  TQApplication::clipboard()->setText( debugView->text(), TQClipboard::Clipboard );
}

#include "k3bdebuggingoutputdialog.moc"
