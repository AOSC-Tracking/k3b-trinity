/*
 *
 * $Id: k3bwelcomewidget.cpp 676186 2007-06-16 08:53:46Z trueg $
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

#include "k3bwelcomewidget.h"
#include "k3b.h"
#include "k3bflatbutton.h"
#include <k3bstdguiitems.h>
#include "k3bapplication.h"
#include <k3bversion.h>
#include "k3bthememanager.h"

#include <tqpixmap.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tqsimplerichtext.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqtooltip.h>
#include <tqcursor.h>
#include <tqimage.h>

#include <kurl.h>
#include <kurldrag.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <kiconloader.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdepopupmenu.h>
#include <tdeaboutdata.h>
#include <tdeactionclasses.h>


static const char* s_allActions[] = {
  "file_new_data",
  "file_new_dvd",
  "file_continue_multisession",
  "_sep_",
  "file_new_audio",
  "_sep_",
  "file_new_mixed",
  "_sep_",
  "file_new_vcd",
  "file_new_video_dvd",
  "_sep_",
  "file_new_movix",
  "file_new_movix_dvd",
  "_sep_",
  "tools_copy_cd",
  "tools_copy_dvd",
  "_sep_",
  "tools_blank_cdrw",
  "tools_format_dvd",
  "_sep_",
  "tools_write_cd_image",
  "tools_write_dvd_iso",
  "_sep_",
  "tools_cdda_rip",
  "tools_videodvd_rip",
  "tools_videocd_rip",
  0
};

K3bWelcomeWidget::Display::Display( K3bWelcomeWidget* parent )
  : TQWidget( parent->viewport() )
{
  setWFlags( TQt::WNoAutoErase );

  TQFont fnt(font());
  fnt.setBold(true);
  fnt.setPointSize( 16 );
  m_header = new TQSimpleRichText( i18n("Welcome to K3b - The CD and DVD Kreator"), fnt );
  m_infoText = new TQSimpleRichText( TQString::fromUtf8("<qt align=\"center\">K3b %1 (c) 1999 - 2007 Sebastian Trüg")
				    .arg(kapp->aboutData()->version()), font() );

  // set a large width just to be sure no linebreak occurs
  m_header->setWidth( 800 );

  setAcceptDrops( true );
  setBackgroundMode( PaletteBase );
  m_rows = m_cols = 1;

  m_buttonMore = new K3bFlatButton( i18n("Further actions..."), this );
  connect( m_buttonMore, TQ_SIGNAL(pressed()), parent, TQ_SLOT(slotMoreActions()) );

  connect( k3bappcore->themeManager(), TQ_SIGNAL(themeChanged()), this, TQ_SLOT(slotThemeChanged()) );

  slotThemeChanged();
}


K3bWelcomeWidget::Display::~Display()
{
  delete m_header;
  delete m_infoText;
}


void K3bWelcomeWidget::Display::addAction( TDEAction* action )
{
  if( action ) {
    m_actions.append(action);
    rebuildGui();
  }
}


void K3bWelcomeWidget::Display::removeAction( TDEAction* action )
{
  if( action ) {
    m_actions.removeRef( action );
    rebuildGui();
  }
}


void K3bWelcomeWidget::Display::removeButton( K3bFlatButton* b )
{
  removeAction( m_buttonMap[b] );
}


void K3bWelcomeWidget::Display::rebuildGui( const TQPtrList<TDEAction>& actions )
{
  m_actions = actions;
  rebuildGui();
}


static void calculateButtons( int width, int numActions, int buttonWidth, int& cols, int& rows )
{
  // always try to avoid horizontal scrollbars
  int wa = width - 40;
  cols = TQMAX( 1, TQMIN( wa / (buttonWidth+4), numActions ) );
  rows = numActions/cols;
  int over = numActions%cols;
  if( over ) {
    rows++;
    // try to avoid useless cols
    while( over && cols - over - 1 >= rows-1 ) {
      --cols;
      over = numActions%cols;
    }
  }
}


void K3bWelcomeWidget::Display::rebuildGui()
{
  // step 1: delete all old buttons in the buttons TQPtrList<K3bFlatButton>
  m_buttonMap.clear();
  m_buttons.setAutoDelete(true);
  m_buttons.clear();

  int numActions = m_actions.count();
  if( numActions > 0 ) {

    // create buttons
    for( TQPtrListIterator<TDEAction> it( m_actions ); it.current(); ++it ) {
      TDEAction* a = it.current();

      K3bFlatButton* b = new K3bFlatButton( a, this );

      m_buttons.append( b );
      m_buttonMap.insert( b, a );
    }

    // determine the needed button size (since all buttons should be equal in size
    // we use the max of all sizes)
    m_buttonSize = m_buttons.first()->sizeHint();
    for( TQPtrListIterator<K3bFlatButton> it( m_buttons ); it.current(); ++it ) {
      m_buttonSize = m_buttonSize.expandedTo( it.current()->sizeHint() );
    }

    repositionButtons();
  }
}


void K3bWelcomeWidget::Display::repositionButtons()
{
  // calculate rows and columns
  calculateButtons( width(), m_actions.count(), m_buttonSize.width(), m_cols, m_rows );

  int availHor = width() - 40;
  int availVert = height() - 20 - 10 - m_header->height() - 10;
  availVert -= m_infoText->height() - 10;
  int leftMargin = 20 + (availHor - (m_buttonSize.width()+4)*m_cols)/2;
  int topOffset = m_header->height() + 20 + ( availVert - (m_buttonSize.height()+4)*m_rows - m_buttonMore->height() )/2;

  int row = 0;
  int col = 0;

  for( TQPtrListIterator<K3bFlatButton> it( m_buttons ); it.current(); ++it ) {
    K3bFlatButton* b = it.current();

    b->setGeometry( TQRect( TQPoint( leftMargin + (col*(m_buttonSize.width()+4) + 2 ),
				   topOffset + (row*(m_buttonSize.height()+4)) + 2 ),
			   m_buttonSize ) );
    b->show();

    col++;
    if( col == m_cols ) {
      col = 0;
      row++;
    }
  }
  if( col > 0 )
    ++row;

  m_buttonMore->setGeometry( TQRect( TQPoint( leftMargin + 2,
					    topOffset + (row*(m_buttonSize.height()+4)) + 2 ),
				    TQSize( m_cols*(m_buttonSize.width()+4) - 4, m_buttonMore->height() ) ) );
}


TQSizePolicy K3bWelcomeWidget::Display::sizePolicy () const
{
  return TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Minimum, true );
}


int K3bWelcomeWidget::Display::heightForWidth( int w ) const
{
  int ow = m_infoText->width();
  m_infoText->setWidth( w );
  int h = m_infoText->height();
  m_infoText->setWidth( ow );

  int cols, rows;
  calculateButtons( w, m_actions.count(), m_buttonSize.width(), cols, rows );

  return (20 + m_header->height() + 20 + 10 + ((m_buttonSize.height()+4)*rows) + 4 + m_buttonMore->height() + 10 + h + 20);
}


TQSize K3bWelcomeWidget::Display::minimumSizeHint() const
{
  TQSize size( TQMAX(40+m_header->widthUsed(), 40+m_buttonSize.width()),
	      20 + m_header->height() + 20 + 10 + m_buttonSize.height() + 10 + m_infoText->height() + 20 );

  return size;
}


void K3bWelcomeWidget::Display::resizeEvent( TQResizeEvent* e )
{
  m_infoText->setWidth( width() - 20 );
  TQWidget::resizeEvent(e);
  repositionButtons();
  if( e->size() != m_bgPixmap.size() )
    updateBgPix();
}


void K3bWelcomeWidget::Display::slotThemeChanged()
{
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() )
    if( theme->backgroundMode() == K3bTheme::BG_SCALE )
      m_bgImage = theme->pixmap( K3bTheme::WELCOME_BG ).convertToImage();

  updateBgPix();
  update();
}

#include "fastscale/scale.h"
void K3bWelcomeWidget::Display::updateBgPix()
{
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    if( theme->backgroundMode() == K3bTheme::BG_SCALE )
      m_bgPixmap.convertFromImage( ImageUtils::scale( m_bgImage, rect().width(), rect().height(), ImageUtils::SMOOTH_FAST ) );
    else
      m_bgPixmap = theme->pixmap( K3bTheme::WELCOME_BG );
  }
}


void K3bWelcomeWidget::Display::paintEvent( TQPaintEvent* )
{
  if( K3bTheme* theme = k3bappcore->themeManager()->currentTheme() ) {
    TQPainter p( this );
    p.setPen( theme->foregroundColor() );

    // draw the background including first filling with the bg color for transparent images
    p.fillRect( rect(), theme->backgroundColor() );
    p.drawTiledPixmap( rect(), m_bgPixmap );

    // rect around the header
    TQRect rect( 10, 10, TQMAX( m_header->widthUsed() + 20, width() - 20 ), m_header->height() + 20 );
    p.fillRect( rect, theme->backgroundColor() );
    p.drawRect( rect );

    // big rect around the whole thing
    p.drawRect( 10, 10, width()-20, height()-20 );

    // draw the header text
    TQColorGroup grp( colorGroup() );
    grp.setColor( TQColorGroup::Text, theme->foregroundColor() );
    int pos = 20;
    pos += TQMAX( (width()-40-m_header->widthUsed())/2, 0 );
    m_header->draw( &p, pos, 20, TQRect(), grp );

    // draw the info box
    //    int boxWidth = 20 + m_infoText->widthUsed();
    int boxHeight = 10 + m_infoText->height();
    TQRect infoBoxRect( 10/*TQMAX( (width()-20-m_infoText->widthUsed())/2, 10 )*/,
		       height()-10-boxHeight,
		       width()-20/*boxWidth*/,
		       boxHeight );
    p.fillRect( infoBoxRect, theme->backgroundColor() );
    p.drawRect( infoBoxRect );
    m_infoText->draw( &p, infoBoxRect.left()+5, infoBoxRect.top()+5, TQRect(), grp );
  }
}


void K3bWelcomeWidget::Display::dragEnterEvent( TQDragEnterEvent* event )
{
  event->accept( KURLDrag::canDecode(event) );
}


void K3bWelcomeWidget::Display::dropEvent( TQDropEvent* e )
{
  KURL::List urls;
  KURLDrag::decode( e, urls );
  emit dropped( urls );
}



K3bWelcomeWidget::K3bWelcomeWidget( K3bMainWindow* mw, TQWidget* parent, const char* name )
  : TQScrollView( parent, name ),
    m_mainWindow( mw )
{
  main = new Display( this );
  addChild( main );

  connect( main, TQ_SIGNAL(dropped(const KURL::List&)), m_mainWindow, TQ_SLOT(addUrls(const KURL::List&)) );

  connect( kapp, TQ_SIGNAL(appearanceChanged()), main, TQ_SLOT(update()) );
}


K3bWelcomeWidget::~K3bWelcomeWidget()
{
}


void K3bWelcomeWidget::loadConfig( TDEConfigBase* c )
{
  TQStringList sl = TDEConfigGroup( c, "Welcome Widget" ).readListEntry( "welcome_actions" );

  if( sl.isEmpty() ) {
    sl.append( "file_new_audio" );
    sl.append( "file_new_data" );
    sl.append( "file_new_dvd" );
    sl.append( "tools_copy_cd" );
    sl.append( "tools_write_cd_image" );
    sl.append( "tools_write_dvd_iso" );
  }

  TQPtrList<TDEAction> actions;
  for( TQStringList::const_iterator it = sl.begin(); it != sl.end(); ++it )
    if( TDEAction* a = m_mainWindow->actionCollection()->action( (*it).latin1() ) )
      actions.append(a);

  main->rebuildGui( actions );

  fixSize();
}


void K3bWelcomeWidget::saveConfig( TDEConfigBase* c )
{
  TDEConfigGroup grp( c, "Welcome Widget" );

  TQStringList sl;
  for( TQPtrListIterator<TDEAction> it( main->m_actions ); it.current(); ++it )
    sl.append( it.current()->name() );

  grp.writeEntry( "welcome_actions", sl );
}


void K3bWelcomeWidget::resizeEvent( TQResizeEvent* e )
{
  TQScrollView::resizeEvent( e );
  fixSize();
}


void K3bWelcomeWidget::showEvent( TQShowEvent* e )
{
  TQScrollView::showEvent( e );
  fixSize();
}


void K3bWelcomeWidget::fixSize()
{
  TQSize s = contentsRect().size();
  s.setWidth( TQMAX( main->minimumSizeHint().width(), s.width() ) );
  s.setHeight( TQMAX( main->heightForWidth(s.width()), s.height() ) );

  main->resize( s );
  viewport()->resize( s );
}


void K3bWelcomeWidget::contentsMousePressEvent( TQMouseEvent* e )
{
  if( e->button() == TQt::RightButton ) {
    TQMap<int, TDEAction*> map;
    TDEPopupMenu addPop;

    for ( int i = 0; s_allActions[i]; ++i ) {
        if ( s_allActions[i][0] != '_' ) {
            TDEAction* a = m_mainWindow->actionCollection()->action( s_allActions[i] );
            if ( a && !main->m_actions.containsRef(a) ) {
                map.insert( addPop.insertItem( a->iconSet(), a->text() ), a );
            }
        }
    }

    // menu identifiers in TQt are always < 0 (when automatically generated)
    // and unique throughout the entire application!
    int r = 0;
    int removeAction = 0;

    TQWidget* widgetAtPos = viewport()->childAt(e->pos());
    if( widgetAtPos && widgetAtPos->inherits( "K3bFlatButton" ) ) {
      TDEPopupMenu pop;
      removeAction = pop.insertItem( SmallIcon("remove"), i18n("Remove Button") );
      if ( addPop.count() > 0 )
          pop.insertItem( i18n("Add Button"), &addPop );
      pop.insertSeparator();
      r = pop.exec( e->globalPos() );
    }
    else {
      addPop.insertTitle( i18n("Add Button"), -1, 0 );
      addPop.insertSeparator();
      r = addPop.exec( e->globalPos() );
    }

    if( r != 0 ) {
      if( r == removeAction )
	main->removeButton( static_cast<K3bFlatButton*>(widgetAtPos) );
      else
	main->addAction( map[r] );
    }

    fixSize();
  }
}


void K3bWelcomeWidget::slotMoreActions()
{
  TDEPopupMenu popup;

  for ( int i = 0; s_allActions[i]; ++i ) {
      if ( s_allActions[i][0] == '_' ) {
          (new TDEActionSeparator( &popup ))->plug( &popup );
      }
      else {
          m_mainWindow->actionCollection()->action( s_allActions[i] )->plug( &popup );
      }
  }

  popup.exec( TQCursor::pos() );
}

#include "k3bwelcomewidget.moc"
