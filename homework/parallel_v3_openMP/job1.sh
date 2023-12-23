#!/bin/bash
#SBATCH --job-name="myHELLO_096"
#SBATCH --partition=debug
#SBATCH --nodes=7
#SBATCH --time=0-00:30:00
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1992

echo "Count node: $1"
echo "Array size: $2"

mpirun -np $1 a.out $2 
