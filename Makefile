all:
	# g++ -O2 src/matrix_expo.cpp -o matrix_expo
	# g++ -lpthread -O2 src/pthreads_1.1.cpp -o matrix_expo_pthreads_1.1
	# g++ -lpthread -O2 src/pthreads_1.cpp -o matrix_expo_pthreads_1
	# g++ -fopenmp -O2 src/openmp_1.cpp -o matrix_expo_openmp_1
	# g++ -fopenmp -O2 src/openmp_1.1.cpp -o matrix_expo_openmp_1.1
	g++ -fopenmp -O2 src/openmp_2.cpp -o matrix_expo_openmp_2
	# g++ -fopenmp -O2 src/openmp_2.1.cpp -o matrix_expo_openmp_2.1
	# g++ -fopenmp -O2 src/openmp_2.1.cpp -o matrix_expo_openmp_2.2

