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
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/src
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### option (static) ###########################

tde_add_library( option STATIC_PIC AUTOMOC
  SOURCES base_k3bcddboptiontab.ui base_k3bmiscoptiontab.ui
    base_k3bpluginoptiontab.ui base_k3bthemeoptiontab.ui k3bmiscoptiontab.cpp
    k3bexternalbinoptiontab.cpp k3bcddboptiontab.cpp k3bburningoptiontab.cpp
    k3boptiondialog.cpp k3bdeviceoptiontab.cpp k3bnotifyoptiontab.cpp
    k3bpluginoptiontab.cpp k3bthemeoptiontab.cpp k3bdevicewidget.cpp
    k3bexternalbinwidget.cpp
)
