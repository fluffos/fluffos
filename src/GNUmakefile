#################################################
# GNUmakefile for MudOS driver                  #
# 	- requires gnu make in order to use it      #
#################################################

###########################
# setup and configuration #
###########################

# Note: this Makefile may need modified depending upon which type of machine
# you are to compile the driver.  In particular, if you are using an HP,
# Sequent, or RS/6000, be sure to search the Makefile for all comments
# pertaining to your machine.

# note: Edit options.h to further configure the driver for your particular mud.
# Also, be sure to look at the EXTRALIBS macro later in the Makefile.
# Some machines require it to be set to a specific value.

# Set INSTALL_DIR to the directory where you want to install the executables.
INSTALL_DIR = ../bin

# Change this if you wish the driver binary to be named something other than
# driver.
DRIVER_BIN = driver

# uncomment RESOLV if your machine has libresolv.a in /lib or /usr/lib
#RESOLV=-lresolv

# uncomment one MALLOC if you wish to use BSD malloc or Smalloc.
# Both of these require the sbrk() system call.  Be sure to read the
# comments in the bsdmalloc.c file before deciding on BSDMALLOC.
# note: These mallocs may not work correctly on the NeXT or other systems
# that don't fully support sbrk().  It is okay not to use either of these
# two mallocs in which case you will get system malloc (read the comments
# in options.h for choices on malloc statistics and debugging).
#
# uncomment this line for BSD Malloc.
#MALLOC=bsdmalloc.c
# uncomment this line for Smalloc
#MALLOC=smalloc.c

# uncomment STR if your machine is missing memcpy(), memset(), strtol(),
# and strcspn().
#STRFUNCS=strfuncs.c

# uncomment UALARM if your machine is missing ualarm() (e.g. those
# running Linux)
#UALARM=ualarm.c

# RS/6000 AIX: use this OSFLAGS line (required).
#OSFLAGS=-D_BSD

# If you don't have strchr() and strrchr(), uncomment following STR line
#STR=-Dstrchr=index -Dstrrchr=rindex

#Enable warnings from the compiler, if wanted.
WARN=-Wall

# define profiling if you want it
# note: the gmon.out file will likely be written in the mudlib dir.
# PROFILE_ON controls whether or not monitoring is active at driver
# startup.  Comment PROFILE_ON to make profiling not active at startup.
# Use moncontrol(1) efun to enable profiling and moncontrol(0) to turn
# it off.
#PROFILE_OFF=-DPROFILE_ON
# Uncomment this if you want to enable profiling of the driver
PROFIL=-pg -DPROFILING $(PROFILE_ON)

# define this if you want (compiler) optimization enabled.
# *WARNING* using high levels of optimization (e.g. -O3) can cause some
# compilers to produce incorrect code.  If the driver is behaving
# inexplicably, try using a lower level of optimization (or none).
# Uncomment one or none of the following optimization lines.  The
# first is the safer, the second is the one likely to produce more
# improvement in driver speed (it uses GCC flags).
#
#OPTIMIZE=-O
#OPTIMIZE=-O2 -fomit-frame-pointer -fstrength-reduce

# MIRE stuff
# you can ignore this, it's for erik's custom use
#MIRE=-DMIRE
#MIRE_C=mire.c
#MIRELIBS=-ltext libDtype.a libeditor.a libdoppelganger.a -ldsys -lndcommon -lndclient -lorwant

# uncomment DEBUG to enable run time debugging.
#DEBUG=-g -DDEBUG
# uncomment DEBUG_MACRO to compile in debug() macro code
#DEBUG_MACRO=-DDEBUG_MACRO
# prevent -DDEBUG from abort()'ing the driver (when in -DDEBUG mode)
#DEBUG_NON_FATAL=-DDEBUG_NON_FATAL

# if you use gcc (including NeXT cc) and have lots of RAM, try uncommenting
#  this for speedier compiles:
#PIPE=-pipe

DEFINES=$(DEBUG) $(DEBUG_MACRO) $(DEBUG_NON_FATAL) $(OSFLAGS) $(MIRE)
CFLAGS= $(WARN) $(OPTIMIZE) $(PROFIL) $(STR) $(PIPE) $(DEFINES)

# set CC=cc on a NeXT (or gcc if you have a newer version than the NeXT
# comes with) and to gcc on other machines (if possible)
# Note: at this time gcc (2.2.2) for HP doesn't implement varargs properly;
# if you have problems compiling on an HP, try using cc instead of gcc.
CC=gcc

CPP=gcc -E

# define this to be bison if you have it, and yacc otherwise
#YACC=bison
#YFLAGS = -d -y
YACC=yacc
YFLAGS = -d

# Add extra libraries here.

# RS/6000 AIX: use this EXTRALIBS line (required).
#EXTRALIBS=-lbsd

# HP-UX: use this EXTRALIBS line for HP-UX (required)
#EXTRALIBS=-lBSD

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

# don't change this line.  Define EXTRALIBS before this line if you
# wish to add any libraries.
LIBS=-lm $(EXTRALIBS) $(MIRELIBS)

SRC=lang.tab.c lex.c main.c rc.c interpret.c simulate.c file.c object.c \
  array.c mapping.c comm.c ed.c regexp.c mudlib_stats.c swap.c malloc.c \
  call_out.c otable.c dumpstat.c stralloc.c hash.c port.c \
  access_check.c parse.c simul_efun.c sprintf.c backend.c \
  socket_efuns.c efunctions.c eoperators.c socket_ctrl.c qsort.c \
  socket_err.c md.c uid.c $(MIRE_C) \
  $(UALARM) $(STRFUNCS) $(MALLOC)

# define this to be where you want the compiled files to go
OBJDIR = obj

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################


VPATH = $(OBJDIR)

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))

all: $(DRIVER_BIN) addr_server

$(DRIVER_BIN): $(OBJDIR) $(OBJ)
	-mv $(DRIVER_BIN) $(DRIVER_BIN).old
	$(CC) $(CFLAGS) $(OBJ) -o $(DRIVER_BIN) $(LIBS)

depend:
	-rm Dependencies
	touch Dependencies
	for i in *.c; do echo -n $(OBJDIR)/ >> Dependencies; $(CC) -E -MM $$i >> Dependencies; done

addr_server:  $(OBJDIR)/addr_server.o addr_server.h $(OBJDIR)/socket_ctrl.o $(OBJDIR)/port.o
	$(CC) $(CFLAGS) $(OBJDIR)/addr_server.o $(OBJDIR)/socket_ctrl.o $(RESOLV) -o addr_server $(EXTRALIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) $(CFLAGS) -o $@ -c $<

%.tab.c: %.y
	$(YACC) $(YFLAGS) $<
	-mv y.tab.c $@
	-mv y.tab.h $*.tab.h

list_funcs:
	@$(CPP) $(DEFINES) func_spec.c

make_func: make_func.tab.c
	$(CC) $< -DFUNC_SPEC=\"$(MAKE)\ list_funcs\" -o make_func

make_malloc: $(OBJDIR)/make_malloc.o
	$(CC) -Iobj $(OBJDIR)/make_malloc.o -o make_malloc

malloc.c: config.h make_malloc
	./make_malloc
	touch malloc.c
	$(CC) $(CFLAGS) -c malloc.c -o $(OBJDIR)/malloc.o

lang.y efun_defs.c: malloc.c func_spec.c make_func prelang.y \
		postlang.y config.h
	./make_func > efun_defs.c

config.h: options.h

tags: $(SRC)
	ctags $(SRC)

TAGS: $(SRC)
	etags $(SRC)

install: $(DRIVER_BIN) addr_server
	install -c $(DRIVER_BIN) $(INSTALL_DIR)/$(DRIVER_BIN)
	install -c addr_server $(INSTALL_DIR)/addr_server

clean:
	-rm -rf obj *.o mon.out gmon.out *.tab.c *.tab.h *.orig *.rej
	-rm -f lang.h lang.y efun_defs.c *.ln tags *~ TAGS
	-rm -f efunctions.h base.h efun_protos.h
	-rm -f addr_server make_func $(DRIVER_BIN) $(DRIVER_BIN).old make_malloc

include Dependencies	# include dependencies generated by gmake depend
