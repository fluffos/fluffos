## Process this file with automake to produce Makefile.in

AM_CFLAGS							= $(OPENMP_CFLAGS)
AM_CPPFLAGS							= -I$(top_srcdir)/src
AM_CXXFLAGS							= $(BOOST_CPPFLAGS) $(OPENMP_CXXFLAGS) -I$(top_srcdir)/src
AM_LDFLAGS							= $(BOOST_LDFLAGS)

AM_CPPFLAGS			+= -DMODULE_DIR=\"$(pkglibexecdir)\"
AM_CPPFLAGS			+= -DCONFIG_DIR=\"$(pkgdatadir)\"

AM_LDFLAGS			+= -export-dynamic

vm_includedir		= $(pkgincludedir)/vm
nobase_vm_include_HEADERS	=

noinst_LTLIBRARIES	= libvm.la
libvm_la_SOURCES	=

libvm_la_SOURCES	+= object.cc
libvm_la_SOURCES	+= random.cc
libvm_la_SOURCES	+= shared_string.cc svalue.cc
libvm_la_SOURCES	+= vm.cc
libvm_la_LDFLAGS	= --static $(AM_LDFLAGS)

nobase_pkginclude_HEADERS		= vm.hh
nobase_vm_include_HEADERS	+= object.hh
nobase_vm_include_HEADERS	+= random.hh
nobase_vm_include_HEADERS	+= shared_string.hh svalue.hh
