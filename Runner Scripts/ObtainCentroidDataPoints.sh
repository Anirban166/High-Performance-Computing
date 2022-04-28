#!/bin/bash
#BATCH --job-name=kmeansCentroidData
#SBATCH --output=/scratch/ac4743/kmeansCentroidData.csv	
#SBATCH --error=/scratch/ac4743/kmeansCentroidData.err
#SBATCH --time=00:30:00
#SBATCH --mem=100000
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH -C sl
#SBATCH --exclusive
#SBATCH --account=cs599-spr22

module load openmpi

mpicc -O3 Kmeans.c -lm -o Kmeans.out

declare -a k=(2 10 25 50 100)
kArrayLength=${#k[@]}
# Looping through the different values of k, executing the program for each combination to get that many number of centroid points: 
for((i = 0; i < ${kArrayLength}; i++)); 
do  
  echo -e "\n\n"
  srun -n20 ./Kmeans.out 5159737 2 ${k[$i]} iono_57min_5.16Mpts_2D.txt
done