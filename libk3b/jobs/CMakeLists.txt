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
  ${CMAKE_SOURCE_DIR}/libk3b/projects/audiocd
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/cddb
  ${CMAKE_SOURCE_DIR}/libk3b/videodvd
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### jobs (static) #############################

if ( WITH_LIBDVDREAD )
  set ( VIDEODVDRIP_SOURCES k3bvideodvdtitletranscodingjob.cpp
    k3bvideodvdtitledetectclippingjob.cpp )
endif ( )

tde_add_library( jobs STATIC_PIC AUTOMOC
  SOURCES k3bdatatrackreader.cpp k3breadcdreader.cpp k3bcdcopyjob.cpp
    k3bclonejob.cpp k3baudiosessionreadingjob.cpp k3bdvdcopyjob.cpp
    k3baudiocuefilewritingjob.cpp k3bbinimagewritingjob.cpp
    k3biso9660imagewritingjob.cpp k3bdvdformattingjob.cpp k3bblankingjob.cpp
    k3bclonetocreader.cpp k3bverificationjob.cpp ${VIDEODVDRIP_SOURCES}
)
