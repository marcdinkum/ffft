CPP = g++
CFLAGS = -Wall -O3
LDFLAGS= -lsndfile -lfft


all: ffft

ffft: ffft.o
	$(CPP) -o $@ $(CFLAGS) ffft.o $(LDFLAGS)

.cpp.o:
	$(CPP) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f `find . -perm +111 -type f`

