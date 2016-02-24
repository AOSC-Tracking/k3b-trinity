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
  ${CMAKE_SOURCE_DIR}/libk3b/videodvd
  ${CMAKE_SOURCE_DIR}/libk3b/jobs
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${LIBDVDREAD_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBDVDREAD_LIBRARY_DIRS}
)


##### videodvdrip (static) ######################

tde_add_library( videodvdripgui STATIC_PIC AUTOMOC
  SOURCES base_k3bvideodvdrippingwidget.ui k3bvideodvdrippingview.cpp
    k3bvideodvdrippingtitlelistview.cpp k3bvideodvdrippingjob.cpp
    k3bvideodvdrippingwidget.cpp k3bvideodvdrippingdialog.cpp
    k3bvideodvdrippingpreview.cpp
  LINK ${LIBDVDREAD_LIBRARIES}
)
