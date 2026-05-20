CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio_Community2212/cplex
CONCOPTDIR    = /opt/ibm/ILOG/CPLEX_Studio_Community2212/concert

SYSTEM        = x86-64_linux
LIBFORMAT     = static_pic

CXX           = g++ -O3 -std=c++11
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(SYSTEM)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCOPTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCFLAGS       = -DIL_STD -I$(CPLEXDIR)/include -I$(CONCOPTDIR)/include
CCLNDIRS      = -L$(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT) -L$(CONCOPTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CCLNFLAGS     = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

all: program checker

program: sdp.o
	$(CXX) $(CCFLAGS) $(CCLNDIRS) -o program sdp.o $(CCLNFLAGS)

sdp.o: sdp.cpp
	$(CXX) -c $(CCFLAGS) sdp.cpp -o sdp.o

checker: checker.o
	$(CXX) -o checker checker.o

checker.o: checker.cc
	$(CXX) -c -O3 -std=c++11 checker.cc -o checker.o

clean:
	rm -f sdp.o checker.o program checker