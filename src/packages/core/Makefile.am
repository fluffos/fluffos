## Process this file with automake to produce Makefile.in
MOSTLYCLEANFILES	=
CLEANFILES			=

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							=

AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

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
