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
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/projects/datacd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/audiocd
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### mixed (static) ############################

tde_add_library( mixed STATIC_PIC AUTOMOC
  SOURCES k3bmixeddoc.cpp k3bmixedjob.cpp
)


##### Headers ###################################

install( FILES k3bmixeddoc.h k3bmixedjob.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
