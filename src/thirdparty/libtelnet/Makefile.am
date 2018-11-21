## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4 --install
AM_CFLAGS = $(zlib_CFLAGS)
EXTRA_DIST = Doxyfile libtelnet.pc.in libtelnet-uninstalled.pc.in msvc2008 msvc2010 autogen.sh autogen-clean.sh
SUBDIRS = . doc util test

include_HEADERS = libtelnet.h

lib_LTLIBRARIES = libtelnet.la
libtelnet_la_SOURCES = libtelnet.c libtelnet.h
libtelnet_la_LDFLAGS = -version-info 2:0:0
libtelnet_la_LIBADD = $(zlib_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtelnet.pc
