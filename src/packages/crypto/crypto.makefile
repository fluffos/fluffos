packages/crypto/crypto.o: packages/crypto/crypto.cc
	@echo [CC] $@
	@$(CXX) -c $(CXXFLAGS) $(OPENSSL_INCLUDES) -x c++ -o $@ $<