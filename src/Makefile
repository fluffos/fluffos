#################################################
#       Standard Makefile for MudOS driver      #
#################################################

SHELL=/bin/sh

###########################
# setup and configuration #
###########################

# Note: this Makefile may need modified depending upon which type of machine
# you are to compile the driver.
#
# Edit options.h to choose which malloc you would like to use.
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

# define this to be -lresolv if your machine has it.
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
#MALLOC_C=bsdmalloc.c
#MALLOC_O=bsdmalloc.o
# uncomment these two lines for Smalloc (slightly slower than BSD malloc
# but quite a bit less overhead).
#MALLOC_C=smalloc.c
#MALLOC_O=smalloc.o

# uncomment STRFUNCS* if your machine is missing memcpy(), memset(), strtol(),
# and strcspn().
#STRFUNCS_C=strfuncs.c
#STRFUNCS_O=strfuncs.o

# uncomment UALARM* if your machine is missing ualarm().
# uncomment for the HP and Solaris (and other machines missing ualarm)
#UALARM_C=ualarm.c
#UALARM_O=ualarm.o

# Command used to install executables in the INSTALL_DIR
INSTALL = install -c
#INSTALL = cp

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
# compile in debug() macro code
#DEBUG_MACRO=-DDEBUG_MACRO
# prevent -DDEBUG from aborting the driver (when in -DDEBUG mode)
#DEBUG_NON_FATAL=-DDEBUG_NON_FATAL

# If you don't have strchr() and strrchr(), then uncomment the following line
#STR=-Dstrchr=index -Dstrrchr=rindex

# define this if you want (compiler) optimization enabled.
# *WARNING* using high levels of optimization (e.g. -O3) can cause some
# compilers to produce incorrect code.  If the driver is behaving
# inexplicably, try using a lower level of optimization (or none).
#
# Uncomment one or none of the following optimization lines.
# 
# -O is usually a safe level of optimization for most compilers
#OPTIMIZE=-O
# uncomment for gcc
#OPTIMIZE=-O2 -fomit-frame-pointer -fstrength-reduce
# uncomment below for RS/6000(AIX) xlc compiler only.
# remove the -Q if xlc complains.
#OPTIMIZE=-O -Q
# uncomment for HP-UX's 7.x/8.x's cc
#OPTIMIZE=+O3 +Obb3000

# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD -D_ALL_SOURCE

# System V Release 4 (386/486)
#OSFLAGS=-DSVR4

# if you use gcc (including NeXT cc) and have lots of RAM, try uncommenting
#  this for speedier compiles:
#PIPE=-pipe

CFLAGS= $(OSFLAGS) $(OPTIMIZE) $(WARN) $(PROFIL) $(DEBUG) $(DEBUG_MACRO) \
   $(DEBUG_NON_FATAL) $(STR) $(PIPE)

# set CC=cc on a NeXT and to gcc on other machines (if possible)
# Note: at this time gcc (2.2.2) for HP doesn't implement varargs properly;
# if you have problems compiling on an HP, try using cc instead of gcc.
# You may want to use xlc on an RS/6000 (produces faster code).
#CC=cc
CC=gcc
#CC=xlc

# define this to be bison if you have it, and yacc otherwise.
# The yaccs on SunOS and A/UX both have problems with lang.y
# because of hardcoded limits on the number of symbols.  You
# can either build lang.tab.* on another machine and move them
# over or you can get bison from prep.ai.mit.edu:/pub/gnu.
#YACC=bison
#YFLAGS=-d -y
YACC=yacc
YFLAGS = -d

# Add extra libraries here (using EXTRALIBS)

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

# Sequent DYNIX/ptx: use this EXTRALIBS line (required).
#EXTRALIBS=-lsocket -linet -lnsl -lseq

# System V Release 4 (386/486)
#EXTRALIBS=-lsocket -lnsl

# Solaris (SunOS 5.1)
# -Xlinker and -R/usr/ucblib are to get "getrusage" and "ualarm" from
# compat lib
#EXTRALIBS=-lnsl -lsocket -lresolv -Xlinker -R/usr/ucblib

# Don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
LIBS=-lm $(EXTRALIBS)

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################

SRC=lex.c main.c rc.c interpret.c simulate.c object.c backend.c file.c\
  array.c mapping.c comm.c ed.c regexp.c swap.c malloc.c\
  call_out.c otable.c dumpstat.c stralloc.c hash.c port.c\
  reclaim.c access_check.c parse.c prelang.y postlang.y mudlib_stats.c \
  simul_efun.c sprintf.c uid.c socket_efuns.c efunctions.c eoperators.c \
  socket_ctrl.c socket_err.c qsort.c math.c matrix.c\
  md.c $(UALARM_C) $(STRFUNCS_C) $(MALLOC_C)

OBJ=lang.tab.o lex.o main.o rc.o interpret.o simulate.o file.o object.o \
  backend.o array.o mapping.o comm.o ed.o regexp.o swap.o \
  malloc.o call_out.o otable.o dumpstat.o stralloc.o hash.o mudlib_stats.o \
  port.o reclaim.o access_check.o parse.o simul_efun.o sprintf.o uid.o \
  socket_efuns.o efunctions.o eoperators.o socket_ctrl.o qsort.o math.o \
  matrix.o socket_err.o md.o $(UALARM_O) $(STRFUNCS_O) $(MALLOC_O)

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
	$(CC) $(CFLAGS) socket_ctrl.o addr_server.o port.o $(RESOLV) -o addr_server \
	$(EXTRALIBS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

lang.tab.h: lang.tab.c

base.h: efun_defs.c

efun_protos.h: efun_defs.c

efunctions.h: efun_defs.c

opc.h: efun_defs.c

list_funcs:
	@$(CC) -E $(CFLAGS) func_spec.c

make_func.tab.c: make_func.y
	$(YACC) $(YFLAGS) make_func.y
	mv y.tab.c make_func.tab.c

make_func: make_func.tab.o
	$(CC) $(CFLAGS) make_func.tab.o -o make_func

make_malloc: make_malloc.o
	$(CC) $(CFLAGS) make_malloc.o -o make_malloc

make_malloc.c: config.h

lang.tab.c lang.h: lang.y
	$(YACC) $(YFLAGS) lang.y
	rm -f lang.tab.*
	ln y.tab.c lang.tab.c
	ln y.tab.h lang.tab.h

lang.y efun_defs.c: malloc.c func_spec.c make_func prelang.y \
	postlang.y config.h
	./make_func > efun_defs.c

malloc.c: config.h make_malloc
	./make_malloc
	touch malloc.c
	$(CC) $(CFLAGS) -c malloc.c

tags: $(SRC)
	ctags $(SRC)

TAGS: $(SRC)
	etags $(SRC)

install: $(DRIVER_BIN) addr_server
	$(INSTALL) $(DRIVER_BIN) $(INSTALL_DIR)/$(DRIVER_BIN)
	$(INSTALL) addr_server $(INSTALL_DIR)/addr_server

clean:
	-rm -rf obj *.o mon.out gmon.out *.tab.c *.tab.h *.orig *.rej
	-rm -f lang.h lang.y efun_defs.c *.ln tags *~ TAGS
	-rm -f efunctions.h opc.h base.h efun_protos.h
	-rm -f make_func make_malloc $(DRIVER_BIN) $(DRIVER_BIN).old addr_server

# DO NOT DELETE THIS LINE -- make depend depends on it.
