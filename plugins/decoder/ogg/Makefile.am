AM_CPPFLAGS = -I$(srcdir)/../../../libk3b/plugin -I$(srcdir)/../../../libk3b/core -I$(srcdir)/../../../libk3bdevice $(all_includes)

kde_module_LTLIBRARIES = libk3boggvorbisdecoder.la

libk3boggvorbisdecoder_la_SOURCES = k3boggvorbisdecoder.cpp

libk3boggvorbisdecoder_la_LIBADD = ../../../libk3b/libk3b.la -logg -lvorbis -lvorbisfile 
libk3boggvorbisdecoder_la_LIBADD = ../../../libk3b/libk3b.la ../../../libk3b/plugin/libk3bplugin.la ../../../libk3bdevice/libk3bdevice.la -logg -lvorbis -lvorbisfile $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO)

libk3boggvorbisdecoder_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)

pluginsdir = $(kde_datadir)/k3b/plugins
plugins_DATA = k3boggvorbisdecoder.plugin

METASOURCES = AUTO
