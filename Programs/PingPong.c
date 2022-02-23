/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Execution: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc PingPong.c -lm -o PingPong && mpirun -np 8 -hostfile myhostfile.txt ./PingPong

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) 
{
  // Set ping pong counter:  
  const int pingPongCount = 5;
  // Initialize variables for holding the unique ranks (assigned to each process in a communicator) and process count:
  int rank, size;
  // Initialize the MPI execution environment:  
  MPI_Init(&argc, &argv);
  // Get the process rank:
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // Get total number of ranks inside our communicator:
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Ensure process count is even:
  // n & 1 would evaluate to be true if n is odd, given that the least significant bit is always set if the number is not even.
  if (size & 1)
  { // Gentle: (complying to exit gracefully as requested in the problem description)
    if (!rank) // Master (Rank 0)
      printf("For ping pong communication, the number of processes cannot be odd. Please try again with an even number!\n"); 
    MPI_Finalize();
    return 0;
    // Brutal: fprintf(stderr, "Process count MUST be even for executing %s\n", argv[0]); MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int buffer, localCounter = 0, partnerRank = rank ^ 1; // Aren't bitwise operators super convenient?
  // For communicator size > 2, or when I don't want pairs, I'd rather use if(!(rank & 1)) partnerRank = (rank + 1) % size;
  for(int i = 0; i < pingPongCount; ++i)
  {
    MPI_Send(&partnerRank, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);
    MPI_Recv(&buffer, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Rank %d recieved a %d from rank %d.\n", rank, buffer, partnerRank); 
    localCounter += partnerRank;
  }  
  printf("The local counter of rank %d is: %d\n\n", rank, localCounter); 
  MPI_Finalize();
}