
OBJ=interface.o
SRC=interface.c

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) -I.. $(CFLAGS) -o $@ -c $<

all: $(OBJ)
	ar rcu mudlib.a $(OBJ)
	$(RANLIB) mudlib.a

clean:
	-rm -f *.o
	-rm -f mudlib.a


