CC = g++
CFLAGS = -O2
LIBS = -lgecodeint -lgecodeminimodel -lgecodesearch -lgecodekernel -lgecodesupport

all: sdp

sdp: sdp.cpp
	$(CC) $(CFLAGS) -o sdp sdp.cpp $(LIBS)

clean:
	rm -f sdp