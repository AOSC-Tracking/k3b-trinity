/* 
 *
 * $Id: k3baudiodoc.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BAUDIODOC_H
#define K3BAUDIODOC_H

#include <k3bdoc.h>

#include <k3bcdtext.h>
#include <k3btoc.h>

#include <tqptrlist.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdatetime.h>
#include <tqtextstream.h>
#include "k3b_export.h"
#include <kurl.h>

class K3bApp;
class K3bAudioTrack;
class TQWidget;
class TQTimer;
class TQDomDocument;
class TQDomElement;
class K3bThreadJob;
class KConfig;
class K3bAudioDataSource;
class K3bAudioDecoder;
class K3bAudioFile;

/**Document class for an audio project. 
 *@author Sebastian Trueg
 */

class LIBK3B_EXPORT K3bAudioDoc : public K3bDoc  
{
  Q_OBJECT
  

  friend class K3bMixedDoc;
  friend class K3bAudioTrack;
  friend class K3bAudioFile;
	
 public:
  K3bAudioDoc( TQObject* );
  ~K3bAudioDoc();

  TQString name() const;
	
  bool newDocument();

  bool hideFirstTrack() const { return m_hideFirstTrack; }
  int numOfTracks() const;

  bool normalize() const { return m_normalize; }

  K3bAudioTrack* firstTrack() const;
  K3bAudioTrack* lastTrack() const;

  /**
   * Slow.
   * \return the K3bAudioTrack with track number trackNum starting at 1 or 0 if trackNum > numOfTracks()
   */
  K3bAudioTrack* getTrack( unsigned int trackNum );

  /**
   * Creates a new audiofile inside this doc which has no track yet.
   */
  K3bAudioFile* createAudioFile( const KURL& url );

  /** get the current size of the project */
  KIO::filesize_t size() const;
  K3b::Msf length() const;
	
  // CD-Text
  bool cdText() const { return m_cdText; }
  const TQString& title() const { return m_cdTextData.title(); }
  const TQString& artist() const { return m_cdTextData.performer(); }
  const TQString& disc_id() const { return m_cdTextData.discId(); }
  const TQString& arranger() const { return m_cdTextData.arranger(); }
  const TQString& songwriter() const { return m_cdTextData.songwriter(); }
  const TQString& composer() const { return m_cdTextData.composer(); }
  const TQString& upc_ean() const { return m_cdTextData.upcEan(); }
  const TQString& cdTextMessage() const { return m_cdTextData.message(); }

  /**
   * Create complete CD-Text including the tracks' data.
   */
  K3bDevice::CdText cdTextData() const;

  int audioRippingParanoiaMode() const { return m_audioRippingParanoiaMode; }
  int audioRippingRetries() const { return m_audioRippingRetries; }
  bool audioRippingIgnoreReadErrors() const { return m_audioRippingIgnoreReadErrors; }

  /**
   * Represent the structure of the doc as CD Table of Contents.
   */
  K3bDevice::Toc toToc() const;

  K3bBurnJob* newBurnJob( K3bJobHandler*, TQObject* parent = 0 );

  /**
   * Shows dialogs.
   */
  void informAboutNotFoundFiles();
  
  /**
   * returns the new after track, ie. the the last added track or null if
   * the import failed.
   *
   * This is a blocking method.
   *
   * \param cuefile The Cuefile to be imported
   * \param after   The track after which the new tracks should be inserted
   * \param decoder The decoder to be used for the new tracks. If 0 a new one will be created.
   *
   * BE AWARE THAT THE DECODER HAS TO FIT THE AUDIO FILE IN THE CUE.
   */
  K3bAudioTrack* importCueFile( const TQString& cuefile, K3bAudioTrack* after, K3bAudioDecoder* decoder = 0 );

  /**
   * Create a decoder for a specific url. If another AudioFileSource with this
   * url is already part of this project the associated decoder is returned.
   *
   * In the first case the decoder will not be initialized yet (K3bAudioDecoder::analyseFile
   * is not called yet).
   *
   * \param url The url for which a decoder is requested.
   * \param reused If not null this variable is set to true if the decoder is already in
   *               use and K3bAudioDecoder::analyseFile() does not have to be called anymore.
   */
  K3bAudioDecoder* getDecoderForUrl( const KURL& url, bool* reused = 0 );

  static bool readPlaylistFile( const KURL& url, KURL::List& playlist );

 public slots:
  void addUrls( const KURL::List& );
  void addTrack( const KURL&, uint );
  void addTracks( const KURL::List&, uint );
  /** 
   * Adds a track without any testing 
   *
   * Slow because it uses getTrack.
   */
  void addTrack( K3bAudioTrack* track, uint position = 0 );

  void addSources( K3bAudioTrack* parent, const KURL::List& urls, K3bAudioDataSource* sourceAfter = 0 );

  void removeTrack( K3bAudioTrack* );
  void moveTrack( K3bAudioTrack* track, K3bAudioTrack* after );

  void setHideFirstTrack( bool b ) { m_hideFirstTrack = b; }
  void setNormalize( bool b ) { m_normalize = b; }

  // CD-Text
  void writeCdText( bool b ) { m_cdText = b; }
  void setTitle( const TQString& v );
  void setArtist( const TQString& v );
  void setPerformer( const TQString& v );
  void setDisc_id( const TQString& v );
  void setArranger( const TQString& v );
  void setSongwriter( const TQString& v );
  void setComposer( const TQString& v );
  void setUpc_ean( const TQString& v );
  void setCdTextMessage( const TQString& v );

  // Audio-CD Ripping
  void setAudioRippingParanoiaMode( int i ) { m_audioRippingParanoiaMode = i; }
  void setAudioRippingRetries( int r ) { m_audioRippingRetries = r; }
  void setAudioRippingIgnoreReadErrors( bool b ) { m_audioRippingIgnoreReadErrors = b; }

  void removeCorruptTracks();

 private slots:
  void slotTrackChanged( K3bAudioTrack* );
  void slotTrackRemoved( K3bAudioTrack* );

 signals:
  void trackChanged( K3bAudioTrack* );
  void trackRemoved( K3bAudioTrack* );

 protected:
  /** reimplemented from K3bDoc */
  bool loadDocumentData( TQDomElement* );
  /** reimplemented from K3bDoc */
  bool saveDocumentData( TQDomElement* );

  TQString typeString() const;

 private:
  // the stuff for adding files
  // ---------------------------------------------------------
  K3bAudioTrack* createTrack( const KURL& url );

  /**
   * Handle directories and M3u files
   */
  KURL::List extractUrlList( const KURL::List& urls );
  // ---------------------------------------------------------

  /**
   * Used by K3bAudioTrack to update the track list
   */
  void setFirstTrack( K3bAudioTrack* track );
  /**
   * Used by K3bAudioTrack to update the track list
   */
  void setLastTrack( K3bAudioTrack* track );

  /**
   * Used by K3bAudioFile to tell the doc that it does not need the decoder anymore.
   */
  void decreaseDecoderUsage( K3bAudioDecoder* );
  void increaseDecoderUsage( K3bAudioDecoder* );

  K3bAudioTrack* m_firstTrack;
  K3bAudioTrack* m_lastTrack;
 	
  bool m_hideFirstTrack;
  bool m_normalize;

  KURL::List m_notFoundFiles;
  KURL::List m_unknownFileFormatFiles;

  // CD-Text
  // --------------------------------------------------
  K3bDevice::CdText m_cdTextData;
  bool m_cdText;
  // --------------------------------------------------

  // Audio ripping
  int m_audioRippingParanoiaMode;
  int m_audioRippingRetries;
  bool m_audioRippingIgnoreReadErrors;

  //
  // decoder housekeeping
  // --------------------------------------------------
  // used to check if we may delete a decoder
  TQMap<K3bAudioDecoder*, int> m_decoderUsageCounterMap;
  // used to check if we already have a decoder for a specific file
  TQMap<TQString, K3bAudioDecoder*> m_decoderPresenceMap;

  class Private;
  Private* d;
};


#endif
