#!/bin/bash

args_nodes=(1 2 4 8 10)
args_size=(
    1000 
    10000 
    100000 
    1000000 
    10000000 
    40000000 
    60000000 
    100000000)

#mpicc main.c -lm

for nodes in "${args_nodes[@]}"
do
    for size in "${args_size[@]}"
    do
        echo "Count node: $nodes"
        echo "Array size: $size"
        
        sbatch job1.sh $nodes $size
    done
done
