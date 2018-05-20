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

pkglib_LTLIBRARIES					= libpackage_core.la
libpackage_core_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackage_core_la_SOURCES			=
libpackage_core_la_SOURCES			+= add_action.cc
libpackage_core_la_SOURCES			+= call_out.cc crc32.cc custom_crypt.cc
libpackage_core_la_SOURCES			+= debug.cc dns.cc dumpstat.cc
libpackage_core_la_SOURCES			+= ed.cc efuns_main.cc
libpackage_core_la_SOURCES			+= file.cc
libpackage_core_la_SOURCES			+= heartbeat.cc
libpackage_core_la_SOURCES			+= mssp.cc
libpackage_core_la_SOURCES			+= outbuf.cc
libpackage_core_la_SOURCES			+= rc.cc reclaim.cc regexp.cc replace_program.cc
libpackage_core_la_SOURCES			+= save.cc sprintf.cc
libpackage_core_la_SOURCES			+= telnet_ext.cc

libpackage_core_la_SOURCES			+= add_action.h
libpackage_core_la_SOURCES			+= call_out.h crc32.h crctab.h custom_crypt.h
libpackage_core_la_SOURCES			+= dns.h dumpstat.h
libpackage_core_la_SOURCES			+= ed.h
libpackage_core_la_SOURCES			+= file.h
libpackage_core_la_SOURCES			+= heartbeat.h
libpackage_core_la_SOURCES			+= mssp.h
libpackage_core_la_SOURCES			+= outbuf.h
libpackage_core_la_SOURCES			+= reclaim.h regexp.h replace_program.h
libpackage_core_la_SOURCES			+= sprintf.h
libpackage_core_la_SOURCES			+= telnet_ext.h

EXTRA_DIST += core.spec
