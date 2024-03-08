#################################################
#
#  (C) 2016 Golubev Alexander
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

cmake_minimum_required( VERSION 3.5 )


##### general package setup #####################

project( k3b )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckCSourceCompiles )
include( CheckIncludeFile )
include( CheckLibraryExists )
include( CheckSymbolExists )


##### include our cmake modules #################

include( TDEMacros )


##### set version number ########################

tde_set_project_version( )


##### setup install paths #######################

include( TDESetupPaths )
tde_setup_paths( )


##### optional stuff ############################

option( WITH_ALL_OPTIONS   "Enable all sane supported options" OFF )
option( WITH_LIBDVDREAD    "Enable support for libdvdread"    ${WITH_ALL_OPTIONS} )
option( WITH_MUSICBRAINZ   "Enable Musicbrainz support" OFF )
option( WITH_SYSTEM_LIBSAMPLERATE
    "Use system libsamplerate instead of bundled one" ON )
option( WITH_TDEHWLIB      "Enable TDE hardware library support" ${WITH_ALL_OPTIONS} )
option( WITH_DEBUG         "Enable extra debug" OFF )

# sound output plugins
option( WITH_ALSA     "Enable alsa support"             ${WITH_ALL_OPTIONS} )
option( WITH_ARTS     "Enable arts sound daemon support" ${WITH_ALL_OPTIONS} )

# decoder plugins
option( WITH_FFMPEG   "Enable support for decoding with ffmpeg"       ${WITH_ALL_OPTIONS} )
option( WITH_FFMPEG_ALL_CODECS "Enable support for all codecs provided by ffmpeg" OFF   )
option( WITH_FLAC     "Enable support for decoding flac files"        ${WITH_ALL_OPTIONS} )
option( WITH_SNDFILE  "Enable support for decoding via libsndfile"    ${WITH_ALL_OPTIONS} )
option( WITH_TAGLIB   "Enable support for taglib"                     ${WITH_ALL_OPTIONS} )
option( WITH_MAD      "Enable support for decoding mp3 via libmad"    ${WITH_ALL_OPTIONS} )
option( WITH_MUSEPACK "Enable support for decoding via musepack (mpcdec)" OFF )
option( WITH_VORBIS   "Enable support for en/decoding ogg via vorbis" ${WITH_ALL_OPTIONS} )
# Note: WITH_VORBIS enables both decoding and encoding
# Note: wave plugin build unconditionally due to it doesn't have any external deps
# Note: k3b requires some acient version of musepack

# encoder plugins
option( WITH_LAME          "Enable support for mp3 encoding via lame" ${WITH_ALL_OPTIONS} )
# see WITH_VORBIS
# Note: external and sox plugins are enabled by default due to they don't have
#       any additional build-time deps

# Note: permanently disabled options:
#       resmgr             - it seams to be completly outdated
#       cdrecord-suid-root - commented out in source code anyway
#       cam library - freebsd stuff should be added by somebody with bsd
#                     experience

##### options comments ##########################

# WITH_LIBDVDREAD     if disabled removes dvd reaping functionality
#                     requires libdvdread: http://dvdnav.mplayerhq.hu/
# WITH_MUSICBRAINZ    if enabled adds support for musicbrainz service for
#                     query metadata about audio cds
#                     requires media-libs/musicbrainz library: http://musicbrainz.org/doc/libmusicbrainz
# WITH_SYSTEM_LIBSAMPLERATE if disabled the bundled version of libsamplerate
#                     is used and staticaly linked into the k3b
#                     requires media-libs/libsamplerate: http://www.mega-nerd.com/SRC/
# WITH_DEBUG          if enabled extra debug output is provided
# WITH_ALSA           if enabled build the alsa output driver; it's a good
#                     idea to enable it on a linux othervice sound playback
#                     may fallback to arts
# WITH_ARTS           if disabled disables fuctionality to play audio (files
#                     or CDs, not sure exactly)
#                     requires arts (an optional part of TDE)
# WITH_FFMPEG         if enabled build an ffmpeg decoding plugin
#                     requires ffmpeg
# WITH_FFMPEG_ALL_CODECS
#                     affects ffmpeg plugin
#                     if enabled makes ffmpeg to decode all formats is
#                     supports; this is untested and may result in some
#                     bugs: e.g. display of wrong length
# WITH_FLAC           if enabled build flac decoding plugin
#                     requires media-sound/flac: http://flac.sourceforge.net
# WITH_LAME           if enabled build mp3 encoding file via lame
#                     requires media-sound/lame: http://lame.sourceforge.net/
# WITH_TAGLIB         if enabled provides support for tags in mp3 and flac
#                     requires media-libs/taglib: https://taglib.github.io/
#                     note: make sence if either WITH_FLAC or WITH_LIBMAD is set
# WITH_SNDFILE        if enabled build libsndfile encoding plugin
#                     requires media-libs/libsndfile: http://www.mega-nerd.com/libsndfile
# WITH_MAD            if enabled build mp3 decoding plugin
#                     requires media-libs/libmad: http://mad.sourceforge.net
# WITH_MUSEPACK       if enabled build musepack decoding plugin
#                     requires musepack (libmpcdec): http://www.musepack.net
#                     WARNING k3b requires some acient version of musepack
# WITH_VORBIS         if enabled build ogg encoding/decoding plugin
#                     requires media-libs/libvorbis: http://xiph.org/vorbis


##### user requested modules ####################

option( BUILD_K3BSETUP "Build K3bSetup2 TDEControl module" OFF )
option( BUILD_DOC "Build documentation" ON )
# option( BUILD_TRANSLATIONS "Build translations" ON )

# Note: probably modern linux systems don't need BUILD_K3BSETUP; so it's
#       disabled by default


##### configure checks ##########################

include( ConfigureChecks.cmake )


###### global compiler settings #################

add_definitions(
  -DHAVE_CONFIG_H
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined" )


##### k3b directories ###########################

add_subdirectory( libk3b          )
add_subdirectory( libk3bdevice    )
add_subdirectory( plugins         )
add_subdirectory( src             )
add_subdirectory( tdefile-plugins )
add_subdirectory( tdeioslaves     )
tde_conditional_add_subdirectory( BUILD_K3BSETUP k3bsetup )
tde_conditional_add_project_docs( BUILD_DOC )


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )
