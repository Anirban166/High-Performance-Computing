#!/bin/bash
#SBATCH --job-name=RandomRunner          
#SBATCH --output=/scratch/ac4743/RandomRunner.out	
#SBATCH --time=01:00
#SBATCH --mem=5000 
#SBATCH --nodes=1
#SBATCH --ntasks=50 
#SBATCH --cpus-per-task=1

module load openmpi
mpicc RandomRunner.c -lm -o RandomRunner && srun -n50 ./RandomRunner