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
  ${TAGLIB_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${MAD_LIBRARY_DIRS}
  ${TAGLIB_LIBRARY_DIRS}
)


##### libk3bmaddecoder (kpart) ##################

tde_add_kpart( libk3bmaddecoder AUTOMOC
  SOURCES k3bmad.cpp k3bmaddecoder.cpp
  LINK tdecore-shared k3b-shared k3bdevice-shared
    ${MAD_LIBRARIES} ${TAGLIB_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE k3bmaddecoder.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
