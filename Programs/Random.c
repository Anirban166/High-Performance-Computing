/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Run: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc Random.c -lm -o Random && mpirun -np 50 -hostfile myhostfile.txt ./Random

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SEED 72
#define TOTALITER 10

int generateRandomRank(int maxRank, int rank);
void loopFunction(int valueToSend, int rank, int size);

int main(int argc, char *argv[]) 
{
  int rank, size; 
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

  int iteration, counter, sender, targetRank;
  if(rank == 0) 
  {
    targetRank = generateRandomRank(size - 1, rank);
    printf("Master (Process Rank 0) randomly generated the first process rank %d.\n", targetRank);    

    loopFunction(targetRank, rank, size);
    iteration = 0; 
    counter = 0;

    MPI_Send(&iteration, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
    MPI_Send(&counter, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
  } 
  else 
  {
    sender = 0;
    for (;;) 
    {
      MPI_Recv(&targetRank, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if(targetRank == 0) 
      {  
        break;
      }
      else if(targetRank == rank) 
      {
        MPI_Recv(&iteration, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&counter, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // printf("Process rank %d received the counter which is at %d.\n", rank, counter);
        printf("My rank: %d, old counter: %d.\n", rank, counter); 
        iteration += 1;
        counter += rank;
        printf("My rank: %d, new counter: %d.\n", rank, counter); 
        // printf("Process rank %d incremented the counter to %d.\n", rank, counter); 

        targetRank = (iteration != TOTALITER) ? generateRandomRank(size - 1, rank) : 0;
        loopFunction(targetRank, rank, size);

        if(iteration == TOTALITER) 
        {  
          break;
        }
        MPI_Send(&iteration, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
        MPI_Send(&counter, 1, MPI_INT, targetRank, 0, MPI_COMM_WORLD);
      } 
      sender = targetRank;
    }
  }
  // printf("Process rank %d has left the chat.\n", rank); 
  MPI_Finalize();
  return 0;
}

// MPI_Bcast() equivalent, except its better - does not send to 0 or self:
void loopFunction(int valueToSend, int rank, int size)
{
  for(int otherRanks = 1; otherRanks < size; otherRanks++) // Avoid 0.
  {
    if(otherRanks != rank) // Avoid the calling rank itself.
    {
      MPI_Send(&valueToSend, 1, MPI_INT, otherRanks, 0, MPI_COMM_WORLD);
    }
  }  
}

// Random rank generator function:
int generateRandomRank(int maxRank, int rank)
{  
  int randomRank = round(maxRank * ((double)(rand()) / RAND_MAX));
  while(randomRank == rank || randomRank == 0)
  {                                         
    randomRank = round(maxRank * ((double)(rand()) / RAND_MAX)); 
  }
  return randomRank;
}