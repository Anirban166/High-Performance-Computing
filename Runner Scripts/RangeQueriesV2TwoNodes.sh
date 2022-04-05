#!/bin/bash
#SBATCH --job-name=RangeQueriesV2TwoNodes       
#SBATCH --output=/scratch/ac4743/RangeQueriesV2TwoNodes.out	
#SBATCH --error=/scratch/ac4743/RangeQueriesV2TwoNodes.err
#SBATCH --time=00:10:00
#SBATCH --mem=50000
#SBATCH --nodes=2
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
  if [ ${processCount[$i]} -eq 1 ]
  then
    echo -e "\nRunning the range query computation with ${processCount[$i]} process:"
    srun -n${processCount[$i]} ./RangeQueriesV2 2000000 100000
  else
    echo -e "\nRunning the range query computation with ${processCount[$i]} processes:"
    srun --nodes=2 --ntasks-per-node=$(( ${processCount[$i]}/2 )) -n${processCount[$i]} ./RangeQueriesV2 2000000 100000
  fi
done