#!/bin/bash
#SBATCH --job-name=RangeQueriesV2Runner       
#SBATCH --output=/scratch/ac4743/RangeQueriesV2.out	
#SBATCH --error=/scratch/ac4743/RangeQueriesV2.err
#SBATCH --time=00:10:00
#SBATCH --mem=50000
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH -C sl
#SBATCH --exclusive
#SBATCH --account=cs599-spr22

module load openmpi

mpic++ -O3 RangeQueriesV2.cpp -lm -o RangeQueriesV2
# Declaring an array to hold different process counts:
declare -a processCount=(1 4 8 12 16 20)
# Extracting the length of it in a variable:
arrayLength=${#processCount[@]}
# Looping through the different process counts and executing the program:
for((i = 0; i < ${arrayLength}; i++)); 
do    
  echo -e "\nRunning the range query computation with ${processCount[$i]} processes:"
  srun -n${processCount[$i]} ./RangeQueriesV2 2000000 100000
done