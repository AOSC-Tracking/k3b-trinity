/* 
 *
 * $Id: k3bdiroperator.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDIROPERATOR_H
#define K3BDIROPERATOR_H

#include <kdiroperator.h>
#include <kbookmarkmanager.h>

class TQIconViewItem;
class TQListViewItem;
class KBookmarkMenu;
class KActionMenu;



/**
  *@author Sebastian Trueg
  */
class K3bDirOperator : public KDirOperator, public KBookmarkOwner
{
  Q_OBJECT
  TQ_OBJECT

 public: 
  K3bDirOperator( const KURL& urlName = KURL(), TQWidget* parent = 0, const char* name = 0 );
  ~K3bDirOperator();

  /**
   * reimplemented from KDirOperator
   */
  void readConfig( KConfig* cfg, const TQString& group );

  /**
   * reimplemented from KDirOperator
   */
  void writeConfig( KConfig* cfg, const TQString& group );

  /**
   * reimplemented from KBookmarkOwner
   */
  void openBookmarkURL( const TQString& url );

  /**
   * reimplemented from KBookmarkOwner
   */
  TQString currentTitle() const;

  /**
   * reimplemented from KBookmarkOwner
   */
  TQString currentURL() const;

  KActionMenu* bookmarkMenu() const { return m_bmPopup; }

 public slots:
  void slotAddFilesToProject();

 protected slots:
  /**
   * reimplemented from KDirOperator
   */
  void activatedMenu( const KFileItem*, const TQPoint& );

 private:
  KBookmarkMenu* m_bmMenu;
  KActionMenu* m_bmPopup;
};

#endif
