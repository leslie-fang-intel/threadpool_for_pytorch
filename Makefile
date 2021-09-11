LDFLAGS=-pthread

main: main.o
	g++ main.o -o main $(LDFLAGS)
main.o: main.cpp
	g++ -c main.cpp -o main.o $(LDFLAGS)
clean:
	rm -rf main *.o