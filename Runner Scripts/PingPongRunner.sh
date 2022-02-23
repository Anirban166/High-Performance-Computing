#!/bin/bash
#SBATCH --job-name=PingPong          
#SBATCH --output=/scratch/ac4743/PingPong.out	
#SBATCH --time=01:00
#SBATCH --mem=1000 
#SBATCH --nodes=1
#SBATCH --ntasks=8 
#SBATCH --cpus-per-task=1

module load openmpi
mpicc PingPong.c -lm -o PingPong && srun -n8 ./PingPong