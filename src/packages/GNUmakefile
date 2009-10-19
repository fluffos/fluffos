
SRC=uids.c db.c compress.c math.c develop.c external.c contrib.c sockets.c async.c dwlib.c 
OBJ=uids.$(O) db.$(O) compress.$(O) math.$(O) develop.$(O) external.$(O) contrib.$(O) sockets.$(O) async.$(O) dwlib.$(O) 

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) -I.. $(CFLAGS) -o $@ -c $<

all: $(OBJ)
	ar rcu packages.$(A) $(OBJ)
	$(RANLIB) packages.$(A)

$(OBJDIR)/db.o: db.c
	$(CC) -I$(OBJDIR) -I.. -I/usr/include/mysql $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.$(O)
	rm -f packages.$(A)
