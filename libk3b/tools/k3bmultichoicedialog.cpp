/* 
 *
 * $Id: k3bmultichoicedialog.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bmultichoicedialog.h"
#include "k3bstdguiitems.h"
#include <k3brichtextlabel.h>

#include <kpushbutton.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <tqlayout.h>
#include <tqsignalmapper.h>
#include <tqptrlist.h>
#include <tqlabel.h>
#include <tqhbox.h>
#include <tqmessagebox.h>


class K3bMultiChoiceDialog::Private
{
public:
  Private()
    : mapper(0),
      buttonLayout(0) {
  }

  TQSignalMapper* mapper;
  TQPtrList<KPushButton> buttons;
  TQHBoxLayout* buttonLayout;

  bool buttonClicked;
};


// from kmessagebox.cpp
static TQPixmap themedMessageBoxIcon(TQMessageBox::Icon icon)
{
  TQString icon_name;
 
  switch(icon) {
  case TQMessageBox::NoIcon:
    return TQPixmap();
    break;
  case TQMessageBox::Information:
    icon_name = "messagebox_info";
    break;
  case TQMessageBox::Warning:
    icon_name = "messagebox_warning";
    break;
  case TQMessageBox::Critical:
    icon_name = "messagebox_critical";
    break;
  default:
    break;
  }
 
  TQPixmap ret = TDEApplication::kApplication()->iconLoader()->loadIcon(icon_name, TDEIcon::NoGroup, TDEIcon::SizeMedium, TDEIcon::DefaultState, 0, true);
 
  if (ret.isNull())
    return TQMessageBox::standardIcon(icon);
  else
    return ret;
}

K3bMultiChoiceDialog::K3bMultiChoiceDialog( const TQString& caption,
					    const TQString& text,
					    TQMessageBox::Icon icon,
					    TQWidget* parent, const char* name )
  : KDialog( parent, name )
{
  d = new Private();
  d->mapper = new TQSignalMapper( TQT_TQOBJECT(this) );
  connect( d->mapper, TQT_SIGNAL(mapped(int)), this, TQT_SLOT(done(int)) );

  setCaption( caption );

  TQGridLayout* mainGrid = new TQGridLayout( this );
  mainGrid->setSpacing( spacingHint() );
  mainGrid->setMargin( marginHint() );

  TQHBox* contents = new TQHBox( this );
  contents->setSpacing( KDialog::spacingHint()*2 );
  contents->setMargin( 0 );

  TQLabel* pixLabel = new TQLabel( contents );
  pixLabel->setPixmap( themedMessageBoxIcon( icon ) );
  pixLabel->setScaledContents( false );
  TQLabel* label = new K3bRichTextLabel( text, contents );
  contents->setStretchFactor( label, 1 );

  d->buttonLayout = new TQHBoxLayout;
  d->buttonLayout->setSpacing( spacingHint() );
  d->buttonLayout->setMargin( 0 );

  mainGrid->addMultiCellWidget( contents, 0, 0, 0, 2 );
  mainGrid->addMultiCellWidget( K3bStdGuiItems::horizontalLine( this ), 1, 1, 0, 2 );
  mainGrid->addLayout( d->buttonLayout, 2, 1 );

  mainGrid->setColStretch( 0, 1 );
  mainGrid->setColStretch( 2, 1 );
  mainGrid->setRowStretch( 0, 1 );
}


K3bMultiChoiceDialog::~K3bMultiChoiceDialog()
{
  delete d;
}


int K3bMultiChoiceDialog::addButton( const KGuiItem& b )
{
  KPushButton* button = new KPushButton( b, this );
  d->buttonLayout->add( button );
  d->buttons.append(button);
  d->mapper->setMapping( TQT_TQOBJECT(button), d->buttons.count() );
  connect( button, TQT_SIGNAL(clicked()), d->mapper, TQT_SLOT(map()) );
  return d->buttons.count();
}


void K3bMultiChoiceDialog::slotButtonClicked( int code )
{
  d->buttonClicked = true;
  done( code );
}


int K3bMultiChoiceDialog::exec()
{
  d->buttonClicked = false;
  return KDialog::exec();
}


void K3bMultiChoiceDialog::closeEvent( TQCloseEvent* e )
{
  // make sure the dialog can only be closed by the buttons
  // otherwise we may get an undefined return value in exec

  if( d->buttonClicked )
    KDialog::closeEvent( e );
  else
    e->ignore();
}


int K3bMultiChoiceDialog::choose( const TQString& caption,
				  const TQString& text,
				  TQMessageBox::Icon icon,
				  TQWidget* parent, 
				  const char* name,
				  int buttonCount,
				  const KGuiItem& b1,
				  const KGuiItem& b2,
				  const KGuiItem& b3,
				  const KGuiItem& b4,
				  const KGuiItem& b5,
				  const KGuiItem& b6 )
{
  K3bMultiChoiceDialog dlg( caption, text, icon, parent, name );
  dlg.addButton( b1 );
  if( buttonCount > 1 )
    dlg.addButton( b2 );
  if( buttonCount > 2 )
    dlg.addButton( b3 );
  if( buttonCount > 3 )
    dlg.addButton( b4 );
  if( buttonCount > 4 )
    dlg.addButton( b5 );
  if( buttonCount > 5 )
    dlg.addButton( b6 );

  return dlg.exec();
}

		     
#include "k3bmultichoicedialog.moc"
