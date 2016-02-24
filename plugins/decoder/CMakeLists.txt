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

add_subdirectory( wave )
tde_conditional_add_subdirectory( WITH_FFMPEG   ffmpeg     )
tde_conditional_add_subdirectory( WITH_FLAC     flac       )
tde_conditional_add_subdirectory( WITH_SNDFILE  libsndfile )
tde_conditional_add_subdirectory( WITH_MAD      mp3        )
tde_conditional_add_subdirectory( WITH_MUSEPACK musepack   )
tde_conditional_add_subdirectory( WITH_VORBIS   ogg        )
