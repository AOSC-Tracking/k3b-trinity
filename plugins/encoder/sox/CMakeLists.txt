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
)


##### libk3bsoxencoder (kpart) ##################

tde_add_kpart( libk3bsoxencoder AUTOMOC
  SOURCES base_k3bsoxencoderconfigwidget.ui k3bsoxencoder.cpp
  LINK tdecore-shared k3b-shared k3bdevice-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE k3bsoxencoder.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
