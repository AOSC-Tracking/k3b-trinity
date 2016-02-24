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
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/libk3b/cddb
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### audio (static) ############################

tde_add_library( audio STATIC_PIC AUTOMOC
  SOURCES k3baudiojob.cpp k3baudiotrack.cpp k3baudiodoc.cpp k3baudiofile.cpp
    k3baudiozerodata.cpp k3baudiodatasource.cpp k3baudionormalizejob.cpp
    k3baudiojobtempdata.cpp k3baudioimager.cpp k3baudiomaxspeedjob.cpp
    k3baudiocdtracksource.cpp k3baudiocdtrackdrag.cpp
    k3baudiodatasourceiterator.cpp
)
