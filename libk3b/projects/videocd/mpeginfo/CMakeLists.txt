# This file is genereted by trinity-automake-cmake-convert script by Fat-Zer

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


##### mpeginfo (static) #########################

tde_add_library( mpeginfo STATIC_PIC AUTOMOC
  SOURCES k3bmpeginfo.cpp
)
