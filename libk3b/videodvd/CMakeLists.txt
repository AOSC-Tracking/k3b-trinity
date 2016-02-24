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

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${LIBDVDREAD_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### videodvd (static) #########################

tde_add_library( videodvdrip STATIC_PIC AUTOMOC
  SOURCES k3bvideodvd.cpp k3bvideodvdtime.cpp k3bvideodvdvideostream.cpp
  LINK ${LIBDVDREAD_LIBRARIES}
)
