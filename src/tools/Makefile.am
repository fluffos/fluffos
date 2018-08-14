## Process this file with automake to produce Makefile.in

AM_CFLAGS			= $(OPENMP_CFLAGS) -I$(top_srcdir)/src
AM_CPPFLAGS			= -I$(top_srcdir)/src
AM_CXXFLAGS			= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS			=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

EXTRA_DIST		=
EXTRA_DIST		+= build_packages_genfiles.sh
EXTRA_DIST		+= make_func.y make_grammar.sh make_options_def.py

noinst_PROGRAMS	= build_applies make_func

build_applies_SOURCES	= build_applies.cc
nodist_make_func_SOURCES		= make_func.cc

make_func.cc: make_func.y
	@$(BISON) -o make_func.cc $(srcdir)/make_func.y
clean-local: cl-make_func
.PHONY: cl-make_func
cl-make_func:
	rm -f make_func.cc
