#!/bin/bash
#SBATCH --job-name=distanceMatrixV2     
#SBATCH --output=/scratch/ac4743/distanceMatrixV2.out	
#SBATCH --error=/scratch/ac4743/distanceMatrixV2.err
#SBATCH --time=00:08:00 # Average time it took to complete for all the eight cases (different block sizes) is less than 8 minutes.
#SBATCH --mem=80000     # Using 80 gigs of ram, given that the data generated accounts for 76GB as per my calculation.
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH -C sl # Using Intel's Skylake Xeon processor.

module load openmpi

mpicc -O3 DistanceMatrixV2.c -lm -o DistanceMatrixV2
# Declaring an array to hold different block sizes:
declare -a blockSize=(5 100 500 1000 2000 3000 4000 5000)
# Extracting the length of it in a variable:
arrayLength=${#blockSize[@]}
# Looping through the different block sizes (process count fixed to 20 as required) and executing the program:
for((i = 0; i < ${arrayLength}; i++)); 
do    
  echo -e "\nRunning the distance matrix computation with 20 processes and a tile size of (${blockSize[$i]} x ${blockSize[$i]}):"
  srun -n20 /usr/bin/perf stat -B -e cache-references,cache-misses ./DistanceMatrixV2 100000 90 ${blockSize[$i]} MSD_year_prediction_normalize_0_1_100k.txt
done