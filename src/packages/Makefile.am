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

EXTRA_DIST							=

pkglib_LTLIBRARIES					= libpackages.la
libpackages_la_LDFLAGS			= --static $(AM_LDFLAGS)

libpackages_la_SOURCES			=
nodist_libpackages_la_SOURCES	=
nodist_libpackages_la_SOURCES	+= applies_table.cc
nodist_libpackages_la_SOURCES	+= efuns.cc

BUILT_SOURCES					= std_header

SUBDIRS					= core ops
libpackages_la_LIBADD	= core/libpackage_core.la ops/libpackage_ops.la

# SUBDIRS		+= async
# SUBDIRS		+= compress contrib crypto
# SUBDIRS		+= db develop dwlib
# SUBDIRS		+= external
# SUBDIRS		+= math matrix mudlib_stats
# SUBDIRS		+= parser
# SUBDIRS		+= pcre
# SUBDIRS		+= sha1 sockets
# SUBDIRS		+= trim
# SUBDIRS		+= uids
SUBDIRS += math mudlib_stats uids sha1 pcre contrib develop sockets compress async matrix trim external
libpackages_la_LIBADD	+= math/libpackage_math.la mudlib_stats/libpackage_mudlib_stats.la uids/libpackage_uids.la sha1/libpackage_sha1.la pcre/libpackage_pcre.la contrib/libpackage_contrib.la develop/libpackage_develop.la sockets/libpackage_sockets.la compress/libpackage_compress.la async/libpackage_async.la matrix/libpackage_matrix.la trim/libpackage_trim.la external/libpackage_external.la
packages_includedir				= $(pkgincludedir)/packages

nobase_pkginclude_HEADERS			= packages.hh

std_header-local: efuns.hh applies_table.hh

gen_verbose = $(gen_verbose_@AM_V@)
gen_verbose_ = $(gen_verbose_@AM_DEFAULT_V@)
gen_verbose_0 = @echo "  GEN      $@";
gen_verbose_1 =

CLEANFILES			+= efuns.cc efuns.hh
efuns.cc efuns.hh: packages.fullspec
	$(gen_verbose)ASAN_OPTIONS="detect_leaks=0" ../tools/make_func packages.fullspec

.PHONY:		packages.fullspec
CLEANFILES			+= packages.fullspec
packages.fullspec:
	$(gen_verbose)$(top_srcdir)/src/tools/build_packages_genfiles.sh $(top_srcdir) $(CXX) $(CXXFLAGS)

CLEANFILES			+= applies_table.cc applies_table.hh
applies_table.cc applies_table.hh:
	$(gen_verbose)../tools/build_applies $(top_srcdir)/src

EXTRA_DIST	+= crypto db dwlib parser
