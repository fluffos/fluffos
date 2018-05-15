# special db stuff

packages/async/async.o: packages/async/async.cc
	@echo [CC] $@
	@$(CXX) -c $(CXXFLAGS) $(MYSQL_CFLAGS) $(POSTGRESQL_CPPFLAGS) $(SQLITE3_CFLAGS) -x c++ -o $@ $<
