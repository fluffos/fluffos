## Process this file with automake to produce Makefile.in
CLEANFILES			=
MOSTLYCLEANFILES	=
BUILT_SOURCES		=

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

SUBDIRS			=

base_internal_includedir		= $(pkgincludedir)/base/internal
nobase_base_internal_include_HEADERS	=

noinst_LTLIBRARIES	= libbase_internal.la
libbase_internal_la_SOURCES	=

libbase_internal_la_SOURCES	+= debugmalloc.cc
libbase_internal_la_SOURCES	+= external_port.cc
libbase_internal_la_SOURCES	+= file.cc
libbase_internal_la_SOURCES	+= hash.cc
libbase_internal_la_SOURCES	+= log.cc
libbase_internal_la_SOURCES	+= md.cc
libbase_internal_la_SOURCES	+= outbuf.cc
libbase_internal_la_SOURCES	+= port.cc
libbase_internal_la_SOURCES	+= rc.cc
libbase_internal_la_SOURCES	+= stats.cc stralloc.cc strput.cc

libbase_internal_la_LDFLAGS	= --static $(AM_LDFLAGS)
libbase_internal_la_LIBADD	=
