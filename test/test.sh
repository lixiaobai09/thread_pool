#!/bin/bash
gcc -o ./test.out ./test.c -O2 -lpthread
gcc -o ./test_openmp.out ./test_openmp.c -O2 -fopenmp
gcc -o ./test_tpool.out ./test_tpool.c ../tpool.c ./time_count/time_cnt.c -I../ -O2 -lpthread

if [ $# -eq 1 ] && [ $1 == "run" ]; then
    for ((i=1; i<=8; ++i))
    do
        echo "threads number is $i"
        ./test_tpool.out -n $i
        echo ""
    done
fi
