SHELL := /bin/bash

# These things need to be defined manually if we are using clang++.
# G++ defines them automatically. They are needed for creating new threads.
D := -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 \
    -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
G := clang++ $(D)

# TIMED LOCKS ARE ALL BROKEN UNTIL libstdc++ v4.9, WHICH IS PRE-RELEASE.
# Version 4.7 at least has a working try_lock_until(), even if it isn't
# standards compliant.
G := g++-4.7

CC := $(G) --std=c++11 -Wall -Wextra --pedantic -c
LD := $(G) --std=c++11 -Wall -Wextra --pedantic
LIBS := -lboost_system -pthread

.PHONY: all
all: Master.o Slave.o Client.o utility.o

.PHONY: test
test: master-test slave-test client-test
	./master-test | ./slave-test | ./slave-test | ./slave-test | ./client-test | ./client-test | ./client-test

master-test: Master-test.cpp Master.o Master.hpp utility.o skein/skein.o
	$(CC) Master-test.cpp
	$(LD) -o master-test Master-test.o Master.o utility.o skein/*.o $(LIBS)

Master.o: Master.hpp Master.cpp streamwrapper.hpp utility.hpp utility_macros.hpp skein/skein.h
	$(CC) Master.cpp

slave-test: Slave-test.cpp Slave.o utility.o skein/skein.o
	$(CC) Slave-test.cpp
	$(LD) -o slave-test Slave-test.o Slave.o utility.o skein/*.o $(LIBS)

Slave.o: Slave.hpp Slave.cpp streamwrapper.hpp utility.hpp utility_macros.hpp skein/skein.h
	$(CC) Slave.cpp

client-test: Client-test.cpp Client.o utility.o skein/skein.o
	$(CC) Client-test.cpp
	$(LD) -o client-test Client-test.o Client.o utility.o skein/*.o $(LIBS)

Client.o: Client.hpp Client.cpp streamwrapper.hpp utility.hpp skein/skein.h
	$(CC) Client.cpp

utility.o: utility.hpp utility_macros.hpp utility.cpp
	$(CC) utility.cpp

skein/skein.o:
	cd skein && gcc -c *.c

.PHONY: clean
clean:
	rm -rf *~ *.gch *.o skein/*~ skein/*.gch skein/*.o
	rm -rf *-test

