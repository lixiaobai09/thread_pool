#!/bin/bash
gcc -o ./test.out ./test.c -O2 -lpthread
gcc -o ./test_openmp.out ./test_openmp.c -O2 -fopenmp
gcc -o ./test_tpool.out ./test_tpool.c ./tpool.c -O2 -lpthread

