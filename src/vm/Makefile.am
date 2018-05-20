## Process this file with automake to produce Makefile.in

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

# for gnulib
AM_CPPFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib
AM_CXXFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib

SUBDIRS			= internal

vm_includedir		= $(pkgincludedir)/vm
nobase_vm_include_HEADERS	=

noinst_LTLIBRARIES	= libvm.la
libvm_la_SOURCES	=
libvm_la_LIBADD		= internal/libvm_internal.la

nobase_pkginclude_HEADERS		= vm.hh
