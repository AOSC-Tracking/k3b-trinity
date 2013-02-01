/*
 *
 * $Id: k3blistview.cpp 768493 2008-01-30 08:44:05Z trueg $
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



#include "k3blistview.h"

#include "k3bmsfedit.h"

#include <tqstringlist.h>
#include <tqfontmetrics.h>
#include <tqpainter.h>
#include <tqheader.h>
#include <tqrect.h>
#include <tqpushbutton.h>
#include <tqiconset.h>
#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqlineedit.h>
#include <tqlistbox.h>
#include <tqevent.h>
#include <tqvalidator.h>
#include <tqfont.h>
#include <tqpalette.h>
#include <tqstyle.h>
#include <tqapplication.h>
#include <tqprogressbar.h>
#include <tqimage.h>

#include <kpixmapeffect.h>

#include <limits.h>



// ///////////////////////////////////////////////
//
// K3BLISTVIEWITEM
//
// ///////////////////////////////////////////////


class K3bListViewItem::ColumnInfo
{
public:
  ColumnInfo()
    : showProgress(false),
      progressValue(0),
      totalProgressSteps(100),
      margin(0),
      validator(0) {
    editorType = NONE;
    button = false;
    comboEditable = false;
    next = 0;
    fontSet = false;
    backgroundColorSet = false;
    foregroundColorSet = false;
  }

  ~ColumnInfo() {
    if( next )
      delete next;
  }

  bool button;
  int editorType;
  TQStringList comboItems;
  bool comboEditable;
  bool fontSet;
  bool backgroundColorSet;
  bool foregroundColorSet;
  TQFont font;
  TQColor backgroundColor;
  TQColor foregroundColor;
  ColumnInfo* next;

  bool showProgress;
  int progressValue;
  int totalProgressSteps;
  int margin;

  TQValidator* validator;
};



K3bListViewItem::K3bListViewItem(TQListView *parent)
  : TDEListViewItem( parent )
{
  init();
}

K3bListViewItem::K3bListViewItem(TQListViewItem *parent)
  : TDEListViewItem( parent )
{
  init();
}

K3bListViewItem::K3bListViewItem(TQListView *parent, TQListViewItem *after)
  : TDEListViewItem( parent, after )
{
  init();
}

K3bListViewItem::K3bListViewItem(TQListViewItem *parent, TQListViewItem *after)
  : TDEListViewItem( parent, after )
{
  init();
}


K3bListViewItem::K3bListViewItem(TQListView *parent,
				 const TQString& s1, const TQString& s2,
				 const TQString& s3, const TQString& s4,
				 const TQString& s5, const TQString& s6,
				 const TQString& s7, const TQString& s8)
  : TDEListViewItem( parent, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(TQListViewItem *parent,
				 const TQString& s1, const TQString& s2,
				 const TQString& s3, const TQString& s4,
				 const TQString& s5, const TQString& s6,
				 const TQString& s7, const TQString& s8)
  : TDEListViewItem( parent, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(TQListView *parent, TQListViewItem *after,
				 const TQString& s1, const TQString& s2,
				 const TQString& s3, const TQString& s4,
				 const TQString& s5, const TQString& s6,
				 const TQString& s7, const TQString& s8)
  : TDEListViewItem( parent, after, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(TQListViewItem *parent, TQListViewItem *after,
				 const TQString& s1, const TQString& s2,
				 const TQString& s3, const TQString& s4,
				 const TQString& s5, const TQString& s6,
				 const TQString& s7, const TQString& s8)
  : TDEListViewItem( parent, after, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::~K3bListViewItem()
{
  if( K3bListView* lv = dynamic_cast<K3bListView*>(listView()) )
    if( lv->currentlyEditedItem() == this )
      lv->hideEditor();

  if( m_columns )
    delete m_columns;
}


void K3bListViewItem::init()
{
  m_columns = 0;
  m_vMargin = 0;
}


int K3bListViewItem::width( const TQFontMetrics& fm, const TQListView* lv, int c ) const
{
  return TDEListViewItem::width( fm, lv, c ) + getColumnInfo(c)->margin*2;
}


void K3bListViewItem::setEditor( int column, int editor, const TQStringList& cs )
{
  ColumnInfo* colInfo = getColumnInfo(column);

  colInfo->editorType = editor;
  if( !cs.isEmpty() )
    colInfo->comboItems = cs;
}


void K3bListViewItem::setValidator( int column, TQValidator* v )
{
  getColumnInfo(column)->validator = v;
}


TQValidator* K3bListViewItem::validator( int col ) const
{
  return getColumnInfo(col)->validator;
}


void K3bListViewItem::setButton( int column, bool on )
{
  ColumnInfo* colInfo = getColumnInfo(column);

  colInfo->button = on;
}


K3bListViewItem::ColumnInfo* K3bListViewItem::getColumnInfo( int col ) const
{
  if( !m_columns )
    m_columns = new ColumnInfo();

  ColumnInfo* info = m_columns;
  int i = 0;
  while( i < col ) {
    if( !info->next )
      info->next = new ColumnInfo();
    info = info->next;
    ++i;
  }

  return info;
}


int K3bListViewItem::editorType( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->editorType;
}


bool K3bListViewItem::needButton( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->button;
}


const TQStringList& K3bListViewItem::comboStrings( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->comboItems;
}


void K3bListViewItem::setFont( int col, const TQFont& f )
{
  ColumnInfo* info = getColumnInfo( col );
  info->fontSet = true;
  info->font = f;
}


void K3bListViewItem::setBackgroundColor( int col, const TQColor& c )
{
  ColumnInfo* info = getColumnInfo( col );
  info->backgroundColorSet = true;
  info->backgroundColor = c;
  repaint();
}


void K3bListViewItem::setForegroundColor( int col, const TQColor& c )
{
 ColumnInfo* info = getColumnInfo( col );
 info->foregroundColorSet = true;
 info->foregroundColor = c;
 repaint();
}


void K3bListViewItem::setDisplayProgressBar( int col, bool displ )
{
  ColumnInfo* info = getColumnInfo( col );
  info->showProgress = displ;
}


void K3bListViewItem::setProgress( int col, int p )
{
  ColumnInfo* info = getColumnInfo( col );
  if( !info->showProgress )
    setDisplayProgressBar( col, true );
  if( info->progressValue != p ) {
    info->progressValue = p;
    repaint();
  }
}


void K3bListViewItem::setTotalSteps( int col, int steps )
{
  ColumnInfo* info = getColumnInfo( col );
  info->totalProgressSteps = steps;

  repaint();
}


void K3bListViewItem::setMarginHorizontal( int col, int margin )
{
  ColumnInfo* info = getColumnInfo( col );
  info->margin = margin;

  repaint();
}


void K3bListViewItem::setMarginVertical( int margin )
{
  m_vMargin = margin;
  repaint();
}


int K3bListViewItem::marginHorizontal( int col ) const
{
  return getColumnInfo( col )->margin;
}


int K3bListViewItem::marginVertical() const
{
  return m_vMargin;
}


void K3bListViewItem::setup()
{
  TDEListViewItem::setup();

  setHeight( height() + 2*m_vMargin );
}


void K3bListViewItem::paintCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align )
{
  ColumnInfo* info = getColumnInfo( col );

  p->save();

  TQFont oldFont( p->font() );
  TQFont newFont = info->fontSet ? info->font : oldFont;
  p->setFont( newFont );
  TQColorGroup cgh(cg);
  if( info->foregroundColorSet )
    cgh.setColor( TQColorGroup::Text, info->foregroundColor );
  if( info->backgroundColorSet )
    cgh.setColor( TQColorGroup::Base, info->backgroundColor );

  // in case this is the selected row has a margin we need to repaint the selection bar
  if( isSelected() &&
      (col == 0 || listView()->allColumnsShowFocus()) &&
      info->margin > 0 ) {

    p->fillRect( 0, 0, info->margin, height(),
		 cgh.brush( TQColorGroup::Highlight ) );
    p->fillRect( width-info->margin, 0, info->margin, height(),
		 cgh.brush( TQColorGroup::Highlight ) );
  }
  else { // in case we use the TDEListView alternate color stuff
    p->fillRect( 0, 0, info->margin, height(),
		 cgh.brush( TQColorGroup::Base ) );
    p->fillRect( width-info->margin, 0, info->margin, height(),
		 cgh.brush( TQColorGroup::Base ) );
  }

  // FIXME: the margin (we can only translate horizontally since height() is used for painting)
  p->translate( info->margin, 0 );

  if( info->showProgress ) {
    paintProgressBar( p, cgh, col, width-2*info->margin );
  }
  else {
    paintK3bCell( p, cgh, col, width-2*info->margin, align );
  }

  p->restore();
}


void K3bListViewItem::paintK3bCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align )
{
  TQListViewItem::paintCell( p, cg, col, width, align );
}


void K3bListViewItem::paintProgressBar( TQPainter* p, const TQColorGroup& cgh, int col, int width )
{
  ColumnInfo* info = getColumnInfo( col );

  TQStyle::SFlags flags = TQStyle::Style_Default;
  if( listView()->isEnabled() )
    flags |= TQStyle::Style_Enabled;
  if( listView()->hasFocus() )
    flags |= TQStyle::Style_HasFocus;

  // FIXME: the TQPainter is translated so 0, m_vMargin is the upper left of our paint rect
  TQRect r( 0, m_vMargin, width, height()-2*m_vMargin );

  // create the double buffer pixmap
  static TQPixmap *doubleBuffer = 0;
  if( !doubleBuffer )
    doubleBuffer = new TQPixmap;
  doubleBuffer->resize( width, height() );

  TQPainter dbPainter( doubleBuffer );

  // clear the background (we cannot use paintEmptyArea since it's protected in TQListView)
  if( K3bListView* lv = dynamic_cast<K3bListView*>(listView()) )
    lv->paintEmptyArea( &dbPainter, r );
  else
    dbPainter.fillRect( 0, 0, width, height(),
			cgh.brush( TQPalette::backgroundRoleFromMode(listView()->viewport()->backgroundMode()) ) );

  // we want a little additional margin
  r.setLeft( r.left()+1 );
  r.setWidth( r.width()-2 );
  r.setTop( r.top()+1 );
  r.setHeight( r.height()-2 );

  // this might be a stupid hack but most styles do not reimplement drawPrimitive PE_ProgressBarChunk
  // so this way the user is happy....
  static TQProgressBar* s_dummyProgressBar = 0;
  if( !s_dummyProgressBar ) {
    s_dummyProgressBar = new TQProgressBar();
  }

  s_dummyProgressBar->setTotalSteps( info->totalProgressSteps );
  s_dummyProgressBar->setProgress( info->progressValue );

  // some styles use the widget's geometry
  s_dummyProgressBar->setGeometry( r );

  listView()->style().drawControl(TQStyle::CE_ProgressBarContents, &dbPainter, s_dummyProgressBar, r, cgh, flags );
  listView()->style().drawControl(TQStyle::CE_ProgressBarLabel, &dbPainter, s_dummyProgressBar, r, cgh, flags );

  // now we really paint the progress in the listview
  p->drawPixmap( 0, 0, *doubleBuffer );
}







K3bCheckListViewItem::K3bCheckListViewItem(TQListView *parent)
  : K3bListViewItem( parent ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(TQListViewItem *parent)
  : K3bListViewItem( parent ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(TQListView *parent, TQListViewItem *after)
  : K3bListViewItem( parent, after ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(TQListViewItem *parent, TQListViewItem *after)
  : K3bListViewItem( parent, after ),
    m_checked(false)
{
}


bool K3bCheckListViewItem::isChecked() const
{
  return m_checked;
}


void K3bCheckListViewItem::setChecked( bool checked )
{
  m_checked = checked;
  repaint();
}


void K3bCheckListViewItem::paintK3bCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align )
{
  K3bListViewItem::paintK3bCell( p, cg, col, width, align );

  if( col == 0 ) {
    if( m_checked ) {
      TQRect r( 0, marginVertical(), width, /*listView()->style().pixelMetric( TQStyle::PM_CheckListButtonSize )*/height()-2*marginVertical() );

      TQStyle::SFlags flags = TQStyle::Style_Default;
      if( listView()->isEnabled() )
	flags |= TQStyle::Style_Enabled;
      if( listView()->hasFocus() )
	flags |= TQStyle::Style_HasFocus;
      if( isChecked() )
	flags |= TQStyle::Style_On;
      else
	flags |= TQStyle::Style_Off;

      listView()->style().tqdrawPrimitive( TQStyle::PE_CheckMark, p, r, cg, flags );
    }
  }
}






// ///////////////////////////////////////////////
//
// K3BLISTVIEW
//
// ///////////////////////////////////////////////


class K3bListView::Private
{
public:
  TQLineEdit* spinBoxLineEdit;
  TQLineEdit* msfEditLineEdit;
};


K3bListView::K3bListView( TQWidget* parent, const char* name )
  : TDEListView( parent, name ),
    m_noItemVMargin( 20 ),
    m_noItemHMargin( 20 )
{
  d = new Private;

  connect( header(), TQT_SIGNAL( sizeChange( int, int, int ) ),
	   this, TQT_SLOT( updateEditorSize() ) );

  m_editorButton = 0;
  m_editorComboBox = 0;
  m_editorSpinBox = 0;
  m_editorLineEdit = 0;
  m_editorMsfEdit = 0;
  m_currentEditItem = 0;
  m_currentEditColumn = 0;
  m_doubleClickForEdit = true;
  m_lastClickedItem = 0;
}

K3bListView::~K3bListView()
{
  delete d;
}


TQWidget* K3bListView::editor( K3bListViewItem::EditorType t ) const
{
  switch( t ) {
  case K3bListViewItem::COMBO:
    return m_editorComboBox;
  case K3bListViewItem::LINE:
    return m_editorLineEdit;
  case K3bListViewItem::SPIN:
    return m_editorSpinBox;
  case K3bListViewItem::MSF:
    return m_editorMsfEdit;
  default:
    return 0;
  }
}


void K3bListView::clear()
{
  hideEditor();
  TDEListView::clear();
}


void K3bListView::editItem( K3bListViewItem* item, int col )
{
  if( item == 0 )
    hideEditor();
  else if( item->isEnabled() ) {
    showEditor( item, col );
  }
}


void K3bListView::hideEditor()
{
  m_lastClickedItem = 0;
  m_currentEditItem = 0;
  m_currentEditColumn = 0;

  if( m_editorSpinBox )
    m_editorSpinBox->hide();
  if( m_editorLineEdit )
    m_editorLineEdit->hide();
  if( m_editorComboBox )
    m_editorComboBox->hide();
  if( m_editorButton )
    m_editorButton->hide();
  if( m_editorMsfEdit )
    m_editorMsfEdit->hide();
}

void K3bListView::showEditor( K3bListViewItem* item, int col )
{
  if( !item )
    return;

  if( item->needButton( col ) || item->editorType(col) != K3bListViewItem::NONE ) {
    m_currentEditColumn = col;
    m_currentEditItem = item;
  }

  placeEditor( item, col );
  if( item->needButton( col ) ) {
    m_editorButton->show();
  }
  switch( item->editorType(col) ) {
  case K3bListViewItem::COMBO:
    m_editorComboBox->show();
    m_editorComboBox->setFocus();
    m_editorComboBox->setValidator( item->validator(col) );
    break;
  case K3bListViewItem::LINE:
    m_editorLineEdit->show();
    m_editorLineEdit->setFocus();
    m_editorLineEdit->setValidator( item->validator(col) );
    break;
  case K3bListViewItem::SPIN:
    m_editorSpinBox->show();
    m_editorSpinBox->setFocus();
    break;
  case K3bListViewItem::MSF:
    m_editorMsfEdit->show();
    m_editorMsfEdit->setFocus();
    break;
  default:
    break;
  }
}


void K3bListView::placeEditor( K3bListViewItem* item, int col )
{
  ensureItemVisible( item );
  TQRect r = itemRect( item );

  r.setX( contentsToViewport( TQPoint(header()->sectionPos( col ), 0) ).x() );
  r.setWidth( header()->sectionSize( col ) - 1 );

  // check if the column is fully visible
  if( visibleWidth() < r.right() )
    r.setRight(visibleWidth());

  r = TQRect( viewportToContents( r.topLeft() ), r.size() );

  if( item->pixmap( col ) ) {
    r.setX( r.x() + item->pixmap(col)->width() );
  }

  // the tree-stuff is painted in the first column
  if( col == 0 ) {
    r.setX( r.x() + item->depth() * treeStepSize() );
    if( rootIsDecorated() )
      r.setX( r.x() + treeStepSize() );
  }

  if( item->needButton(col) ) {
    prepareButton( item, col );
    m_editorButton->setFixedHeight( r.height() );
    // for now we make a square button
    m_editorButton->setFixedWidth( m_editorButton->height() );
    r.setWidth( r.width() - m_editorButton->width() );
    moveChild( m_editorButton, r.right(), r.y() );
  }

  if( TQWidget* editor = prepareEditor( item, col ) ) {
    editor->resize( r.size() );
    //    editor->resize( TQSize( r.width(), editor->minimumSizeHint().height() ) );
    moveChild( editor, r.x(), r.y() );
  }
}


void K3bListView::prepareButton( K3bListViewItem*, int )
{
  if( !m_editorButton ) {
    m_editorButton = new TQPushButton( viewport() );
    connect( m_editorButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotEditorButtonClicked()) );
  }

  // TODO: do some useful things
  m_editorButton->setText( "..." );
}


TQWidget* K3bListView::prepareEditor( K3bListViewItem* item, int col )
{
  switch( item->editorType(col) ) {
  case K3bListViewItem::COMBO:
    if( !m_editorComboBox ) {
      m_editorComboBox = new TQComboBox( viewport() );
      connect( m_editorComboBox, TQT_SIGNAL(activated(const TQString&)),
	       this, TQT_SLOT(slotEditorComboBoxActivated(const TQString&)) );
      m_editorComboBox->installEventFilter( this );
    }
    m_editorComboBox->clear();
    if( item->comboStrings( col ).isEmpty() ) {
      m_editorComboBox->insertItem( item->text( col ) );
    }
    else {
      m_editorComboBox->insertStringList( item->comboStrings(col) );
      int current = item->comboStrings(col).findIndex( item->text(col) );
      if( current != -1 )
	m_editorComboBox->setCurrentItem( current );
    }
    return m_editorComboBox;

  case K3bListViewItem::LINE: {
    if( !m_editorLineEdit ) {
      m_editorLineEdit = new TQLineEdit( viewport() );
      m_editorLineEdit->setFrameStyle( TQFrame::Box | TQFrame::Plain );
      m_editorLineEdit->setLineWidth(1);
      m_editorLineEdit->installEventFilter( this );
    }

    TQString txt = item->text( col );
    m_editorLineEdit->setText( txt );

    // select the edit text (handle extensions while doing so)
    int pos = txt.findRev( '.' );
    if( pos > 0 )
      m_editorLineEdit->setSelection( 0, pos );
    else
      m_editorLineEdit->setSelection( 0, txt.length() );

    return m_editorLineEdit;
  }

  //
  // A TQSpinBox (and thus also a K3bMsfEdit) uses a TQLineEdit), thus
  // we have to use this TQLineEdit as the actual object to dead with
  //

  case K3bListViewItem::SPIN:
    if( !m_editorSpinBox ) {
      m_editorSpinBox = new TQSpinBox( viewport() );
      d->spinBoxLineEdit = static_cast<TQLineEdit*>(TQT_TQWIDGET( m_editorSpinBox->child( 0, TQLINEEDIT_OBJECT_NAME_STRING ) ));
      connect( m_editorSpinBox, TQT_SIGNAL(valueChanged(int)),
	       this, TQT_SLOT(slotEditorSpinBoxValueChanged(int)) );
      //      m_editorSpinBox->installEventFilter( this );
      d->spinBoxLineEdit->installEventFilter( this );
    }
    // set the range
    m_editorSpinBox->setValue( item->text(col).toInt() );
    return m_editorSpinBox;

  case K3bListViewItem::MSF:
    if( !m_editorMsfEdit ) {
      m_editorMsfEdit = new K3bMsfEdit( viewport() );
      d->msfEditLineEdit = static_cast<TQLineEdit*>(TQT_TQWIDGET( m_editorMsfEdit->child( 0, TQLINEEDIT_OBJECT_NAME_STRING ) ));
      connect( m_editorMsfEdit, TQT_SIGNAL(valueChanged(int)),
	       this, TQT_SLOT(slotEditorMsfEditValueChanged(int)) );
      //      m_editorMsfEdit->installEventFilter( this );
      d->msfEditLineEdit->installEventFilter( this );
    }
    m_editorMsfEdit->setText( item->text(col) );
    return m_editorMsfEdit;

  default:
    return 0;
  }
}

void K3bListView::setCurrentItem( TQListViewItem* i )
{
  if( !i || i == currentItem() )
    return;

  // I cannot remember why I did this here exactly. However, it resets the
  // m_lastClickedItem and thus invalidates the editing.
//   doRename();
//   hideEditor();
//   m_currentEditItem = 0;
  TDEListView::setCurrentItem( i );
}


void K3bListView::setNoItemText( const TQString& text )
{
  m_noItemText = text;
  triggerUpdate();
}


void K3bListView::viewportPaintEvent( TQPaintEvent* e )
{
  TDEListView::viewportPaintEvent( e );
}


// FIXME: move this to viewportPaintEvent
void K3bListView::drawContentsOffset( TQPainter * p, int ox, int oy, int cx, int cy, int cw, int ch )
{
  TDEListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );

  if( childCount() == 0 && !m_noItemText.isEmpty()) {

    p->setPen( TQt::darkGray );

    TQStringList lines = TQStringList::split( "\n", m_noItemText );
    int xpos = m_noItemHMargin;
    int ypos = m_noItemVMargin + p->fontMetrics().height();

    TQStringList::Iterator end ( lines.end() );
    for( TQStringList::Iterator str = lines.begin(); str != end; ++str ) {
      p->drawText( xpos, ypos, *str );
      ypos += p->fontMetrics().lineSpacing();
    }
  }
}

void K3bListView::paintEmptyArea( TQPainter* p, const TQRect& rect )
{
  TDEListView::paintEmptyArea( p, rect );

//   if( childCount() == 0 && !m_noItemText.isEmpty()) {

//     TQPainter pp( viewport() );
//     pp.fillRect( viewport()->rect(), viewport()->paletteBackgroundColor() );
//     pp.end();

//     p->setPen( TQt::darkGray );

//     TQStringList lines = TQStringList::split( "\n", m_noItemText );
//     int xpos = m_noItemHMargin;
//     int ypos = m_noItemVMargin + p->fontMetrics().height();

//     for( TQStringList::Iterator str = lines.begin(); str != lines.end(); str++ ) {
//       p->drawText( xpos, ypos, *str );
//       ypos += p->fontMetrics().lineSpacing();
//  }
//   }
}

void K3bListView::resizeEvent( TQResizeEvent* e )
{
  TDEListView::resizeEvent( e );
  updateEditorSize();
}


void K3bListView::updateEditorSize()
{
  if( m_currentEditItem )
    placeEditor( m_currentEditItem, m_currentEditColumn );
}


void K3bListView::slotEditorLineEditReturnPressed()
{
  if( doRename() ) {
    // edit the next line
    // TODO: add config for this
    if( K3bListViewItem* nextItem = dynamic_cast<K3bListViewItem*>( m_currentEditItem->nextSibling() ) )
      editItem( nextItem, currentEditColumn() );
    else {
      hideEditor();

      // keep the focus here
      viewport()->setFocus();
    }
  }
}


void K3bListView::slotEditorComboBoxActivated( const TQString& )
{
  doRename();
//   if( renameItem( m_currentEditItem, m_currentEditColumn, str ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, str );
//     emit itemRenamed( m_currentEditItem, str, m_currentEditColumn );
//   }
//   else {
//     for( int i = 0; i < m_editorComboBox->count(); ++i ) {
//       if( m_editorComboBox->text(i) == m_currentEditItem->text(m_currentEditColumn) ) {
// 	m_editorComboBox->setCurrentItem( i );
// 	break;
//       }
//     }
//   }
}


void K3bListView::slotEditorSpinBoxValueChanged( int )
{
//   if( renameItem( m_currentEditItem, m_currentEditColumn, TQString::number(value) ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, TQString::number(value) );
//     emit itemRenamed( m_currentEditItem, TQString::number(value), m_currentEditColumn );
//   }
//   else
//     m_editorSpinBox->setValue( m_currentEditItem->text( m_currentEditColumn ).toInt() );
}


void K3bListView::slotEditorMsfEditValueChanged( int )
{
  // FIXME: do we always need to update the value. Isn't it enough to do it at the end?
//   if( renameItem( m_currentEditItem, m_currentEditColumn, TQString::number(value) ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, TQString::number(value) );
//     emit itemRenamed( m_currentEditItem, TQString::number(value), m_currentEditColumn );
//   }
//   else
//     m_editorMsfEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
}


bool K3bListView::doRename()
{
  if( m_currentEditItem ) {
    TQString newValue;
    switch( m_currentEditItem->editorType( m_currentEditColumn ) ) {
    case K3bListViewItem::COMBO:
      newValue = m_editorComboBox->currentText();
      break;
    case K3bListViewItem::LINE:
      newValue = m_editorLineEdit->text();
      break;
    case K3bListViewItem::SPIN:
      newValue = TQString::number(m_editorSpinBox->value());
      break;
    case K3bListViewItem::MSF:
      newValue = TQString::number(m_editorMsfEdit->value());
      break;
    }

    if( renameItem( m_currentEditItem, m_currentEditColumn, newValue ) ) {
      m_currentEditItem->setText( m_currentEditColumn, newValue );
      emit itemRenamed( m_currentEditItem, newValue, m_currentEditColumn );
      return true;
    }
    else {
      switch( m_currentEditItem->editorType( m_currentEditColumn ) ) {
      case K3bListViewItem::COMBO:
	for( int i = 0; i < m_editorComboBox->count(); ++i ) {
	  if( m_editorComboBox->text(i) == m_currentEditItem->text(m_currentEditColumn) ) {
	    m_editorComboBox->setCurrentItem( i );
	    break;
	  }
	}
	break;
      case K3bListViewItem::LINE:
	m_editorLineEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
	break;
      case K3bListViewItem::SPIN:
	m_editorSpinBox->setValue( m_currentEditItem->text( m_currentEditColumn ).toInt() );
	break;
      case K3bListViewItem::MSF:
	m_editorMsfEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
	break;
      }
    }
  }


  return false;
}


void K3bListView::slotEditorButtonClicked()
{
  slotEditorButtonClicked( m_currentEditItem, m_currentEditColumn );
}


bool K3bListView::renameItem( K3bListViewItem* item, int col, const TQString& text )
{
  Q_UNUSED(item);
  Q_UNUSED(col);
  Q_UNUSED(text);
  return true;
}


void K3bListView::slotEditorButtonClicked( K3bListViewItem* item, int col )
{
  emit editorButtonClicked( item, col );
}


bool K3bListView::eventFilter( TQObject* o, TQEvent* e )
{
  if( e->type() == TQEvent::KeyPress ) {
     TQKeyEvent* ke = TQT_TQKEYEVENT(e);
     if( ke->key() == Key_Tab ) {
       if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->msfEditLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->spinBoxLineEdit) ) {
	 K3bListViewItem* lastEditItem = m_currentEditItem;

	 doRename();

	 if( lastEditItem ) {
	   // can we rename one of the other columns?
	   int col = currentEditColumn()+1;
	   while( col < columns() && lastEditItem->editorType( col ) == K3bListViewItem::NONE )
	     ++col;
	   if( col < columns() )
	     editItem( lastEditItem, col );
	   else {
	     hideEditor();

	     // keep the focus here
	     viewport()->setFocus();

	     // search for the next editable item
	     while( K3bListViewItem* nextItem =
		    dynamic_cast<K3bListViewItem*>( lastEditItem->nextSibling() ) ) {
	       // edit first column
	       col = 0;
	       while( col < columns() && nextItem->editorType( col ) == K3bListViewItem::NONE )
		 ++col;
	       if( col < columns() ) {
		 editItem( nextItem, col );
		 break;
	       }

	       lastEditItem = nextItem;
	     }
	   }
	 }

	 return true;
       }
     }
     if( ke->key() == Key_Return ||
         ke->key() == Key_Enter ) {
       if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->msfEditLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->spinBoxLineEdit) ) {
	 K3bListViewItem* lastEditItem = m_currentEditItem;
	 doRename();

	 if( K3bListViewItem* nextItem =
	     dynamic_cast<K3bListViewItem*>( lastEditItem->nextSibling() ) )
	   editItem( nextItem, currentEditColumn() );
	 else {
	   hideEditor();

	   // keep the focus here
	   viewport()->setFocus();
	 }

	 return true;
       }
     }
     else if( ke->key() == Key_Escape ) {
       if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->msfEditLineEdit) ||
	   TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->spinBoxLineEdit) ) {
	 hideEditor();

	 // keep the focus here
	 viewport()->setFocus();

	 return true;
       }
     }
  }

  else if( e->type() == TQEvent::MouseButtonPress && TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(viewport()) ) {

    // first let's grab the focus
    viewport()->setFocus();

    TQMouseEvent* me = TQT_TQMOUSEEVENT( e );
    TQListViewItem* item = itemAt( me->pos() );
    int col = header()->sectionAt( me->pos().x() );
    if( K3bCheckListViewItem* ci = dynamic_cast<K3bCheckListViewItem*>( item ) ) {
      if( col == 0 ) {
	// FIXME: improve this click area!
	ci->setChecked( !ci->isChecked() );
	return true;
      }
    }

    if( me->button() == Qt::LeftButton ) {
      if( item != m_currentEditItem || m_currentEditColumn != col ) {
	doRename();
	if( K3bListViewItem* k3bItem = dynamic_cast<K3bListViewItem*>(item) ) {
	  if( me->pos().x() > item->depth()*treeStepSize() &&
	      item->isEnabled() &&
	      (m_lastClickedItem == item || !m_doubleClickForEdit) )
	    showEditor( k3bItem, col );
	  else {
	    hideEditor();

	    // keep the focus here
	    viewport()->setFocus();
	  }
	}
	else {
	  hideEditor();

	  // keep the focus here
	  viewport()->setFocus();
	}

	// do not count clicks in the item tree for editing
	if( item && me->pos().x() > item->depth()*treeStepSize() )
	  m_lastClickedItem = item;
      }
    }
  }

  else if( e->type() == TQEvent::FocusOut ) {
    if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorLineEdit) ||
	TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->msfEditLineEdit) ||
	TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(d->spinBoxLineEdit) ||
	TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(m_editorComboBox) ) {
      // make sure we did not lose the focus to one of the edit widgets' children
      if( !tqApp->focusWidget() || TQT_BASE_OBJECT(tqApp->focusWidget()->parentWidget()) != TQT_BASE_OBJECT(o) ) {
	doRename();
	hideEditor();
      }
    }
  }

  return TDEListView::eventFilter( o, e );
}


void K3bListView::setK3bBackgroundPixmap( const TQPixmap& pix, int pos )
{
  m_backgroundPixmap = pix;
  m_backgroundPixmapPosition = pos;
}


void K3bListView::viewportResizeEvent( TQResizeEvent* e )
{
  if( !m_backgroundPixmap.isNull() ) {

    TQSize size = viewport()->size().expandedTo( TQSize( contentsWidth(), contentsHeight() ) );

    TQPixmap bgPix( size );

    // FIXME: let the user specify the color
    bgPix.fill( colorGroup().base() );

    if( bgPix.width() < m_backgroundPixmap.width() ||
	bgPix.height() < m_backgroundPixmap.height() ) {
      TQPixmap newBgPix( m_backgroundPixmap.convertToImage().scale( bgPix.size(), TQ_ScaleMin ) );
      if( m_backgroundPixmapPosition == TOP_LEFT )
	bitBlt( &bgPix, 0, 0,
		&newBgPix, 0, 0,
		newBgPix.width(), newBgPix.height() );
      else {
	int dx = bgPix.width() / 2 - m_backgroundPixmap.width() /2;
	int dy = bgPix.height() / 2 - m_backgroundPixmap.height() /2;
	bitBlt( &bgPix, dx, dy, &newBgPix, 0, 0,
		newBgPix.width(), newBgPix.height() );
      }
    }
    else {
      if( m_backgroundPixmapPosition == TOP_LEFT )
	bitBlt( &bgPix, 0, 0,
		&m_backgroundPixmap, 0, 0,
		m_backgroundPixmap.width(), m_backgroundPixmap.height() );
      else {
	int dx = bgPix.width() / 2 - m_backgroundPixmap.width() /2;
	int dy = bgPix.height() / 2 - m_backgroundPixmap.height() /2;
	bitBlt( &bgPix, dx, dy, &m_backgroundPixmap, 0, 0,
		m_backgroundPixmap.width(), m_backgroundPixmap.height() );
      }
    }

    viewport()->setPaletteBackgroundPixmap( bgPix );
  }

  TDEListView::viewportResizeEvent( e );
}


TQListViewItem* K3bListView::parentItem( TQListViewItem* item )
{
  if( !item )
    return 0;
  if( item->parent() )
    return item->parent();
  else
    return K3bListView::parentItem( item->itemAbove() );
}


KPixmap K3bListView::createDragPixmap( const TQPtrList<TQListViewItem>& items )
{
  //
  // Create drag pixmap.
  // If there are too many items fade the pixmap using the mask
  // always fade invisible items
  //
  int width = header()->width();
  int height = 0;
  for( TQPtrListIterator<TQListViewItem> it( items ); *it; ++it ) {
    TQRect r = itemRect( *it );

    if( r.isValid() ) {
      height += ( *it )->height();
    }
  }

  // now we should have a range top->bottom which includes all visible items

  // there are two situations in which we fade the pixmap on the top or the bottom:
  // 1. there are invisible items above (below) the visible
  // 2. the range is way too big

  // FIXME: how do we determine if there are invisible items outside our range?

  KPixmap pix;
  pix.resize( width, height );
  pix.fill( TQt::white );
  //  TQBitmap mask( width, bottom-top );

  // now paint all the visible items into the pixmap
  // FIXME: only paint the visible items
  TQPainter p( &pix );
  for( TQListViewItemIterator it( this ); *it; ++it ) {
    TQListViewItem* item = *it;

    // FIXME: items on other than the top level have a smaller first column
    //        the same goes for all items if root is decorated
    bool alreadyDrawing = false;
    TQRect r = itemRect( item );
    if( r.isValid() ) {
      if( items.containsRef( item ) ) {
	// paint all columns
	int x = 0;
	for( int i = 0; i < columns(); ++i ) {
	  item->paintCell( &p, colorGroup(), i, columnWidth( i ), columnAlignment( i ) );
	  p.translate( columnWidth( i ), 0 );
	  x += columnWidth( i );
	}

	p.translate( -x, item->height() );

	alreadyDrawing = true;
      }
      else if( alreadyDrawing )
	p.translate( 0, item->height() );

      if( p.worldMatrix().dy() >= pix.height() )
	break;
    }
  }

  // make it a little lighter
  KPixmapEffect::fade( pix, 0.3, TQt::white );

  // FIXME: fade the pixmap at the right side if the items are longer than width

  return pix;
}

#include "k3blistview.moc"
