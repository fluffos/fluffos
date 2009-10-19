
SRC=uids.c db.c compress.c math.c develop.c external.c contrib.c sockets.c async.c dwlib.c 
OBJ=uids.$(O) db.$(O) compress.$(O) math.$(O) develop.$(O) external.$(O) contrib.$(O) sockets.$(O) async.$(O) dwlib.$(O) 

.c.$(O):
	$(CC) $(CFLAGS) -I.. -c $*.c

all: $(OBJ)
	ar rcu packages.$(A) $(OBJ)
	$(RANLIB) packages.$(A)

db.$(O): db.c
	$(CC) $(CFLAGS) -I.. -I/usr/include/mysql -c $*.c

clean:
	rm -f *.$(O)
	rm -f packages.$(A)
