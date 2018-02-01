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

pkglib_LTLIBRARIES					= libpackage_parser.la
libpackage_parser_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackage_parser_la_SOURCES			=
libpackage_parser_la_SOURCES			+= parser.cc
