CXXFLAGS=$(shell pkg-config --cflags libbitcoin_blockchain) -ggdb
LIBS=$(shell pkg-config --libs libbitcoin_blockchain) -lgmp

default: bench

hashtable_database.o: hashtable_database.cpp hashtable_database.hpp
	g++ -c -o hashtable_database.o $(CXXFLAGS) hashtable_database.cpp

initdb.o: initdb.cpp
	g++ -c -o initdb.o initdb.cpp $(CXXFLAGS)
initdb: initdb.o
	g++ -o initdb initdb.o $(LIBS)

bench.o: bench.cpp
	g++ -c -o bench.o bench.cpp $(CXXFLAGS)
bench: bench.o hashtable_database.o
	g++ -o bench bench.o hashtable_database.o $(LIBS)

