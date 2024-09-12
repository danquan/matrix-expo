all:
	g++ -O2 src/matrix_expo.cpp -o matrix_expo
	g++ -lpthread -O2 src/pthreads_1.1.cpp -o matrix_expo_pthreads