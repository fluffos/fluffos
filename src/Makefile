#################################################
#       Standard Makefile for MudOS driver      #
#################################################

###########################
# setup and configuration #
###########################

# edit config.h to choose which malloc you would like to use.

# define this to be -lresolv if your machine has it
RESOLV=

# Set INSTALL_DIR to the directory where you want to install the executables.
INSTALL_DIR = /home/dogstar/worlds/bin

#Enable warnings from the compiler, if wanted.
WARN=

# define profiling if you want it
#PROFIL=-pg

# Enable run time debugging.
#DEBUG=-DDEBUG
#DEBUG_MACRO=-DDEBUG_MACRO

# If you don't have 'strchr', then add next flag to CFLAGS.
# -Dstrchr=index -Dstrrchr=rindex

CFLAGS= -g $(WARN) $(PROFIL) $(DEBUG) $(DEBUG_MACRO)

# set CC=cc on a NeXT and to gcc on other machines (if possible)
CC=cc

# define this to be bison if you have it, and yacc otherwise
YACC=yacc
YFLAGS = -d

# Add extra libraries here.
# This is the default case.
LIBS= -lm
# link with MallocDebug if you have a NeXT with NeXTOS 2.1 or later and
# you wish to search for memory leaks (see /NextDeveloper/Apps/MallocDebug).
#LIBS= -lm -lMallocDebug -lsys_s

#################################################
# the meat of things                            #
# don't change anything below this section      #
#################################################

# Add str.c here if you don't have memcpy() or strtol()
# add alloca.c here if you don't have alloc()
#
SRC=lex.c main.c rc.c interpret.c simulate.c object.c backend.c file.c\
  array.c mapping.c comm1.c ed.c regexp.c wiz_list.c swap.c malloc.c\
  call_out.c otable.c dumpstat.c stralloc.c hash.c indentp.c port.c\
  access_check.c clilib.c parse.c prelang.y postlang.y \
  simul_efun.c sprintf.c

# Add str.o here if you don't have memcpy() or strtol().
# Add alloca.o if you don't have alloca() on your machine.
#
OBJ=lang.tab.o lex.o main.o rc.o interpret.o simulate.o file.o object.o \
  backend.o array.o mapping.o comm1.o ed.o regexp.o wiz_list.o swap.o \
  malloc.o call_out.o otable.o dumpstat.o stralloc.o hash.o indentp.o \
  port.o access_check.o clilib.o parse.o simul_efun.o sprintf.o

driver: $(OBJ)
	-mv driver driver.old
	$(CC) $(CFLAGS) $(OBJ) -o driver $(LIBS)

all: driver hname

depend:
	makedepend *.c

hname:  hname.c
	$(CC) hname.c $(RESOLV) -o hname

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $*.c

list_funcs:
	@$(CC) -E $(CFLAGS) func_spec.c

make_func.tab.c: make_func.y
	$(YACC) $(YFLAGS) make_func.y
	mv y.tab.c make_func.tab.c

make_func: make_func.tab.o
	$(CC) make_func.tab.o -o make_func

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

install: driver hname
	install -c driver $(INSTALL_DIR)/driver
	install -c hname $(INSTALL_DIR)/hname

clean:
	-rm -rf obj *.o mon.out gmon.out *.tab.c *.tab.h
	-rm -f lang.h lang.y efun_defs.c *.ln tags *~ TAGS
	-rm -f make_func make_malloc driver driver.old hname

# DO NOT DELETE THIS LINE -- make depend depends on it.
