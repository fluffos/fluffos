
OBJ=interface.$(O)
SRC=interface.c

OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))

$(OBJDIR)/%.o: %.c
	$(CC) -I$(OBJDIR) -I.. $(CFLAGS) -o $@ -c $<

all: $(OBJ)
	ar rcu mudlib.$(A) $(OBJ)
	$(RANLIB) mudlib.$(A)

clean:
	-rm -f *.$(O)
	-rm -f mudlib.$(A)

