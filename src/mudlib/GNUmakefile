
OBJ=interface.o
SRC=interface.c

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) -I.. $(CFLAGS) -o $@ -c $<

all: $(OBJ)
	ar rlcu mudlib.a $(OBJ)

clean:
	-rm -f *.o
	-rm -f mudlib.a


