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
SERVER_BIN = addr_server

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
# uncomment for the HP (and other machines missing ualarm)
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
# OPTIMIZE=-O
# high optimization for gcc:
#OPTIMIZE=-O2 -fomit-frame-pointer -fstrength-reduce
# uncomment below for RS/6000(AIX) xlc compiler only.
# remove the -Q if xlc complains.
#OPTIMIZE=-O -Q
# high optimization for HP-UX 7.x/8.x's cc (don't use with 9.x)
#OPTIMIZE=+O3 +Obb3000
# amiga:
OPTIMIZE=OPTIMIZE OPTSIZE CPU=68030

# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD -D_ALL_SOURCE

# System V Release 4 (386/486)
#OSFLAGS=-DSVR4

# Amiga SAS/C
OSFLAGS=DEFINE=NO_IP_DEMON DATA=FAR CODE=FAR IGNORE=100 IGNORE=161 IDIR=amiga

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
# try using cc instead (some installations of gcc have an error in one of
# the system include (.h) files that prevents some socket ioctl operations
# from working correctly).
#
#CC=cc
#CC=gcc
#CC=xlc
CC=sc
LINK=slink

# define this to be bison if you have it, and yacc otherwise.
# The yaccs on SunOS and A/UX both have problems with compiler.y
# because of hardcoded limits on the number of symbols.  You
# can either build compiler.tab.* on another machine and move them
# over or you can get bison from prep.ai.mit.edu:/pub/gnu.  Expect
# one shift/reduce conflict if using Bison (from the IF ELSE construct).
#YACC=bison
#YFLAGS=-d -y
YACC=byacc
YFLAGS = -d

# amiga stuff
#AMIGA_C = amiga/amiga.c amiga/signal.c amiga/signal_rr.c amiga/socket.c \
AMIGA_C = amiga/signal.c amiga/signal_rr.c amiga/socket.c \
	amiga/crypt.c
#AMIGA_O = amiga/amiga.o amiga/signal.o amiga/signal_rr.o amiga/socket.o \
AMIGA_O = amiga/signal.o amiga/signal_rr.o amiga/socket.o \
	amiga/crypt.o
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
#EXTRALIBS=-lnsl -lsocket -lresolv

# Don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
#LIBS=-lm $(EXTRALIBS)
# equivalant sas libs:
LIBS=LIB LIB:scm.lib LIB:sc.lib LIB:amiga.lib

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################

SRC=compiler.tab.c lex.c main.c rc.c interpret.c simulate.c object.c \
  backend.c file.c array.c mapping.c comm.c ed.c regexp.c swap.c malloc.c \
  call_out.c otable.c dumpstat.c stralloc.c hash.c port.c reclaim.c \
  access_check.c parse.c mudlib_stats.c simul_efun.c sprintf.c uid.c \
  socket_efuns.c eoperators.c socket_ctrl.c socket_err.c \
  qsort.c md.c disassembler.c $(UALARM_C) $(STRFUNCS_C) efuns_sock.c \
  efuns_main.c efuns_math.c efuns_matrix.c efuns_port.c $(MALLOC_C) $(AMIGA_C)

OBJ=compiler.tab.o lex.o main.o rc.o interpret.o simulate.o file.o object.o \
  backend.o array.o mapping.o comm.o ed.o regexp.o swap.o malloc.o \
  call_out.o otable.o dumpstat.o stralloc.o hash.o mudlib_stats.o port.o \
  reclaim.o access_check.o parse.o simul_efun.o sprintf.o uid.o \
  socket_efuns.o socket_ctrl.o qsort.o eoperators.o \
  socket_err.o md.o disassembler.o $(UALARM_O) $(STRFUNC_O) efuns_sock.o \
  efuns_main.o efuns_math.o efuns_matrix.o efuns_port.o $(MALLOC_O) $(AMIGA_O)

# 
all: compiler.tab.c $(DRIVER_BIN) $(SERVER_BIN) #don't ask

$(DRIVER_BIN): $(OBJ)
	-delete $(DRIVER_BIN).old
	-rename $(DRIVER_BIN) $(DRIVER_BIN).old
	list $(OBJ) lformat "%f%s" >obj.list
	$(LINK) lib:c.o with obj.list TO $(DRIVER_BIN) stripdebug $(LIBS)

#all: $(DRIVER_BIN) addr_server

depend:
	makedepend *.c

$(SERVER_BIN):  addr_server.o socket_ctrl.o port.o addr_server.h
	$(LINK) lib:c.o socket_ctrl.o addr_server.o port.o $(AMIGA_O) $(RESOLV) \
		to addr_server lib lib:sc.lib lib:amiga.lib lib:scm.lib

.c.o:
	$(CC) $(CFLAGS) $*.c

#amiga files:
amiga/amiga.o:  amiga/amiga.c

amiga/socket.o: amiga/socket.c amiga/socket_sim.c

amiga/signal.o: amiga/signal.c

amiga/signal_rr.o:      amiga/signal_rr.c

amiga/crypt.o:	amiga/crypt.c

lex.c:	compiler.tab.h

compiler.tab.h: compiler.tab.c

base.h: efun_defs.c

efun_protos.h: efun_defs.c

efunctions.h: efun_defs.c

opc.h: efun_defs.c

# program to remove comments from the preprocessor output.
# (SAS has no option for it)
amiga/strip: amiga/strip.c
	sc link amiga/strip.c

func_spec.cpp: func_spec.c op_spec.c config.h options.h port.h amiga/strip
	sc PPONLY DEF=NO_IP_DEMON= func_spec.c
	amiga/strip func_spec.p >func_spec.cpp

make_func.tab.c: make_func.y
	$(YACC) $(YFLAGS) make_func.y
	-delete make_func.tab.c
	rename y.tab.c make_func.tab.c

make_func: make_func.tab.c
	$(CC) $(CFLAGS) LINK make_func.tab.c 
	-delete make_func
	rename make_func.tab make_func

make_malloc: make_malloc.c
	$(CC) $(CFLAGS) LINK make_malloc.c 

compiler.tab.o: opcodes.h

compiler.tab.c: compiler.y
	$(YACC) $(YFLAGS) compiler.y
	-delete compiler.tab.?
	rename y.tab.c compiler.tab.c
	rename y.tab.h compiler.tab.h

efun_defs.c opcodes.h: malloc.c func_spec.c make_func config.h func_spec.cpp
	make_func > efun_defs.c

malloc.c: config.h make_malloc
	make_malloc
	touch malloc.c
	$(CC) $(CFLAGS) malloc.c

tags: $(SRC)
	ctags $(SRC)

TAGS: $(SRC)
	etags $(SRC)

install: $(DRIVER_BIN) addr_server
	-mkdir $(INSTALL_DIR)
	$(INSTALL) $(DRIVER_BIN) $(INSTALL_DIR)
	$(INSTALL) addr_server $(INSTALL_DIR)

clean:
	-delete obj *.o mon.out gmon.out *.tab.? *.orig *.rej
	-delete *.ln tags TAGS *.p efun_defs.c 
	-delete opcodes.h efunctions.h opc.h base.h efun_protos.h
	-delete make_func make_malloc $(DRIVER_BIN) $(DRIVER_BIN).old addr_server
	-delete func_spec.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.
