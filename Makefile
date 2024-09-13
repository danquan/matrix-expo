all:
	# g++ -O2 src/matrix_expo -o matrix_expo
	mpicxx -O2 src/mpi_1.cpp -o matrix_expo_mpi_1
