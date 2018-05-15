packages/pcre/pcre.o: packages/pcre/pcre.cc
	@echo [CC] $@
	@$(CXX) -c $(CXXFLAGS) $(PCRE_CFLAGS) -x c++ -o $@ $<