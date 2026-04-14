CC = g++
CFLAGS = -O2
LIBS = -lgecodeint -lgecodeminimodel -lgecodesearch -lgecodekernel -lgecodesupport

all: sdp checker

sdp: sdp.cpp
	$(CC) $(CFLAGS) -o sdp sdp.cpp $(LIBS)

checker: checker.cpp
	$(CC) $(CFLAGS) -o checker checker.cpp

clean:
	rm -f sdp checker
