/* 
 *
 * $Id: k3blistview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BLISTVIEW_H
#define K3BLISTVIEW_H


#include <klistview.h>
#include "k3b_export.h"
#include <tqptrvector.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <kpixmap.h>

class TQPainter;
class TQPushButton;
class TQIconSet;
class TQResizeEvent;
class TQComboBox;
class TQSpinBox;
class TQLineEdit;
class TQEvent;
class TQValidator;
class K3bMsfEdit;

class K3bListView;


class LIBK3B_EXPORT K3bListViewItem : public TDEListViewItem
{
 public:
  K3bListViewItem(TQListView *parent);
  K3bListViewItem(TQListViewItem *parent);
  K3bListViewItem(TQListView *parent, TQListViewItem *after);
  K3bListViewItem(TQListViewItem *parent, TQListViewItem *after);

  K3bListViewItem(TQListView *parent,
		  const TQString&, const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString());

  K3bListViewItem(TQListViewItem *parent,
		  const TQString&, const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString());

  K3bListViewItem(TQListView *parent, TQListViewItem *after,
		  const TQString&, const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString());

  K3bListViewItem(TQListViewItem *parent, TQListViewItem *after,
		  const TQString&, const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString(),
		  const TQString& = TQString(), const TQString& = TQString());

  virtual ~K3bListViewItem();

  /**
   * reimplemented from TDEListViewItem
   */
  void setup();

  virtual int width( const TQFontMetrics& fm, const TQListView* lv, int c ) const;

  void setEditor( int col, int type, const TQStringList& = TQStringList() );
  void setButton( int col, bool );

  void setValidator( int col, TQValidator* v );
  TQValidator* validator( int col ) const;

  int editorType( int col ) const;
  bool needButton( int col ) const;
  const TQStringList& comboStrings( int col ) const;

  enum EditorType { NONE, COMBO, LINE, SPIN, MSF };

  void setFont( int col, const TQFont& f );
  void setBackgroundColor( int col, const TQColor& );
  void setForegroundColor( int col, const TQColor& );

  void setDisplayProgressBar( int col, bool );
  void setProgress( int, int );
  void setTotalSteps( int col, int steps );

  /**
   * The margin left and right of the cell
   */
  void setMarginHorizontal( int col, int margin );

  /**
   * The top and button margin of the cell
   */
  void setMarginVertical( int margin );

  int marginHorizontal( int col ) const;
  int marginVertical() const;

  /**
   * Do not reimplement this but paintK3bCell to use the margin and background stuff.
   */
  virtual void paintCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align );

  virtual void paintK3bCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align );

 private:
  void paintProgressBar( TQPainter* p, const TQColorGroup& cgh, int col, int width );

  class ColumnInfo;
  mutable ColumnInfo* m_columns;

  ColumnInfo* getColumnInfo( int ) const;
  void init();

  int m_vMargin;
};


class LIBK3B_EXPORT K3bCheckListViewItem : public K3bListViewItem
{
 public:
  K3bCheckListViewItem(TQListView *parent);
  K3bCheckListViewItem(TQListViewItem *parent);
  K3bCheckListViewItem(TQListView *parent, TQListViewItem *after);
  K3bCheckListViewItem(TQListViewItem *parent, TQListViewItem *after);

  virtual bool isChecked() const;
  virtual void setChecked( bool checked );

 protected:
  virtual void paintK3bCell( TQPainter* p, const TQColorGroup& cg, int col, int width, int align );

 private:
  bool m_checked;
};



class LIBK3B_EXPORT K3bListView : public TDEListView
{
  friend class K3bListViewItem;

  Q_OBJECT
  

 public:
  K3bListView (TQWidget *parent = 0, const char *name = 0);
  virtual ~K3bListView();

  virtual void setCurrentItem( TQListViewItem* );

  K3bListViewItem* currentlyEditedItem() const { return m_currentEditItem; }

  TQWidget* editor( K3bListViewItem::EditorType ) const;

  enum BgPixPosition {
    TOP_LEFT,
    CENTER 
  };

  /**
   * This will set a background pixmap which is not tiled.
   * @param pos position on the viewport.
   */
  void setK3bBackgroundPixmap( const TQPixmap&, int pos = CENTER );

  /**
   * Create a faded pixmap showing the items.
   */
  KPixmap createDragPixmap( const TQPtrList<TQListViewItem>& items );

  /**
   * Searches for the first item above @p i which is one level higher.
   * For 1st level items this will always be the listview's root item.
   */
  static TQListViewItem* parentItem( TQListViewItem* i );

 signals:
  void editorButtonClicked( K3bListViewItem*, int );

 public slots:
  void setNoItemText( const TQString& );
  //  void setNoItemPixmap( const TQPixmap& );
  void setNoItemVerticalMargin( int i ) { m_noItemVMargin = i; }
  void setNoItemHorizontalMargin( int i ) { m_noItemHMargin = i; }
  void setDoubleClickForEdit( bool b ) { m_doubleClickForEdit = b; }
  void hideEditor();
  void editItem( K3bListViewItem*, int );

  virtual void clear();

 private slots:
  void updateEditorSize();
  virtual void slotEditorLineEditReturnPressed();
  virtual void slotEditorComboBoxActivated( const TQString& );
  virtual void slotEditorSpinBoxValueChanged( int );
  virtual void slotEditorMsfEditValueChanged( int );
  virtual void slotEditorButtonClicked();

 protected slots:
  void showEditor( K3bListViewItem*, int col );
  void placeEditor( K3bListViewItem*, int col );

  /**
   * This is called whenever one of the editor's contents changes
   * the default implementation just returnes true
   *
   * FIXME: should be called something like mayRename
   */
  virtual bool renameItem( K3bListViewItem*, int, const TQString& );

  /**
   * default impl just emits signal
   * editorButtonClicked(...)
   */
  virtual void slotEditorButtonClicked( K3bListViewItem*, int );

 protected:
  /**
   * calls TDEListView::drawContentsOffset
   * and paints a the noItemText if no item is in the list
   */
  virtual void drawContentsOffset ( TQPainter * p, int ox, int oy, int cx, int cy, int cw, int ch );
  virtual void resizeEvent( TQResizeEvent* );
  virtual void paintEmptyArea( TQPainter*, const TQRect& rect );

  /**
   * Reimplemented for internal reasons.
   *
   * Further reimplementations should call this function or else some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportResizeEvent( TQResizeEvent* );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportPaintEvent(TQPaintEvent*);

  virtual bool eventFilter( TQObject*, TQEvent* );

  K3bListViewItem* currentEditItem() const { return m_currentEditItem; }
  int currentEditColumn() const { return m_currentEditColumn; }

 private:
  TQWidget* prepareEditor( K3bListViewItem* item, int col );
  void prepareButton( K3bListViewItem* item, int col );
  bool doRename();

  TQString m_noItemText;
  //  TQPixmap m_noItemPixmap;
  int m_noItemVMargin;
  int m_noItemHMargin;

  K3bListViewItem* m_currentEditItem;
  int m_currentEditColumn;

  bool m_doubleClickForEdit;
  TQListViewItem* m_lastClickedItem;

  TQPushButton* m_editorButton;
  TQComboBox* m_editorComboBox;
  TQSpinBox* m_editorSpinBox;
  TQLineEdit* m_editorLineEdit;
  K3bMsfEdit* m_editorMsfEdit;

  TQPixmap m_backgroundPixmap;
  int m_backgroundPixmapPosition;

  class Private;
  Private* d;
};


#endif
