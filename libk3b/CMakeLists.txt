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

add_subdirectory( core )
add_subdirectory( plugin )
add_subdirectory( tools )
add_subdirectory( projects )
add_subdirectory( cddb )
add_subdirectory( jobs )
# add_subdirectory( scripts ) # the directory sciped in autotools either

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### k3b (shared) ##############################

if ( WITH_LIBDVDREAD )
  add_subdirectory( videodvd )
  set ( VIDEODVDRIP_LIBRARIES videodvdrip-static)
endif ( )

# Note: Due to embending into static libraries doesn't work correctly we have
#       to embend all the substuff here.

# From plugin
if ( NOT WITH_SYSTEM_LIBSAMPLERATE )
  list ( APPEND SUBLIBRARIES samplerate-static )
endif ( )

# From tools
list ( APPEND SUBLIBRARIES isofs-static )

# From projects
list ( APPEND SUBLIBRARIES data-static dvd-static mixed-static vcd-static
  videodvd-static movix-static movixdvd-static audio-static mpeginfo-static )

tde_add_library( k3b SHARED AUTOMOC
  SOURCES dummy.cpp
  VERSION 3.0.0
  EMBED k3bcore-static cddb-static k3bproject-static k3btools-static
    jobs-static k3bplugin-static ${VIDEODVDRIP_LIBRARIES} ${SUBLIBRARIES}
  LINK dl tdeio-shared tdeui-shared k3bdevice-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
