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


add_subdirectory( sox )
add_subdirectory( external )
tde_conditional_add_subdirectory( WITH_LAME   lame )
tde_conditional_add_subdirectory( WITH_VORBIS ogg  )
