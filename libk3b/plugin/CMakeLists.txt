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
  ${CMAKE_SOURCE_DIR}/libk3b
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

if ( WITH_SYSTEM_LIBSAMPLERATE )
  include_directories ( ${LIBSAMPLERATE_INCLUDE_DIRS} )
  link_directories ( ${LIBSAMPLERATE_LIBRARY_DIRS} )
else ( )
  add_subdirectory( libsamplerate )
  include_directories( libsamplerate )
  set ( LIBSUMPLERATE_LIBRARIES samplerate-static )
endif ( )


##### k3bplugin (static) ########################

list( APPEND k3bplugin_SOURCES k3bplugin.cpp k3bpluginconfigwidget.cpp k3bpluginmanager.cpp
    k3baudiodecoder.cpp k3baudioencoder.cpp k3baudioclient.cpp
    k3baudioserver.cpp )
# We have to manually moc the file due to there is no corresponding source
tde_moc( k3bplugin_SOURCES k3bprojectplugin.h )

tde_add_library( k3bplugin STATIC_PIC AUTOMOC
  SOURCES ${k3bplugin_SOURCES}
  LINK m ${LIBSAMPLERATE_LIBRARIES}
)


##### Headers ###################################

install( FILES k3bplugin.h k3bpluginfactory.h k3bpluginmanager.h
    k3baudiodecoder.h k3baudioencoder.h k3bpluginconfigwidget.h
    k3baudiooutputplugin.h k3bprojectplugin.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
