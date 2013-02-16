/*
 *
 * $Id: k3baudiocdview.cpp 624215 2007-01-16 19:10:03Z trueg $
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

#include "k3baudiocdview.h"
#include "k3baudiorippingdialog.h"
#include "k3baudiocdlistview.h"

#include <k3bpassivepopup.h>
#include <k3btoc.h>
#include <k3bdiskinfo.h>
#include <k3bdevicehandler.h>
#include <k3blistview.h>
#include <k3bcddbresult.h>
#include <k3bmsf.h>
#include <k3bcddb.h>
#include <k3bcddbquery.h>
#include <k3btoolbox.h>
#include <kcutlabel.h>
#include <k3bstdguiitems.h>
#include <k3bapplication.h>
#include <k3bthememanager.h>
#include <k3baudiocdtrackdrag.h>
#include <k3bthemedlabel.h>

#include <tdelocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <tdeaction.h>
#include <kstdaction.h>
#include <tdemessagebox.h>
#include <tdeconfig.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kstandarddirs.h>
#include <kdialogbase.h>

#include <tqlayout.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqframe.h>
#include <tqspinbox.h>
#include <tqfont.h>
#include <tqdragobject.h>




class K3bAudioCdView::AudioTrackViewItem : public K3bCheckListViewItem
{
public:
  AudioTrackViewItem( TQListView* parent, 
		      TQListViewItem* after,
		      int _trackNumber,
		      const K3b::Msf& length) 
    : K3bCheckListViewItem( parent, after ) {

    setText( 1, TQString::number(_trackNumber).rightJustify( 2, ' ' ) );
    setText( 3, i18n("Track %1").arg(_trackNumber) );
    setText( 4, " " + length.toString() + " " );
    setText( 5, " " + TDEIO::convertSize( length.audioBytes() ) + " " );

    trackNumber = _trackNumber;

    setEditor( 2, LINE );
    setEditor( 3, LINE );

    setChecked(true);
  }

  void setup() {
    K3bCheckListViewItem::setup();
    
    setHeight( height() + 4 );
  }

  int trackNumber;

  void updateCddbData( const K3bCddbResultEntry& entry ) {
    setText( 2, entry.artists[trackNumber-1] );
    setText( 3, entry.titles[trackNumber-1] );
  }
};


K3bAudioCdView::K3bAudioCdView( TQWidget* parent, const char *name )
  : K3bMediaContentsView( true, 
			  K3bMedium::CONTENT_AUDIO,
			  K3bDevice::MEDIA_CD_ALL,
			  K3bDevice::STATE_INCOMPLETE|K3bDevice::STATE_COMPLETE,
			  parent, name )
{
  TQGridLayout* mainGrid = new TQGridLayout( mainWidget() );

  // toolbox
  // ----------------------------------------------------------------------------------
  TQHBoxLayout* toolBoxLayout = new TQHBoxLayout( 0, 0, 0, "toolBoxLayout" );
  m_toolBox = new K3bToolBox( mainWidget() );
  toolBoxLayout->addWidget( m_toolBox );
  TQSpacerItem* spacer = new TQSpacerItem( 10, 10, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  toolBoxLayout->addItem( spacer );
  m_labelLength = new TQLabel( mainWidget() );
  m_labelLength->setAlignment( int( TQLabel::AlignVCenter | TQLabel::AlignRight ) );
  toolBoxLayout->addWidget( m_labelLength );


  // the track view
  // ----------------------------------------------------------------------------------
  m_trackView = new K3bAudioCdListView( this, mainWidget() );

  connect( m_trackView, TQT_SIGNAL(itemRenamed(TQListViewItem*, const TQString&, int)),
	   this, TQT_SLOT(slotItemRenamed(TQListViewItem*, const TQString&, int)) );
  connect( m_trackView, TQT_SIGNAL(contextMenu(TDEListView*, TQListViewItem*, const TQPoint&)),
	   this, TQT_SLOT(slotContextMenu(TDEListView*, TQListViewItem*, const TQPoint&)) );
//   connect( m_trackView, TQT_SIGNAL(selectionChanged(TQListViewItem*)), 
// 	   this, TQT_SLOT(slotTrackSelectionChanged(TQListViewItem*)) );

  mainGrid->addLayout( toolBoxLayout, 0, 0 );
  mainGrid->addWidget( m_trackView, 1, 0 );


  m_cddb = new K3bCddb( TQT_TQOBJECT(this) );

  connect( m_cddb, TQT_SIGNAL(queryFinished(int)),
	   this, TQT_SLOT(slotCddbQueryFinished(int)) );

  initActions();
  //  slotTrackSelectionChanged(0);

  setLeftPixmap( K3bTheme::MEDIA_LEFT );
  setRightPixmap( K3bTheme::MEDIA_AUDIO );

  m_busyInfoLabel = new K3bThemedLabel( i18n("Searching for Artist information..."), this );
  m_busyInfoLabel->setFrameStyle( TQFrame::Box|TQFrame::Plain );
  m_busyInfoLabel->setMargin( 6 );
  m_busyInfoLabel->hide();
}


K3bAudioCdView::~K3bAudioCdView()
{
}


void K3bAudioCdView::reloadMedium()
{
  m_toc = medium().toc();
  m_device = medium().device();

  // initialize cddb info for editing
  m_cddbInfo = K3bCddbResultEntry();
  m_cddbInfo.discid = TQString::number( medium().toc().discId(), 16 );

  for( int i = 0; i < (int)m_toc.count(); ++i ) {
    m_cddbInfo.titles.append("");
    m_cddbInfo.artists.append("");
    m_cddbInfo.extInfos.append("");
  }

  m_trackView->clear();
  showBusyLabel(true);

  // create a listviewItem for every audio track
  int index = 1;
  for( K3bDevice::Toc::const_iterator it = m_toc.begin(); 
       it != m_toc.end(); ++it ) {

    // for now skip data tracks since we are not able to rip them to iso
    if( (*it).type() == K3bTrack::AUDIO ) {
      K3b::Msf length( (*it).length() );
      (void)new AudioTrackViewItem( m_trackView,
				    m_trackView->lastItem(),
				    index,
				    length );
    }

    index++;
  }

  m_cdText = medium().cdText();
      
  // simulate a cddb entry with the cdtext data
  m_cddbInfo.cdTitle = m_cdText.title();
  m_cddbInfo.cdArtist = m_cdText.performer();
  m_cddbInfo.cdExtInfo = m_cdText.message();

  for( unsigned int i = 0; i < m_cdText.count(); ++i ) {
    m_cddbInfo.titles[i] = m_cdText[i].title();
    m_cddbInfo.artists[i] = m_cdText[i].performer();
    m_cddbInfo.extInfos[i] = m_cdText[i].message();
  }

  updateDisplay();

  TDEConfig* c = k3bcore->config();
  c->setGroup("Cddb");
  bool useCddb = ( c->readBoolEntry( "use local cddb query", true ) || 
		   c->readBoolEntry( "use remote cddb", false ) );

  if( useCddb &&
      ( m_cdText.isEmpty() ||
	KMessageBox::questionYesNo( this, 
				    i18n("Found Cd-Text. Do you want to use it instead of querying CDDB?"),
				    i18n("Found Cd-Text"), 
				    i18n("Use CD-Text"),
				    i18n("Query CDDB"),
				    "prefereCdTextOverCddb" ) == KMessageBox::No ) )
    queryCddb();
  else
    showBusyLabel(false);
}


void K3bAudioCdView::initActions()
{
  m_actionCollection = new TDEActionCollection( this );

  TDEAction* actionSelectAll = new TDEAction( i18n("Check All"), 0, 0, TQT_TQOBJECT(this),
					  TQT_SLOT(slotCheckAll()), actionCollection(),
					  "check_all" );
  TDEAction* actionDeselectAll = new TDEAction( i18n("Uncheck All"), 0, 0, TQT_TQOBJECT(this),
					    TQT_SLOT(slotUncheckAll()), actionCollection(),
					    "uncheck_all" );
  TDEAction* actionSelect = new TDEAction( i18n("Check Track"), 0, 0, TQT_TQOBJECT(this),
				       TQT_SLOT(slotSelect()), actionCollection(),
				       "select_track" );
  TDEAction* actionDeselect = new TDEAction( i18n("Uncheck Track"), 0, 0, TQT_TQOBJECT(this),
					 TQT_SLOT(slotDeselect()), actionCollection(),
					 "deselect_track" );
  TDEAction* actionEditTrackCddbInfo = new TDEAction( i18n("Edit Track cddb Info"), "edit", 0, TQT_TQOBJECT(this),
						  TQT_SLOT(slotEditTrackCddb()), actionCollection(),
						  "edit_track_cddb" );
  TDEAction* actionEditAlbumCddbInfo = new TDEAction( i18n("Edit Album cddb Info"), "edit", 0, TQT_TQOBJECT(this),
						  TQT_SLOT(slotEditAlbumCddb()), actionCollection(),
						  "edit_album_cddb" );

  TDEAction* actionStartRip = new TDEAction( i18n("Start Ripping"), "cddarip", 0, TQT_TQOBJECT(this),
					 TQT_SLOT(startRip()), actionCollection(), "start_rip" );

  TDEAction* actionQueryCddb = new TDEAction( i18n("Query cddb"), "reload", 0, TQT_TQOBJECT(this),
					  TQT_SLOT(queryCddb()), actionCollection(), "query_cddb" );

  TDEAction* actionSaveCddbLocally = new TDEAction( i18n("Save Cddb Entry Locally"), "filesave", 0, TQT_TQOBJECT(this),
						TQT_SLOT(slotSaveCddbLocally()), actionCollection(), "save_cddb_local" );

  // TODO: set the actions tooltips and whatsthis infos

  // setup the popup menu
  m_popupMenu = new TDEActionMenu( actionCollection(), "popup_menu" );
  TDEAction* separator = new TDEActionSeparator( actionCollection(), "separator" );
  m_popupMenu->insert( actionSelect );
  m_popupMenu->insert( actionDeselect );
  m_popupMenu->insert( actionSelectAll );
  m_popupMenu->insert( actionDeselectAll );
  m_popupMenu->insert( separator );
  m_popupMenu->insert( actionEditTrackCddbInfo );
  m_popupMenu->insert( actionEditAlbumCddbInfo );
  m_popupMenu->insert( separator );
  m_popupMenu->insert( actionStartRip );

  // setup the toolbox
  m_toolBox->addButton( actionStartRip, true );
  m_toolBox->addSpacing();
  m_toolBox->addButton( actionQueryCddb );
  m_toolBox->addButton( actionSaveCddbLocally );
  m_toolBox->addButton( actionEditTrackCddbInfo );
  m_toolBox->addButton( actionEditAlbumCddbInfo );
}


void K3bAudioCdView::slotContextMenu( TDEListView*, TQListViewItem*, const TQPoint& p )
{
  m_popupMenu->popup(p);
}


void K3bAudioCdView::slotItemRenamed( TQListViewItem* item, const TQString& str, int col )
{
  AudioTrackViewItem* a = (AudioTrackViewItem*)item;
  if( col == 2 )
    m_cddbInfo.artists[a->trackNumber-1] = str;
  else if( col == 3 )
    m_cddbInfo.titles[a->trackNumber-1] = str;
  else if( col == 6 )
    m_cddbInfo.extInfos[a->trackNumber-1] = str;
}


void K3bAudioCdView::slotTrackSelectionChanged( TQListViewItem* item )
{
  actionCollection()->action("edit_track_cddb")->setEnabled( item != 0 );
  actionCollection()->action("select_track")->setEnabled( item != 0 );
  actionCollection()->action("deselect_track")->setEnabled( item != 0 );
}


void K3bAudioCdView::startRip()
{
  TQValueList<int> trackNumbers;
  for( TQListViewItemIterator it( m_trackView ); it.current(); ++it ) {
    AudioTrackViewItem* a = (AudioTrackViewItem*)it.current();
    if( a->isChecked() )
      trackNumbers.append( a->trackNumber );
  }

  if( trackNumbers.count() == 0 ) {
    KMessageBox::error( this, i18n("Please select the tracks to rip."),
			i18n("No Tracks Selected") );
  }
  else {
    K3bAudioRippingDialog rip( m_toc,
			       m_device, 
			       m_cddbInfo, 
			       trackNumbers, 
			       this );
    rip.exec();
  }
}


void K3bAudioCdView::slotEditTrackCddb()
{
  TQPtrList<TQListViewItem> items( m_trackView->selectedItems() );
  if( !items.isEmpty() ) {
    AudioTrackViewItem* a = static_cast<AudioTrackViewItem*>(items.first());

    KDialogBase d( this, "trackCddbDialog", true, i18n("Cddb Track %1").arg(a->trackNumber),
		   KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);
    TQWidget* w = new TQWidget( &d );

    KLineEdit* editTitle = new KLineEdit( m_cddbInfo.titles[a->trackNumber-1], w );
    KLineEdit* editArtist = new KLineEdit( m_cddbInfo.artists[a->trackNumber-1], w );
    KLineEdit* editExtInfo = new KLineEdit( m_cddbInfo.extInfos[a->trackNumber-1], w );
    TQFrame* line = new TQFrame( w );
    line->setFrameShape( TQFrame::HLine );
    line->setFrameShadow( TQFrame::Sunken );

    TQGridLayout* grid = new TQGridLayout( w );
    grid->setSpacing( KDialog::spacingHint() );

    grid->addWidget( new TQLabel( i18n("Title:"), w ), 0, 0 );
    grid->addWidget( editTitle, 0, 1 );
    grid->addMultiCellWidget( line, 1, 1, 0, 1 );
    grid->addWidget( new TQLabel( i18n("Artist:"), w ), 2, 0 );
    grid->addWidget( editArtist, 2, 1 );
    grid->addWidget( new TQLabel( i18n("Extra info:"), w ), 3, 0 );
    grid->addWidget( editExtInfo, 3, 1 );
    grid->setRowStretch( 4, 1 );

    d.setMainWidget(w);
    d.resize( TQMAX( TQMAX(d.sizeHint().height(), d.sizeHint().width()), 300), d.sizeHint().height() );

    if( d.exec() == TQDialog::Accepted ) {
      m_cddbInfo.titles[a->trackNumber-1] = editTitle->text();
      m_cddbInfo.artists[a->trackNumber-1] = editArtist->text();
      m_cddbInfo.extInfos[a->trackNumber-1] = editExtInfo->text();
      a->updateCddbData( m_cddbInfo );
    }
  }
}


void K3bAudioCdView::slotEditAlbumCddb()
{
  KDialogBase d( this, "trackCddbDialog", true, i18n("Album Cddb"),
		 KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);
  TQWidget* w = new TQWidget( &d );

  KLineEdit* editTitle = new KLineEdit( m_cddbInfo.cdTitle, w );
  KLineEdit* editArtist = new KLineEdit( m_cddbInfo.cdArtist, w );
  KLineEdit* editExtInfo = new KLineEdit( m_cddbInfo.cdExtInfo, w );
  KLineEdit* editGenre = new KLineEdit( m_cddbInfo.genre, w );
  TQSpinBox* spinYear = new TQSpinBox( 0, 9999, 1, w );
  spinYear->setValue( m_cddbInfo.year );
  TQFrame* line = new TQFrame( w );
  line->setFrameShape( TQFrame::HLine );
  line->setFrameShadow( TQFrame::Sunken );
  KComboBox* comboCat = new KComboBox( w );
  comboCat->insertStringList( K3bCddbQuery::categories() );

  // set the category
  for( int i = 0; i < comboCat->count(); ++i )
    if( comboCat->text(i) == m_cddbInfo.category ) {
      comboCat->setCurrentItem(i);
      break;
    }

  TQGridLayout* grid = new TQGridLayout( w );
  grid->setSpacing( KDialog::spacingHint() );

  grid->addWidget( new TQLabel( i18n("Title:"), w ), 0, 0 );
  grid->addWidget( editTitle, 0, 1 );
  grid->addMultiCellWidget( line, 1, 1, 0, 1 );
  grid->addWidget( new TQLabel( i18n("Artist:"), w ), 2, 0 );
  grid->addWidget( editArtist, 2, 1 );
  grid->addWidget( new TQLabel( i18n("Extra info:"), w ), 3, 0 );
  grid->addWidget( editExtInfo, 3, 1 );
  grid->addWidget( new TQLabel( i18n("Genre:"), w ), 4, 0 );
  grid->addWidget( editGenre, 4, 1 );
  grid->addWidget( new TQLabel( i18n("Year:"), w ), 5, 0 );
  grid->addWidget( spinYear, 5, 1 );
  grid->addWidget( new TQLabel( i18n("Category:"), w ), 6, 0 );
  grid->addWidget( comboCat, 6, 1 );
  grid->setRowStretch( 7, 1 );

  d.setMainWidget(w);
  d.resize( TQMAX( TQMAX(d.sizeHint().height(), d.sizeHint().width()), 300), d.sizeHint().height() );

  if( d.exec() == TQDialog::Accepted ) {
    m_cddbInfo.cdTitle = editTitle->text();
    m_cddbInfo.cdArtist = editArtist->text();
    m_cddbInfo.cdExtInfo = editExtInfo->text();
    m_cddbInfo.category = comboCat->currentText();
    m_cddbInfo.genre = editGenre->text();
    m_cddbInfo.year = spinYear->value();

    updateDisplay();
  }
}


void K3bAudioCdView::queryCddb()
{
  TDEConfig* c = k3bcore->config();
  c->setGroup("Cddb");

  m_cddb->readConfig( c );

  if( c->readBoolEntry( "use local cddb query", true ) || 
      c->readBoolEntry( "use remote cddb", false ) ) {

    showBusyLabel(true);
 
    m_cddb->query( m_toc );
  }
}


void K3bAudioCdView::slotCddbQueryFinished( int error )
{
  if( error == K3bCddbQuery::SUCCESS ) {
    m_cddbInfo = m_cddb->result();

    // save the entry locally
    TDEConfig* c = k3bcore->config();
    c->setGroup( "Cddb" );
    if( c->readBoolEntry( "save cddb entries locally", true ) )
      m_cddb->saveEntry( m_cddbInfo );

    updateDisplay();
  }
  else if( error == K3bCddbQuery::NO_ENTRY_FOUND ) {
    if( !TDEConfigGroup( k3bcore->config(), "Cddb" ).readBoolEntry( "use remote cddb", false ) )
      K3bPassivePopup::showPopup( i18n("<p>No CDDB entry found. Enable remote CDDB queries in the K3b settings to get access "
				       "to more entries through the internet."), i18n("CDDB") );
    else
      K3bPassivePopup::showPopup( i18n("No CDDB entry found."), i18n("CDDB") );
  }
  else if( error != K3bCddbQuery::CANCELED ) {
    K3bPassivePopup::showPopup( m_cddb->errorString(), i18n("CDDB Error") );
  }

  enableInteraction(true);
}


void K3bAudioCdView::slotSaveCddbLocally()
{
  // check if the minimal info has been inserted
  if( m_cddbInfo.category.isEmpty() ) {
    KMessageBox::sorry( this, i18n("Please set the category before saving.") );
    return;
  }
    
  if( m_cddbInfo.cdTitle.isEmpty() || m_cddbInfo.cdArtist.isEmpty() ) {
    KMessageBox::sorry( this, i18n("Please set CD artist and title before saving.") );
    return;
  }

  bool missingTitle = false;
  bool missingArtist = false;
  bool allTrackArtistsEmpty = true;
  for( unsigned int i = 0; i < m_cddbInfo.titles.count(); ++i ) {
    if( m_cddbInfo.titles[i].isEmpty() )
      missingTitle = true;
    if( m_cddbInfo.artists[i].isEmpty() )
      missingArtist = true;
    if( !m_cddbInfo.artists[i].isEmpty() )
      allTrackArtistsEmpty = false;
  }
  
  if( missingTitle ||
      ( missingArtist && !allTrackArtistsEmpty ) ) {
    KMessageBox::sorry( this, i18n("Please set at least artist and title on all tracks before saving.") );
    return;
  }

  // make sure the data gets updated (bad design like a lot in the cddb stuff! :(
  m_cddbInfo.rawData.truncate(0);

  TDEConfig* c = k3bcore->config();
  c->setGroup("Cddb");

  m_cddb->readConfig( c );

  m_cddb->saveEntry( m_cddbInfo );
  K3bPassivePopup::showPopup( i18n("Saved entry (%1) in category %2.")
			      .arg(m_cddbInfo.discid)
			      .arg(m_cddbInfo.category),
			      i18n("CDDB") );
}


void K3bAudioCdView::slotCheckAll()
{
  for( TQListViewItemIterator it( m_trackView ); it.current(); ++it )
    ((AudioTrackViewItem*)it.current())->setChecked(true);
}

void K3bAudioCdView::slotUncheckAll()
{
  for( TQListViewItemIterator it( m_trackView ); it.current(); ++it )
    ((AudioTrackViewItem*)it.current())->setChecked(false);
}

void K3bAudioCdView::slotSelect()
{
  TQPtrList<TQListViewItem> items( m_trackView->selectedItems() );
  for( TQPtrListIterator<TQListViewItem> it( items );
       it.current(); ++it )
    static_cast<AudioTrackViewItem*>(it.current())->setChecked(true);
}

void K3bAudioCdView::slotDeselect()
{
  TQPtrList<TQListViewItem> items( m_trackView->selectedItems() );
  for( TQPtrListIterator<TQListViewItem> it( items );
       it.current(); ++it )
    static_cast<AudioTrackViewItem*>(it.current())->setChecked(false);
}

void K3bAudioCdView::updateDisplay()
{
  // update the listview
  for( TQListViewItemIterator it( m_trackView ); it.current(); ++it ) {
    AudioTrackViewItem* item = (AudioTrackViewItem*)it.current();
    item->updateCddbData( m_cddbInfo );
  }

  if( !m_cddbInfo.cdTitle.isEmpty() ) {
    TQString s = m_cddbInfo.cdTitle;
    if( !m_cddbInfo.cdArtist.isEmpty() )
      s += " (" + m_cddbInfo.cdArtist + ")";
    setTitle( s );
  }
  else
    setTitle( i18n("Audio CD") );

  m_labelLength->setText( i18n("1 track (%1)", 
			       "%n tracks (%1)", 
			       m_toc.count()).arg(K3b::Msf(m_toc.length()).toString()) );
}


void K3bAudioCdView::showBusyLabel( bool b )
{
  if( !b ) {
    actionCollection()->action( "start_rip" )->setEnabled( true );
    m_trackView->setEnabled( true );
    m_busyInfoLabel->hide();
  }
  else {
    // the themed label is a cut label, thus its size hint is
    // based on the cut text, we force it to be full
    m_busyInfoLabel->resize( width(), height() );
    m_busyInfoLabel->resize( m_busyInfoLabel->sizeHint() );
    int x = (width() - m_busyInfoLabel->width())/2;
    int y = (height() - m_busyInfoLabel->height())/2;
    TQRect r( TQPoint( x, y ), m_busyInfoLabel->size() );
    m_busyInfoLabel->setGeometry( r );
    m_busyInfoLabel->show();

    m_trackView->setEnabled( false );
    enableInteraction( false );
  }
}


void K3bAudioCdView::enableInteraction( bool b )
{
  // we leave the track view enabled in default disabled mode
  // since drag'n'drop to audio projects does not need an inserted CD
  actionCollection()->action( "start_rip" )->setEnabled( b );
  if( b )
    showBusyLabel( false );
}


TQDragObject* K3bAudioCdView::dragObject()
{
  TQPtrList<TQListViewItem> items = m_trackView->selectedItems();
  TQValueList<int> tracks;
  for( TQPtrListIterator<TQListViewItem> it( items );
       it.current(); ++it )
    tracks.append( static_cast<AudioTrackViewItem*>(it.current())->trackNumber );

  if( !items.isEmpty() ) {
    TQDragObject* drag = new K3bAudioCdTrackDrag( m_toc, 
						 tracks, 
						 m_cddbInfo,
						 m_device,
						 this );
    drag->setPixmap( m_trackView->createDragPixmap( items ) );
    return drag;
  }
  else
    return 0;
}

#include "k3baudiocdview.moc"
