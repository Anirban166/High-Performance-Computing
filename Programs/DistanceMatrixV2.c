/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Execution: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename> <numberoflines> <dimensionality> <blocksize> <filename>
// Example: mpicc -O3 DistanceMatrixV2.c -lm -o DistanceMatrixV2 && mpirun -np 2 -hostfile myhostfile.txt ./DistanceMatrixV2 100000 90 100 MSD_year_prediction_normalize_0_1_100k.txt
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int importDataset(char *fname, int N, double **dataset);

int main(int argc, char *argv[]) 
{
  // Declaring variables for holding the unique ranks (assigned to each process in a communicator) and process count:
  int rank, size;
  // Initializing the MPI execution environment, along with the ranks and the count of them:
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  // Declaring pointers to the dataset and the distance matrix:
  double **dataset, **distanceMatrix; 

  // Process command-line arguments: (N, Dimensionality, block/tile size and filename of the dataset)
  int N, DIM, blockSize;
  char inputFilename[500];
  if(argc != 5) 
  {
    fprintf(stderr, "\nPlease provide the following on the command line: N (number of lines in the file), dimensionality (number of coordinates per point), block size, dataset filename.\n");
    MPI_Finalize();
    exit(0);
  }
  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &DIM);
  sscanf(argv[3], "%d", &blockSize);
  strcpy(inputFilename, argv[4]);
  if(N < 1 || DIM < 1)
  {
    fprintf(stderr, "\nN or DIM is invalid!\n");
    MPI_Finalize();
    exit(0);
  }
  else // All ranks import the dataset:
  {
    if(!rank) 
        fprintf(stdout, "\nNumber of lines (N): %d, Dimensionality: %d, Block size: %d, Filename: %s\n", N, DIM, blockSize, inputFilename); 
    // Allocating memory for the dataset:
    dataset = (double**)malloc(sizeof(double*) * N);
    for(int i = 0; i < N; i++)
    {
      dataset[i] = (double*)malloc(sizeof(double) * DIM);
    }
    if(importDataset(inputFilename, N, dataset))
    {
      MPI_Finalize();
      return 0;
    }
  }
  
  // Initializing variables to hold the timings and make rank 0 collect the start time:
  double startTime, endTime;
  if(!rank) startTime = MPI_Wtime();

  // Initializing variables to hold the range for each rank: (send, receive)
  int *range, *localRange;
  // Allocating memory for and initializing the entire range:
  range = (int *)malloc(sizeof(int) * N);
  if(!rank) 
  {
    for (int i = 0; i < N; ++i) 
      range[i] = i;
  }
  // Assigning row size to each process rank based on the divisibility of the dataset size to the number of ranks:
  int localRowSize = N / size;
  // Taking care of the special case for the last rank, and assigning memory for all of them:
  if(rank == (size - 1) && (N % size) != 0) 
  {
    localRowSize = N / size + N % size;
    localRange = (int *)malloc(sizeof(int) * localRowSize);
  } 
  else localRange = (int *)malloc(sizeof(int) * localRowSize);

  // Sending the range to work on for a rank (local to it), distributed accordingly (in order) among all the ranks using a scatter:
  int workloadSize = N / size;  
  MPI_Scatter(range, workloadSize, MPI_INT, localRange, workloadSize, MPI_INT, 0, MPI_COMM_WORLD);
  // Increasing the size of the range for the last rank if not sufficient (for the leftover rows, if N mod size isn't zero):
  if(rank == (size - 1) && (N % size) != 0) 
  {
    for(int i = workloadSize; i < localRowSize; i++) 
      localRange[i] = localRange[i - 1] + 1;
  }

  // Allocating memory for, initializing and computing the distance matrix:
  distanceMatrix = (double **)malloc(sizeof(double *) * localRowSize); // Ranks allocate memory only for their portion of the distance matrix
  for(int i = 0; i < localRowSize; i++) 
  {
    distanceMatrix[i] = (double *)malloc(sizeof(double) * N);
  }
  int stepSize = (localRowSize < blockSize) ? localRowSize : blockSize;
  for(int x = 0; x < localRowSize; x += stepSize)
  {
    for(int y = 0; y < N; y += blockSize) 
    {
      for(int i = x; i < (x + stepSize) && i < localRowSize; i++) 
      {
        for(int j = y; j < (y + blockSize) && j < N; j++) 
        {
          double distance = 0;
          for (int k = 0; k < DIM; k++) 
          {
            int localIndex = localRange[i];
            distance += (dataset[localIndex][k] - dataset[j][k]) * (dataset[localIndex][k] - dataset[j][k]);
          }
          distanceMatrix[i][j] = sqrt(distance);
        }
      }
    }
  }

  // Printing the elapsed time for the distance matrix computation:
  if(!rank) 
  {
    endTime = MPI_Wtime();
    fprintf(stdout, "Time taken to compute the distance matrix in parallel: %f seconds\n", endTime - startTime);   
  }

  // Computing the local sum in all ranks and then sending those to rank 0 for it to do a reduction on it:
  double globalSum, localSum = 0;  
  for(int i = 0; i < localRowSize; i++) 
  {
    for(int j = 0; j < N; j++) 
      localSum += distanceMatrix[i][j];
  }
  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if(!rank) 
  {
    fprintf(stdout, "Global sum of all the distances computed locally by each rank: %f\n", globalSum);
  }

  // Deallocating memory for the dataset, distance matrix and range variables:
  for(int i = 0; i < N; i++)
    free(dataset[i]);
  free(dataset);
  for (int i = 0; i < localRowSize; i++) 
    free(distanceMatrix[i]);
  free(range);
  free(localRange);
  free(distanceMatrix);
  
  MPI_Finalize();
  return 0;
}

// Function to import the dataset:
int importDataset(char *fname, int N, double **dataset)
{
    FILE *fp = fopen(fname, "r");
    if(!fp) 
    {
        fprintf(stderr, "Unable to open file!\n");
        return(1);
    }
    char buffer[4096];
    int rowCount = 0;
    int colCount = 0;
    while(fgets(buffer, 4096, fp) && rowCount < N) 
    {
        colCount = 0;
        char *field = strtok(buffer, ",");
        double temp;
        sscanf(field, "%lf", &temp);
        dataset[rowCount][colCount] = temp;
        while(field) 
        {
          colCount++;
          field = strtok(NULL, ",");
          if(field != NULL)
          {
            double temp;
            sscanf(field,"%lf",&temp);
            dataset[rowCount][colCount] = temp;
          }   
        }
        rowCount++;
    }
    fclose(fp);
    return 0;
}