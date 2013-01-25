/* 
 *
 * $Id: k3bmovixdoc.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_MOVIX_DOC_H_
#define _K3B_MOVIX_DOC_H_


#include <k3bdatadoc.h>

#include <tqptrlist.h>
#include "k3b_export.h"
//class K3bView;
class KURL;
class TQDomElement;
class K3bFileItem;
class K3bMovixFileItem;
class K3bDataItem;
class TDEConfig;


class LIBK3B_EXPORT K3bMovixDoc : public K3bDataDoc
{
  Q_OBJECT
  

 public:
  K3bMovixDoc( TQObject* parent = 0 );
  virtual ~K3bMovixDoc();

  virtual int type() const { return MOVIX; }

  virtual K3bBurnJob* newBurnJob( K3bJobHandler* hdl, TQObject* parent );

  bool newDocument();

  const TQPtrList<K3bMovixFileItem>& movixFileItems() const { return m_movixFiles; }

  int indexOf( K3bMovixFileItem* );


  bool shutdown() const { return m_shutdown; }
  bool reboot() const { return m_reboot; }
  bool ejectDisk() const { return m_ejectDisk; }
  bool randomPlay() const { return m_randomPlay; }
  const TQString& subtitleFontset() const { return m_subtitleFontset; }
  const TQString& bootMessageLanguage() const { return m_bootMessageLanguage; }
  const TQString& audioBackground() const { return m_audioBackground; }
  const TQString& keyboardLayout() const { return m_keyboardLayout; }
  const TQStringList& codecs() const { return m_codecs; }
  const TQString& defaultBootLabel() const { return m_defaultBootLabel; }
  const TQString& additionalMPlayerOptions() const { return m_additionalMPlayerOptions; }
  const TQString& unwantedMPlayerOptions() const { return m_unwantedMPlayerOptions; }
  int loopPlaylist() const { return m_loopPlaylist; }
  bool noDma() const { return m_noDma; }

  void setShutdown( bool v ) { m_shutdown = v; }
  void setReboot( bool v ) { m_reboot = v; }
  void setEjectDisk( bool v ) { m_ejectDisk = v; }
  void setRandomPlay( bool v ) { m_randomPlay = v; }
  void setSubtitleFontset( const TQString& v ) { m_subtitleFontset = v; }
  void setBootMessageLanguage( const TQString& v ) { m_bootMessageLanguage = v; }
  void setAudioBackground( const TQString& b ) { m_audioBackground = b; }
  void setKeyboardLayout( const TQString& l ) { m_keyboardLayout = l; }
  void setCodecs( const TQStringList& c ) { m_codecs = c; }
  void setDefaultBootLabel( const TQString& v ) { m_defaultBootLabel = v; }
  void setAdditionalMPlayerOptions( const TQString& v ) { m_additionalMPlayerOptions = v; }
  void setUnwantedMPlayerOptions( const TQString& v ) { m_unwantedMPlayerOptions = v; }
  void setLoopPlaylist( int v ) { m_loopPlaylist = v; }
  void setNoDma( bool b ) { m_noDma = b; }

 signals:
  void newMovixFileItems();
  void movixItemRemoved( K3bMovixFileItem* );
  void subTitleItemRemoved( K3bMovixFileItem* );

 public slots:
  void addUrls( const KURL::List& urls );
  void addMovixFile( const KURL& url, int pos = -1 );
  void moveMovixItem( K3bMovixFileItem* item, K3bMovixFileItem* itemAfter );
  void addSubTitleItem( K3bMovixFileItem*, const KURL& );
  void removeSubTitleItem( K3bMovixFileItem* );

 protected:
  /** reimplemented from K3bDoc */
  bool loadDocumentData( TQDomElement* root );
  /** reimplemented from K3bDoc */
  bool saveDocumentData( TQDomElement* );

  virtual TQString typeString() const { return "movix"; }

 private slots:
  void slotDataItemRemoved( K3bDataItem* );

 private:
  TQPtrList<K3bMovixFileItem> m_movixFiles;

  bool m_shutdown;
  bool m_reboot;
  bool m_ejectDisk;
  bool m_randomPlay;
  TQString m_subtitleFontset;
  TQString m_bootMessageLanguage;
  TQString m_audioBackground;
  TQString m_keyboardLayout;
  TQStringList m_codecs;
  TQString m_defaultBootLabel;
  TQString m_additionalMPlayerOptions;
  TQString m_unwantedMPlayerOptions;
  int m_loopPlaylist;
  bool m_noDma;
};

#endif
