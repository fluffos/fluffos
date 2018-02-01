## Process this file with automake to produce Makefile.in

AM_CFLAGS		= $(OPENMP_CFLAGS)
AM_CPPFLAGS		= -I$(top_srcdir)/src $(LIBEVENT_CPPFLAGS)
AM_CXXFLAGS		= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src $(LIBEVENT_CPPFLAGS)
AM_LDFLAGS		= $(LIBEVENT_LDFLAGS)

# AM_CFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
# AM_CFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

# AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
# AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS		+= -export-dynamic

BUILT_SOURCES	= std_header

SUBDIRS			=
SUBDIRS			+= thirdparty
SUBDIRS			+= base
SUBDIRS			+= net
SUBDIRS			+= packages
SUBDIRS			+= vm

driver_LDADD	=
driver_LDADD	+= net/libnet.la
driver_LDADD	+= vm/libvm.la
driver_LDADD	+= packages/libpackages.la
driver_LDADD	+= thirdparty/libtelnet/libtelnet.lo
driver_LDADD	+= base/libbase.la

driver_LDADD	+= $(LIBEVENT_LIBS)

bin_PROGRAMS	= driver portbind
driver_SOURCES	=
driver_SOURCES	+= backend.cc
driver_SOURCES	+= comm.cc
driver_SOURCES	+= fliconv.cc
driver_SOURCES	+= main.cc mainlib.cc
driver_SOURCES	+= user.cc

driver_LDFLAGS	= $(AM_LDFLAGS)

portbind_SOURCES	=
portbind_SOURCES	+= portbind.cc
