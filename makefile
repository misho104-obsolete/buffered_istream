
target = buffered_istream.hpp
test_programs  = $(basename $(wildcard test/*_test.cpp))

exec_tests = $(addprefix exec_, $(test_programs))

.PHONY: test $(exec_tests) clean

test: $(exec_tests)

$(test_programs) : LDFLAGS=-lcppunit
$(test_programs) : % : %.cpp $(target)
	$(CXX) $< $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LOADLIBES) -o $@

$(exec_tests) : exec_% : %
	@echo Execute test $< ...
	@$<

clean:
	@rm -f *~
	@rm -f $(test_programs)
