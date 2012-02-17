/* 
 *
 * $Id: k3baudioplayer.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BAUDIOPLAYER_H
#define K3BAUDIOPLAYER_H

#include <klistview.h>

#include <config.h>

#ifdef WITH_ARTS
#include <arts/kmedia2.h>
#include <arts/kartsdispatcher.h>
#endif

class TQTimer;
class TQLabel;
class TQToolButton;
class TQSlider;
class TQPainter;
class TQColorGroup;
class TQDropEvent;
class TQDragObject;
class KAction;
class KActionMenu;


/**
 * Special ListViewItem for the K3bAudioPlayer playlist
 * @author Sebastian Trueg
 */
class K3bPlayListViewItem : public KListViewItem
{
 public:
  K3bPlayListViewItem( const TQString&, TQListView* parent );
  K3bPlayListViewItem( const TQString&, TQListView* parent, TQListViewItem* after );
  ~K3bPlayListViewItem();

  /** @returns the filename for the first column and the 
   *           length in format 00:00.00 for the second column
   */
  virtual TQString text( int c ) const;

  void setLength( unsigned long l ) { m_length = l; }
  unsigned long length() const { return m_length; }
  const TQString& filename() const { return m_filename; }

  /**
   * reimplemented from TQListViewItem
   * takes the m_bActive flag into account.
   */
  virtual void paintCell( TQPainter*, const TQColorGroup&, int, int, int );

  void setActive( bool a ) { m_bActive = a; }

 protected:
  /** path to the associated file */
  TQString m_filename;

  /** length in frames (1/75 second) */
  unsigned long m_length;

  bool m_bActive;
};



/**
 * Playlistview just needed to accept 
 * url drags
 */ 
class K3bPlayListView : public KListView
{
Q_OBJECT
  

 public:
  K3bPlayListView( TQWidget* parent = 0, const char* name = 0 );
  ~K3bPlayListView();

 protected:
  bool acceptDrag( TQDropEvent* e ) const;
  TQDragObject* dragObject();
};




/**
 * @author Sebastian Trueg
 */
class K3bAudioPlayer : public TQWidget
{
Q_OBJECT
  

 public: 
  K3bAudioPlayer( TQWidget* parent = 0, const char* name = 0 );
  ~K3bAudioPlayer();

  bool supportsMimetype( const TQString& mimetype );

  /**
   * length of current playing in seconds
   */
  long length();

  /**
   * current position in seconds
   */
  long position();

  /**
   * EMPTY - no file loaded
   */
  enum player_state { PLAYING, PAUSED, STOPPED, EMPTY };

  int state();

 signals:
  void started( const TQString& filename );
  void started();
  void stopped();
  void paused();
  void ended();

 public slots:
  void playFile( const TQString& filename );
  void playFiles( const TQStringList& files );
  void enqueueFile( const TQString& filename );
  void enqueueFiles( const TQStringList& files );

  /** clears the playlist */
  void clear();
  void play();
  void forward();
  void back();
  void stop();
  void pause();
  void seek( long pos );
  void seek( int pos );

/*  protected: */
/*   void dragEnterEvent( TQDragEnterEvent* e ); */
/*   void dropEvent( TQDropEvent* e ); */

 private slots:
  void slotCheckEnd();
  void slotUpdateDisplay();
  void slotUpdateCurrentTime( int time );
  void slotUpdateLength( long time );
  void slotUpdateFilename();
  void slotPlayItem( TQListViewItem* item );
  void slotDropped( TQDropEvent* e, TQListViewItem* after );

  /**
   * set the actual item. Will set m_currentItem and 
   * handle highlighting of the current item
   */
  void setCurrentItem( TQListViewItem* item );
  void slotRemoveSelected();
  void slotShowContextMenu( KListView*, TQListViewItem* item, const TQPoint& p );

 private:
#ifdef WITH_ARTS
  Arts::PlayObject m_playObject;
  KArtsDispatcher m_dispatcher;
#endif
  TQString m_filename;

  TQLabel* m_labelFilename;
  TQLabel* m_labelCurrentTime;
  TQLabel* m_labelOverallTime;
  
  TQToolButton* m_buttonPlay;
  TQToolButton* m_buttonPause;
  TQToolButton* m_buttonStop;
  TQToolButton* m_buttonForward;
  TQToolButton* m_buttonBack;
  
  K3bPlayListView* m_viewPlayList;
  
  TQSlider* m_seekSlider;
  
  TQTimer* m_updateTimer;

  K3bPlayListViewItem* m_currentItem;

  bool m_bLengthReady;

  KAction* m_actionRemove;
  KAction* m_actionClear;
  KActionMenu* m_contextMenu;
};


#endif
