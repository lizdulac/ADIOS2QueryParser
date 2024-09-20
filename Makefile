all: queryEx

CXX=g++
CXXFLAGS=-std=c++11

queryEx: queryEx.o adiosheaders.hpp QueryParser.h
	$(CXX) $(CXXFLAGS) queryEx.o QueryParser.o -o testQuery

queryEx.o: queryEx.cpp adiosheaders.hpp QueryParser.o
	$(CXX) $(CXXFLAGS) -c queryEx.cpp

QueryParser.o: QueryParser.cpp adiosheaders.hpp
	$(CXX) $(CXXFLAGS) -c QueryParser.cpp

.PHONY: all clean cleanall
clean:
	rm -f *.o

cleanall: clean
	rm -f testQuery
