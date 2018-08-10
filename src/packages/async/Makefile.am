## Process this file with automake to produce Makefile.in
MOSTLYCLEANFILES	=
CLEANFILES			=

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

# for gnulib
AM_CPPFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib
AM_CXXFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib

SUBDIRS								=
EXTRA_DIST							=
net_includedir		= $(pkgincludedir)/net
nobase_net_include_HEADERS	=

pkglib_LTLIBRARIES					= libpackage_async.la
libpackage_async_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackage_async_la_SOURCES			=
libpackage_async_la_SOURCES			+= async.cc async.h

EXTRA_DIST += async.spec
