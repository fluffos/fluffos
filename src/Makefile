#####################################################
#       Standard Makefile for the MudOS driver      #
#####################################################
 
SHELL=/bin/sh
 
###########################
# setup and configuration #
###########################
 
# Note: this Makefile may need modified depending upon which type of machine
# you are using to compile the driver.  Be sure to read all of the comments
# in this file since they may pertain to your particular machine.
#
# If you get some weird compile-time error message from "make" about not being
# able to find some .h file, then try one of the following two things:
#
# 1) type "make depend"
# 2) deleting all of the lines below the comment that mentions "make depend"
#    (near the bottom of this file).
 
# change this if you wish the driver binary to be named something else
DRIVER_BIN = driver

# uncomment PROOF if using CenterLine's TestCenter to debug the driver.
#PROOF=proof
 
# RESOLV: define this to be -lresolv if your machine has it.
# HP and NeXT don't have it.
#RESOLV=-lresolv
 
# uncomment STRFUNCS if your machine is missing memcpy(), memset(), strtol(),
# and strcspn(). (needed by Sequent Dynix/3)
#STRFUNCS=strfuncs.o
 
# uncomment UALARM if your machine is missing the ualarm() system call.
# needed by HP, A/UX (Apple Mac), EP/IX (MIPS), Sequent Dynix/3, Ultrix
#UALARM=ualarm.o
 
# Command used to install executables in the INSTALL_DIR
# Use the -f version for HP/UX
#INSTALL = install -f
#INSTALL = install -c
# Use cp if all else fails
INSTALL = cp
 
# Set INSTALL_DIR to the directory where you want to install the executables.
INSTALL_DIR = ../bin
 
#Enable warnings from the compiler (gcc), if wanted.
#WARN=-Wall
 
#Use this with gcc if you have lots of RAM, speeds up compile
#PIPE=-pipe

# define profiling if you want it
# note: the gmon.out file will likely be written in the mudlib dir.
# PROFILE_ON controls whether or not monitoring is active at driver
# startup.  Comment PROFILE_ON to make profiling not active at startup.
# Use moncontrol(1) efun to enable profiling and moncontrol(0) to turn
# it off.
#PROFILE_ON=-DPROFILE_ON
# Uncomment this if you want to enable profiling of the driver (gcc)
#PROFIL=-pg -DPROFILING $(PROFILE_ON)
 
# Enable run time debugging
#DEBUG=-g
# With extra driver debug code
#DEBUG=-g -DDEBUG 
# For DEC Alpha: to have optimization and debugging
#DEBUG=-g3 -DDEBUG
# Prevent -DDEBUG from aborting the driver (when in -DDEBUG mode)
#DEBUG_NON_FATAL=-DDEBUG_NON_FATAL
# Compile in debug() macro code
#DEBUG_MACRO=-DDEBUG_MACRO
 
# If you don't have strchr() and strrchr(), then uncomment the following line
#STR=-Dstrchr=index -Dstrrchr=rindex
 
# define this if you want (compiler) optimization enabled.
#
# *WARNING* using high levels of optimization (e.g. -O3) can cause some
# compilers to produce incorrect code.  If the driver is behaving
# inexplicably, try using a lower level of optimization (or none).
#
# Uncomment one or none of the following optimization lines.
#
# -O is usually a safe level of optimization for most compilers
OPTIMIZE=-O
# high optimization for gcc:
#OPTIMIZE=-O2 -fomit-frame-pointer -fstrength-reduce
# uncomment below for RS/6000(AIX) xlc compiler only.
# remove the -Q if xlc complains.
#OPTIMIZE=-O -Q
# might need this one with newer versions of AIX (ie 3.2.4)
#OPTIMIZE=-O -Q -qMAXMEM=16000
# high optimization for HP-UX 7.x/8.x's cc (don't use with 9.x)
#OPTIMIZE=+O3 +Obb3000
# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#OPTIMIZE=-Olimit 1802
# DEC Alpha's cc's highest optimization: -O5
#OPTIMIZE=-O -Olimit 2000
# DEC Ultrix's cc's highest optimization:
#OPTIMIZE=-O2 -Olimit 5000
# high optimization for cc on SGI
#OPTIMIZE=-Olimit 2500
 
# use this for NeXTSTEP 3.1+ (at least)
#CPP=$(CC) -E -traditional-cpp
# the usual
CPP=$(CC) -E

# System V Release 4 (386/486 or if using Sun's cc under Solaris 2.x)
#OSFLAGS=-DSVR4

# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#OSFLAGS=-I/usr/include/bsd
 
# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD -D_ALL_SOURCE
 
# Solaris (SunOS 5): for BSD compatible ioctl()
# If using CC=cc, you can also add:
#      -w     to turn off warnings, in order to get a cleaner compile
# For 5.4, also add the following:
#      -I/usr/bsdinclude
#OSFLAGS=-DBSD_COMP -DSunOS_5

# SCO 3.2v4.2
#OSFLAGS=-DSCO

# try uncommenting this if you are using gcc and at runtime you see socket
# errors saying that the "set socket nonblocking" operation is not supported.
# That error is caused by old-style macros (that gcc doesn't normally grok)
# used by ioctl on some systems.
#NEED_OLD_CPP=-traditional-cpp

CFLAGS= $(STATIC) $(OSFLAGS) $(OPTIMIZE) $(WARN) $(PROFIL) $(DEBUG) \
   $(DEBUG_MACRO) $(DEBUG_NON_FATAL) $(STR) $(PIPE) $(NEED_OLD_CPP)
 
# set CC=cc on a NeXT and to gcc on other machines (if possible)
# You may want to use xlc on an RS/6000 (produces faster code).
# if xlc doesn't like the code, try bsdcc; it's less picky
#
# NOTE: If you are using gcc (especially on a sun4), and the driver gives
# a runtime error regarding "operation not supported on socket", then
# try using cc instead (some UNIX's use unsupported hacks in some of
# the system #include (.h) files which prevents some (socket) ioctl operations
# from working correctly when using a less crufty compiler than that provided
# by default on the system in question).
#
#CC=cc
CC=gcc
# CenterLine cc
#CC=clcc
#CC=xlc
#CC=bsdcc

# define this to be what you use for make
MAKE = make
#MAKE = gmake
 
# define this to be bison if you have it, and yacc otherwise.
#YACC=bison
#YFLAGS=-d -y
YACC=yacc
YFLAGS = -d
 
# A/UX: may be required for Apple's UNIX (and possibly others).  Try
# uncommenting this if the make results in link errors seemingly related
# to yacc (e.g. undefined symbol "yyerror").  May also need -lbsd.
#EXTRALIBS=-ly
#EXTRALIBS=-ly -lbsd
 
# HP-UX: use this EXTRALIBS line for HP-UX
#EXTRALIBS=-lBSD
 
# RS/6000 AIX: use this EXTRALIBS line 
#EXTRALIBS=-lbsd
 
# NetBSD 386: due to export restrictions...you may be missing or
#  have a crippled crypt package
#EXTRALIBS=-lcrypt
 
# NeXT: link with MallocDebug if you have a NeXT with NeXTOS 2.1 or later and
# you wish to search for memory leaks (see /NextDeveloper/Apps/MallocDebug).
# Note: linking with MallocDebug will cause the virtual size of the
# driver process to reach appoximately 40MB however the amount of real memory
# used will remain close to normal.
#EXTRALIBS=-lMallocDebug -lsys_s
 
# Sequent DYNIX/ptx: use this EXTRALIBS line
#EXTRALIBS=-lsocket -linet -lnsl -lseq -lmalloc
 
# System V Release 4 (386/486)
#EXTRALIBS=-lsocket -lnsl
 
# Solaris (SunOS 5.1)
#EXTRALIBS=-lnsl -lsocket -lresolv
 
# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#EXTRALIBS=-lbsd
 
# SCO 3.2v4.2
#EXTRALIBS=-lsocket -lnsl_s -lcrypt

# CX/UX: This seems wierd, but DrFeelgood claims it's right.
#EXTRALIBS=-lcurses

# Don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
LIBS=-lm $(EXTRALIBS)

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################

EFUNS=efuns_main.o efuns_port.o $(EXTRA_EFUNS)

OBJ=grammar.tab.o lex.o main.o rc.o interpret.o simulate.o file.o object.o \
  backend.o array.o mapping.o comm.o ed.o regexp.o swap.o buffer.o crc32.o \
  malloc.o mallocwrapper.o \
  call_out.o otable.o dumpstat.o stralloc.o hash.o mudlib_stats.o \
  port.o reclaim.o parse.o simul_efun.o sprintf.o uid.o program.o \
  compiler.o avltree.o icode.o trees.o generate.o scratchpad.o \
  socket_efuns.o socket_ctrl.o qsort.o eoperators.o socket_err.o md.o \
  strstr.o disassembler.o binaries.o $(UALARM) $(STRFUNCS) \
  $(EFUNS) replace_program.o functab_tree.o $(EXTRA_OBJS) \
  packages/*.o

all: $(DRIVER_BIN) addr_server

remake: neat all

# For Sequent DYNIX/ptx compile $(OBJ) is parallel:
# $(DRIVER_BIN):& $(OBJ)
$(DRIVER_BIN): $(OBJ)
	-mv -f $(DRIVER_BIN) $(DRIVER_BIN).old
	$(PROOF) $(CC) $(CFLAGS) $(OBJ) -o $(DRIVER_BIN) $(LIBS)

depend: opcodes.h efun_protos.h grammar.tab.h cc.h efunctions.h efun_defs.c
	makedepend *.c

addr_server:  addr_server.o socket_ctrl.o port.o addr_server.h
	$(CC) $(CFLAGS) socket_ctrl.o addr_server.o port.o \
	-o addr_server $(RESOLV) $(EXTRALIBS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

packages/*.o: packages/Makefile
	(cd packages; $(MAKE))

grammar.tab.h: grammar.tab.c

cc.h: Makefile
	rm -f cc.h
	echo "/* this file automatically generated by the Makefile */" > cc.h
	echo '#define COMPILER "$(CC)"' >> cc.h
	echo '#define OPTIMIZE "$(OPTIMIZE)"' >> cc.h
	echo '#define CFLAGS2  "$(CFLAGS2)"' >> cc.h
	echo '#define CFLAGS   "$(CFLAGS)"' >> cc.h
	echo '#define OBJDIR   "$(OBJDIR)"' >> cc.h

lex.o: cc.h lex.c grammar.tab.h

interpret.o: interpret.c patchlevel.h

base.h: efun_defs.c

efun_protos.h: efun_defs.c

efunctions.h: efun_defs.c

opc.h: efun_defs.c

func_spec.cpp: func_spec.c op_spec.c options.h options.h port.h
	$(CPP) $(CFLAGS) func_spec.c > func_spec.cpp

make_func.tab.c: make_func.y cc.h
	$(YACC) $(YFLAGS) make_func.y
	-rm -f make_func_tab.c
	mv y.tab.c make_func.tab.c

make_func: make_func.tab.o hash.o
	$(CC) $(CFLAGS) make_func.tab.o hash.o -o make_func

make_malloc: make_malloc.o
	$(CC) $(CFLAGS) make_malloc.o -o make_malloc

make_malloc.o: make_malloc.c options.h

grammar.tab.o: opcodes.h

grammar.tab.c: grammar.y
	$(YACC) $(YFLAGS) grammar.y
	-rm -f grammar.tab.*
	sed "s/y.tab.c/grammar.tab.c/g" y.tab.c  > grammar.tab.c
	-mv y.tab.h grammar.tab.h

grammar.y: comp_warning grammar.pre

efun_defs.c opcodes.h grammar.y: malloc.c func_spec.c make_func options.h func_spec.cpp
	./make_func > efun_defs.c

malloc.c: options.h make_malloc
	./make_malloc 
	touch malloc.c
	$(CC) $(CFLAGS) -c malloc.c
	touch mallocwrapper.c
	$(CC) $(CFLAGS) -c mallocwrapper.c

tags: force
	ctags *.c *.y

TAGS: force
	etags *.c *.y

force:

install: $(DRIVER_BIN) addr_server
	-mkdir $(INSTALL_DIR)
	$(INSTALL) $(DRIVER_BIN) $(INSTALL_DIR)
	$(INSTALL) addr_server $(INSTALL_DIR)

neat:
	-(cd packages; $(MAKE) clean)
	-rm packages/GNUmakefile packages/Makefile packages/SMakefile
	-rm -rf obj *.o *.tab.c *.tab.h
	-rm -f efun_defs.c option_defs.c
	-rm -f cc.h
	-rm -f opcodes.h efunctions.h opc.h efun_protos.h
	-rm -f make_func make_malloc malloc.c mallocwrapper.c
	-rm -f func_spec.cpp
	-rm -f grammar.y

clean: neat
	-rm -f *.orig *.rej
	-rm -f */*.orig */*.rej
	-rm -f *.ln tags */*~ *~ TAGS
	-rm -f $(DRIVER_BIN) $(DRIVER_BIN).old addr_server

# DO NOT DELETE THIS LINE -- make depend depends on it.
