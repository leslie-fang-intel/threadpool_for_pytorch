CC=g++
LDFLAGS=-lpthread
HEADER=
FLAGS=-std=c++14 -O3
main: main.o threadpool.o
	$(CC) -o main main.o threadpool.o $(LDFLAGS) $(HEADER) $(FLAGS)
main.o: main.cpp ThreadPool.h
	$(CC) -c -o main.o main.cpp $(FLAGS)
threadpool.o: ThreadPool.cpp ThreadPool.h
	$(CC) -c -o threadpool.o ThreadPool.cpp $(FLAGS)
clean:
	rm -rf main *.o