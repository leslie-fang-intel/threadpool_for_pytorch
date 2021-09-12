LDFLAGS=-pthread
HEADER=
main: main.o
	g++ main.o -o main $(LDFLAGS) $(HEADER)
main.o: main.cpp
	g++ -c main.cpp -o main.o $(LDFLAGS) $(HEADER)
clean:
	rm -rf main *.o