LDFLAGS=-pthread
HEADER=
FLAGS=-std=c++14
main: main.o
	g++ main.o -o main $(LDFLAGS) $(HEADER) $(FLAGS)
main.o: main.cpp
	g++ -c main.cpp -o main.o $(LDFLAGS) $(HEADER) $(FLAGS)
clean:
	rm -rf main *.o