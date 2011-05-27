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

#ifndef _K3B_THEME_MANAGER_H_
#define _K3B_THEME_MANAGER_H_

#include <tqobject.h>
#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqmap.h>
#include <tqcolor.h>
#include <tqpixmap.h>



class KConfigBase;


class K3bTheme
{
 public:
  K3bTheme();

  TQColor backgroundColor() const;
  TQColor foregroundColor() const;

  enum PixmapType {
    MEDIA_AUDIO,      /**< Media information header, right side when showing an audio CD. */
    MEDIA_DATA,       /**< Media information header, right side when showing a data media. */
    MEDIA_VIDEO,      /**< Media information header, right side when showing a video media. */
    MEDIA_EMPTY,      /**< Media information header, right side when showing an empty media. */
    MEDIA_MIXED,      /**< Media information header, right side when showing a mixed mode CD. */
    MEDIA_NONE,       /**< Media information header, right side default pixmap (no media). */
    MEDIA_LEFT,       /**< Media information header, left side. */
    PROGRESS_WORKING, /**< Progress dialog, left top while working. */
    PROGRESS_SUCCESS, /**< Progress dialog, left top on success. */
    PROGRESS_FAIL,    /**< Progress dialog, left top on failure. */
    PROGRESS_RIGHT,   /**< Progress dialog, right top. */
    DIALOG_LEFT,      /**< Action dialog, left top. */
    DIALOG_RIGHT,     /**< Action dialog, right top. */
    SPLASH,           /**< K3b splash screen. Size not important. */
    PROJECT_LEFT,     /**< Project header left side. */
    PROJECT_RIGHT,    /**< Project header right side. */
    WELCOME_BG        /**< Background pixmap of the welcome window. */
  };

  enum BackgroundMode {
    BG_TILE,         /**< Keep the pixmap's size and tile the welcome widget */
    BG_SCALE         /**< Scale the pixmap to fill the welcome widget. */
  };

  const TQPixmap& pixmap( PixmapType ) const;

  /**
   * \deprecated use pixmap( PixmapType )
   */
  const TQPixmap& pixmap( const TQString& name ) const;

  BackgroundMode backgroundMode() const;

  const TQString& name() const { return m_name; }
  const TQString& author() const { return m_author; }
  const TQString& comment() const { return m_comment; }
  const TQString& version() const { return m_version; }

  /**
   * Global themes are installed for all users and cannot be deleted.
   */
  bool global() const { return !local(); }

  /**
   * Local themes are installed in the user's home directory and can be deleted.
   */
  bool local() const { return m_local; }

  const TQString& path() const { return m_path; }

  static TQString filenameForPixmapType( PixmapType );

 private:
  TQString m_path;
  bool m_local;
  TQString m_name;
  TQString m_author;
  TQString m_comment;
  TQString m_version;
  TQColor m_bgColor;
  TQColor m_fgColor;
  BackgroundMode m_bgMode;

  mutable TQMap<TQString, TQPixmap> m_pixmapMap;

  TQPixmap m_emptyPixmap;

  friend class K3bThemeManager;
};


class K3bThemeManager : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

 public:
  K3bThemeManager( TQObject* tqparent = 0, const char* name = 0 );
  ~K3bThemeManager();

  const TQValueList<K3bTheme*>& themes() const;

  /**
   * This is never null. If no theme could be found an empty dummy theme
   * will be returnes which does not contains any pixmaps.
   */
  K3bTheme* currentTheme() const;
  K3bTheme* findTheme( const TQString& ) const;

 signals:
  void themeChanged();
  void themeChanged( K3bTheme* );

 public slots:
  void readConfig( KConfigBase* );
  void saveConfig( KConfigBase* );
  void setCurrentTheme( const TQString& );
  void setCurrentTheme( K3bTheme* );
  void loadThemes();

 private:
  void loadTheme( const TQString& name );

  class Private;
  Private* d;
};

#endif
