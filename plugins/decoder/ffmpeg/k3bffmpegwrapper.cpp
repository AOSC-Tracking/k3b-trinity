/*
 *
 * $Id: k3bffmpegwrapper.cpp 641819 2007-03-12 17:29:23Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#include <config.h>

#include "k3bffmpegwrapper.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <string.h>

#include <tdelocale.h>


#if LIBAVFORMAT_BUILD < 4629
#define FFMPEG_BUILD_PRE_4629
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 64, 0)
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define AVMEDIA_TYPE_SUBTITLE CODEC_TYPE_SUBTITLE
#endif

// From libavcodec version 54.25, CodecID have been renamed to AVCodecID and all CODEC_ID_* to AV_CODEC_ID_*.
// This code can be simplyfied once all supported distros have updated to libavcodec version >=54.25
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 25, 0)
#define K3B_CODEC_ID_WMAV1 CODEC_ID_WMAV1
#define K3B_CODEC_ID_WMAV2 CODEC_ID_WMAV2
#define K3B_CODEC_ID_MP3   CODEC_ID_MP3
#define K3B_CODEC_ID_AAC   CODEC_ID_AAC
#else
#define K3B_CODEC_ID_WMAV1 AV_CODEC_ID_WMAV1
#define K3B_CODEC_ID_WMAV2 AV_CODEC_ID_WMAV2
#define K3B_CODEC_ID_MP3   AV_CODEC_ID_MP3
#define K3B_CODEC_ID_AAC   AV_CODEC_ID_AAC
#endif


K3bFFMpegWrapper* K3bFFMpegWrapper::s_instance = 0;


class K3bFFMpegFile::Private
{
public:
  AVFormatContext* formatContext;
  AVCodec* codec;

  K3b::Msf length;

  // for decoding
  char outputBuffer[192000];
  char* outputBufferPos;
  int outputBufferSize;
  AVPacket packet;
  TQ_UINT8* packetData;
  int packetSize;
};


K3bFFMpegFile::K3bFFMpegFile( const TQString& filename )
  : m_filename(filename)
{
  d = new Private;
  d->formatContext = 0;
  d->codec = 0;
}


K3bFFMpegFile::~K3bFFMpegFile()
{
  close();
  delete d;
}


bool K3bFFMpegFile::open()
{
  close();

  // open the file
# if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 2, 0)
  int err = avformat_open_input( &d->formatContext, m_filename.local8Bit(), 0, 0);
# else
  int err = av_open_input_file( &d->formatContext, m_filename.local8Bit(), 0, 0, 0);
# endif
  if( err < 0 ) {
    kdDebug() << "(K3bFFMpegFile) unable to open " << m_filename << " with error " << err << endl;
    return false;
  }

  // analyze the streams
# if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 6, 0)
  avformat_find_stream_info( d->formatContext, NULL );
# else
  av_find_stream_info( d->formatContext );
# endif

  // we only handle files containing one audio stream
  if( d->formatContext->nb_streams != 1 ) {
    kdDebug() << "(K3bFFMpegFile) more than one stream in " << m_filename << endl;
    return false;
  }

  // urgh... ugly
#ifdef FFMPEG_BUILD_PRE_4629
  AVCodecContext* codecContext =  &d->formatContext->streams[0]->codec;
#else
  AVCodecContext* codecContext =  d->formatContext->streams[0]->codec;
#endif
  if( codecContext->codec_type != AVMEDIA_TYPE_AUDIO ) {
    kdDebug() << "(K3bFFMpegFile) not a simple audio stream: " << m_filename << endl;
    return false;
  }

  // get the codec
  d->codec = avcodec_find_decoder(codecContext->codec_id);
  if( !d->codec ) {
    kdDebug() << "(K3bFFMpegFile) no codec found for " << m_filename << endl;
    return false;
  }

  // open the codec on our context
  kdDebug() << "(K3bFFMpegFile) found codec for " << m_filename << endl;
  if(
#    if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 8, 0)
     avcodec_open2( codecContext, d->codec, NULL ) < 0
#    else
     avcodec_open( codecContext, d->codec ) < 0
#    endif
    ) {
    kdDebug() << "(K3bFFMpegDecoderFactory) could not open codec." << endl;
    return false;
  }

  // determine the length of the stream
  d->length = K3b::Msf::fromSeconds( (double)d->formatContext->duration / (double)AV_TIME_BASE );

  if( d->length == 0 ) {
    kdDebug() << "(K3bFFMpegDecoderFactory) invalid length." << endl;
    return false;
  }

  // dump some debugging info
# if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 101, 0)
  av_dump_format( d->formatContext, 0, m_filename.local8Bit(), 0 );
# else
  dump_format( d->formatContext, 0, m_filename.local8Bit(), 0 );
# endif

  return true;
}


void K3bFFMpegFile::close()
{
  d->outputBufferSize = 0;
  d->packetSize = 0;
  d->packetData = 0;

  if( d->codec ) {
#ifdef FFMPEG_BUILD_PRE_4629
    avcodec_close( &d->formatContext->streams[0]->codec );
#else
    avcodec_close( d->formatContext->streams[0]->codec );
#endif
    d->codec = 0;
  }

  if( d->formatContext ) {
#   if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0)
    avformat_close_input( &d->formatContext );
#   else
    av_close_input_file( d->formatContext );
#   endif
    d->formatContext = 0;
  }
}


K3b::Msf K3bFFMpegFile::length() const
{
  return d->length;
}


int K3bFFMpegFile::sampleRate() const
{
#ifdef FFMPEG_BUILD_PRE_4629
  return d->formatContext->streams[0]->codec.sample_rate;
#else
  return d->formatContext->streams[0]->codec->sample_rate;
#endif
}


int K3bFFMpegFile::channels() const
{
#ifdef FFMPEG_BUILD_PRE_4629
  return d->formatContext->streams[0]->codec.channels;
#else
  return d->formatContext->streams[0]->codec->channels;
#endif
}


int K3bFFMpegFile::type() const
{
#ifdef FFMPEG_BUILD_PRE_4629
  return d->formatContext->streams[0]->codec.codec_id;
#else
  return d->formatContext->streams[0]->codec->codec_id;
#endif
}


TQString K3bFFMpegFile::typeComment() const
{
  switch( type() ) {
  case K3B_CODEC_ID_WMAV1:
    return i18n("Windows Media v1");
  case K3B_CODEC_ID_WMAV2:
    return i18n("Windows Media v2");
  case K3B_CODEC_ID_MP3:
    return i18n("MPEG 1 Layer III");
  case K3B_CODEC_ID_AAC:
    return i18n("Advanced Audio Coding (AAC)");
  default:
    return TQString::fromLocal8Bit( d->codec->name );
  }
}


TQString K3bFFMpegFile::title() const
{
  // FIXME: is this UTF8 or something??
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
  if( d->formatContext->title[0] != '\0' )
    return TQString::fromLocal8Bit( d->formatContext->title );
#else
  AVDictionaryEntry *entry = av_dict_get(d->formatContext->metadata, "title", NULL, 0);
  if( entry->value[0] != '\0' )
    return TQString::fromLocal8Bit( entry->value );
#endif
  else
    return TQString();
}


TQString K3bFFMpegFile::author() const
{
  // FIXME: is this UTF8 or something??
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
  if( d->formatContext->author[0] != '\0' )
    return TQString::fromLocal8Bit( d->formatContext->author );
#else
  AVDictionaryEntry *entry = av_dict_get(d->formatContext->metadata, "author", NULL, 0);
  if( entry->value[0] != '\0' )
    return TQString::fromLocal8Bit( entry->value );
#endif
  else
    return TQString();
}


TQString K3bFFMpegFile::comment() const
{
  // FIXME: is this UTF8 or something??
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
  if( d->formatContext->comment[0] != '\0' )
    return TQString::fromLocal8Bit( d->formatContext->comment );
#else
  AVDictionaryEntry *entry = av_dict_get(d->formatContext->metadata, "comment", NULL, 0);
  if( entry->value[0] != '\0' )
    return TQString::fromLocal8Bit( entry->value );
#endif
  else
    return TQString();
}


int K3bFFMpegFile::read( char* buf, int bufLen )
{
  if( fillOutputBuffer() > 0 ) {
    int len = TQMIN(bufLen, d->outputBufferSize);
    ::memcpy( buf, d->outputBufferPos, len );

    // TODO: only swap if needed
    for( int i = 0; i < len-1; i+=2 ) {
      char a = buf[i];
      buf[i] = buf[i+1];
      buf[i+1] = a;
    }

    d->outputBufferPos += len;
    d->outputBufferSize -= len;
    return len;
  }
  else
    return 0;
}


// fill d->packetData with data to decode
int K3bFFMpegFile::readPacket()
{
  if( d->packetSize <= 0 ) {
    av_init_packet( &d->packet );

    if( av_read_frame( d->formatContext, &d->packet ) < 0 ) {
      return 0;
    }

    d->packetSize = d->packet.size;
    d->packetData = d->packet.data;
  }

  return d->packetSize;
}


// decode data in d->packetData and fill d->outputBuffer
int K3bFFMpegFile::fillOutputBuffer()
{
  // decode if the output buffer is empty
  if( d->outputBufferSize <= 0 ) {

    // make sure we have data to decode
    if( readPacket() == 0 ) {
      return 0;
    }

    d->outputBufferPos = d->outputBuffer;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 64, 0)
    AVPacket avp;
    av_init_packet( &avp );
    avp.data = d->packetData;
    avp.size = d->packetSize;
#   if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 25, 0)
int len = avcodec_decode_audio4( d->formatContext->streams[0]->codec,
				    (AVFrame*)d->outputBuffer, &d->outputBufferSize,
				    &avp );
#   else
int len = avcodec_decode_audio3( d->formatContext->streams[0]->codec,
				    (short*)d->outputBuffer, &d->outputBufferSize,
				    &avp );
#   endif
#else
#ifdef FFMPEG_BUILD_PRE_4629
    int len = avcodec_decode_audio2( &d->formatContext->streams[0]->codec,
#else
    int len = avcodec_decode_audio2( d->formatContext->streams[0]->codec,
#endif
				    (short*)d->outputBuffer, &d->outputBufferSize,
				    d->packetData, d->packetSize );
#endif

    d->packetSize -= len;
    d->packetData += len;

    if( d->packetSize <= 0 )
      av_free_packet( &d->packet );
  }

  // if it is still empty try again
  if( d->outputBufferSize <= 0 )
    return fillOutputBuffer();
  else
    return d->outputBufferSize;
}


bool K3bFFMpegFile::seek( const K3b::Msf& msf )
{
  d->outputBufferSize = 0;
  d->packetSize = 0;

  double seconds = (double)msf.totalFrames()/75.0;
  TQ_UINT64 timestamp = (TQ_UINT64)(seconds * (double)AV_TIME_BASE);

  // FIXME: do we really need the start_time and why?
#if LIBAVFORMAT_BUILD >= 4619
  return ( av_seek_frame( d->formatContext, -1, timestamp + d->formatContext->start_time, 0 ) >= 0 );
#else
  return ( av_seek_frame( d->formatContext, -1, timestamp + d->formatContext->start_time ) >= 0 );
#endif
}






K3bFFMpegWrapper::K3bFFMpegWrapper()
{
  av_register_all();
}


K3bFFMpegWrapper::~K3bFFMpegWrapper()
{
  s_instance = 0;
}


K3bFFMpegWrapper* K3bFFMpegWrapper::instance()
{
  if( !s_instance ) {
    s_instance = new K3bFFMpegWrapper();
  }

  return s_instance;
}


K3bFFMpegFile* K3bFFMpegWrapper::open( const TQString& filename ) const
{
  K3bFFMpegFile* file = new K3bFFMpegFile( filename );
  if( file->open() ) {
#ifndef K3B_FFMPEG_ALL_CODECS
    //
    // only allow tested formats. ffmpeg seems not to be too reliable with every format.
    // mp3 being one of them sadly. Most importantly: allow the libsndfile decoder to do
    // its thing.
    //
    if( file->type() == K3B_CODEC_ID_WMAV1 ||
	file->type() == K3B_CODEC_ID_WMAV2 ||
	file->type() == K3B_CODEC_ID_AAC )
#endif
      return file;
  }

  delete file;
  return 0;
}
