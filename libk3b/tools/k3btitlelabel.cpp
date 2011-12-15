/* 
 *
 * $Id: k3btitlelabel.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3btitlelabel.h"

#include <k3bstringutils.h>

#include <tqpainter.h>
#include <tqevent.h>
#include <tqfontmetrics.h>
#include <tqfont.h>
#include <tqtooltip.h>


class K3bTitleLabel::Private
{
public:
  Private() {
    titleLength = subTitleLength = 0;
    margin = 2;
    alignment = TQt::AlignLeft;
    cachedMinimumWidth = 0;
    titleBaseLine = 0;
  }

  TQString title;
  TQString subTitle;

  TQString displayTitle;
  TQString displaySubTitle;

  int alignment;

  int titleLength;
  int subTitleLength;
  int displayTitleLength;
  int displaySubTitleLength;
  int titleBaseLine;
  int subTitleBaseLine;
  int margin;

  int cachedMinimumWidth;
};


class K3bTitleLabel::ToolTip : public TQToolTip
{
public:
  ToolTip( K3bTitleLabel* label )
    : TQToolTip( label ),
      m_label(label) {
  }
  
  void maybeTip( const TQPoint &pos ) {
    TQRect r = m_label->contentsRect();

    int neededWidth = m_label->d->displayTitleLength;
    if( !m_label->d->displaySubTitle.isEmpty() )
      neededWidth += m_label->d->displaySubTitleLength + 5;

    int startPos = 0;
    if( m_label->d->alignment & TQt::AlignHCenter )
      startPos = r.left() + ( r.width() - 2*m_label->d->margin - neededWidth ) / 2;
    else if( m_label->d->alignment & TQt::AlignRight )
      startPos = r.right() - m_label->d->margin - neededWidth;
    else
      startPos = r.left() + m_label->d->margin;
    
    TQRect titleTipRect( startPos, 0, m_label->d->displayTitleLength, m_label->height() );
    TQRect subTitleTipRect( startPos + m_label->d->displayTitleLength, 0, m_label->d->displaySubTitleLength, m_label->height() );

    if( titleTipRect.contains( pos ) &&
	m_label->d->displayTitle != m_label->d->title )
      tip( titleTipRect, m_label->d->title );
    else if( subTitleTipRect.contains( pos ) &&
	m_label->d->displaySubTitle != m_label->d->subTitle )
      tip( subTitleTipRect, m_label->d->subTitle );
  }

  K3bTitleLabel* m_label;
};



K3bTitleLabel::K3bTitleLabel( TQWidget* parent, const char* name )
  : TQFrame( parent, name )
{
  d = new Private();
  m_toolTip = new ToolTip( this );
}


K3bTitleLabel::~K3bTitleLabel()
{
  delete m_toolTip;
  delete d;
}


void K3bTitleLabel::setTitle( const TQString& title, const TQString& subTitle )
{
  d->title = title;
  d->subTitle = subTitle;
  updatePositioning();
  update();
}


void K3bTitleLabel::setSubTitle( const TQString& subTitle )
{
  d->subTitle = subTitle;
  updatePositioning();
  update();
}


void K3bTitleLabel::setAlignment( int align )
{
  d->alignment = align;
  update();
}


TQSize K3bTitleLabel::sizeHint() const
{
  return TQSize( d->titleLength + d->subTitleLength + 2*d->margin, d->titleBaseLine );
}

TQSize K3bTitleLabel::minimumSizeHint() const
{
  return TQSize( d->cachedMinimumWidth, d->titleBaseLine );
}

void K3bTitleLabel::resizeEvent( TQResizeEvent* e )
{
  TQFrame::resizeEvent( e );
  updatePositioning();
  update();
}

void K3bTitleLabel::drawContents( TQPainter* p )
{
  p->save();

  TQRect r = contentsRect();
  p->eraseRect( r );

  TQFont f(font());
  f.setBold(true);
  f.setPointSize( f.pointSize() + 2 );

  p->setFont(f);

  int neededWidth = d->displayTitleLength;
  if( !d->displaySubTitle.isEmpty() )
    neededWidth += d->displaySubTitleLength + 5;

  int startPos = 0;
  if( d->alignment & TQt::AlignHCenter )
    startPos = r.left() + ( r.width() - 2*d->margin - neededWidth ) / 2;
  else if( d->alignment & TQt::AlignRight )
    startPos = r.right() - d->margin - neededWidth;
  else
    startPos = r.left() + d->margin;

  // paint title
  p->drawText( startPos, r.top() + d->titleBaseLine, d->displayTitle );

  if( !d->subTitle.isEmpty() ) {
    f.setBold(false);
    f.setPointSize( f.pointSize() - 4 );
    p->setFont(f);
    p->drawText( startPos + d->displayTitleLength + 5, r.top() + d->subTitleBaseLine, d->displaySubTitle );
  }

  p->restore();
}


void K3bTitleLabel::setMargin( int m )
{
  d->margin = m;
  updatePositioning();
  update();
}


void K3bTitleLabel::updatePositioning()
{
  TQFont f(font());
  f.setBold(true);
  f.setPointSize( f.pointSize() + 2 );
  TQFontMetrics titleFm(f);

  f.setBold(false);
  f.setPointSize( f.pointSize() - 4 );
  TQFontMetrics subTitleFm(f);

  d->titleBaseLine = contentsRect().height()/2 + titleFm.height()/2 - titleFm.descent();
  d->titleLength = titleFm.width( d->title );

  d->subTitleBaseLine = d->titleBaseLine - titleFm.underlinePos() + subTitleFm.underlinePos();

  d->subTitleLength = ( d->subTitle.isEmpty() ? 0 : subTitleFm.width( d->subTitle ) );

  // cut the text to window width
  d->displayTitle = d->title;
  d->displaySubTitle = d->subTitle;
  int widthAvail = contentsRect().width() - 2*margin();

  // 5 pix spacing between title and subtitle
  if( !d->subTitle.isEmpty() )
    widthAvail -= 5;

  if( d->titleLength > widthAvail/2 ) {
    if( d->subTitleLength <= widthAvail/2 )
      d->displayTitle = K3b::cutToWidth( titleFm, d->title, widthAvail - d->subTitleLength );
    else
      d->displayTitle = K3b::cutToWidth( titleFm, d->title, widthAvail/2 );
  }
  if( d->subTitleLength > widthAvail/2 ) {
    if( d->titleLength <= widthAvail/2 )
      d->displaySubTitle = K3b::cutToWidth( subTitleFm, d->subTitle, widthAvail - d->titleLength );
    else
      d->displaySubTitle = K3b::cutToWidth( subTitleFm, d->subTitle, widthAvail/2 );
  }

  d->displayTitleLength = titleFm.width( d->displayTitle );
  d->displaySubTitleLength = subTitleFm.width( d->displaySubTitle );


  //
  // determine the minimum width for the minumum size hint
  //
  d->cachedMinimumWidth = 2*d->margin;
  
  TQString cutTitle = d->title;
  if( cutTitle.length() > 2 ) {
    cutTitle.truncate( 2 );
    cutTitle += "...";
  }
  TQString cutSubTitle = d->subTitle;
  if( cutSubTitle.length() > 2 ) {
    cutSubTitle.truncate( 2 );
    cutSubTitle += "...";
  }

  d->cachedMinimumWidth += titleFm.width( cutTitle ) + subTitleFm.width( cutSubTitle );
  // 5 pix spacing between title and subtitle
  if( !d->subTitle.isEmpty() )
    d->cachedMinimumWidth += 5;
}

#include "k3btitlelabel.moc"
