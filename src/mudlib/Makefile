
OBJ=interface.$(O)
SRC=interface.c

.c.$(O):
	$(CC) $(CFLAGS) -I.. -c $*.c

all: $(OBJ)
	ar rcu mudlib.$(A) $(OBJ)
	$(RANLIB) mudlib.$(A)

clean:
	-rm -f *.$(O)
	-rm -f mudlib.$(A)

