#!/bin/bash
#SBATCH --job-name="myHELLO_096"
#SBATCH --partition=debug
#SBATCH --nodes=2
#SBATCH --time=0-00:30:00
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1992

echo 'Array size: ' $1

mpirun -np 1 a.out $1
