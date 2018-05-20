## Process this file with automake to produce Makefile.in

# meaning (see configure.ac): ACLOCAL_AMFLAGS  = -I $(macro_dir)
ACLOCAL_AMFLAGS			= -I build-aux/m4

AM_CPPFLAGS				=
AM_LDFLAGS				=
BUILT_SOURCES			=
include_HEADERS			=
noinst_LTLIBRARIES		=
lib_LTLIBRARIES			=
EXTRA_LTLIBRARIES		=
EXTRA_DIST				=
CLEANFILES				=
MOSTLYCLEANFILES		=

DISTCLEANFILES			=
MAINTAINERCLEANFILES	=

SUBDIRS					= $(ltdl_dir) build-aux/gnulib src/tools . src
DIST_SUBDIRS			= $(SUBDIRS)

gen_verbose = $(gen_verbose_@AM_V@)
gen_verbose_ = $(gen_verbose_@AM_DEFAULT_V@)
gen_verbose_0 = @echo "  GEN      $@";
gen_verbose_1 =

BUILT_SOURCES			+= $(top_srcdir)/.version

# Using 'cd' in backquotes may print the directory name, use this instead:
lt__cd		= CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd

git_version_gen = '$(SHELL)' '$(aux_dir)/git-version-gen' '--fallback' '$(VERSION)' '--prefix' 'fluffos-' '.tarball-version'
rebuild = rebuild=:; revision=`$(lt__cd) $(srcdir) && $(git_version_gen) | $(SED) 's|-.*$$||'`

EXTRA_DIST += .version .coveralls.yml .travis_build.sh .travis.yml
EXTRA_DIST += autogen.sh build.FluffOS ChangeLog Copyright Credits README.md TODO.md
EXTRA_DIST += doc compat
EXTRA_DIST += build-aux/compile build-aux/config.guess build-aux/config.rpath
EXTRA_DIST += build-aux/config.sub build-aux/depcomp build-aux/git-version-gen
EXTRA_DIST += build-aux/install-sh build-aux/ltmain.sh build-aux/m4 build-aux/missing
# $(top_srcdir)/test

$(top_srcdir)/.version:
	$(gen_verbose)echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	$(gen_verbose)echo $(VERSION) > $(distdir)/.tarball-version

libtool: @LIBTOOL_DEPS@
	$(gen_verbose)cd $(srcdir) && $(SHELL) ./config.status lihtool

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
