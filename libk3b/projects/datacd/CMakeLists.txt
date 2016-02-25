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
  ${CMAKE_SOURCE_DIR}/libk3b/jobs
  ${CMAKE_SOURCE_DIR}/libk3b/projects
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


##### data (static) #############################

tde_add_library( data STATIC_PIC AUTOMOC
  SOURCES k3bdatajob.cpp k3bdatadoc.cpp k3bdataitem.cpp k3bdiritem.cpp
    k3bfileitem.cpp k3bisoimager.cpp k3bmsinfofetcher.cpp k3bbootitem.cpp
    k3bisooptions.cpp k3bfilecompilationsizehandler.cpp
    k3bsessionimportitem.cpp k3bmkisofshandler.cpp k3bdatapreparationjob.cpp
)


##### Headers ###################################

install( FILES k3bdatadoc.h k3bdatajob.h k3bdataitem.h k3bdiritem.h
    k3bfileitem.h k3bbootitem.h k3bisooptions.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
