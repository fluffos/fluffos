include packages

.c.$(O):
	$(CC) $(CFLAGS) -I.. -c $*.c

all: $(OBJ)

# special db stuff
db.$(O): db.c
	$(CC) $(CFLAGS) -I.. -I/usr/include/mysql -c $*.c

clean:
	rm -f *.$(O)
