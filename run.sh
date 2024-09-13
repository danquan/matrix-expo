# ./matrix_expo --input1 test/101010.inp --input2 test/101010.inp --output output/101010-true.out
# ./matrix_expo_pthreads_1.1 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_pthreads_1.1_2.out
# ./matrix_expo_pthreads_1 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_pthreads_1_2.out
# ./matrix_expo_openmp_1 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_openmp_1_2.out
# ./matrix_expo_openmp_1.1 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_openmp_1.1_2.out
./matrix_expo_openmp_2 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_openmp_2_2.out
# ./matrix_expo_openmp_2.1 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_openmp_2.1_2.out
# ./matrix_expo_openmp_2.2 -nt 2 --input1 test/101010.inp --input2 test/101010.inp --output output/101010_openmp_2.2_2.out

