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

vm_int_base_includedir		= $(pkgincludedir)/vm/internal/base
nobase_vm_int_base_include_HEADERS	=

noinst_LTLIBRARIES	= libvm_int_base.la
libvm_int_base_la_SOURCES	=

libvm_int_base_la_SOURCES	+= apply_cache.cc array.cc
libvm_int_base_la_SOURCES	+= buffer.cc
libvm_int_base_la_SOURCES	+= class.cc
libvm_int_base_la_SOURCES	+= function.cc
libvm_int_base_la_SOURCES	+= interpret.cc
libvm_int_base_la_SOURCES	+= mapping.cc
libvm_int_base_la_SOURCES	+= object.cc
libvm_int_base_la_SOURCES	+= program.cc
libvm_int_base_la_SOURCES	+= svalue.cc

libvm_int_base_la_LDFLAGS	= --static $(AM_LDFLAGS)

libvm_int_base_la_SOURCES	+= apply_cache.h array.h
libvm_int_base_la_SOURCES	+= buffer.h
libvm_int_base_la_SOURCES	+= class.h
libvm_int_base_la_SOURCES	+= function.h
libvm_int_base_la_SOURCES	+= interpret.h
libvm_int_base_la_SOURCES	+= machine.h mapping.h
libvm_int_base_la_SOURCES	+= number.h
libvm_int_base_la_SOURCES	+= object.h old_qsort_inc.h
libvm_int_base_la_SOURCES	+= program.h
libvm_int_base_la_SOURCES	+= svalue.h
