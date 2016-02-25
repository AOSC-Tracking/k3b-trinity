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
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/projects/datacd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/datadvd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/movixcd
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


##### movixdvd (static) #########################

tde_add_library( movixdvd STATIC_PIC AUTOMOC
  SOURCES k3bmovixdvddoc.cpp k3bmovixdvdjob.cpp
)


##### Headers ###################################

install( FILES k3bmovixdvddoc.h k3bmovixdvdjob.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
