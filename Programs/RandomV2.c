/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Run: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc RandomV2.c -lm -o RandomV2 && mpirun -np 50 -hostfile myhostfile.txt ./RandomV2

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SEED 72
#define TOTALITER 10

int generateRandomRank(int maxRank, int rank);

int main(int argc, char *argv[]) 
{
  int totalIterations = TOTALITER, rank, size;  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(size < 3 && !rank)
  {     
    printf("\nProcess rank value must be more than 2!\n");
    MPI_Finalize();
    return 0;
  }  
  // Seed the random number generation for fixed results: (counter ending up at 222 for 10 iterations with 50 processes)
  srand(SEED + rank);

  int iteration, counter, targetRank;
  if(rank == 0) 
  {
    targetRank = generateRandomRank(size - 1, rank);  
    printf("Master (Process Rank 0) randomly generated the first process rank %d.\n", targetRank);  

    iteration = 0; 
    counter = 0;

    MPI_Send(&iteration, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
    MPI_Send(&counter, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
  } 
  else 
  {
    for (;;) 
    {
      MPI_Recv(&iteration, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if(iteration == totalIterations) 
        break; 
      MPI_Recv(&counter, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      printf("My rank: %d, old counter: %d.\n", rank, counter); 
      iteration += 1;
      counter += rank;
      printf("My rank: %d, new counter: %d.\n", rank, counter);
      
      if(iteration < totalIterations) 
      {
        targetRank = generateRandomRank(size - 1, rank);
        MPI_Send(&iteration, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
        MPI_Send(&counter, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
      } 
      else 
      {
        for(int otherRanks = 1; otherRanks < size; otherRanks++) 
        {
          if(otherRanks != rank) 
            MPI_Send(&totalIterations, 1, MPI_INT, otherRanks, 0, MPI_COMM_WORLD);
        }
        break;  
      }
    }
  } 
  MPI_Finalize();
  return 0;
}

// Random rank generator function:
int generateRandomRank(int maxRank, int rank)
{  
  int randomRank = round(maxRank * ((double)(rand()) / RAND_MAX));
  while(randomRank == rank || randomRank == 0) // Again, same deal.
  {                                           // (Not returning the rank passed as argument itself, or rank 0)
    randomRank = round(maxRank * ((double)(rand()) / RAND_MAX)); 
  }
  return randomRank;
}