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
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${VORBIS_LIBRARY_DIRS}
  ${VORBISENC_LIBRARY_DIRS}
  ${OGG_LIBRARY_DIRS}
)


##### libk3boggvorbisencoder (kpart) ############

tde_add_kpart( libk3boggvorbisencoder AUTOMOC
  SOURCES base_k3boggvorbisencodersettingswidget.ui k3boggvorbisencoder.cpp
  LINK tdecore-shared k3b-shared k3bdevice-shared
    ${VORBIS_LIBRARIES} ${VORBISENC_LIBRARIES} ${OGG_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE k3boggvorbisencoder.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
