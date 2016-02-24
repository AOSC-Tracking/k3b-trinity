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
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### kcm_k3bsetup2 (kpart) #####################

tde_add_kpart( kcm_k3bsetup2 AUTOMOC
  SOURCES base_k3bsetup2.ui k3bsetup2.cpp
  LINK k3b-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### k3bsetup ##################################

install( PROGRAMS k3bsetup
  DESTINATION ${BIN_INSTALL_DIR}
)


##### other data ################################

install( FILES k3bsetup2.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/System
)
