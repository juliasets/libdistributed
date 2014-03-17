SHELL := /bin/bash

INCLUDE := -Iboost/asio/include/ -Iboost/system/include/ \
    -Iboost/config/include/ -Iboost/assert/include/ -Iboost/integer/include/ \
    -Iboost/utility/include/ -Iboost/iterator/include/ \
    -Iboost/exception/include/ -Iboost/date_time/include/ \
    -Iboost/smart_ptr/include/ -Iboost/static_assert/include \
    -Iboost/mpl/include/ -Iboost/preprocessor/include/ \
    -Iboost/type_traits/include/ -Iboost/bind/include/ \
    -Iboost/regex/include/ -Iboost/array/include/ -Iboost/detail/include/ \
    -Iboost/functional/include/

CC := g++ --std=c++11 -Wall -Wextra --pedantic $(INCLUDE) -c

.PHONY: all
all: libdistributed.o

libdistributed.o: libdistributed.hpp.gch libdistributed.cpp
	$(CC) libdistributed.cpp

libdistributed.hpp.gch: libdistributed.hpp boost.hpp.gch
	$(CC) libdistributed.hpp

.PHONY: update
update: boost
	cd boost/asio && git pull
	cd boost/system && git pull
	cd boost/config && git pull

boost.hpp.gch: boost boost.hpp
	$(CC) boost.hpp

boost: boost/asio boost/system boost/config boost/assert boost/integer \
    boost/utility boost/iterator boost/exception boost/date_time \
    boost/smart_ptr boost/static_assert boost/mpl boost/preprocessor \
    boost/type_traits boost/bind boost/regex boost/array boost/detail \
    boost/functional

boost/%:
	rm -rf $@
	git clone https://github.com/boostorg/$(subst boost/,,$@).git $@

.PHONY: cleanish
cleanish:
	rm -rf *~ *.gch *.o

.PHONY: clean
clean: cleanish
	rm -rf boost

