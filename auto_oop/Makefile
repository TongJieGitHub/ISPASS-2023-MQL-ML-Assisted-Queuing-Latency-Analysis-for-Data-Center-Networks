CC = g++
CFLAGS = -Wall -std=c++17 -O3
#CFLAGS = -Wall -std=c++17 -ggdb
PROM = analytical_model
SOURCE = main.cpp util.cpp fattree.cpp fattreeL3.cpp fattreeL2Custom.cpp
# LIB = ~/gperftools-2.10/lib/libtcmalloc.so ~/gperftools-2.10/lib/libprofiler.so

$(PROM): main.o util.o fattree.o fattreeL3.o fattreeL2Custom.o regression_tree_model.o
	$(CC) $(CFLAGS) -o $(PROM) main.o util.o fattree.o fattreeL3.o fattreeL2Custom.o regression_tree_model.o

main.o: main.cpp util.h fattree.h fattreeL3.h fattreeL2Custom.h
	$(CC) $(CFLAGS) -c main.cpp

util.o: util.cpp util.h
	$(CC) $(CFLAGS) -c util.cpp

fattree.o: fattree.cpp util.h fattree.h
	$(CC) $(CFLAGS) -c fattree.cpp

fattreeL3.o: fattreeL3.cpp util.h fattree.h fattreeL3.h
	$(CC) $(CFLAGS) -c fattreeL3.cpp

fattreeL2Custom.o: fattreeL2Custom.cpp util.h fattree.h fattreeL2Custom.h
	$(CC) $(CFLAGS) -c fattreeL2Custom.cpp

regression_tree_model.o: regression_tree_model.cpp regression_tree_model.h
	$(CC) $(CFLAGS) -c regression_tree_model.cpp

test:
	./$(PROM)

clean:
	rm -f *.o $(PROM)

cleanall:
	rm -f *.o $(PROM) *.csv
