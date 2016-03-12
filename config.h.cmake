#cmakedefine VERSION "@VERSION@"

// User specified build options

#cmakedefine HAVE_K3BSETUP
#cmakedefine K3B_DEBUG

#cmakedefine HAVE_LIBDVDREAD
#cmakedefine HAVE_LIBSAMPLERATE
#cmakedefine HAVE_MUSICBRAINZ
#cmakedefine WITH_ARTS

#cmakedefine HAVE_HAL

#cmakedefine K3B_FFMPEG_ALL_CODECS

// System configuration

#cmakedefine HAVE_ICONV_H
#cmakedefine HAVE_ENDIAN_H
#cmakedefine HAVE_SYS_ENDIAN_H
#cmakedefine HAVE_SYS_STATVFS_H
#cmakedefine HAVE_SYS_VFS_H
#cmakedefine HAVE_BYTESWAP_H
#cmakedefine HAVE_STDINT_H
#define HAVE_LRINT @HAVE_LRINT@
#define HAVE_LRINTF @HAVE_LRINTF@
#cmakedefine HAVE_STAT64

// Don't define HAVE_RESMGR due to it seems to be totally outdated
// #cmakedefine HAVE_RESMGR
