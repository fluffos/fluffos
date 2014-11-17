# special db stuff

# FIXME:
MYSQL_CPPFLAGS=-I/usr/include/mysql
POSTGRESQL_CPPFLAGS=-I/usr/include/postgresql
SQLITE3_CFLAGS=-I/usr/include

packages/async/async.o: packages/async/async.cc
	$(CXX) -c $(CXXFLAGS) -iquote. $(MYSQL_CFLAGS) $(POSTGRESQL_CPPFLAGS) $(SQLITE3_CFLAGS) -x c++ -o $@ $<