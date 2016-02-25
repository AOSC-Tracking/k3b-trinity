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

add_subdirectory( datacd )
add_subdirectory( audiocd )
add_subdirectory( mixedcd )
add_subdirectory( movixcd )
add_subdirectory( videocd )
add_subdirectory( datadvd )
add_subdirectory( movixdvd )
add_subdirectory( videodvd )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/datacd
  ${CMAKE_CURRENT_SOURCE_DIR}/audiocd
  ${CMAKE_CURRENT_SOURCE_DIR}/mixedcd
  ${CMAKE_CURRENT_SOURCE_DIR}/movixcd
  ${CMAKE_CURRENT_SOURCE_DIR}/videocd
  ${CMAKE_CURRENT_SOURCE_DIR}/datadvd
  ${CMAKE_CURRENT_SOURCE_DIR}/movixdvd
  ${CMAKE_CURRENT_SOURCE_DIR}/videodvd
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### k3bproject (static) #######################

tde_add_library( k3bproject STATIC_PIC AUTOMOC
  SOURCES k3babstractwriter.cpp k3bgrowisofswriter.cpp k3bgrowisofshandler.cpp
    k3bdoc.cpp k3bcdrdaowriter.cpp k3bcdrecordwriter.cpp k3binffilewriter.cpp
    k3btocfilewriter.cpp k3bimagefilereader.cpp k3bcuefileparser.cpp
    k3bpipebuffer.cpp
)


##### Headers ###################################

install( FILES k3bdoc.h k3bgrowisofswriter.h k3bcdrdaowriter.h
    k3bcdrecordwriter.h k3binffilewriter.h k3btocfilewriter.h
    k3bcuefileparser.h k3bimagefilereader.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
