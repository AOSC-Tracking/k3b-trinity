#cmakedefine VERSION "@VERSION@"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#cmakedefine WORDS_BIGENDIAN @WORDS_BIGENDIAN@

// User specified build options

#cmakedefine HAVE_K3BSETUP
#cmakedefine K3B_DEBUG

#cmakedefine HAVE_LIBDVDREAD
#cmakedefine HAVE_LIBSAMPLERATE
#cmakedefine HAVE_MUSICBRAINZ
#cmakedefine WITH_ARTS

#cmakedefine HAVE_TDEHWLIB

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

#cmakedefine HAVE_TAGLIB

// Defined if you have MMX support
#cmakedefine HAVE_X86_MMX 1
