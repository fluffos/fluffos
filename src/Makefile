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

# change this if you wish the driver binary to be named anything other than
# driver (we call it driver because it does more than parse)
DRIVER_BIN = driver

# RESOLV: define this to be -lresolv if your machine has it.
# HP and NeXT don't have it.
#RESOLV=-lresolv

# uncomment MALLOC* if you wish to use BSD malloc or Smalloc.
#   Both of these require the sbrk() system call.  Be sure to read the
#   comments in the bsdmalloc.c file before deciding on BSDMALLOC.
#   note: These mallocs may not work correctly on the NeXT or other systems
#   that don't fully support sbrk().  It is okay not to use either of these
#   two mallocs in which case you will get system malloc (read the comments
#   in options.h for choices on malloc statistics and debugging).
# uncomment these two lines for BSD Malloc.
#
#MALLOC_C=bsdmalloc.c
#MALLOC_O=bsdmalloc.o
#
# uncomment these two lines for Smalloc (slightly slower than BSD malloc
# but quite a bit less memory overhead).
#
#MALLOC_C=smalloc.c
#MALLOC_O=smalloc.o

# uncomment STRFUNCS* if your machine is missing memcpy(), memset(), strtol(),
# and strcspn(). (needed by Sequent Dynix/3)
#STRFUNCS_C=strfuncs.c
#STRFUNCS_O=strfuncs.o

# uncomment UALARM* if your machine is missing ualarm().
# uncomment for the HP, A/UX (Apple Mac), EP/IX (MIPS),
# Sequent Dynix/3 (BSD 4.3)
# (and other machines missing ualarm)
#UALARM_C=ualarm.c
#UALARM_O=ualarm.o

# Command used to install executables in the INSTALL_DIR
#Use the -f version for HP/UX
#INSTALL = install -f
#INSTALL = install -c
# Use cp if all else fails
INSTALL = cp

# Set INSTALL_DIR to the directory where you want to install the executables.
INSTALL_DIR = ../bin

#Enable warnings from the compiler (gcc), if wanted.
#WARN=-Wall

# define profiling if you want it
# note: the gmon.out file will likely be written in the mudlib dir.
# PROFILE_ON controls whether or not monitoring is active at driver
# startup.  Comment PROFILE_ON to make profiling not active at startup.
# Use moncontrol(1) efun to enable profiling and moncontrol(0) to turn
# it off.
#PROFILE_ON=-DPROFILE_ON
# Uncomment this if you want to enable profiling of the driver
#PROFIL=-pg -DPROFILING $(PROFILE_ON)

# Enable run time debugging.
#DEBUG=-g -DDEBUG
#DEBUG=-g
# compile in debug() macro code
#DEBUG_MACRO=-DDEBUG_MACRO
# prevent -DDEBUG from aborting the driver (when in -DDEBUG mode)
#DEBUG_NON_FATAL=-DDEBUG_NON_FATAL

# If you don't have strchr() and strrchr(), then uncomment the following line.
#STR=-Dstrchr=index -Dstrrchr=rindex

# define this if you want (compiler) optimization enabled.
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
# high optimization for HP-UX 7.x/8.x's cc (don't use with 9.x)
#OPTIMIZE=+O3 +Obb3000
# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#OPTIMIZE=-Olimit 1802
# DEC Ultrix's cc:
#OPTIMIZE=-O2 -Olimit 5000

# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD -D_ALL_SOURCE

# System V Release 4 (386/486)
#OSFLAGS=-DSVR4

# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#OSFLAGS=-I/usr/include/bsd

# if you use gcc (including NeXT cc) and have lots of RAM, try uncommenting
#  this for speedier compiles:
#PIPE=-pipe

# try uncommenting this if you are using gcc and at runtime you see socket
# errors saying that the "set socket nonblocking" operation is not supported.
# That error is caused by old-style macros (that gcc doesn't normally grok)
# used by ioctl on some systems.
#NEED_OLD_CPP=-traditional-cpp

CFLAGS= $(OSFLAGS) $(OPTIMIZE) $(WARN) $(PROFIL) $(DEBUG) $(DEBUG_MACRO) \
   $(DEBUG_NON_FATAL) $(STR) $(PIPE) $(NEED_OLD_CPP)

# set CC=cc on a NeXT and to gcc on other machines (if possible)
# You may want to use xlc on an RS/6000 (produces faster code).
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
#CC=xlc

# define this to be bison if you have it, and yacc otherwise.
#YACC=bison
#YFLAGS=-d -y
YACC=yacc
YFLAGS = -d

# Add extra libraries here (using EXTRALIBS)

# A/UX: may be required for Apple's UNIX (and possibly others).  Try
# uncommenting this if the make results in link errors seemingly related
# to yacc (e.g. undefined symbol "yyerror").
#EXTRALIBS=-ly

# HP-UX: use this EXTRALIBS line for HP-UX (required).
#EXTRALIBS=-lBSD

# RS/6000 AIX: use this EXTRALIBS line (required).
#EXTRALIBS=-lbsd

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

# Don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
LIBS=-lm $(EXTRALIBS)

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################

EFUNS_SRC=efuns_main.c efuns_port.c efuns_math.c efuns_matrix.c efuns_sock.c
EFUNS_OBJ=efuns_main.o efuns_port.o efuns_math.o efuns_matrix.o efuns_sock.o

SRC=lex.c main.c rc.c interpret.c simulate.c object.c backend.c file.c\
  array.c mapping.c comm.c ed.c regexp.c swap.c malloc.c\
  call_out.c otable.c dumpstat.c stralloc.c hash.c port.c\
  reclaim.c access_check.c parse.c compiler.y mudlib_stats.c \
  simul_efun.c sprintf.c uid.c socket_efuns.c eoperators.c \
  socket_ctrl.c socket_err.c qsort.c strstr.c \
  md.c disassembler.c binaries.c $(UALARM_C) $(STRFUNCS_C) $(MALLOC_C) $(EFUNS_SRC)

OBJ=compiler.tab.o lex.o main.o rc.o interpret.o simulate.o file.o object.o \
  backend.o array.o mapping.o comm.o ed.o regexp.o swap.o \
  malloc.o call_out.o otable.o dumpstat.o stralloc.o hash.o mudlib_stats.o \
  port.o reclaim.o access_check.o parse.o simul_efun.o sprintf.o uid.o \
  socket_efuns.o socket_ctrl.o qsort.o eoperators.o socket_err.o md.o \
  strstr.o disassembler.o binaries.o $(UALARM_O) $(STRFUNCS_O) $(MALLOC_O) $(EFUNS_OBJ)

all: $(DRIVER_BIN) addr_server

# For Sequent DYNIX/ptx compile $(OBJ) is parallel:
# $(DRIVER_BIN):& $(OBJ)
$(DRIVER_BIN): $(OBJ)
	-mv $(DRIVER_BIN) $(DRIVER_BIN).old
	$(CC) $(CFLAGS) $(OBJ) -o $(DRIVER_BIN) $(LIBS)

all: $(DRIVER_BIN) addr_server

depend:
	makedepend *.c

addr_server:  addr_server.o socket_ctrl.o port.o addr_server.h
	$(CC) $(CFLAGS) socket_ctrl.o addr_server.o port.o $(RESOLV) \
	-o addr_server $(EXTRALIBS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

compiler.tab.h: compiler.tab.c

cc.h: Makefile
	rm -f cc.h
	echo "/* this file automatically generated by the Makefile */" > cc.h
	echo '#define COMPILER "$(CC)"' >> cc.h
	echo '#define OPTIMIZE "$(OPTIMIZE)"' >> cc.h

lex.o: cc.h lex.c compiler.tab.h

interpret.o: interpret.c patchlevel.h

base.h: efun_defs.c

efun_protos.h: efun_defs.c

efunctions.h: efun_defs.c

opc.h: efun_defs.c

func_spec.cpp: func_spec.c op_spec.c config.h options.h port.h
	@$(CC) -E $(CFLAGS) func_spec.c > func_spec.cpp

make_func.tab.c: make_func.y
	$(YACC) $(YFLAGS) make_func.y
	mv y.tab.c make_func.tab.c

make_func: make_func.tab.o
	$(CC) $(CFLAGS) make_func.tab.o -o make_func

make_malloc: make_malloc.o
	$(CC) $(CFLAGS) make_malloc.o -o make_malloc

make_malloc.o: make_malloc.c config.h

compiler.tab.o: opcodes.h

compiler.tab.c: compiler.y
	$(YACC) $(YFLAGS) compiler.y
	rm -f compiler.tab.*
	ln y.tab.c compiler.tab.c
	ln y.tab.h compiler.tab.h

efun_defs.c opcodes.h: malloc.c func_spec.c make_func config.h func_spec.cpp
	./make_func > efun_defs.c

malloc.c: config.h make_malloc
	./make_malloc $(MALLOC_C)
	touch malloc.c
	$(CC) $(CFLAGS) -c malloc.c

tags: $(SRC)
	ctags $(SRC)

TAGS: $(SRC)
	etags $(SRC)

install: $(DRIVER_BIN) addr_server
	-mkdir $(INSTALL_DIR)
	$(INSTALL) $(DRIVER_BIN) $(INSTALL_DIR)
	$(INSTALL) addr_server $(INSTALL_DIR)

clean:
	-rm -rf obj *.o mon.out gmon.out *.tab.c *.tab.h *.orig *.rej
	-rm -f efun_defs.c *.ln tags *~ TAGS
	-rm -f cc.h
	-rm -f opcodes.h efunctions.h opc.h base.h efun_protos.h
	-rm -f make_func make_malloc $(DRIVER_BIN) $(DRIVER_BIN).old addr_server
	-rm -f func_spec.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.
