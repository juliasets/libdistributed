SHELL := /bin/bash

CC := g++ --std=c++11 -Wall -Wextra --pedantic -c

LD := g++ --std=c++11 -Wall -Wextra --pedantic

.PHONY: all
all: libdistributed.o

test: libdistributed.o test.o /usr/lib/libboost_system.a
	$(LD) -o test test.o libdistributed.o -lboost_system -pthread

test.o: test.cpp
	$(CC) test.cpp

libdistributed.o: libdistributed.hpp libdistributed.cpp utility.hpp \
    /usr/include/boost/asio.hpp
	$(CC) libdistributed.cpp

# Yes, this is awful, but it will do for now.
/usr/lib/libboost_system.a:
	echo 'Please install libboost-system-dev.'
	false

# Same goes for this one.
/usr/include/boost/asio.hpp:
	echo 'Please install libasio-dev.'
	false

.PHONY: clean
clean:
	rm -rf *~ *.gch *.o

