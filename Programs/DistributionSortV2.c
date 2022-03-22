/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Execution: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc -O3 DistributionSortV2.c -lm -o DistributionSortV2 && mpirun -np 2 -hostfile myhostfile.txt ./DistributionSortV2
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SEED 72
#define N 1000000000
#define MAXVAL 1000000

void generateData(int *data, int SIZE);
double randomExponential(double lambda); 
int compareFunction(const void *a, const void *b);

int main(int argc, char *argv[]) 
{
  // Declaring variables for holding the unique ranks (assigned to each process in a communicator) and process count:
  int rank, size;
  // Initializing the MPI execution environment, along with the ranks and the count of them:
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  // Seeding the random number generation:
  srand(SEED + rank);
  // Initializing the local input size: 
  const unsigned int localN = N / size;
  // All ranks generate their own portion of data:
  int *data = (int *)malloc(sizeof(int) * localN);
  generateData(data, localN);

  // Allocating memory for the send and receive buffers for each rank, and for the dataset in general:
  int *sendDataSetBuffer = (int *)malloc(sizeof(int) * localN);
  int *recvDatasetBuffer = (int *)malloc(sizeof(int) * localN);
  int *myDataSet = (int *)malloc(sizeof(int) * N);

  // Declaring variables to keep track of the different time measurements required:
  double startTotalTime, endTotalTime, startDistributionTime, endDistributionStartSortingTime, distributionTime, sortingTime, totalTime;

  // Computing the rank-specific sum:
  long unsigned int localSum = 0;  
  for(int i = 0; i < localN; i++) 
  {
    localSum += data[i];
  }

  // Performing a reduction (MPI_SUM) of the local sums to obtain the global sum on rank 0:
  long unsigned int globalSum;  
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  // Check prior to sorting:
  if(!rank) 
  {
    printf("\nThe global sum of all elements across all ranks before sorting is: %lu", globalSum);
  }

  // Collecting start time:
  MPI_Barrier(MPI_COMM_WORLD);
  startTotalTime = MPI_Wtime();

  // Allocating memory for the data range:
  unsigned int **dataRange = (unsigned int **)malloc(sizeof(unsigned int *) * size);
  for(int i = 0; i < size; i++) 
  {
    dataRange[i] = (unsigned int *)malloc(sizeof(unsigned int) * 2);
  }

  // Computing data ranges in rank 0:
  if(!rank) 
  {
    unsigned int intervalSize = MAXVAL / size;
    unsigned int shift = 0;
    for (int i = 0; i < size; i++) 
    {
      dataRange[i][0] = shift;
      dataRange[i][1] = (i == (size - 1)) ? MAXVAL : shift + intervalSize;
      shift += intervalSize;
    }
  }
  // Broadcasting the ranges to all the ranks:
  for(int i = 0; i < size; i++) 
  {
    MPI_Bcast(dataRange[i], 2, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  }

  // Collecting the start time prior to data distribution:
  MPI_Barrier(MPI_COMM_WORLD);
  startDistributionTime = MPI_Wtime();

  // Performing the bucket-wise data distribution:
  unsigned int datasetCount = 0;
  for(int i = 0; i < size; i++) 
  {
    if(i == rank)
    {
      for(int j = 0; j < localN; j++) 
      {
        if(data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) 
        {
          myDataSet[datasetCount] = data[j];
          datasetCount += 1;
        }
      }
    }
    else 
    {
      unsigned int sendCount = 0;
      unsigned int receiveCount = 0;
      for(int j = 0; j < localN; j++) 
      {
        if(data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) 
        {
          sendDataSetBuffer[sendCount] = data[j];
          sendCount += 1;
        }
      }
      // Sending and receiving the data in between ranks as per set bounds:
      MPI_Request requestCount, requestDataSetBuffer;
      MPI_Isend(&sendCount, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &requestCount);
      MPI_Isend(sendDataSetBuffer, sendCount, MPI_INT, i, 1, MPI_COMM_WORLD, &requestDataSetBuffer);
      MPI_Recv(&receiveCount, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(recvDatasetBuffer, receiveCount, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for(int k = 0; k < receiveCount; k++) 
      {
        myDataSet[datasetCount] = recvDatasetBuffer[k];
        datasetCount += 1;
      }
      MPI_Wait(&requestCount, MPI_STATUS_IGNORE);
      MPI_Wait(&requestDataSetBuffer, MPI_STATUS_IGNORE);
    }
  }

  // Collecting the time right after data distribution and right before sorting:
  MPI_Barrier(MPI_COMM_WORLD);
  endDistributionStartSortingTime = MPI_Wtime();

  // Sorting the data and collect the time post-sorting:
  qsort(myDataSet, datasetCount, sizeof(myDataSet[0]), compareFunction);
  MPI_Barrier(MPI_COMM_WORLD);
  endTotalTime = MPI_Wtime();

  // Computing the duration for the data distribution, sorting and overall time(s) before applying a reduction (MPI_MAX) on them:
  double localDistributionTime = endDistributionStartSortingTime - startDistributionTime, localSortingTime = endTotalTime - endDistributionStartSortingTime, localTotalTime = endTotalTime - startTotalTime;
  MPI_Reduce(&localDistributionTime, &distributionTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Reduce(&localSortingTime, &sortingTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Reduce(&localTotalTime, &totalTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  // Printing the required time measurements:
  if(!rank) 
  {
    printf("\nTime taken to distribute the data across ranks: %f", distributionTime);
    printf("\nTime taken to sort the data using qsort: %f", sortingTime);
    printf("\nTotal time taken: %f", totalTime);
  }

  // Computing the global sum from a reduction on the local sums again, post-sorting:
  localSum = 0;
  for(int i = 0; i < datasetCount; i++) 
  {
    localSum += myDataSet[i];
  }
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // Sanity check:
  if(!rank) 
  {
    printf("\nThe global sum of all elements across all ranks after sorting is: %lu\n", globalSum);
  }

  // Deallocating memory for the used variables:
  free(data);
  free(sendDataSetBuffer);
  free(recvDatasetBuffer);
  free(myDataSet);
  for(int i = 0; i < size; i++) 
  {
    free(dataRange[i]);
  }
  free(dataRange);

  MPI_Finalize();
  return 0;
}

void generateData(int *data, int SIZE) 
{
  for(int i = 0; i < SIZE; i++) 
  {
    double tmp = 0;
    do // Generating values between 0-1 using the exponential distribution:
    {
      tmp = randomExponential(4.0);
    } while (tmp >= 1.0);
    data[i] = tmp * MAXVAL;
  }
}

double randomExponential(double lambda) 
{
  double u = rand() / (RAND_MAX + 1.0);
  return -log(1 - u) / lambda;
}

int compareFunction(const void *a, const void *b) 
{ 
    return (*(int *)a - *(int *)b); 
}