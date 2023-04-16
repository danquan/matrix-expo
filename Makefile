all:
	mpicxx -O2 Matrix_Expo_1_async.cpp -o Matrix_Expo
	mpirun -np 16 ./Matrix_Expo