#!/bin/bash
#SBATCH --job-name=distributionSortV2       
#SBATCH --output=/scratch/ac4743/distributionSortV2.out	
#SBATCH --error=/scratch/ac4743/distributionSortV2.err
#SBATCH --time=00:30:00
#SBATCH --mem=30000
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH -C sl
#SBATCH --exclusive

module load openmpi

mpicc -O3 DistributionSortV2.c -lm -o DistributionSortV2
# Declaring an array to hold different process counts:
declare -a processCount=(1 2 4 8 12 16 20)
# Extracting the length of it in a variable:
arrayLength=${#processCount[@]}
# Looping through the different process counts and executing the program:
for((i = 0; i < ${arrayLength}; i++)); 
do    
  echo -e "\nRunning the distribution sort computation with ${processCount[$i]} processes on exponentially distributed data:"
  srun -n${processCount[$i]} ./DistributionSortV2
done