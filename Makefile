all:
	# g++ -O2 src/matrix_expo.cpp -o matrix_expo
	./matrix_expo --input1 test/1k1k1e9.inp --input2 test/1k1k1e9.inp --output test/1k1k1e9-true.out

	g++ -lpthread -O2 src/pthreads_1.1.cpp -o matrix_expo_pthreads
	./matrix_expo_pthreads rix_Expo -nt 5 --input1 test/1k1k1e9.inp --input2 test/1k1k1e9.inp --output test/1k1k1e9.out