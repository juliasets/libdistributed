SHELL := /bin/bash

D := -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
G := clang++ $(D)
G := g++

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := $(G) --std=c++11 -Wall -Wextra --pedantic

.PHONY: all
all: libdistributed.o

test: libdistributed.o utility.o test.o
	$(LD) -o test test.o libdistributed.o utility.o -lboost_system -pthread

test.o: test.cpp libdistributed.hpp
	$(CC) test.cpp

libdistributed.o: libdistributed.hpp libdistributed.cpp \
    utility.hpp utility_macros.hpp
	$(CC) libdistributed.cpp

utility.o: utility.hpp utility_macros.hpp utility.cpp
	$(CC) utility.cpp

.PHONY: clean
clean:
	rm -rf *~ *.gch *.o

