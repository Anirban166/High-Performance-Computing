#!/bin/bash
#SBATCH --job-name=Ring          
#SBATCH --output=/scratch/ac4743/Ring.out	
#SBATCH --time=01:00
#SBATCH --mem=1000 
#SBATCH --nodes=1
#SBATCH --ntasks=6 
#SBATCH --cpus-per-task=1

module load openmpi
mpicc Ring.c -lm -o Ring && srun -n6 ./Ring