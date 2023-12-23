#!/bin/bash
args_size=(1000 
           10000 
           100000 
           1000000
           10000000 
           40000000 
           60000000 
           100000000)

mpicc main.c -lm -fopenmp

for size in "${args_size[@]}"
do
    echo "$size"
    sbatch job1.sh $size
done
