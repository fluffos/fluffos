## Process this file with automake to produce Makefile.in
MOSTLYCLEANFILES	=
CLEANFILES			=

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

SUBDIRS								=
EXTRA_DIST							=

pkglib_LTLIBRARIES					= libpackage_async.la
libpackage_async_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackage_async_la_SOURCES			=
libpackage_async_la_SOURCES			+= async.cc
