CXXFLAGS=$(shell pkg-config --cflags libbitcoin) -ggdb
LIBS=$(shell pkg-config --libs libbitcoin) -lgmp

default: initdb dispdb writedb showbucket bucketidx readdb importdb \
    listbuckets benchmark compare-prepare compare-lvl-store compare-fastdb-store

mmfile.o: mmfile.cpp mmfile.hpp
	g++ -c -o mmfile.o $(CXXFLAGS) mmfile.cpp
hashtable_database.o: hashtable_database.cpp hashtable_database.hpp
	g++ -c -o hashtable_database.o $(CXXFLAGS) hashtable_database.cpp
transaction_database.o: transaction_database.cpp transaction_database.hpp
	g++ -c -o transaction_database.o $(CXXFLAGS) transaction_database.cpp

initdb.o: initdb.cpp mmfile.hpp
	g++ -c -o initdb.o initdb.cpp $(CXXFLAGS)
initdb: initdb.o mmfile.o
	g++ -o initdb initdb.o mmfile.o $(LIBS)

showbucket.o: showbucket.cpp mmfile.hpp
	g++ -c -o showbucket.o showbucket.cpp $(CXXFLAGS)
showbucket: showbucket.o mmfile.o
	g++ -o showbucket showbucket.o mmfile.o $(LIBS)

bucketidx.o: bucketidx.cpp mmfile.hpp
	g++ -c -o bucketidx.o bucketidx.cpp $(CXXFLAGS)
bucketidx: bucketidx.o mmfile.o
	g++ -o bucketidx bucketidx.o mmfile.o $(LIBS)

listbuckets.o: listbuckets.cpp mmfile.hpp
	g++ -c -o listbuckets.o listbuckets.cpp $(CXXFLAGS)
listbuckets: listbuckets.o mmfile.o
	g++ -o listbuckets listbuckets.o mmfile.o $(LIBS)

compare-prepare.o: compare-prepare.cpp mmfile.hpp
	g++ -c -o compare-prepare.o compare-prepare.cpp $(CXXFLAGS)
compare-prepare: compare-prepare.o mmfile.o
	g++ -o compare-prepare compare-prepare.o mmfile.o $(LIBS)

compare-lvl-store.o: compare-lvl-store.cpp mmfile.hpp
	g++ -c -o compare-lvl-store.o compare-lvl-store.cpp $(CXXFLAGS)
compare-lvl-store: compare-lvl-store.o mmfile.o
	g++ -o compare-lvl-store compare-lvl-store.o mmfile.o $(LIBS)

compare-fastdb-store.o: compare-fastdb-store.cpp mmfile.hpp
	g++ -c -o compare-fastdb-store.o compare-fastdb-store.cpp $(CXXFLAGS)
compare-fastdb-store: compare-fastdb-store.o mmfile.o transaction_database.o hashtable_database.o
	g++ -o compare-fastdb-store compare-fastdb-store.o mmfile.o transaction_database.o hashtable_database.o $(LIBS)

dispdb.o: dispdb.cpp mmfile.hpp
	g++ -c -o dispdb.o dispdb.cpp $(CXXFLAGS)
dispdb: dispdb.o mmfile.o
	g++ -o dispdb dispdb.o mmfile.o $(LIBS)

benchmark.o: benchmark.cpp mmfile.hpp
	g++ -c -o benchmark.o benchmark.cpp $(CXXFLAGS)
benchmark: benchmark.o mmfile.o transaction_database.o hashtable_database.o
	g++ -o benchmark benchmark.o mmfile.o transaction_database.o hashtable_database.o $(LIBS)

importdb.o: importdb.cpp mmfile.hpp
	g++ -c -o importdb.o importdb.cpp $(CXXFLAGS)
importdb: importdb.o mmfile.o transaction_database.o hashtable_database.o
	g++ -o importdb importdb.o mmfile.o transaction_database.o hashtable_database.o $(LIBS)

writedb.o: writedb.cpp mmfile.hpp
	g++ -c -o writedb.o writedb.cpp $(CXXFLAGS)
writedb: writedb.o mmfile.o transaction_database.o hashtable_database.o
	g++ -o writedb writedb.o mmfile.o transaction_database.o hashtable_database.o $(LIBS)

readdb.o: readdb.cpp mmfile.hpp
	g++ -c -o readdb.o readdb.cpp $(CXXFLAGS)
readdb: readdb.o mmfile.o transaction_database.o hashtable_database.o
	g++ -o readdb readdb.o mmfile.o transaction_database.o hashtable_database.o $(LIBS)

