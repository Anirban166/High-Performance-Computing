#!/bin/bash
#SBATCH --job-name=distanceMatrix       
#SBATCH --output=/scratch/ac4743/distanceMatrix.out	
#SBATCH --error=/scratch/ac4743/distanceMatrix.err
#SBATCH --time=01:00:00 # Average time it took to complete for all the six cases (different process count) is less than an hour.
#SBATCH --mem=80000     # Using 80 gigs of ram, given that the data generated accounts for 76GB as per my calculation.
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH -C sl # Using Intel's Skylake Xeon processor.

module load openmpi

mpicc -O3 DistanceMatrix.c -lm -o DistanceMatrix
# Declaring an array to hold different process counts:
declare -a processCount=(1 4 8 12 16 20)
# Extracting the length of it in a variable:
arrayLength=${#processCount[@]}
# Looping through the different process counts and executing the program:
for((i = 0; i < ${arrayLength}; i++)); 
do    
  echo -e "\nRunning the distance matrix computation with ${processCount[$i]} processes:"
  srun -n${processCount[$i]} /usr/bin/perf stat -B -e cache-references,cache-misses ./DistanceMatrix 100000 90 MSD_year_prediction_normalize_0_1_100k.txt
done