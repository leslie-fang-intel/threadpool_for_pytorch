CC=g++
LDFLAGS=-lpthread
HEADER=
FLAGS=-march=native -std=c++14 -O3 -fopenmp -liomp5 -L/home/lesliefang/pytorch_1_7_1/anaconda3/pkgs/intel-openmp-2021.2.0-h06a4308_610/lib
main: main.o threadpool.o
	$(CC) -o main main.o threadpool.o $(LDFLAGS) $(HEADER) $(FLAGS)
main.o: main.cpp ThreadPool.h
	$(CC) -c -o main.o main.cpp $(FLAGS)
threadpool.o: ThreadPool.cpp ThreadPool.h
	$(CC) -c -o threadpool.o ThreadPool.cpp $(FLAGS)
clean:
	rm -rf main *.o