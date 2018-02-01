## Process this file with automake to produce Makefile.in

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

vm_int_compiler_includedir		= $(pkgincludedir)/vm/internal/base
nobase_vm_int_compiler_include_HEADERS	=

noinst_LTLIBRARIES	= libvm_int_compiler.la
libvm_int_compiler_la_SOURCES	=

libvm_int_compiler_la_SOURCES	+= compiler.cc
libvm_int_compiler_la_SOURCES	+= generate.cc grammar.cc
libvm_int_compiler_la_SOURCES	+= icode.cc
libvm_int_compiler_la_SOURCES	+= lex.cc
libvm_int_compiler_la_SOURCES	+= scratchpad.cc
libvm_int_compiler_la_SOURCES	+= trees.cc

libvm_int_compiler_la_LDFLAGS	= --static $(AM_LDFLAGS)

.PHONY: grammar.hh
grammar.cc grammar.hh:
	@echo [Generating grammar...]
	@$(top_srcdir)/src/tools/make_grammar.sh $(top_srcdir) $(srcdir) $(CXX) $(CXXFLAGS)
	@$(BISON) -o grammar.cc \
		--defines=grammar.hh \
		grammar.y
clean-local: cl-grammar
.PHONY: cl-grammar
cl-grammar:
	rm -f grammar.cc grammar.hh grammar.y
