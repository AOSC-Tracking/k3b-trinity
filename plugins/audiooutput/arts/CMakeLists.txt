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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${ARTS_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ARTS_LIBRARY_DIRS}
)



##### libk3bartsoutputplugin (kpart) ############

tde_add_kpart( libk3bartsoutputplugin AUTOMOC
  SOURCES k3bartsoutputplugin.cpp
  LINK tdecore-shared k3b-shared ${ARTS_LIBRARIES} artsc
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### other data ################################

install( FILES k3bartsoutputplugin.plugin
  DESTINATION ${DATA_INSTALL_DIR}/k3b/plugins
)
