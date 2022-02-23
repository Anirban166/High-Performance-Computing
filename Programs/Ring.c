/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Run: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc Ring.c -lm -o Ring && mpirun -np 6 -hostfile myhostfile.txt ./Ring

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) 
{
  // Set communication count: 
  const int communicationCount = 10;    
  int rank, size; 
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(size < 2)
  {
    printf("\nProcess rank value must be at least 2!\n");
    MPI_Finalize();
    return 0;
  }  

  int token, localCounter = 0;
  for(int i = 0; i < communicationCount; ++i)
  {
    // Receive from the previous process and send to the next process:
    if(rank != 0) 
    {
        MPI_Recv(&token, 1, MPI_INT, (rank - 1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // printf("Process %d received token %d from process %d!\n", rank, token, rank - 1);
        localCounter += token;
        // printf("Local counter of process %d is at: %d\n", rank, localCounter);
    } 
    token = rank;
    MPI_Send(&token, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);

    // Now process 0 can receive from the last process. 
    // This makes sure that at least one MPI_Send is initialized before all MPI_Recvs (again, to prevent deadlock)
    if(rank == 0) 
    {
        MPI_Recv(&token, 1, MPI_INT, (size - 1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // printf("Process %d received token %d from process %d!\n", rank, token, (size - 1));
        localCounter += token;
        // printf("Local counter of process %d is at: %d\n", rank, localCounter);   
    }
  } // Process exits for loop
   // Using a barrier in order to synchronize the prints at the end: MPI_Barrier(MPI_COMM_WORLD);
  // Time to print the final counter! 
  printf("\nFinally, the local counter of process %d is at: %d\n", rank, localCounter);   

  MPI_Finalize();
}