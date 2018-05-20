## Process this file with automake to produce Makefile.in
MOSTLYCLEANFILES	=
CLEANFILES			=

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

# for gnulib
AM_CPPFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib
AM_CXXFLAGS		+= -I$(top_builddir)/src/gnulib -I$(top_srcdir)/src/gnulib

SUBDIRS								=
EXTRA_DIST							=

pkglib_LTLIBRARIES					= libpackage_math.la
libpackage_math_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackage_math_la_SOURCES			=
libpackage_math_la_SOURCES			+= math.cc

EXTRA_DIST += math.spec
