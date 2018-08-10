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
AM_CPPFLAGS			+= -DHAVE_ZLIB

AM_LDFLAGS			+= -export-dynamic

EXTRA_DIST		=
EXTRA_DIST		+= jemalloc
EXTRA_DIST		+= libtelnet
SUBDIRS			=

BUILT_SOURCES		+= libtelnet/libtelnet.lo

# noinst_LTLIBRARIES	= libthirdparty.la
# libthirdparty_la_SOURCES	=

# libthirdparty_la_LDFLAGS	= --static $(AM_LDFLAGS)
# libthirdparty_la_LIBADD	= libtelnet/libtelnet.la

libtelnet/libtelnet.lo:
	@echo building libtelnet.lo ...
	@mkdir libtelnet >/dev/null 2>&1 || true
	@cd libtelnet && $(abs_srcdir)/libtelnet/configure && make libtelnet.lo
