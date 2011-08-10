/* 
 *
 * $Id: k3bcdtext.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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


#ifndef _K3B_CDTEXT_H_
#define _K3B_CDTEXT_H_

#include <tqstring.h>
#include <tqvaluevector.h>
#include "k3bdevice_export.h"

namespace K3bDevice
{
  struct cdtext_pack;

  class TrackCdText
    {
      friend class Device;
      
    public:
      TrackCdText() {
      }

      void clear() {
	m_title.truncate(0);
	m_performer.truncate(0);
	m_songwriter.truncate(0);
	m_composer.truncate(0);
	m_arranger.truncate(0);
	m_message.truncate(0);
	m_isrc.truncate(0);
      }

      const TQString& title() const { return m_title; }
      const TQString& performer() const { return m_performer; }
      const TQString& songwriter() const { return m_songwriter; }
      const TQString& composer() const { return m_composer; }
      const TQString& arranger() const { return m_arranger; }
      const TQString& message() const { return m_message; }
      const TQString& isrc() const { return m_isrc; }

      // TODO: use the real CD-TEXT charset (a modified ISO8859-1)
      void setTitle( const TQString& s ) { m_title = s; fixup(m_title); }
      void setPerformer( const TQString& s ) { m_performer = s; fixup(m_performer); }
      void setSongwriter( const TQString& s ) { m_songwriter = s; fixup(m_songwriter); }
      void setComposer( const TQString& s ) { m_composer = s; fixup(m_composer); }
      void setArranger( const TQString& s ) { m_arranger = s; fixup(m_arranger); }
      void setMessage( const TQString& s ) { m_message = s; fixup(m_message); }
      void setIsrc( const TQString& s ) { m_isrc = s; fixup(m_isrc); }

      bool isEmpty() const {
	if( !m_title.isEmpty() )
	  return false;
	if( !m_performer.isEmpty() )
	  return false;
	if( !m_songwriter.isEmpty() )
	  return false;
	if( !m_composer.isEmpty() )
	  return false;
	if( !m_arranger.isEmpty() )
	  return false;
	if( !m_message.isEmpty() )
	  return false;
	if( !m_isrc.isEmpty() )
	  return false;

	return true;
      }

      bool operator==( const TrackCdText& ) const;
      bool operator!=( const TrackCdText& ) const;

    private:
      // TODO: remove this (see above)
      void fixup( TQString& s ) { s.replace( '/', "_" ); s.replace( '\"', "_" ); }

      TQString m_title;
      TQString m_performer;
      TQString m_songwriter;
      TQString m_composer;
      TQString m_arranger;
      TQString m_message;
      TQString m_isrc;

      friend class CdText;
    };

  class LIBK3BDEVICE_EXPORT CdText : public TQValueVector<TrackCdText>
    {
      friend class Device;

    public:
      CdText();
      CdText( const unsigned char* data, int len );
      CdText( const TQByteArray& );
      CdText( int size );
      CdText( const CdText& );

      void setRawPackData( const unsigned char*, int );
      void setRawPackData( const TQByteArray& );

      TQByteArray rawPackData() const;

      bool empty() const {
	if( !m_title.isEmpty() )
	  return false;
	if( !m_performer.isEmpty() )
	  return false;
	if( !m_songwriter.isEmpty() )
	  return false;
	if( !m_composer.isEmpty() )
	  return false;
	if( !m_arranger.isEmpty() )
	  return false;
	if( !m_message.isEmpty() )
	  return false;
	if( !m_discId.isEmpty() )
	  return false;
	if( !m_upcEan.isEmpty() )
	  return false;
	
	for( unsigned int i = 0; i < count(); ++i )
	  if( !at(i).isEmpty() )
	    return false;

	return true;
      }

      bool isEmpty() const {
	return empty();
      }

      void clear();

      const TQString& title() const { return m_title; }
      const TQString& performer() const { return m_performer; }
      const TQString& songwriter() const { return m_songwriter; }
      const TQString& composer() const { return m_composer; }
      const TQString& arranger() const { return m_arranger; }
      const TQString& message() const { return m_message; }
      const TQString& discId() const { return m_discId; }
      const TQString& upcEan() const { return m_upcEan; }

      // TODO: use the real CD-TEXT charset (a modified ISO8859-1)
      void setTitle( const TQString& s ) { m_title = s; fixup(m_title); }
      void setPerformer( const TQString& s ) { m_performer = s; fixup(m_performer); }
      void setSongwriter( const TQString& s ) { m_songwriter = s; fixup(m_songwriter); }
      void setComposer( const TQString& s ) { m_composer = s; fixup(m_composer); }
      void setArranger( const TQString& s ) { m_arranger = s; fixup(m_arranger); }
      void setMessage( const TQString& s ) { m_message = s; fixup(m_message); }
      void setDiscId( const TQString& s ) { m_discId = s; fixup(m_discId); }
      void setUpcEan( const TQString& s ) { m_upcEan = s; fixup(m_upcEan); }

      void debug() const;

      /**
       * Returns false if found a crc error in the raw cdtext block or it has a
       * wrong length.
       */
      static bool checkCrc( const unsigned char*, int );
      static bool checkCrc( const TQByteArray& );

      bool operator==( const CdText& ) const;
      bool operator!=( const CdText& ) const;
	
    private:
      // TODO: remove this (see above)
      void fixup( TQString& s ) { s.replace( '/', "_" ); s.replace( '\"', "_" ); }

      const TQString& textForPackType( int packType, unsigned int track ) const;
      unsigned int textLengthForPackType( int packType ) const;
      TQByteArray createPackData( int packType, unsigned int& ) const;
      void savePack( cdtext_pack* pack, TQByteArray& data, unsigned int& dataFill ) const;
      void appendByteArray( TQByteArray& a, const TQByteArray& b ) const;

      TQString m_title;
      TQString m_performer;
      TQString m_songwriter;
      TQString m_composer;
      TQString m_arranger;
      TQString m_message;
      TQString m_discId;
      TQString m_upcEan;
    };

  TQCString encodeCdText( const TQString& s, bool* illegalChars = 0 );
}

#endif
