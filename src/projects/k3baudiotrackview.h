/* 
 *
 * $Id: k3baudiotrackview.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_AUDIO_TRACK_VIEW_H_
#define _K3B_AUDIO_TRACK_VIEW_H_

#include <k3blistview.h>

#include <tqmap.h>
#include <tqptrlist.h>
#include <kurl.h>

class K3bAudioTrack;
class K3bAudioTrackViewItem;
class K3bAudioDataSource;
class K3bAudioDoc;
class KActionCollection;
class KAction;
class TQDropEvent;
class TQKeyEvent;
class TQFocusEvent;
class TQMouseEvent;
class TQDragMoveEvent;
class TQTimer;
class KPopupMenu;
class TQPainter;
class K3bListViewItemAnimator;
class K3bAudioTrackPlayer;


class K3bAudioTrackView : public K3bListView
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bAudioTrackView( K3bAudioDoc*, TQWidget* tqparent, const char* name = 0 );
  ~K3bAudioTrackView();

  KActionCollection* actionCollection() const { return m_actionCollection; }

  K3bAudioTrackPlayer* player() const { return m_player; }

  void getSelectedItems( TQPtrList<K3bAudioTrack>& tracks, 
			 TQPtrList<K3bAudioDataSource>& sources );

 public slots:
  void showPlayerIndicator( K3bAudioTrack* );
  void togglePauseIndicator( bool b );
  void removePlayerIndicator();

 private:
  void setupColumns();
  void setupActions();
  void showAllSources();
  K3bAudioTrackViewItem* findTrackItem( const TQPoint& pos ) const;
  K3bAudioTrackViewItem* getTrackViewItem( K3bAudioTrack* track, bool* isNew = 0 );

  K3bAudioDoc* m_doc;

  KAction* m_actionProperties;
  KAction* m_actionRemove;
  KAction* m_actionAddSilence;
  KAction* m_actionMergeTracks;
  KAction* m_actionSplitSource;
  KAction* m_actionSplitTrack;
  KAction* m_actionEditSource;
  KAction* m_actionPlayTrack;
  KActionCollection* m_actionCollection;

  bool m_updatingColumnWidths;

  TQMap<K3bAudioTrack*, K3bAudioTrackViewItem*> m_trackItemMap;

  K3bAudioTrackViewItem* m_currentMouseOverItem;
  TQTimer* m_autoOpenTrackTimer;
  TQTimer* m_animationTimer;

  KPopupMenu* m_popupMenu;

  K3bAudioTrackPlayer* m_player;

  // used for the audiotrackplayer indicator
  K3bAudioTrack* m_currentlyPlayingTrack;

  // to animate the player icon
  K3bListViewItemAnimator* m_playerItemAnimator;

  // used for the drop-event hack
  KURL::List m_dropUrls;
  K3bAudioTrack* m_dropTrackAfter;
  K3bAudioTrack* m_dropTrackParent;
  K3bAudioDataSource* m_dropSourceAfter;

 private slots:
  void slotAnimation();
  void slotDropped( TQDropEvent* e, TQListViewItem* tqparent, TQListViewItem* after );
  void slotChanged();
  void slotTrackChanged( K3bAudioTrack* );
  void slotTrackRemoved( K3bAudioTrack* );
  void slotDragTimeout();

  // action slots
  void slotAddSilence();
  void slotRemove();
  void slotMergeTracks();
  void slotSplitSource();
  void slotSplitTrack();
  void showPopupMenu( KListView*, TQListViewItem* item, const TQPoint& pos );
  void slotProperties();
  void slotPlayTrack();
  void slotQueryMusicBrainz();
  void slotEditSource();

  // drop-event hack slot
  void slotAddUrls();

 protected:
  void keyPressEvent( TQKeyEvent* e );
  void keyReleaseEvent( TQKeyEvent* e );
  void focusOutEvent( TQFocusEvent* e );
  void contentsMouseMoveEvent( TQMouseEvent* e );
  void contentsDragMoveEvent( TQDragMoveEvent* e );
  void contentsDragLeaveEvent( TQDragLeaveEvent* e );
  void resizeEvent( TQResizeEvent* e );
  void resizeColumns();
  bool acceptDrag(TQDropEvent* e) const;
  TQDragObject* dragObject();
};

#endif
