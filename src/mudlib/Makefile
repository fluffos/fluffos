
OBJ=interface.o
SRC=interface.c

.c.o:
	$(CC) $(CFLAGS) -I.. -c $*.c

all: $(OBJ)
	ar rcu mudlib.a $(OBJ)
	$(RANLIB) mudlib.a

clean:
	-rm -f *.o
	-rm -f mudlib.a


