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

add_subdirectory( libisofs )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libk3b/core
  ${CMAKE_SOURCE_DIR}/libk3b
  ${CMAKE_SOURCE_DIR}/libk3bdevice
  #  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### k3btools (static) #########################

tde_add_library( k3btools STATIC_PIC AUTOMOC
  SOURCES k3bwavefilewriter.cpp k3bbusywidget.cpp k3bdeviceselectiondialog.cpp
    k3bmd5job.cpp k3btitlelabel.cpp k3bcutcombobox.cpp k3bstringutils.cpp
    k3bdevicecombobox.cpp kcutlabel.cpp k3bstdguiitems.cpp k3bvalidators.cpp
    k3bthroughputestimator.cpp k3biso9660.cpp k3bmultichoicedialog.cpp
    k3bdevicehandler.cpp k3bcdparanoialib.cpp k3blistview.cpp k3bmsfedit.cpp
    k3bcdtextvalidator.cpp k3bintvalidator.cpp k3bexceptions.cpp
    k3bprogressdialog.cpp k3btoolbox.cpp k3bpushbutton.cpp
    k3blistviewitemanimator.cpp k3bthreadwidget.cpp k3bradioaction.cpp
    k3bsignalwaiter.cpp k3blibdvdcss.cpp k3biso9660backend.cpp k3bpipe.cpp
    k3bchecksumpipe.cpp k3btoolbutton.cpp k3bintmapcombobox.cpp
    k3bdirsizejob.cpp k3brichtextlabel.cpp k3btempfile.cpp k3bactivepipe.cpp
    k3bfilesplitter.cpp k3bfilesysteminfo.cpp
)
