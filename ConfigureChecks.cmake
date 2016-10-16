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

##### check for gcc visibility support #########
# FIXME
# This should check for [T]Qt3 visibility support

if( WITH_GCC_VISIBILITY )
  if( NOT UNIX )
    tde_message_fatal(FATAL_ERROR "\ngcc visibility support was requested, but your system is not *NIX" )
  endif( NOT UNIX )
  set( __KDE_HAVE_GCC_VISIBILITY 1 )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif( WITH_GCC_VISIBILITY )

tde_setup_architecture_flags( )

# Check system configuration
check_include_file ( "iconv.h"       HAVE_ICONV_H       )
check_include_file ( "endian.h"      HAVE_ENDIAN_H      )
check_include_file ( "sys/endian.h"  HAVE_SYS_ENDIAN_H  )
check_include_file ( "sys/statvfs.h" HAVE_SYS_STATVFS_H )
check_include_file ( "sys/vfs.h"     HAVE_SYS_VFS_H     )
check_include_file ( "byteswap.h"    HAVE_BYTESWAP_H    )
check_include_file ( "stdint.h"      HAVE_STDINT_H      )
tde_save_and_set( CMAKE_REQUIRED_LIBRARIES "m" )
check_symbol_exists ( lrint  "math.h" HAVE_LRINT  )
check_symbol_exists ( lrintf "math.h" HAVE_LRINTF )
tde_restore( CMAKE_REQUIRED_LIBRARIES )
if( NOT HAVE_LRINT )
  set( HAVE_LRINT 0 )
endif( NOT HAVE_LRINT )
if( NOT HAVE_LRINTF )
  set( HAVE_LRINTF 0 )
endif( NOT HAVE_LRINTF )
check_symbol_exists ( stat64 "sys/types.h;sys/stat.h;unistd.h" HAVE_STAT64 )

##### k3bsetup ##################################

if ( WITH_K3BSETUP )
  set ( HAVE_K3BSETUP 1 )
endif ( )


##### k3b_debug #################################

if ( WITH_DEBUG )
  set ( K3B_DEBUG 1 )
endif ( )


##### libdvdread ################################

if ( WITH_LIBDVDREAD )
  pkg_search_module ( LIBDVDREAD dvdread REQUIRED )
	if ( LIBDVDREAD_FOUND )
    set ( HAVE_LIBDVDREAD 1 )
  else ( )
    tde_message_fatal( "libdvdread is required, but was not found on your system" )
  endif ( )
endif ( )


##### musicbrainz ###############################

if ( WITH_MUSICBRAINZ )
  pkg_search_module ( MUSICBRAINZ libmusicbrainz )
  if ( MUSICBRAINZ_FOUND )
    set ( HAVE_MUSICBRAINZ 1 )
  else ( )
    tde_message_fatal( "musicbrainz is required, but was not found on your system" )
  endif ( )
endif ( )


##### libsamplerate #############################

if ( WITH_SYSTEM_LIBSAMPLERATE )
  pkg_search_module ( LIBSAMPLERATE samplerate )
  if ( LIBSAMPLERATE_FOUND )
    set ( HAVE_LIBSAMPLERATE 1 )
  else ( )
    tde_message_fatal( "libsamplerate is required, but was not found on your system" )
  endif ( )
endif ( )


##### hal #######################################

if( WITH_HAL )
  pkg_search_module( HAL hal )
  if( HAL_FOUND )
    set ( HAVE_HAL 1 )
  else ( )
    tde_message_fatal( "hal is required, but was not found on your system" )
  endif( )
endif( )

##### arts ######################################

if( WITH_ARTS )
  pkg_search_module( ARTS artsc )
  if( NOT ARTS_FOUND )
    tde_message_fatal( "aRtsC is requested, but was not found on your system" )
  endif( )
endif( )

##### check for ALSA ############################

if( WITH_ALSA )
  find_package( ALSA )
  if( NOT ALSA_FOUND )
    message(FATAL_ERROR "\nALSA support is requested, but was not found on your system" )
  endif( NOT ALSA_FOUND )
endif( WITH_ALSA )


##### ffmpeg ####################################

if( WITH_FFMPEG )
  pkg_search_module( LIBAVCODEC libavcodec )
  pkg_search_module( LIBAVFORMAT libavformat )
  pkg_search_module( LIBAVUTIL libavutil )
  # TODO chech if avutil required on all systems
  if( NOT LIBAVCODEC_FOUND )
    tde_message_fatal( "ffmpeg is requested, but libavcodec was not found on your system" )
  elseif( NOT LIBAVFORMAT_FOUND )
    tde_message_fatal( "ffmpeg is requested, but libavformat was not found on your system" )
  else( )
    if( WITH_FFMPEG_ALL_CODECS )
      set( K3B_FFMPEG_ALL_CODECS 1 )
    endif( )
  endif( )
endif( )


##### flac ######################################

if( WITH_FLAC )
  pkg_search_module( FLAC flac++ )
  if( NOT FLAC_FOUND )
    tde_message_fatal( "flac is requested, but was not found on your system" )
  endif( )
endif( )

##### sndfile ###################################

if( WITH_SNDFILE )
  pkg_search_module( SNDFILE sndfile )
  if( NOT SNDFILE_FOUND )
    tde_message_fatal( "sndfile is requested, but was not found on your system" )
  endif( )
endif( )

##### taglib ####################################

if( WITH_TAGLIB )
  pkg_search_module( TAGLIB taglib )
  if( NOT TAGLIB_FOUND )
    tde_message_fatal( "taglib is requested, but was not found on your system" )
  endif( )
endif( )


##### mad #######################################

if( WITH_MAD )
  pkg_search_module( MAD mad )
  if( NOT MAD_FOUND )
    tde_message_fatal( "mad is requested, but was not found on your system" )
  endif( )
endif( )


##### musepack ##################################

if( WITH_MUSEPACK )

  check_library_exists ( mpcdec mpc_decoder_setup  "" MPCDEC_FOUND )

  # check common include locations
  foreach ( _mpc_dir  "mpc" "musepack" "mpcdec" )
    check_include_file( "${_mpc_dir}/mpcdec.h" ${_mpc_dir}_MPCDEC_H_FOUND )
    if ( ${${_mpc_dir}_MPCDEC_H_FOUND} )
      set( MPCDEC_HEADER_FILE "<${_mpc_dir}/mpcdec.h>" CACHE INTERNAL
        "mpcdec.h header file with an upper level directory")
      break( )
    endif( )
  endforeach( )

  if( NOT MPCDEC_FOUND OR NOT MPCDEC_HEADER_FILE )
    tde_message_fatal( "musepack is requested, but mpcdec was not found on your system" )
  endif( )
endif( )


##### vorbis ####################################

if( WITH_VORBIS )
  pkg_search_module( VORBIS     vorbis     )
  pkg_search_module( VORBISFILE vorbisfile )
  pkg_search_module( VORBISENC  vorbisenc  )
  pkg_search_module( OGG        ogg        )
  if( NOT VORBIS_FOUND )
    tde_message_fatal( "vorbis is requested, but was not found on your system" )
  elseif( NOT VORBISFILE_FOUND OR NOT VORBISENC_FOUND OR NOT OGG_FOUND )
    tde_message_fatal( "vorbis is requested, but some of it's essential parts wasn't found on your system" )
  endif( )
endif( )


##### lame ######################################

if( WITH_LAME )
  check_library_exists ( mp3lame lame_init  "" LAME_FOUND )
  check_include_file( "lame/lame.h" LAME_H_FOUND )
  if( NOT LAME_FOUND OR NOT LAME_H_FOUND )
    tde_message_fatal( "lame is requested, but was not found on your system" )
  endif( )
endif( )


# # gettext
# if( BUILD_TRANSLATIONS )
#   include( FindGettext )
#   if( GETTEXT_FOUND )
#     set( MSGFMT_EXECUTABLE ${GETTEXT_MSGFMT_EXECUTABLE}
#       CACHE FILEPATH "path to msgfmt executable" )
#   endif( GETTEXT_FOUND )
#
#   if( NOT MSGFMT_EXECUTABLE )
#     tde_message_fatal( "msgfmt is required but was not found on your system." )
#   endif( NOT MSGFMT_EXECUTABLE )
# endif( BUILD_TRANSLATIONS )

# required stuff
find_package( TQt )
find_package( TDE )
