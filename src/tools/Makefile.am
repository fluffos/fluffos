## Process this file with automake to produce Makefile.in

AM_CFLAGS			= $(OPENMP_CFLAGS) -I$(top_srcdir)/src
AM_CPPFLAGS			= -I$(top_srcdir)/src
AM_CXXFLAGS			= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS			=

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

# for gnulib
AM_CPPFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib
AM_CXXFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib

GNULIB			= $(top_builddir)/build-aux/gnulib/libgnu.la
GNULIB			+= $(ACOS_LIBM) $(ASIN_LIBM) $(ATAN_LIBM)
GNULIB			+= $(CEIL_LIBM) $(COS_LIBM)
GNULIB			+= $(EXP_LIBM)
GNULIB			+= $(FLOOR_LIBM) $(FREXP_LIBM)
GNULIB			+= $(GETADDRINFO_LIB) $(GETHOSTNAME_LIB)
GNULIB			+= $(HOSTENT_LIB)
GNULIB			+= $(INET_NTOP_LIB) $(ISNAND_LIBM)
GNULIB			+= $(LIBSOCKET) $(LIB_PTHREAD) $(LOG10_LIBM) $(LOG2_LIBM) $(LOG_LIBM)
GNULIB			+= $(LTLIBICONV) $(LTLIBINTL) $(LTLIBTHREAD)
GNULIB			+= $(SERVENT_LIB)
LDADD			= $(GNULIB)

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
