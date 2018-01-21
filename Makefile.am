## Process this file with automake to produce Makefile.in

# meaning (see configure.ac): ACLOCAL_AMFLAGS  = -I $(macro_dir)
ACLOCAL_AMFLAGS			= -I build-aux/m4

AM_CPPFLAGS				=
AM_LDFLAGS				=
BUILT_SOURCES			=
include_HEADERS			=
noinst_LIBRARIES		=
lib_LIBRARIES			=
EXTRA_LIBRARIES			=
EXTRA_DIST				=
CLEANFILES				=
MOSTLYCLEANFILES		=

DISTCLEANFILES			=
MAINTAINERCLEANFILES	=

SUBDIRS					= src
#tests
DIST_SUBDIRS			= $(SUBDIRS)

BUILT_SOURCES			+= $(top_srcdir)/.version

git_version_gen = '$(SHELL)' '$(aux_dir)/git-version-gen' '--fallback' '$(VERSION)' '.tarball-version'
rebuild = rebuild=:; revision=`cd $(srcdir) >/dev/zero 2>&1 && $(git_version_gen) | $(SED) 's|-.*$$||'`


EXTRA_DIST += $(top_srcdir)/.version $(top_srcdir)/*.md						\
	$(top_srcdir)/.*.yml $(top_srcdir)/Makefile.am	$(top_srcdir)/contrib	\
	$(top_srcdir)/doc $(top_srcdir)/build-aux $(top_srcdir)/build.FluffOS	\
	$(top_srcdir)/ChangeLog $(top_srcdir)/Copyright							\
	$(top_srcdir)/Credits $(top_srcdir)/compat  $(top_srcdir)/.travis*
# $(top_srcdir)/test

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

libtool: @LIBTOOL_DEPS@
	cd $(srcdir) && $(SHELL) ./config.status lihtool

#  Files automatically distributed if found (always):
#    ABOUT-GNU           INSTALL             config.rpath        ltconfig
#    ABOUT-NLS           NEWS                config.sub          ltmain.sh
#    AUTHORS             README              depcomp             mdate-sh
#    BACKLOG             THANKS              install-sh          missing
#    COPYING             TODO                libversion.in       mkinstalldirs
#    COPYING.DOC         ar-lib              ltcf-c.sh           py-compile
#    COPYING.LESSER      compile             ltcf-cxx.sh         texinfo.tex
#    COPYING.LIB         config.guess        ltcf-gcj.sh         ylwrap
#    ChangeLog
#
#  Files automatically distributed if found (under certain conditions):
#    acconfig.h          config.h.bot        configure           configure.in
#    aclocal.m4          config.h.top        configure.ac        stamp-vti
#	./configure.ac
#	./configure.scan*
#	./src/
#	./src/compiler/
#	./src/compiler/Makefile
#	./src/compiler/pre_proc.l
#	./src/tags
#	./src/vm/
