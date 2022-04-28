#!/bin/bash
#BATCH --job-name=kmeansSingleNode
#SBATCH --output=/scratch/ac4743/kmeansSingleNode.out	
#SBATCH --error=/scratch/ac4743/kmeansSingleNode.err
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

# Declaring an array to hold different process counts, and one to hold different values of k:
declare -a processCount=(1 4 8 12 16 20)
declare -a k=(2 25 50 100)
# Extracting the length of the arrays in separate variables:
processArrayLength=${#processCount[@]}
kArrayLength=${#k[@]}
# Looping through the different process counts and sizes of k, executing the program for each combination:
for((i = 0; i < ${processArrayLength}; i++)); 
do for((j = 0; j < ${kArrayLength}; j++));   
   do
     echo -e "\nRunning the K-means computation with k = ${k[$j]} and ${processCount[$i]} processes:"
     srun -n${processCount[$i]} ./Kmeans.out 5159737 2 ${k[$j]} iono_57min_5.16Mpts_2D.txt
   done  
done