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
  ${CMAKE_SOURCE_DIR}/libk3b/jobs
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/src
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### misc (static) #############################

tde_add_library( misc STATIC_PIC AUTOMOC
  SOURCES k3bdvdformattingdialog.cpp k3bcdcopydialog.cpp k3bdvdcopydialog.cpp
    k3bcdimagewritingdialog.cpp k3bisoimagewritingdialog.cpp
    k3bblankingdialog.cpp
)
