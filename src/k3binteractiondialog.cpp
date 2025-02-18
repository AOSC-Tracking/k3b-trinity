/*
 *
 * $Id$
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


#include "k3binteractiondialog.h"
#include "k3btitlelabel.h"
#include "kcutlabel.h"
#include "k3bstdguiitems.h"
#include "k3bpushbutton.h"
#include "k3bthemedheader.h"
#include "k3bthememanager.h"
#include <k3bapplication.h>
#include <k3btoolbutton.h>
#include <k3bmultichoicedialog.h>

#include <tqlabel.h>
#include <tqtoolbutton.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqstring.h>
#include <tqpoint.h>
#include <tqfont.h>
#include <tqpopupmenu.h>
#include <tqeventloop.h>
#include <tqapplication.h>
#include <tqtimer.h>

#include <tdelocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <tdeglobalsettings.h>
#include <tdeversion.h>


K3bInteractionDialog::K3bInteractionDialog( TQWidget* parent,
					    const char* name,
					    const TQString& title,
					    const TQString& subTitle,
					    int buttonMask,
					    int defaultButton,
					    const TQString& configGroup,
					    bool modal,
					    WFlags fl )
  : KDialog( parent, name, modal, fl ),
    m_mainWidget(0),
    m_defaultButton(defaultButton),
    m_configGroup(configGroup),
    m_exitLoopOnHide(true),
    m_inLoop(false),
    m_inToggleMode(false),
    m_delayedInit(false)
{
  installEventFilter( this );

  mainGrid = new TQGridLayout( this );
  mainGrid->setSpacing( spacingHint() );
  mainGrid->setMargin( marginHint() );

  // header
  // ---------------------------------------------------------------------------------------------------
  m_dialogHeader = new K3bThemedHeader( this );
  mainGrid->addMultiCellWidget( m_dialogHeader, 0, 0, 0, 2 );


  // settings buttons
  // ---------------------------------------------------------------------------------------------------
  if( !m_configGroup.isEmpty() ) {
    TQHBoxLayout* layout2 = new TQHBoxLayout( 0, 0, spacingHint(), "layout2");
    m_buttonLoadSettings = new K3bToolButton( /*i18n("User Defaults"), */this );
    ((K3bToolButton*)m_buttonLoadSettings)->setIconSet( SmallIconSet( "document-revert" ) );
    TQPopupMenu* userDefaultsPopup = new TQPopupMenu( m_buttonLoadSettings );
    userDefaultsPopup->insertItem( i18n("Load default settings"), this, TQ_SLOT(slotLoadK3bDefaults()) );
    userDefaultsPopup->insertItem( i18n("Load saved settings"), this, TQ_SLOT(slotLoadUserDefaults()) );
    userDefaultsPopup->insertItem( i18n("Load last used settings"), this, TQ_SLOT(slotLoadLastSettings()) );
    ((TQToolButton*)m_buttonLoadSettings)->setPopup( userDefaultsPopup );
    ((K3bToolButton*)m_buttonLoadSettings)->setInstantMenu( true );
    layout2->addWidget( m_buttonLoadSettings );

    m_buttonSaveSettings = new TQToolButton( /*i18n("Save User Defaults"), */this, "m_buttonSaveSettings" );
    ((TQToolButton*)m_buttonSaveSettings)->setIconSet( SmallIconSet( "document-save" ) );
    layout2->addWidget( m_buttonSaveSettings );

    mainGrid->addLayout( layout2, 2, 0 );
  }

  TQSpacerItem* spacer = new TQSpacerItem( 10, 10, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  mainGrid->addItem( spacer, 2, 1 );


  // action buttons
  // ---------------------------------------------------------------------------------------------------
  TQHBoxLayout* layout5 = new TQHBoxLayout( 0, 0, spacingHint(), "layout5");

  if( buttonMask & START_BUTTON ) {
    KGuiItem startItem = KStdGuiItem::ok();
    m_buttonStart = new KPushButton( startItem, this, "m_buttonStart" );
    // refine the button text
    setButtonText( START_BUTTON,
		   i18n("Start"),
		   i18n("Start the task") );
    TQFont fnt( m_buttonStart->font() );
    fnt.setBold(true);
    m_buttonStart->setFont( fnt );
  }
  else
    m_buttonStart = 0;

  if( buttonMask & SAVE_BUTTON ) {
    m_buttonSave = new KPushButton( KStdGuiItem::save(), this, "m_buttonSave" );
  }
  else
    m_buttonSave = 0;

  if( buttonMask & CANCEL_BUTTON ) {
    m_buttonCancel = new KPushButton( TDEConfigGroup( k3bcore->config(), "General Options" )
				      .readBoolEntry( "keep action dialogs open", false )
				      ? KStdGuiItem::close()
				      : KStdGuiItem::cancel(),
				      this,
				      "m_buttonCancel" );
  }
  else
    m_buttonCancel = 0;

  // we only handle some of the possible settings since
  // our buttons are always to the right of the dialog
  int btl = 0;
#if KDE_IS_VERSION(3,3,0)
  btl = TDEGlobalSettings::buttonLayout();
#endif
  switch( btl ) {
  case 0: // KDE default
  default:
      if ( m_buttonStart )
          layout5->addWidget( m_buttonStart );
      if ( m_buttonSave )
          layout5->addWidget( m_buttonSave );
      if ( m_buttonCancel )
          layout5->addWidget( m_buttonCancel );
      break;

  case 1: // something different
      if ( m_buttonCancel )
          layout5->addWidget( m_buttonCancel );
      if ( m_buttonSave )
          layout5->addWidget( m_buttonSave );
      if ( m_buttonStart )
          layout5->addWidget( m_buttonStart );
      break;

  case 2: // GTK-Style
      if ( m_buttonSave )
          layout5->addWidget( m_buttonSave );
      if ( m_buttonCancel )
          layout5->addWidget( m_buttonCancel );
      if ( m_buttonStart )
          layout5->addWidget( m_buttonStart );
      break;
  }

  mainGrid->addLayout( layout5, 2, 2 );

  mainGrid->setRowStretch( 1, 1 );

  setTitle( title, subTitle );

  initConnections();
  initToolTipsAndWhatsThis();

  setDefaultButton( START_BUTTON );
}

K3bInteractionDialog::~K3bInteractionDialog()
{
}


void K3bInteractionDialog::show()
{
  KDialog::show();
  if( KPushButton* b = getButton( m_defaultButton ) )
    b->setFocus();
}


TQSize K3bInteractionDialog::sizeHint() const
{
  TQSize s = KDialog::sizeHint();
  // I want the dialogs to look good.
  // That means their height should never outgrow their width
  if( s.height() > s.width() )
    s.setWidth( s.height() );

  return s;
}


void K3bInteractionDialog::initConnections()
{
  if( m_buttonStart ) {
    connect( m_buttonStart, TQ_SIGNAL(clicked()),
	     this, TQ_SLOT(slotStartClickedInternal()) );
  }
  if( m_buttonSave ) {
//     connect( m_buttonSave, TQ_SIGNAL(clicked()),
// 	     this, TQ_SLOT(slotSaveLastSettings()) );
    connect( m_buttonSave, TQ_SIGNAL(clicked()),
	     this, TQ_SLOT(slotSaveClicked()) );
  }
  if( m_buttonCancel )
    connect( m_buttonCancel, TQ_SIGNAL(clicked()),
	     this, TQ_SLOT(slotCancelClicked()) );

  if( !m_configGroup.isEmpty() ) {
    connect( m_buttonSaveSettings, TQ_SIGNAL(clicked()),
	     this, TQ_SLOT(slotSaveUserDefaults()) );
  }
}


void K3bInteractionDialog::initToolTipsAndWhatsThis()
{
  if( !m_configGroup.isEmpty() ) {
    // ToolTips
    // -------------------------------------------------------------------------
    TQToolTip::add( m_buttonLoadSettings, i18n("Load default or saved settings") );
    TQToolTip::add( m_buttonSaveSettings, i18n("Save current settings to reuse them later") );

    // What's This info
    // -------------------------------------------------------------------------
    TQWhatsThis::add( m_buttonLoadSettings, i18n("<p>Load a set of settings either from the default K3b settings, "
						"settings saved before, or the last used ones.") );
    TQWhatsThis::add( m_buttonSaveSettings, i18n("<p>Saves the current settings of the action dialog."
						"<p>These settings can be loaded with the <em>Load saved settings</em> "
						"button."
						"<p><b>The K3b defaults are not overwritten by this.</b>") );
  }
}


void K3bInteractionDialog::setTitle( const TQString& title, const TQString& subTitle )
{
  m_dialogHeader->setTitle( title, subTitle );

  setCaption( title );
}


void K3bInteractionDialog::setMainWidget( TQWidget* w )
{
  w->reparent( this, TQPoint(0,0) );
  mainGrid->addMultiCellWidget( w, 1, 1, 0, 2 );
  m_mainWidget = w;
}

TQWidget* K3bInteractionDialog::mainWidget()
{
  if( !m_mainWidget ) {
    setMainWidget( new TQWidget( this ) );
  }
  return m_mainWidget;
}

void K3bInteractionDialog::slotLoadK3bDefaults()
{
  loadK3bDefaults();
}

void K3bInteractionDialog::slotLoadUserDefaults()
{
  TDEConfigGroup c( k3bcore->config(), m_configGroup );
  loadUserDefaults( &c );
}

void K3bInteractionDialog::slotSaveUserDefaults()
{
  TDEConfigGroup c( k3bcore->config(), m_configGroup );
  saveUserDefaults( &c );
}


void K3bInteractionDialog::slotLoadLastSettings()
{
  TDEConfigGroup c( k3bcore->config(), "last used " + m_configGroup );
  loadUserDefaults( &c );
}


void K3bInteractionDialog::saveLastSettings()
{
  TDEConfigGroup c( k3bcore->config(), "last used " + m_configGroup );
  saveUserDefaults( &c );
}


void K3bInteractionDialog::slotStartClickedInternal()
{
  saveLastSettings();

  TDEConfigGroup c( k3bcore->config(), "General Options" );
  if( !c.readNumEntry( "action dialog startup settings", 0 ) ) {
    // first time saving last used settings
    switch( K3bMultiChoiceDialog::choose( i18n("Action Dialog Settings"),
					  i18n("<p>K3b handles three sets of settings in action dialogs: "
					       "the defaults, the saved settings, and the last used settings. "
					       "Please choose which of these sets should be loaded if an action "
					       "dialog is opened again."
					       "<p><em>Be aware that this choice can always be changed from the K3b "
					       "configuration dialog.</em>"),
					  TQMessageBox::Question,
					  this,
					  0,
					  3,
					  i18n("Default Settings"),
					  i18n("Saved Settings"),
					  i18n("Last Used Settings") ) ) {
    case 1:
      c.writeEntry( "action dialog startup settings", LOAD_K3B_DEFAULTS );
      break;
    case 2:
      c.writeEntry( "action dialog startup settings", LOAD_SAVED_SETTINGS );
      break;
    case 3:
      c.writeEntry( "action dialog startup settings", LOAD_LAST_SETTINGS );
      break;
    }
  }

  slotStartClicked();
}


void K3bInteractionDialog::slotStartClicked()
{
  emit started();
}

void K3bInteractionDialog::slotCancelClicked()
{
  emit canceled();
  close( false );
}

void K3bInteractionDialog::slotSaveClicked()
{
  emit saved();
}


void K3bInteractionDialog::setDefaultButton( int button )
{
  m_defaultButton = button;

  // reset all other default buttons
  if( KPushButton* b = getButton( START_BUTTON ) )
    b->setDefault( true );
  if( KPushButton* b = getButton( SAVE_BUTTON ) )
    b->setDefault( true );
  if( KPushButton* b = getButton( CANCEL_BUTTON ) )
    b->setDefault( true );

  // set the selected default
  if( KPushButton* b = getButton( button ) )
    b->setDefault( true );
}


bool K3bInteractionDialog::eventFilter( TQObject* o, TQEvent* ev )
{
  if( dynamic_cast<K3bInteractionDialog*>(o) == this &&
      ev->type() == TQEvent::KeyPress ) {

    TQKeyEvent* kev = dynamic_cast<TQKeyEvent*>(ev);

    switch ( kev->key() ) {
    case Key_Enter:
    case Key_Return:
      // if the process finished this closes the dialog
      if( m_defaultButton == START_BUTTON ) {
	if( m_buttonStart->isEnabled() )
	  slotStartClickedInternal();
      }
      else if( m_defaultButton == CANCEL_BUTTON ) {
	if( m_buttonCancel->isEnabled() )
	  slotCancelClicked();
      }
      else if( m_defaultButton == SAVE_BUTTON ) {
	if( m_buttonSave->isEnabled() )
	  slotSaveClicked();
      }
      return true;

    case Key_Escape:
      // simulate button clicks
      if( m_buttonCancel ) {
	if( m_buttonCancel->isEnabled() )
	  slotCancelClicked();
      }
      return true;
    }
  }

  return KDialog::eventFilter( o, ev );
}


KPushButton* K3bInteractionDialog::getButton( int button )
{
  switch( button ) {
  case START_BUTTON:
    return m_buttonStart;
  case SAVE_BUTTON:
    return m_buttonSave;
  case CANCEL_BUTTON:
    return m_buttonCancel;
  default:
    return 0;
  }
}


void K3bInteractionDialog::setButtonGui( int button,
					 const KGuiItem& item )
{
  if( KPushButton* b = getButton( button ) )
    b->setGuiItem( item );
}


void K3bInteractionDialog::setButtonText( int button,
					  const TQString& text,
					  const TQString& tooltip,
					  const TQString& whatsthis )
{
  if( KPushButton* b = getButton( button ) ) {
    b->setText( text );
    TQToolTip::remove( b );
    TQWhatsThis::remove( b );
    TQToolTip::add( b, tooltip );
    TQWhatsThis::add( b, whatsthis );
  }
}


void K3bInteractionDialog::setButtonEnabled( int button, bool enabled )
{
  if( KPushButton* b = getButton( button ) ) {
    b->setEnabled( enabled );
    // make sure the correct button is selected as default again
    setDefaultButton( m_defaultButton );
  }
}


void K3bInteractionDialog::setButtonShown( int button, bool shown )
{
  if( KPushButton* b = getButton( button ) ) {
    b->setShown( shown );
    // make sure the correct button is selected as default again
    setDefaultButton( m_defaultButton );
  }
}


void K3bInteractionDialog::setStartButtonText( const TQString& text,
					       const TQString& tooltip,
					       const TQString& whatsthis )
{
  if( m_buttonStart ) {
    m_buttonStart->setText( text );
    TQToolTip::remove( m_buttonStart );
    TQWhatsThis::remove( m_buttonStart );
    TQToolTip::add( m_buttonStart, tooltip );
    TQWhatsThis::add( m_buttonStart, whatsthis );
  }
}


void K3bInteractionDialog::setCancelButtonText( const TQString& text,
						const TQString& tooltip,
						const TQString& whatsthis )
{
  if( m_buttonCancel ) {
    m_buttonCancel->setText( text );
    TQToolTip::remove( m_buttonCancel );
    TQWhatsThis::remove( m_buttonCancel );
    TQToolTip::add( m_buttonCancel, tooltip );
    TQWhatsThis::add( m_buttonCancel, whatsthis );
  }
}


void K3bInteractionDialog::setSaveButtonText( const TQString& text,
					      const TQString& tooltip,
					      const TQString& whatsthis )
{
  if( m_buttonSave ) {
    m_buttonSave->setText( text );
    TQToolTip::remove( m_buttonSave );
    TQWhatsThis::remove( m_buttonSave );
    TQToolTip::add( m_buttonSave, tooltip );
    TQWhatsThis::add( m_buttonSave, whatsthis );
  }
}


void K3bInteractionDialog::saveUserDefaults( TDEConfigBase* )
{
}


void K3bInteractionDialog::loadUserDefaults( TDEConfigBase* )
{
}


void K3bInteractionDialog::loadK3bDefaults()
{
}


void K3bInteractionDialog::loadStartupSettings()
{
  TDEConfigGroup c( k3bcore->config(), "General Options" );

  // earlier K3b versions loaded the saved settings
  // so that is what we do as a default
  int i = c.readNumEntry( "action dialog startup settings", LOAD_SAVED_SETTINGS );
  switch( i ) {
  case LOAD_K3B_DEFAULTS:
    slotLoadK3bDefaults();
    break;
  case LOAD_SAVED_SETTINGS:
    slotLoadUserDefaults();
    break;
  case LOAD_LAST_SETTINGS:
    slotLoadLastSettings();
    break;
  }
}


int K3bInteractionDialog::exec()
{
  return exec( true );
}


int K3bInteractionDialog::exec( bool returnOnHide )
{
  m_exitLoopOnHide = returnOnHide;

  // the following code is mainly taken from TQDialog::exec

  if( m_inLoop ) {
    kdError() << "(K3bInteractionDialog::exec) Recursive call detected." << endl;
    return -1;
  }

  bool destructiveClose = testWFlags( WDestructiveClose );
  clearWFlags( WDestructiveClose );

  bool wasShowModal = testWFlags( WShowModal );
  setWFlags( WShowModal );
  setResult( 0 );

  loadStartupSettings();
  show();
  if( m_delayedInit )
    TQTimer::singleShot( 0, this, TQ_SLOT(slotDelayedInit()) );
  else
    init();

  m_inLoop = true;
  TQApplication::eventLoop()->enterLoop();

  if( !wasShowModal )
    clearWFlags( WShowModal );

  int res = result();

  if( destructiveClose )
    delete this;

  return res;
}


void K3bInteractionDialog::hide()
{
  if( isHidden() )
    return;

  KDialog::hide();

  if( m_inLoop && m_exitLoopOnHide ) {
    m_inLoop = false;
    TQApplication::eventLoop()->exitLoop();
  }
}


bool K3bInteractionDialog::close( bool alsoDelete )
{
  if( m_inLoop && !m_exitLoopOnHide ) {
    m_inLoop = false;
    TQApplication::eventLoop()->exitLoop();
  }

  return KDialog::close( alsoDelete );
}


void K3bInteractionDialog::done( int r )
{
  if( m_inLoop && !m_exitLoopOnHide ) {
    m_inLoop = false;
    TQApplication::eventLoop()->exitLoop();
  }

  return KDialog::done( r );
}


void K3bInteractionDialog::slotToggleAll()
{
  if( !m_inToggleMode ) {
    m_inToggleMode = true;
    toggleAll();
    m_inToggleMode = false;
  }
}


void K3bInteractionDialog::toggleAll()
{
}


void K3bInteractionDialog::slotDelayedInit()
{
  init();
}

#include "k3binteractiondialog.moc"
