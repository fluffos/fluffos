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

# for gnulib
AM_CPPFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib
AM_CXXFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib

EXTRA_DIST		= applies
SUBDIRS			= base compiler

vm_internal_includedir		= $(pkgincludedir)/vm/internal
nobase_vm_internal_include_HEADERS	=

noinst_LTLIBRARIES	= libvm_internal.la
libvm_internal_la_SOURCES	=

libvm_internal_la_SOURCES	+= apply.cc
libvm_internal_la_SOURCES	+= eval_limit.cc
libvm_internal_la_SOURCES	+= master.cc
libvm_internal_la_SOURCES	+= otable.cc otable_test.cc
libvm_internal_la_SOURCES	+= simul_efun.cc simulate.cc
libvm_internal_la_SOURCES	+= trace.cc
libvm_internal_la_SOURCES	+= vm.cc

libvm_internal_la_SOURCES	+= apply.h
libvm_internal_la_SOURCES	+= eval_limit.h
libvm_internal_la_SOURCES	+= master.h
libvm_internal_la_SOURCES	+= otable.h
libvm_internal_la_SOURCES	+= simul_efun.h simulate.h
libvm_internal_la_SOURCES	+= trace.h

libvm_internal_la_LDFLAGS	= --static $(AM_LDFLAGS)
libvm_internal_la_LIBADD	=
libvm_internal_la_LIBADD	+= base/libvm_int_base.la
libvm_internal_la_LIBADD	+= compiler/libvm_int_compiler.la

BUILT_SOURCES	+= std_header

std_header-local: options.hh

gen_verbose = $(gen_verbose_@AM_V@)
gen_verbose_ = $(gen_verbose_@AM_DEFAULT_V@)
gen_verbose_0 = @echo "  GEN      $@";
gen_verbose_1 =

CLEANFILES		+= options.hh
options.hh: $(top_srcdir)/src/base/internal/options_incl.h
	$(gen_verbose)$(CXX) $(CXXFLAGS) -E -undef -dM $(top_srcdir)/src/base/internal/options_incl.h | $(PYTHON) $(top_srcdir)/src/tools/make_options_def.py options.hh
