
OBJ=interface.o
SRC=interface.c

.c.o:
	$(CC) $(CFLAGS) -I.. -c $*.c

all: $(OBJ)
	ar rlcu mudlib.a $(OBJ)
	ranlib mudlib.a

clean:
	-rm -f *.o
	-rm -f mudlib.a


