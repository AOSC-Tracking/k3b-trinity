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
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### tdeio_videodvd (kpart) ####################

tde_add_kpart( tdeio_videodvd AUTOMOC
  SOURCES videodvd.cpp
  LINK k3b-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES videodvd.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services
)

install( FILES videodvd.protocol
  DESTINATION ${SERVICES_INSTALL_DIR}
)
