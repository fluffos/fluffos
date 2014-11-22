# special db stuff

packages/db/db.o: packages/db/db.cc
	@echo [CC] $@
	@$(CXX) -c $(CXXFLAGS) $(MYSQL_CFLAGS) $(POSTGRESQL_CPPFLAGS) $(SQLITE3_CFLAGS) -x c++ -o $@ $<
