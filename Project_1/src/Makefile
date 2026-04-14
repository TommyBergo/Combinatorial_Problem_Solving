CC = g++
CFLAGS = -O2
LIBS = -lgecodeint -lgecodeminimodel -lgecodesearch -lgecodekernel -lgecodesupport

all: sdp checker

sdp: sdp.cpp
	$(CC) $(CFLAGS) -o sdp sdp.cpp $(LIBS)

checker: checker.cc
	$(CC) $(CFLAGS) -o checker checker.cc

clean:
	rm -f sdp checker
