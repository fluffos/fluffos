#################################################
#    GNUmakefile for the MudOS driver           #
#################################################
 
###########################
# setup and configuration #
###########################
 
# Note: this Makefile may need modified depending upon which type of machine
# you are using to compile the driver.  Be sure to read all of the comments
# in this file since they may pertain to your particular machine.

# define this to be where you want the compiled files to go
OBJDIR = obj
 
# Set INSTALL_DIR to the directory where you want to install the executables.
INSTALL_DIR = ../bin
 
# change this if you wish the driver binary to be named something else
DRIVER_BIN = driver
 
# uncomment RESOLV if your machine has libresolv.a in /lib or /usr/lib
#RESOLV=-lresolv
 
# uncomment STRFUNCS if your machine is missing memcpy(), memset(), strtol(),
# and strcspn().  (ie Sequent Dynix/3)
#STRFUNCS=strfuncs.c
 
# uncomment UALARM if your machine is missing ualarm() (e.g. those
# running Linux, HP, A/UX, EP/IX, Sequent Dynix/3, Ultrix)
#UALARM=ualarm.c
 
# System V Release 4 (386/486)
#OSFLAGS=-DSVR4
 
# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#OSFLAGS=-I/usr/include/bsd
 
# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD

# Solaris (SunOS 5): for BSD compatible ioctl()
# for 5.4, also add the following:
#       -I/usr/bsdinclude
#OSFLAGS=-DBSD_COMP

# If you don't have strchr() and strrchr(), uncomment following STR line
#STR=-Dstrchr=index -Dstrrchr=rindex
 
#Enable warnings from the compiler, if wanted.
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
 
# MIRE stuff
# you can ignore this, it's for erik's custom use
#MIRE=-DMIRE
#MIRE_C=mire.c
#MIRELIBS=-ltext libDtype.a libeditor.a libdoppelganger.a -ldsys -lndcommon -lndclient -lorwant
 
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
 
# if you use gcc (including NeXT cc) and have lots of RAM, try uncommenting
#  this for speedier compiles:
#PIPE=-pipe
 
# try uncommenting this if you are using gcc and at runtime you see socket
# errors saying that the "set socket nonblocking" operation is not supported.
# That error is caused by old-style macros (that gcc doesn't normally grok)
# used by ioctl on some systems.
#NEED_OLD_CPP=-traditional-cpp

DEFINES=$(DEBUG) $(DEBUG_MACRO) $(DEBUG_NON_FATAL) $(OSFLAGS) $(MIRE)
CFLAGS= $(STATIC) $(WARN) $(OPTIMIZE) $(PROFIL) $(STR) $(PIPE) $(DEFINES) \
	$(NEED_OLD_CPP)
 
# set CC=cc on a NeXT (or gcc if you have a newer version than the NeXT
# comes with) and to gcc on other machines (if possible)
#
# NOTE: If you are using gcc (especially on a sun4), and the driver gives
# a runtime error regarding "operation not supported on socket", then
# try using cc instead (some installations of gcc have an error in one of
# the system include (.h) files that prevents the socket operation from
# working correctly).
 
CC=gcc
#CC=cc

# define this to be what you use for make
MAKE = gmake
#MAKE = make
 
# use this for NeXTSTEP 3.1+ (at least)
CPP=$(CC) -E -traditional-cpp
# the usual
#CPP=$(CC) -E
 
# define this to be bison if you have it, and yacc otherwise
# Note: bison uses the alloca() system call so if your machine doesn't
# have alloca(), use yacc instead of bison.
YACC=bison
YFLAGS = -d -y
#YACC=yacc
#YFLAGS = -d
 
# RS/6000 AIX: use this EXTRALIBS line (required).
#EXTRALIBS=-lbsd
 
# A/UX: may be required for Apple's UNIX (and possibly others).  Try
# uncommenting this if the make results in link errors seemingly related
# to yacc (e.g. undefined symbol "yyerror").  May also need -lbsd
#EXTRALIBS=-ly
#EXTRALIBS=-ly -lbsd 

# HP-UX: use this EXTRALIBS line for HP-UX (required)
#EXTRALIBS=-lBSD
 
# NetBSD 386: due to export restrictions...you may be missing or
#  have a crippled crypt package
#EXTRALIBS=-lcrypt
 
# NeXT: link with MallocDebug if you have a NeXT with NeXTOS 2.1 or later and
# you wish to search for memory leaks (see /NextDeveloper/Apps/MallocDebug).
# Note: linking with MallocDebug will cause the virtual size of the
# driver process to reach appoximately 40MB however the amount of real memory
# used will remain close to normal.
#EXTRALIBS=-lMallocDebug -lsys_s
 
# Sequent DYNIX/ptx: use this EXTRALIBS line (required).
#EXTRALIBS=-lsocket -linet -lnsl -lseq -lmalloc
 
# System V Release 4 (386/486)
#EXTRALIBS=-lsocket -lnsl
 
# Solaris (SunOS 5.1) 
#EXTRALIBS=-lnsl -lsocket -lresolv
 
# MIPS R3000 running EP/IX Version 1.4.3. Compiler is RISCompiler C 2.11 (cc).
#EXTRALIBS=-lbsd
 
# don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
LIBS=-lm $(EXTRALIBS) $(MIRELIBS)
 
EFUNS=efuns_main.c efuns_port.c $(EXTRA_EFUNS)

SRC=grammar.tab.c lex.c main.c rc.c interpret.c simulate.c file.c object.c \
  array.c mapping.c comm.c ed.c regexp.c mudlib_stats.c swap.c malloc.c \
  call_out.c otable.c dumpstat.c stralloc.c hash.c port.c buffer.c crc32.c \
  replace_program.c parse.c simul_efun.c sprintf.c reclaim.c backend.c \
  generate.c scratchpad.c program.c mallocwrapper.c \
  strstr.c socket_efuns.c socket_ctrl.c qsort.c eoperators.c compiler.c \
  avltree.c trees.c socket_err.c md.c uid.c disassembler.c $(MIRE_C) $(EFUNS) \
  binaries.c functab_tree.c icode.c $(UALARM) $(STRFUNCS) $(EXTRA_SRC)
 
#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################


VPATH = $(OBJDIR)

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC))) $(OBJDIR)/packages/*.o

all: $(DRIVER_BIN) addr_server

remake: neat all

$(DRIVER_BIN): $(OBJDIR) $(OBJ)
	-mv $(DRIVER_BIN) $(DRIVER_BIN).old
	$(CC) $(CFLAGS) $(OBJ) -o $(DRIVER_BIN) $(LIBS)

depend:
# $(OBJDIR) opcodes.h efun_protos.h grammar.tab.h cc.h efunctions.h \
#	 efun_defs.c
	-rm tmpdepend Dependencies
	for i in *.c do $(CPP) -MM $$i; done
	sed -e"s!^!$(OBJ)!" <tmpdepend >Dependencies
	-rm tmpdepend

interpret.o: interpret.c patchlevel.h

grammar.tab.h: grammar.tab.c

cc.h: GNUmakefile
	rm -f cc.h
	echo "/* this file automatically generated by the GNUmakefile */" > cc.h
	echo '#define COMPILER "$(CC)"' >> cc.h
	echo '#define OPTIMIZE "$(OPTIMIZE)"' >> cc.h
	echo '#define CFLAGS   "$(CFLAGS)"' >> cc.h
	echo '#define OBJDIR   "$(OBJDIR)"' >> cc.h

$(OBJDIR)/lex.o: cc.h lex.c grammar.tab.h

addr_server:  $(OBJDIR)/addr_server.o addr_server.h $(OBJDIR)/socket_ctrl.o $(OBJDIR)/port.o
	$(CC) $(CFLAGS) $(OBJDIR)/addr_server.o $(OBJDIR)/socket_ctrl.o \
	-o addr_server $(RESOLV) $(EXTRALIBS)

$(OBJDIR)/packages/*.o:
	(cd packages; $(MAKE))

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) $(CFLAGS) -o $@ -c $<

grammar.tab.c: grammar.y
	$(YACC) $(YFLAGS) grammar.y
	-rm -f grammar.tab.*
	sed "s/y.tab.c/grammar.tab.c/g" y.tab.c > grammar.tab.c
	-mv y.tab.h grammar.tab.h

opc.h: efun_defs.c

func_spec.cpp: op_spec.c func_spec.c config.h options.h port.h
	@$(CPP) $(DEFINES) func_spec.c > func_spec.cpp

$(OBJDIR)/make_func: make_func.tab.c cc.h $(OBJDIR)/hash.o
	$(CC) $< $(OBJDIR)/hash.o -o $(OBJDIR)/make_func

make_func.tab.c: make_func.y
	$(YACC) $(YFLAGS) make_func.y
	-rm -f make_func.tab.c
	mv y.tab.c make_func.tab.c

make_malloc: $(OBJDIR)/make_malloc.o
	$(CC) $(CFLAGS) -Iobj $(OBJDIR)/make_malloc.o -o make_malloc

malloc.c: config.h make_malloc
	./make_malloc
	touch malloc.c
	$(CC) $(CFLAGS) -c malloc.c -o $(OBJDIR)/malloc.o
	touch mallocwrapper.c
	$(CC) $(CFLAGS) -c mallocwrapper.c -o $(OBJDIR)/mallocwrapper.o

grammar.y: opcodes.h

efun_defs.c opcodes.h: malloc.c func_spec.c op_spec.c $(OBJDIR)/make_func config.h \
	func_spec.cpp
	$(OBJDIR)/make_func > efun_defs.c

config.h: options.h

tags: $(SRC)
	ctags $(SRC)

TAGS: $(SRC)
	etags $(SRC)

install: $(DRIVER_BIN) addr_server
	install -c $(DRIVER_BIN) $(INSTALL_DIR)/$(DRIVER_BIN)
	install -c addr_server $(INSTALL_DIR)/addr_server

neat:
	-(cd packages; make clean)
	-rm -f packages/GNUmakefile packages/Makefile packages/SMakefile
	-rm -rf $(OBJDIR) *.o *.tab.c *.tab.h
	-rm -f cc.h
	-rm -f efun_defs.c option_defs.c
	-rm -f opcodes.h efunctions.h opc.h efun_protos.h
	-rm -f make_func make_malloc malloc.c mallocwrapper.c
	-rm -f func_spec.cpp
	-rm -f grammar.y
	
clean: neat
	-rm -f mon.out gmon.out *.orig *.rej
	-rm -f */*.orig */*.rej
	-rm -f core
	-rm -f *.ln tags */*~ *~ TAGS
	-rm -f $(DRIVER_BIN) $(DRIVER_BIN).old addr_server
	-rm -f Dependencies
	-touch Dependencies

include Dependencies	# include dependencies generated by gmake depend
