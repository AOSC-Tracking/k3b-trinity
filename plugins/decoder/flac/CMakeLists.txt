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
  ${FLAC_LIBRARY_DIRS}
)

if( ${FLAC_VERSION} VERSION_GREATER "1.1.1" )
  add_definitions( -DFLAC_NEWER_THAN_1_1_1=1 )
endif( )

##### libk3bflacdecoder (kpart) #################

tde_add_kpart( libk3bflacdecoder AUTOMOC
  SOURCES k3bflacdecoder.cpp
  LINK tdecore-shared k3b-shared k3bdevice-shared
    ${FLAC_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES k3bflacdecoder.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
