CXXFLAGS=$(shell pkg-config --cflags libbitcoin) -ggdb
LIBS=$(shell pkg-config --libs libbitcoin) -lgmp

default: initdb dispdb writedb showbucket bucketidx readdb

mmfile.o: mmfile.cpp mmfile.hpp
	g++ -c -o mmfile.o $(CXXFLAGS) mmfile.cpp
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

dispdb.o: dispdb.cpp mmfile.hpp
	g++ -c -o dispdb.o dispdb.cpp $(CXXFLAGS)
dispdb: dispdb.o mmfile.o
	g++ -o dispdb dispdb.o mmfile.o $(LIBS)

writedb.o: writedb.cpp mmfile.hpp
	g++ -c -o writedb.o writedb.cpp $(CXXFLAGS)
writedb: writedb.o mmfile.o transaction_database.o
	g++ -o writedb writedb.o mmfile.o transaction_database.o $(LIBS)

readdb.o: readdb.cpp mmfile.hpp
	g++ -c -o readdb.o readdb.cpp $(CXXFLAGS)
readdb: readdb.o mmfile.o transaction_database.o
	g++ -o readdb readdb.o mmfile.o transaction_database.o $(LIBS)

