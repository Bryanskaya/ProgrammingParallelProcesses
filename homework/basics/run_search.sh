#!/bin/bash
args_size=(40000000 60000000)

mpicc main.c -lm

for size in "${args_size[@]}"
do
    echo "$size"
    sbatch job1.sh $size
done
