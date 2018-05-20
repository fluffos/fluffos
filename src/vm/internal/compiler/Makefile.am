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

EXTRA_DIST		= grammar.y.pre

noinst_LTLIBRARIES	= libvm_int_compiler.la
libvm_int_compiler_la_SOURCES	=
nodist_libvm_int_compiler_la_SOURCES	=

libvm_int_compiler_la_SOURCES	+= compiler.cc
libvm_int_compiler_la_SOURCES	+= generate.cc
libvm_int_compiler_la_SOURCES	+= icode.cc
libvm_int_compiler_la_SOURCES	+= lex.cc
libvm_int_compiler_la_SOURCES	+= scratchpad.cc
libvm_int_compiler_la_SOURCES	+= trees.cc

libvm_int_compiler_la_SOURCES	+= compiler.h
libvm_int_compiler_la_SOURCES	+= generate.h
libvm_int_compiler_la_SOURCES	+= icode.h
libvm_int_compiler_la_SOURCES	+= keyword.h
libvm_int_compiler_la_SOURCES	+= lex.h
libvm_int_compiler_la_SOURCES	+= scratchpad.h
libvm_int_compiler_la_SOURCES	+= trees.h

nodist_libvm_int_compiler_la_SOURCES	+= grammar.cc grammar.hh

libvm_int_compiler_la_LDFLAGS	= --static $(AM_LDFLAGS)

std_header-local: grammar.hh

gen_verbose = $(gen_verbose_@AM_V@)
gen_verbose_ = $(gen_verbose_@AM_DEFAULT_V@)
gen_verbose_0 = @echo "  GEN      $@";
gen_verbose_1 =

grammar.y: grammar.y.pre
	$(gen_verbose)$(top_srcdir)/src/tools/make_grammar.sh $(top_srcdir) $(srcdir) $(CXX) $(CXXFLAGS)
grammar.cc grammar.hh: grammar.y
	$(gen_verbose)$(BISON) -o grammar.cc --defines=grammar.hh grammar.y
clean-local: cl-grammar
.PHONY: cl-grammar
cl-grammar:
	rm -f grammar.cc grammar.hh grammar.y
