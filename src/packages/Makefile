include packages

.c.$(O):
	$(CXX) $(CXXFLAGS) -I.. -c -x c++ $*.c

all: $(OBJ)

# special db stuff
db.$(O): db.c
	$(CXX) $(CXXFLAGS) -I.. -I/usr/include/mysql -c -x c++ $*.c

clean:
	rm -f *.$(O)
