AM_CPPFLAGS = -I$(srcdir)/../../libk3b/core \
	-I$(srcdir)/../../libk3b/core \
	-I$(srcdir)/../../libk3b/plugin \
	-I$(srcdir)/../../libk3b/tools \
	-I$(srcdir)/../../libk3b/projects \
	-I$(srcdir)/../../libk3b/projects/datacd \
	-I$(srcdir)/../../libk3bdevice \
	-I$(srcdir)/../../src/projects \
	$(all_includes)

# these are the headers for your project
noinst_HEADERS   = tdefile_k3bprojectfileplugin.h

kde_module_LTLIBRARIES = tdefile_k3b.la

tdefile_k3b_la_SOURCES = tdefile_k3bprojectfileplugin.cpp
tdefile_k3b_la_LDFLAGS = $(all_libraries) $(LIB_QT) $(LIB_TDECORE) $(LIB_TDEUI) $(LIB_KIO) -module $(KDE_PLUGIN)
tdefile_k3b_la_LIBADD = $(LIB_KIO) ../../libk3b/libk3b.la ../../src/projects/kostore/libkostore.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

services_DATA = tdefile_k3b.desktop
servicesdir = $(kde_servicesdir)

messages:
	$(XGETTEXT) *.cpp -o $(podir)/tdefile_k3b.pot
