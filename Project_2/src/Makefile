CPLEXSTUDIODIR = /opt/ibm/ILOG/CPLEX_Studio2212
CPLEXDIR       = $(CPLEXSTUDIODIR)/cplex
CONCOPTDIR     = $(CPLEXSTUDIODIR)/concert

SYSTEM        = x86-64_linux
LIBFORMAT     = static_pic

CXX           = g++ -O3 -std=c++11
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(SYSTEM)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCOPTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCFLAGS       = -DIL_STD -I$(CPLEXDIR)/include -I$(CONCOPTDIR)/include
CCLNDIRS      = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CCLNFLAGS     = -lilocplex -lconcert -lcplex -lm -lpthread -ldl

all: sdp checker

sdp: sdp.o
	$(CXX) $(CCFLAGS) $(CCLNDIRS) sdp.o -o sdp $(CCLNFLAGS)

sdp.o: sdp.cpp
	$(CXX) -c $(CCFLAGS) sdp.cpp -o sdp.o

checker: checker.o
	$(CXX) -o checker checker.o

checker.o: checker.cc
	$(CXX) -c -O3 -std=c++11 checker.cc -o checker.o

clean:
	rm -f sdp.o checker.o sdp checker