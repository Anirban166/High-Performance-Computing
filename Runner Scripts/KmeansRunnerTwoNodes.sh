#!/bin/bash
#BATCH --job-name=kmeansTwoNodes
#SBATCH --output=/scratch/ac4743/kmeansTwoNodes.out	
#SBATCH --error=/scratch/ac4743/kmeansTwoNodes.err
#SBATCH --time=00:30:00
#SBATCH --mem=100000
#SBATCH --nodes=2
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH -C sl
#SBATCH --exclusive
#SBATCH --account=cs599-spr22

module load openmpi

mpicc -O3 Kmeans.c -lm -o Kmeans.out

# Declaring an array to hold different process counts, and one to hold different values of k:
declare -a processCount=(24 28 32 36 40)
declare -a k=(2 25 50 100)
# Extracting the length of the arrays in separate variables:
processArrayLength=${#processCount[@]}
kArrayLength=${#k[@]}
# Looping through the different process counts and sizes of k, executing the program for each combination:
for((i = 0; i < ${processArrayLength}; i++)); 
do for((j = 0; j < ${kArrayLength}; j++));   
   do
     echo -e "\nRunning the kmeans computation with k = ${k[$j]} and ${processCount[$i]} processes:"
     srun --nodes=2 --ntasks-per-node=$(( ${processCount[$i]}/2 )) -n${processCount[$i]} ./Kmeans.out 5159737 2 ${k[$j]} iono_57min_5.16Mpts_2D.txt
   done  
done