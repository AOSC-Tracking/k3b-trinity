/*
 *
 *
 * Copyright (C) 2004-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include <config.h>

#include "k3bffmpegwrapper.h"
#include <tdelocale.h>

extern "C" {
/*
 Recent versions of FFmpeg uses C99 constant macros which are not present in C++
 standard. The macro __STDC_CONSTANT_MACROS allow C++ to use these macros.
 Although it's not defined by C++ standard it's supported by many
 implementations. See bug 236036 and discussion:
 https://lists.ffmpeg.org/pipermail/ffmpeg-devel/2010-May/095488.html
 */
#define __STDC_CONSTANT_MACROS
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <math.h>
#include <string.h>

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(52, 101, 0)
#define av_dump_format(c, x, f, y) dump_format(c, x, f, y)
#endif
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
//      this works because the parameters/options are not used
#define avformat_open_input(c, s, f, o) av_open_input_file(c, s, f, 0, o)
#endif
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 6, 0)
#define avformat_find_stream_info(c, o) av_find_stream_info(c)
#endif
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 17, 0)
#define avformat_close_input(c) av_close_input_file(*c)
#endif
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 41, 100)
#define codecpar codec
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 45, 101)
#define av_frame_alloc avcodec_alloc_frame
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 28, 0)
#define av_frame_free(f) av_free(*(f))
#elif LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 45, 101)
#define av_frame_free  avcodec_free_frame
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 64, 0)
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#define AVMEDIA_TYPE_SUBTITLE CODEC_TYPE_SUBTITLE
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
#define avcodec_open2(a, c, o) avcodec_open(a, c)
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 25, 0)
// From libavcodec version 54.25, CodecID have been renamed to AVCodecID and all
// CODEC_ID_* to AV_CODEC_ID_*. This code can be simplified once all supported
// distros have updated to libavcodec version >=54.25
#define AV_CODEC_ID_WMAV1 CODEC_ID_WMAV1
#define AV_CODEC_ID_WMAV2 CODEC_ID_WMAV2
#define AV_CODEC_ID_MP3 CODEC_ID_MP3
#define AV_CODEC_ID_AAC CODEC_ID_AAC
#define AV_CODEC_ID_APE CODEC_ID_APE
#define AV_CODEC_ID_WAVPACK CODEC_ID_WAVPACK
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,39,101)
#define av_packet_unref av_free_packet
#endif

// TODO:  most of the used av_functions there are deprecated and there
//        are troubles with improper frame/packet processing that
//        leads to aborting of decode process
//
//        [wmav2 @ 0xxxxx] Multiple frames in a packet.
//        [wmav2 @ 0xxxxx] Got unexpected packet size after a partial decode

K3bFFMpegWrapper *K3bFFMpegWrapper::s_instance = NULL;

class K3bFFMpegFile::Private {
public:
  TQ_UINT8 *packetData;
  K3b::Msf length;

  ::AVFormatContext *formatContext;
  ::AVCodec *codec;
  ::AVStream *audio_stream;
  ::AVCodecContext *audio_stream_ctx;
  ::AVSampleFormat sampleFormat;
  ::AVFrame *frame;
  ::AVPacket *packet;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 12, 100)
  ::AVPacket _packet;
#endif

  char *outputBufferPos;
  int outputBufferSize;
  int packetSize;
  bool isSpacious;
};

K3bFFMpegFile::K3bFFMpegFile(const TQString &filename) : m_filename(filename) {
  d = new Private;
  d->formatContext = NULL;
  d->codec = NULL;
  d->audio_stream = NULL;
  d->audio_stream_ctx = NULL;
  d->frame = av_frame_alloc();
  d->outputBufferPos = NULL;
  d->packet = NULL;
}

K3bFFMpegFile::~K3bFFMpegFile() {
  close();
  av_frame_free(&d->frame);
  delete d;
}

bool K3bFFMpegFile::open() {
  close();

  // open the file
  int err = ::avformat_open_input(&d->formatContext, m_filename.local8Bit(),
                                  NULL, NULL);
  if (err < 0) {
    kdDebug() << "(K3bFFMpegFile) unable to open " << m_filename
              << " with error " << err;
    return false;
  }

  // analyze the streams
  ::avformat_find_stream_info(d->formatContext, NULL);

  // we only handle files containing one audio stream
  for (uint i = 0; i < d->formatContext->nb_streams; ++i) {
    if (d->formatContext->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_AUDIO) {
      if (!d->audio_stream) {
        d->audio_stream = d->formatContext->streams[i];
      } else {
        d->audio_stream = NULL;
        kdDebug() << "(K3bFFMpegFile) more than one audio stream in "
                  << m_filename;
        return false;
      }
    }
  }

  // urgh... ugly
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100)
  if (d->audio_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
#else
  d->audio_stream_ctx = d->audio_stream->codec;
  if (d->audio_stream_ctx->codec_type != AVMEDIA_TYPE_AUDIO)
#endif
  {
    kdDebug() << "(K3bFFMpegFile) not a simple audio stream: " << m_filename;
    return false;
  }

  // get the codec
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100)
  d->codec = (AVCodec *)::avcodec_find_decoder(d->audio_stream->codecpar->codec_id);
#else
  d->codec = (AVCodec *)::avcodec_find_decoder(d->audio_stream_ctx->codec_id);
#endif
  if (!d->codec) {
    kdDebug() << "(K3bFFMpegFile) no codec found for " << m_filename;
    return false;
  }

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100)
  // allocate a codec context
  d->audio_stream_ctx = avcodec_alloc_context3(d->codec);
  if (d->audio_stream_ctx) {
    avcodec_parameters_to_context(d->audio_stream_ctx, d->audio_stream->codecpar);
  }
  else {
    kdDebug() << "(K3bFFMpegFile) failed to allocate a codec context for "
              << m_filename;
  }
#endif

  // open the codec on our context
  kdDebug() << "(K3bFFMpegFile) found codec for " << m_filename << endl;
  if (::avcodec_open2(d->audio_stream_ctx, d->codec, NULL) < 0) {
    kdDebug() << "(K3bFFMpegDecoderFactory) could not open codec.";
    return false;
  }

  // determine the length of the stream
  d->length = K3b::Msf::fromSeconds(double(d->formatContext->duration) /
                                    double(AV_TIME_BASE));

  if (d->length == 0) {
    kdDebug() << "(K3bFFMpegDecoderFactory) invalid length.";
    return false;
  }

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57, 41, 100)
  d->sampleFormat = d->audio_stream->codec->sample_fmt;
#else
  d->sampleFormat = static_cast<::AVSampleFormat>(d->audio_stream->codecpar->format);
#endif
# if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 24, 100)
  d->isSpacious = ::av_sample_fmt_is_planar(d->sampleFormat) &&
                  d->audio_stream->codecpar->channels > 1;
# else
  d->isSpacious = ::av_sample_fmt_is_planar(d->sampleFormat) &&
                  d->audio_stream->codecpar->ch_layout.nb_channels > 1;
# endif

  // dump some debugging info
  ::av_dump_format(d->formatContext, 0, m_filename.local8Bit(), 0);

  return true;
}

void K3bFFMpegFile::close() {
  d->outputBufferSize = 0;
  d->packetSize = 0;
  d->packetData = NULL;

  if (d->codec) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100)
    ::avcodec_free_context(&d->audio_stream_ctx);
#else
    ::avcodec_close(d->audio_stream_ctx);
    d->codec = NULL;
#endif
  }

  if (d->formatContext) {
    ::avformat_close_input(&d->formatContext);
    d->formatContext = NULL;
  }

  d->audio_stream = NULL;
}

K3b::Msf K3bFFMpegFile::length() const { return d->length; }

int K3bFFMpegFile::sampleRate() const {
  return d->audio_stream->codecpar->sample_rate;
}

int K3bFFMpegFile::channels() const {
# if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 24, 100)
  return d->audio_stream->codecpar->channels;
# else
  return d->audio_stream->codecpar->ch_layout.nb_channels;
# endif
}

int K3bFFMpegFile::type() const { return d->audio_stream->codecpar->codec_id; }

TQString K3bFFMpegFile::typeComment() const {
  switch (type()) {
  case AV_CODEC_ID_WMAV1:
    return i18n("Windows Media v1");
  case AV_CODEC_ID_WMAV2:
    return i18n("Windows Media v2");
  case AV_CODEC_ID_WAVPACK:
    return i18n("WavPack");
  case AV_CODEC_ID_APE:
    return i18n("Monkey's Audio (APE)");
  case AV_CODEC_ID_AAC:
    return i18n("Advanced Audio Coding (AAC)");
  default:
    return TQString::fromLocal8Bit(d->codec->name);
  }
}

TQString K3bFFMpegFile::title() const {
  // FIXME: is this UTF8 or something??
  AVDictionaryEntry *ade =
      av_dict_get(d->formatContext->metadata, "TITLE", NULL, 0);
  return ade && ade->value && ade->value[0] != '\0'
             ? TQString::fromLocal8Bit(ade->value)
             : TQString();
}

TQString K3bFFMpegFile::author() const {
  // FIXME: is this UTF8 or something??
  AVDictionaryEntry *ade =
      av_dict_get(d->formatContext->metadata, "ARTIST", NULL, 0);
  return ade && ade->value && ade->value[0] != '\0'
             ? TQString::fromLocal8Bit(ade->value)
             : TQString();
}

TQString K3bFFMpegFile::comment() const {
  // FIXME: is this UTF8 or something??
  AVDictionaryEntry *ade =
      av_dict_get(d->formatContext->metadata, "COMMENT", NULL, 0);
  return ade && ade->value && ade->value[0] != '\0'
             ? TQString::fromLocal8Bit(ade->value)
             : TQString();
}

int K3bFFMpegFile::read(char *buf, int bufLen) {

  int ret = fillOutputBuffer();
  if (ret <= 0) {
    return ret;
  }

  int len = TQMIN(bufLen, d->outputBufferSize);
  ::memcpy(buf, d->outputBufferPos, len);

  if (d->isSpacious && bufLen > d->outputBufferSize)
    delete[] d->outputBufferPos; // clean up allocated space

  // TODO: only swap if needed
  for (int i = 0; i < len - 1; i += 2)
    tqSwap(buf[i], buf[i + 1]); // BE -> LE

  d->outputBufferSize -= len;
  if (d->outputBufferSize > 0)
    d->outputBufferPos += len;
  return len;
}

// fill d->packetData with data to decode
int K3bFFMpegFile::readPacket() {
  if (d->packetSize <= 0) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    d->packet = ::av_packet_alloc();
#else
    ::av_init_packet(&d->_packet);
    d->packet = &d->_packet;
#endif

    if (::av_read_frame(d->formatContext, d->packet) < 0) {
      return 0;
    }
    d->packetSize = d->packet->size;
    d->packetData = d->packet->data;
  }

  return d->packetSize;
}

// decode data in d->packetData and fill d->outputBuffer
int K3bFFMpegFile::fillOutputBuffer() {
  // decode if the output buffer is empty
  while (d->outputBufferSize <= 0) {

    // make sure we have data to decode
    if (readPacket() == 0) {
      return 0;
    }

    int gotFrame = 0;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 100)
    int len = avcodec_receive_frame(d->audio_stream_ctx, d->frame);
    if (len == 0) {
      gotFrame = 1;
    }
    else if (len == AVERROR(EAGAIN)) {
      len = 0;
    }

    if (len == 0) {
      len = avcodec_send_packet(d->audio_stream_ctx, d->packet);
      if (len == AVERROR(EAGAIN)) {
        len = 0;
      }
    }
#else
    int len = ::avcodec_decode_audio4(d->audio_stream_ctx, d->frame,
                                      &gotFrame, d->packet);
#endif

    if (d->packetSize <= 0 || len < 0) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
      ::av_packet_free(&d->packet);
#else
      ::av_packet_unref(d->packet);
      d->packet = NULL;
#endif
    }
    if (len < 0) {
      kdDebug() << "(K3bFFMpegFile) decoding failed for " << m_filename;
      return -1;
    }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    len = d->packet->size;
#endif

    if (gotFrame) {
      int nb_s = d->frame->nb_samples;
      int nb_ch = 2; // copy only two channels even if there're more
      d->outputBufferSize = nb_s * nb_ch * 2; // 2 means 2 bytes (16bit)
      d->outputBufferPos = reinterpret_cast<char *>(d->frame->extended_data[0]);
      if (d->isSpacious) {
        d->outputBufferPos = new char[d->outputBufferSize];
        if (d->sampleFormat == AV_SAMPLE_FMT_FLTP) {
          int width = sizeof(float); // sample width of float audio
          for (int sample = 0; sample < nb_s; sample++) {
            for (int ch = 0; ch < nb_ch; ch++) {
              double val = *(reinterpret_cast<float *>(
                  d->frame->extended_data[ch] + sample * width));
              val = ::abs(val) > 1 ? ::copysign(1.0, val) : val;
              int16_t result =
                  static_cast<int16_t>(val * 32767.0 + 32768.5) - 32768;
              ::memcpy(d->outputBufferPos + (sample * nb_ch + ch) * 2, &result,
                       2); // 2 is sample width of 16 bit audio
            }
          }
        } else {
          for (int sample = 0; sample < nb_s; sample++) {
            for (int ch = 0; ch < nb_ch; ch++) {
              ::memcpy(d->outputBufferPos + (sample * nb_ch + ch) * 2,
                       d->frame->extended_data[ch] + sample * 2,
                       2); // 16 bit here as well
            }
          }
        }
      }
    }
    d->packetSize -= len;
    d->packetData += len;
  }

  return d->outputBufferSize;
}

bool K3bFFMpegFile::seek(const K3b::Msf &msf) {
  d->outputBufferSize = 0;
  d->packetSize = 0;

  double seconds = double(msf.totalFrames()) / 75.0;
  int64_t timestamp = static_cast<int64_t>(seconds * double(AV_TIME_BASE));

  // FIXME: do we really need the start_time and why?
  return (::av_seek_frame(d->formatContext, -1,
                          timestamp + d->formatContext->start_time, 0) >= 0);
}

//
// av_register_all is deprecated since ffmpeg 4.0, can be dropped
K3bFFMpegWrapper::K3bFFMpegWrapper() {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58,9,100)
  ::av_register_all();
#endif
}

K3bFFMpegWrapper::~K3bFFMpegWrapper() { s_instance = NULL; }

K3bFFMpegWrapper *K3bFFMpegWrapper::instance() {
  if (!s_instance) {
    s_instance = new K3bFFMpegWrapper();
  }

  return s_instance;
}

K3bFFMpegFile *K3bFFMpegWrapper::open(const TQString &filename) const {
  K3bFFMpegFile *file = new K3bFFMpegFile(filename);
  if (file->open()) {
#ifndef K3B_FFMPEG_ALL_CODECS
    //
    // only allow tested formats. ffmpeg seems not to be too reliable with every
    // format. mp3 being one of them sadly. Most importantly: allow the
    // libsndfile decoder to do its thing.
    //
    if (file->type() == AV_CODEC_ID_WMAV1 ||
        file->type() == AV_CODEC_ID_WMAV2 || file->type() == AV_CODEC_ID_AAC ||
        file->type() == AV_CODEC_ID_APE || file->type() == AV_CODEC_ID_WAVPACK)
#endif
      return file;
  }

  delete file;
  return NULL;
}
