LDFLAGS=-pthread
HEADER=
main: main.o threadpool.o
	g++ main.o threadpool.o -o main $(LDFLAGS) $(HEADER)
main.o: main.cpp
	g++ -c main.cpp -o main.o $(LDFLAGS) $(HEADER)
threadpool.o: threadpool.cpp
	g++ -c threadpool.cpp -o threadpool.o $(LDFLAGS) $(HEADER)
clean:
	rm -rf main *.o