all:
	mpicxx -O2 Matrix_Expo_1_async_noWait.cpp -o Matrix_Expo
	mpirun -np 5 ./Matrix_Expo