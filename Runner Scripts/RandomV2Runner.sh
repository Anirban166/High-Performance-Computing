#!/bin/bash
#SBATCH --job-name=RandomV2Runner         
#SBATCH --output=/scratch/ac4743/RandomV2Runner.out	
#SBATCH --time=01:00
#SBATCH --mem=5000 
#SBATCH --nodes=1
#SBATCH --ntasks=50 
#SBATCH --cpus-per-task=1

module load openmpi
mpicc RandomV2Runner.c -lm -o RandomV2Runner && srun -n50 ./RandomV2Runner