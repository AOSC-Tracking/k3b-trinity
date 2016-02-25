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
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### k3bcore (static) ##########################

tde_add_library( k3bcore STATIC_PIC AUTOMOC
  SOURCES k3bcore.cpp k3bglobals.cpp k3bdefaultexternalprograms.cpp
    k3bexternalbinmanager.cpp k3bversion.cpp k3bprocess.cpp k3bjob.cpp
    k3bthread.cpp k3bthreadjob.cpp k3bglobalsettings.cpp
    k3bsimplejobhandler.cpp
)


##### Headers ###################################

install( FILES k3bcore.h k3bdefaultexternalprograms.h k3bexternalbinmanager.h
    k3bprocess.h k3bversion.h k3bglobals.h k3bjob.h k3bthread.h
    k3bthreadjob.h k3bglobalsettings.h k3bjobhandler.h
    k3b_export.h k3bsimplejobhandler.h
  DESTINATION ${INCLUDE_INSTALL_DIR}
)
