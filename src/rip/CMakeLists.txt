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
  ${CMAKE_SOURCE_DIR}/libk3b/cddb
  ${CMAKE_SOURCE_DIR}/libk3b/jobs
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/projects/audiocd
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/projects
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### rip (static) ##############################

if ( WITH_LIBDVDREAD )
  add_subdirectory( videodvd )
  set ( VIDEODVDRIPGUI_LIBRARIES videodvdripgui-static)
endif ( )

tde_add_library( rip STATIC_PIC AUTOMOC
  SOURCES base_k3baudiorippingoptionwidget.ui base_k3bcddbpatternwidget.ui
    k3bpatternparser.cpp k3baudiorippingdialog.cpp k3baudioripthread.cpp
    k3baudiocdview.cpp k3bcddbpatternwidget.cpp k3bvideocdinfo.cpp
    k3bvideocdview.cpp k3bvideocdrip.cpp k3bvideocdrippingdialog.cpp
    k3bcuefilewriter.cpp k3baudioconvertingoptionwidget.cpp
    k3baudiocdlistview.cpp k3baudioprojectconvertingdialog.cpp
    k3baudioprojectconvertingthread.cpp k3baudioripjob.cpp
  LINK ${VIDEODVDRIPGUI_LIBRARIES}
)
