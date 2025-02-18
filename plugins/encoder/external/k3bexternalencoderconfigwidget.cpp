/* 
 *
 * $Id: k3bexternalencoder.cpp 567280 2006-07-28 13:26:27Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bexternalencoderconfigwidget.h"

#include <k3bcore.h>

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqlistview.h>
#include <tqpushbutton.h>

#include <klineedit.h>
#include <tdemessagebox.h>
#include <tdeconfig.h>
#include <tdelocale.h>




K3bExternalEncoderEditDialog::K3bExternalEncoderEditDialog( TQWidget* parent )
  : KDialogBase( Swallow,
		 i18n("Editing external audio encoder"),
		 Ok|Cancel,
		 Ok,
		 parent )
{
  m_editW = new base_K3bExternalEncoderEditWidget( this );
  setMainWidget( m_editW );
}


K3bExternalEncoderEditDialog::~K3bExternalEncoderEditDialog()
{
}


K3bExternalEncoderCommand K3bExternalEncoderEditDialog::currentCommand() const
{
  K3bExternalEncoderCommand cmd;
  cmd.name = m_editW->m_editName->text();
  cmd.extension = m_editW->m_editExtension->text();
  cmd.command = m_editW->m_editCommand->text();
  cmd.swapByteOrder = m_editW->m_checkSwapByteOrder->isChecked();
  cmd.writeWaveHeader = m_editW->m_checkWriteWaveHeader->isChecked();
  return cmd;
}


void K3bExternalEncoderEditDialog::setCommand( const K3bExternalEncoderCommand& cmd )
{
  m_editW->m_editName->setText( cmd.name );
  m_editW->m_editExtension->setText( cmd.extension );
  m_editW->m_editCommand->setText( cmd.command );
  m_editW->m_checkSwapByteOrder->setChecked( cmd.swapByteOrder );
  m_editW->m_checkWriteWaveHeader->setChecked( cmd.writeWaveHeader );
}


void K3bExternalEncoderEditDialog::slotOk()
{
  if( m_editW->m_editName->text().isEmpty() ) {
    KMessageBox::error( this, 
			i18n("Please specify a name for the command."),
			i18n("No name specified") );
  }
  else if( m_editW->m_editExtension->text().isEmpty() ) {
    KMessageBox::error( this, 
			i18n("Please specify an extension for the command."),
			i18n("No extension specified") );
  }
  else if( m_editW->m_editCommand->text().isEmpty() ) {
    KMessageBox::error( this, 
			i18n("Please specify the command line."),
			i18n("No command line specified") );
  }
  else if( !m_editW->m_editCommand->text().contains( "%f" ) ) {
    KMessageBox::error( this, 
			i18n("Please add the output filename (%f) to the command line."),
			i18n("No filename specified") );
  }
  // FIXME: check for name and extension uniqueness
  else {
    KDialogBase::slotOk();
  }
}






class K3bExternalEncoderSettingsWidget::Private
{
public:
  TQMap<TQListViewItem*, K3bExternalEncoderCommand> commands;
};


K3bExternalEncoderSettingsWidget::K3bExternalEncoderSettingsWidget( TQWidget* parent, const char* name )
  : K3bPluginConfigWidget( parent, name )
{
  d = new Private();

  w = new base_K3bExternalEncoderConfigWidget( this );
  TQHBoxLayout* lay = new TQHBoxLayout( this );
  lay->setMargin( 0 );
  lay->addWidget( w );

  connect( w->m_viewEncoders, TQ_SIGNAL(selectionChanged()),
	   this, TQ_SLOT(slotSelectionChanged()) );
  connect( w->m_buttonAdd, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotNewCommand()) );
  connect( w->m_buttonEdit, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotEditCommand()) );
  connect( w->m_buttonRemove, TQ_SIGNAL(clicked()),
	   this, TQ_SLOT(slotRemoveCommand()) );

  m_editDlg = new K3bExternalEncoderEditDialog( this );
}


K3bExternalEncoderSettingsWidget::~K3bExternalEncoderSettingsWidget()
{
  delete d;
}


void K3bExternalEncoderSettingsWidget::slotNewCommand()
{
  // clear the dialog
  m_editDlg->setCommand( K3bExternalEncoderCommand() );

  if( m_editDlg->exec() == TQDialog::Accepted ) {
    K3bExternalEncoderCommand cmd = m_editDlg->currentCommand();
    d->commands.insert( new TQListViewItem( w->m_viewEncoders, 
					   w->m_viewEncoders->lastItem(),
					   cmd.name,
					   cmd.extension,
					   cmd.command ),
			cmd );
  }
}


void K3bExternalEncoderSettingsWidget::slotSelectionChanged()
{
  w->m_buttonRemove->setEnabled( w->m_viewEncoders->selectedItem() != 0 );
  w->m_buttonEdit->setEnabled( w->m_viewEncoders->selectedItem() != 0 );
}


void K3bExternalEncoderSettingsWidget::slotEditCommand()
{
  if( TQListViewItem* item = w->m_viewEncoders->selectedItem() ) {
    m_editDlg->setCommand( d->commands[item] );
    if( m_editDlg->exec() == TQDialog::Accepted ) {
      d->commands[item] = m_editDlg->currentCommand();
    }
  }
}


void K3bExternalEncoderSettingsWidget::slotRemoveCommand()
{
  if( TQListViewItem* item = w->m_viewEncoders->selectedItem() ) {
    d->commands.erase( item );
    delete item;
  }
}


void K3bExternalEncoderSettingsWidget::loadConfig()
{
  d->commands.clear();
  w->m_viewEncoders->clear();

  TQValueList<K3bExternalEncoderCommand> cmds( K3bExternalEncoderCommand::readCommands() );
  for( TQValueList<K3bExternalEncoderCommand>::iterator it = cmds.begin();
       it != cmds.end(); ++it ) {
    K3bExternalEncoderCommand& cmd = *it;
    d->commands.insert( new TQListViewItem( w->m_viewEncoders, 
					   w->m_viewEncoders->lastItem(),
					   cmd.name,
					   cmd.extension,
					   cmd.command ),
			cmd );
  }
}


void K3bExternalEncoderSettingsWidget::saveConfig()
{
  TDEConfig* c = k3bcore->config();
  c->deleteGroup( "K3bExternalEncoderPlugin", true );
  c->setGroup( "K3bExternalEncoderPlugin" );

  TQStringList cmdNames;
  for( TQMapIterator<TQListViewItem*, K3bExternalEncoderCommand> it = d->commands.begin();
       it != d->commands.end(); ++it ) {
    TQStringList cmd;
    cmd << it.data().name << it.data().extension << it.data().command;
    if( it.data().swapByteOrder )
      cmd << "swap";
    if( it.data().writeWaveHeader )
      cmd << "wave";
    c->writeEntry( "command_" + it.data().name, cmd );
    cmdNames << it.data().name;
  }
  c->writeEntry( "commands", cmdNames );
}




#include "k3bexternalencoderconfigwidget.moc"
