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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${LIBAVUTIL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBAVCODEC_LIBRARY_DIRS}
  ${LIBAVFORMAT_LIBRARY_DIRS}
  ${LIBAVUTIL_LIBRARY_DIRS}
)

add_definitions(
  -D__STDC_CONSTANT_MACROS
)

##### libk3bffmpegdecoder (kpart) ###############

tde_add_kpart( libk3bffmpegdecoder AUTOMOC
  SOURCES k3bffmpegdecoder.cpp k3bffmpegwrapper.cpp
  LINK tdecore-shared k3b-shared k3bdevice-shared ${LIBAVFORMAT_LIBRARIES}
    ${LIBAVCODEC_LIBRARIES} ${LIBAVUTIL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE k3bffmpegdecoder.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
