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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${HAL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### k3bdevice (shared) ########################

tde_add_library( k3bdevice SHARED AUTOMOC
  SOURCES k3bdevice.cpp k3bdevice_mmc.cpp k3bscsicommand.cpp k3btrack.cpp
    k3btoc.cpp k3bdevicemanager.cpp k3bmsf.cpp k3bdiskinfo.cpp
    k3bdeviceglobals.cpp k3bcrc.cpp k3bcdtext.cpp k3bhalconnection.cpp
    k3bdebug.cpp
  VERSION 5.0.0
  LINK tdecore-shared ${TDEHW_LIBRARIES} ${HAL_LIBRARIES} ${DBUS_TQT_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)


##### Headers ###################################

install( FILES k3bdevicemanager.h k3bdevice.h k3btoc.h k3btrack.h
    k3bdeviceglobals.h k3bdiskinfo.h k3bcdtext.h k3bmsf.h k3bdevicetypes.h
    k3bdevice_export.h k3bhalconnection.h k3bdebug.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
