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

add_subdirectory( cdi )
add_subdirectory( extra )
add_subdirectory( mpeginfo )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### vcd (static) ##############################

tde_add_library( vcd STATIC_PIC AUTOMOC
  SOURCES k3bvcddoc.cpp k3bvcdtrack.cpp k3bvcdjob.cpp k3bvcdoptions.cpp
    k3bvcdxmlview.cpp
)


##### Headers ###################################

install( FILES k3bvcdjob.h k3bvcddoc.h k3bvcdoptions.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
