## Process this file with automake to produce Makefile.in

AM_CFLAGS		= $(OPENMP_CFLAGS)
AM_CPPFLAGS		= -I$(top_srcdir)/src $(LIBEVENT_CPPFLAGS)
AM_CXXFLAGS		= $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src $(LIBEVENT_CPPFLAGS)
AM_LDFLAGS		= $(LIBEVENT_LDFLAGS)

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
EXTRA_DIST		+= include format_source.sh
EXTRA_DIST		+= local_options outdated_define.txt
EXTRA_DIST		+= Config.example

# AM_CFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
# AM_CFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

# AM_CPPFLAGS		+= -DMODULE_DIR=\"$(pkglibexecdir)\"
# AM_CPPFLAGS		+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS		+= -export-dynamic

gen_verbose = $(gen_verbose_@AM_V@)
gen_verbose_ = $(gen_verbose_@AM_DEFAULT_V@)
gen_verbose_0 = @echo "  GEN      $@";
gen_verbose_1 =

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

driver_LDADD	+= $(LIBEVENT_LIBS) $(GNULIB)

bin_PROGRAMS	= driver portbind
driver_SOURCES	=
driver_SOURCES	+= backend.cc backend.h
driver_SOURCES	+= comm.cc comm.h
driver_SOURCES	+= fliconv.cc fliconv.h interactive.h
driver_SOURCES	+= main.cc mainlib.cc mainlib.h
driver_SOURCES	+= user.cc user.h

driver_LDFLAGS	= $(AM_LDFLAGS)

portbind_SOURCES	=
portbind_SOURCES	+= portbind.cc
