#!/bin/bash
#SBATCH --job-name=RingV2          
#SBATCH --output=/scratch/ac4743/RingV2.out	
#SBATCH --time=01:00
#SBATCH --mem=1000 
#SBATCH --nodes=1
#SBATCH --ntasks=6
#SBATCH --cpus-per-task=1

module load openmpi
mpicc RingV2.c -lm -o RingV2 && srun -n6 ./RingV2
