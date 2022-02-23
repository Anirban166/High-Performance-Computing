/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Run: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc RingV2.c -lm -o RingV2 && mpirun -np 6 -hostfile myhostfile.txt ./RingV2

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
    MPI_Status status;  
    MPI_Request request;  
    token = rank;
    MPI_Isend(&token, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD, &request);
    if(rank != 0)
    {  
        MPI_Recv(&token, 1, MPI_INT, (rank - 1), 0, MPI_COMM_WORLD, &status); 
        // printf("Process %d received token %d from process %d!\n", rank, token, (rank - 1));        
        MPI_Wait(&request, &status); 
    }
    else
    {   // Send from the last element (size - 1) to rank 0:
        MPI_Recv(&token, 1, MPI_INT, (size - 1), 0, MPI_COMM_WORLD, &status); 
        // printf("Process %d received token %d from process %d!\n", rank, token, (size - 1));
        MPI_Wait(&request, &status); 
    }   
    localCounter += token;      
  }
  printf("\nFinally, the local counter of process %d is at: %d\n", rank, localCounter);   

  MPI_Finalize();
}