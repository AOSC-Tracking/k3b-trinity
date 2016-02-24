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
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

add_definitions(
)


##### cddb (static) #############################

tde_add_library( cddb STATIC_PIC AUTOMOC
  SOURCES k3bcddbquery.cpp k3bcddb.cpp k3bcddbresult.cpp k3bcddbhttpquery.cpp
    k3bcddbpquery.cpp k3bcddblocalquery.cpp k3bcddbsubmit.cpp
    k3bcddblocalsubmit.cpp k3bcddbmultientriesdialog.cpp
)
