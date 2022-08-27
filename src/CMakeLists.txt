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

add_subdirectory( option )
add_subdirectory( misc )
add_subdirectory( rip )
add_subdirectory( projects )
add_subdirectory( fastscale )
add_subdirectory( pics )
add_subdirectory( icons )
add_subdirectory( konqi )
add_subdirectory( mimetypes )
add_subdirectory( sounds )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/cddb
  ${CMAKE_SOURCE_DIR}/libk3b/videodvd
  ${CMAKE_SOURCE_DIR}/libk3b/jobs
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  ${CMAKE_SOURCE_DIR}/libk3b/projects
  ${CMAKE_SOURCE_DIR}/libk3b/projects/datacd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/datadvd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/audiocd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/videocd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/mixedcd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/movixcd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/videodvd
  ${CMAKE_SOURCE_DIR}/libk3b/projects/movixdvd
  ${CMAKE_SOURCE_DIR}/libk3b/plugin
  ${CMAKE_CURRENT_SOURCE_DIR}/projects
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${MUSICBRAINZ_LIBRARY_DIRS}
)


##### k3b (executable) ##########################

tde_add_executable( k3b AUTOMOC
  SOURCES k3bwelcomewidget.cpp k3bapplication.cpp k3bdiroperator.cpp
    k3bfiletreeview.cpp k3bprojecttabwidget.cpp k3bsplash.cpp k3bfileview.cpp
    k3bdirview.cpp k3b.cpp main.cpp k3bstatusbarmanager.cpp
    k3bfiletreecombobox.cpp k3binterface.cpp k3binterface.skel
    k3bjobinterface.cpp k3bjobinterface.skel k3bprojectinterface.cpp
    k3bprojectinterface.skel k3bdataprojectinterface.cpp
    k3bdataprojectinterface.skel k3bsystemproblemdialog.cpp
    k3bmediacontentsview.cpp k3bsidepanel.cpp k3bcontentsview.cpp
    k3bjobprogressdialog.cpp k3bburnprogressdialog.cpp
    k3btempdirselectionwidget.cpp k3bdatamodewidget.cpp
    k3bwritingmodewidget.cpp k3bwriterselectionwidget.cpp
    k3binteractiondialog.cpp k3bthememanager.cpp k3bprojectmanager.cpp
    k3btrm.cpp k3bmusicbrainz.cpp k3baudioprojectinterface.cpp
    k3bmixedprojectinterface.cpp k3baudioprojectinterface.skel
    k3bmixedprojectinterface.skel k3bflatbutton.cpp k3bemptydiscwaiter.cpp
    k3bjobprogressosd.cpp k3bdebuggingoutputdialog.cpp
    k3bdebuggingoutputfile.cpp k3bappdevicemanager.cpp k3bmediacache.cpp
    k3bmedium.cpp k3bmediaselectioncombobox.cpp k3btooltip.cpp
    k3bwidgetshoweffect.cpp k3bmediaselectiondialog.cpp k3bdiskinfoview.cpp
    k3bpassivepopup.cpp k3btimeoutwidget.cpp k3bminibutton.cpp
    k3bthemedheader.cpp k3bthemedlabel.cpp k3blsofwrapper.cpp
    k3blsofwrapperdialog.cpp k3bservicemenuinstaller.cpp k3bfirstrun.cpp
  LINK tdeui-shared tdeparts-shared k3bdevice-shared k3b-shared option-static projects-static rip-static
    misc-static fastscale-static ${MUSICBRAINZ_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE k3b-cue.desktop
  DESTINATION ${APPS_INSTALL_DIR}/.hidden/
)

tde_create_translated_desktop(
  SOURCE eventsrc
  DESTINATION ${DATA_INSTALL_DIR}/k3b
)

install(
  FILES k3bui.rc tips
  DESTINATION ${DATA_INSTALL_DIR}/k3b
)

tde_create_translated_desktop(
  SOURCE k3b.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
)
