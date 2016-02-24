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

add_subdirectory( kostore )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/tools
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b/cddb
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
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/rip
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### projects (static) #########################

tde_add_library( projects STATIC_PIC AUTOMOC
  SOURCES k3baudioburndialog.cpp k3baudiocdtextwidget.cpp
    k3baudiodatasourceviewitem.cpp k3baudioeditorwidget.cpp
    k3baudiotrackdialog.cpp k3baudiotrackwidget.cpp k3baudiotrackplayer.cpp
    k3baudiotracksplitdialog.cpp k3baudiotrackview.cpp
    k3baudiotrackviewitem.cpp k3baudioview.cpp base_k3baudiocdtextwidget.ui
    base_k3baudiocdtextallfieldswidget.ui base_k3baudiotrackwidget.ui
    k3bmixedburndialog.cpp k3bmixeddirtreeview.cpp k3bmixedview.cpp
    base_k3badvanceddataimagesettings.ui base_k3bbootimageview.ui
    base_k3bdataimagesettings.ui base_k3bdatavolumedescwidget.ui
    k3bdataadvancedimagesettingswidget.cpp k3bdatadirtreeview.cpp
    k3bdatafileview.cpp k3bdataimagesettingswidget.cpp
    k3bdatapropertiesdialog.cpp k3bdataview.cpp k3bdataviewitem.cpp
    k3bdatavolumedescwidget.cpp k3bdataburndialog.cpp k3bbootimagedialog.cpp
    k3bbootimageview.cpp k3bdvdburndialog.cpp k3bdvdview.cpp
    base_k3bmovixoptionswidget.ui k3bmovixburndialog.cpp k3bmovixlistview.cpp
    k3bmovixoptionswidget.cpp k3bmovixview.cpp k3bmovixdvdburndialog.cpp
    k3bmovixdvdview.cpp k3bvideodvdburndialog.cpp k3bvideodvdview.cpp
    k3bvcdburndialog.cpp k3bvcdlistview.cpp k3bvcdlistviewitem.cpp
    k3bvcdtrackdialog.cpp k3bvcdview.cpp k3bfillstatusdisplay.cpp k3bview.cpp
    k3bprojectburndialog.cpp k3bprojectplugindialog.cpp
    k3baudiotracktrmlookupdialog.cpp k3bdatamultisessioncombobox.cpp
    k3bmusicbrainzjob.cpp k3bdataurladdingdialog.cpp
    k3bdatasessionimportdialog.cpp k3baudiodatasourceeditwidget.cpp
    k3baudiotrackaddingdialog.cpp k3bencodingconverter.cpp
  LINK kostore-static
)
