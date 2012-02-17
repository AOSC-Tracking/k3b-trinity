/* 
 *
 * $Id: sourceheader 380067 2005-01-19 13:03:46Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_DATA_URL_ADDING_DIALOG_H_
#define _K3B_DATA_URL_ADDING_DIALOG_H_

#include <kdialogbase.h>
#include <kurl.h>
#include <tqstringlist.h>
#include <tqpair.h>
#include <tqdir.h>

class KProgress;
class TQLabel;
class K3bDataItem;
class K3bDirItem;
class K3bEncodingConverter;
class K3bDirSizeJob;
class K3bDataDoc;


class K3bDataUrlAddingDialog : public KDialogBase
{
  Q_OBJECT
  

 public:
  ~K3bDataUrlAddingDialog();

  /**
   * \return \see TQDialog::exec()
   */
  static int addUrls( const KURL::List& urls, K3bDirItem* dir = 0,
		      TQWidget* parent = 0 );

  static int moveItems( const TQValueList<K3bDataItem*>& items, K3bDirItem* dir,
			TQWidget* parent = 0 );

  static int copyItems( const TQValueList<K3bDataItem*>& items, K3bDirItem* dir,
			TQWidget* parent = 0 );

  static int copyMoveItems( const TQValueList<K3bDataItem*>& items, K3bDirItem* dir,
			    TQWidget* parent, bool copy );

 private slots:
  void slotAddUrls();
  void slotCopyMoveItems();
  void slotCancel();
  void slotDirSizeDone( bool );
  void updateProgress();

 private:
  K3bDataUrlAddingDialog( K3bDataDoc* doc, TQWidget* parent = 0, const char* name = 0 );

  bool getNewName( const TQString& oldName, K3bDirItem* dir, TQString& newName );

  bool addHiddenFiles();
  bool addSystemFiles();

  TQString resultMessage() const;

  KProgress* m_progressWidget;
  TQLabel* m_infoLabel;
  TQLabel* m_counterLabel;
  K3bEncodingConverter* m_encodingConverter;

  KURL::List m_urls;
  TQValueList< TQPair<KURL, K3bDirItem*> > m_urlQueue;

  TQValueList< TQPair<K3bDataItem*, K3bDirItem*> > m_items;

  TQValueList<KURL> m_dirSizeQueue;

  bool m_bExistingItemsReplaceAll;
  bool m_bExistingItemsIgnoreAll;
  bool m_bFolderLinksFollowAll;
  bool m_bFolderLinksAddAll;
  int m_iAddHiddenFiles;
  int m_iAddSystemFiles;

  TQStringList m_unreadableFiles;
  TQStringList m_notFoundFiles;
  TQStringList m_nonLocalFiles;
  TQStringList m_tooBigFiles;
  TQStringList m_mkisofsLimitationRenamedFiles;
  TQStringList m_invalidFilenameEncodingFiles;

  bool m_bCanceled;

  bool m_copyItems;

  KIO::filesize_t m_totalFiles;
  KIO::filesize_t m_filesHandled;
  K3bDirSizeJob* m_dirSizeJob;

  unsigned int m_lastProgress;
};

#endif
