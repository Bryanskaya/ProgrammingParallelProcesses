#! /bin/bash
#SBATCH --job-name="myHELLO"
#SBATCH --partition=debug
#SBATCH --nodes=4
#SBATCH --time=0-00:05:00
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1992

mpirun -np 5 a.out