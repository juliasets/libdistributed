SHELL := /bin/bash

D := -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
G := clang++ $(D)
G := g++-4.7

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := $(G) --std=c++11 -Wall -Wextra --pedantic
LIBS := -lboost_system -pthread

#.PHONY: all
#all: libdistributed.o

.PHONY: test
test: master-test slave-test

#communicator-test: Communicator-test.cpp Communicator.hpp
#	$(CC) Communicator-test.cpp
#	$(LD) -o communicator-test Communicator-test.o $(LIBS)

master-test: Master-test.cpp Master.o Master.hpp utility.o
	$(CC) Master-test.cpp
	$(LD) -o master-test Master-test.o Master.o utility.o $(LIBS)

Master.o: Master.hpp Master.cpp utility.hpp utility_macros.hpp
	$(CC) Master.cpp

slave-test: Slave-test.cpp Slave.o utility.o
	$(CC) Slave-test.cpp
	$(LD) -o slave-test Slave-test.o Slave.o utility.o $(LIBS)

Slave.o: Slave.hpp Slave.cpp utility.hpp utility_macros.hpp
	$(CC) Slave.cpp

#test: libdistributed.o utility.o test.o
#	$(LD) -o test test.o libdistributed.o utility.o -lboost_system -pthread

#test.o: test.cpp libdistributed.hpp
#	$(CC) test.cpp

#libdistributed.o: libdistributed.hpp libdistributed.cpp \
#    utility.hpp utility_macros.hpp
#	$(CC) libdistributed.cpp

utility.o: utility.hpp utility_macros.hpp utility.cpp
	$(CC) utility.cpp

.PHONY: clean
clean:
	rm -rf *~ *.gch *.o
	rm -rf test communicator-test master-test

