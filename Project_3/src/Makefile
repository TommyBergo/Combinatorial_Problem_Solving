CXX      = g++ -O3 -std=c++11
KISSAT_DIR = ../solvers/kissat

all: kissat_solver sdp checker

kissat_solver:
	cd $(KISSAT_DIR) && ./configure && $(MAKE)

sdp: sdp.o
	$(CXX) sdp.o -o sdp

sdp.o: sdp.cpp
	$(CXX) -c sdp.cpp -o sdp.o

checker: checker.o
	$(CXX) -o checker checker.o

checker.o: checker.cc
	$(CXX) -c -O3 -std=c++11 checker.cc -o checker.o

clean:
	rm -f sdp.o checker.o sdp checker
	cd $(KISSAT_DIR) && $(MAKE) clean || true