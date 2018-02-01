## Process this file with automake to produce Makefile.in

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

SUBDIRS			= internal

base_includedir		= $(pkgincludedir)/base
nobase_base_include_HEADERS	=

noinst_LTLIBRARIES	= libbase.la
libbase_la_SOURCES	=
libbase_la_LIBADD		= internal/libbase_internal.la

nobase_pkginclude_HEADERS		= std.h
