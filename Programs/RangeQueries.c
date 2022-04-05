/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Execution: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc -O3 RangeQueries.c -lm -o RangeQueries && mpirun -np 2 -hostfile myhostfile.txt ./RangeQueries 1000000 1000
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SEED 72
#define MAXVAL 100.0
#define QUERYRNG 10.0

struct dataStruct  { double x, y; };
struct queryStruct { double xMin, yMin, xMax, yMax; };

void generateData(struct dataStruct *data, unsigned int localN);
void generateQueries(struct queryStruct *data, unsigned int localQ, int rank);

int main(int argc, char *argv[]) 
{
  // Declaring variables for holding the unique ranks (assigned to each process in a communicator) and process count:
  int rank, size;
  // Initializing the MPI execution environment, along with the ranks and the count of them:
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Process command-line arguments: (Size of input and query datasets)
  int N, Q;
  if(argc != 3) 
  {
    fprintf(stderr, "Please provide the following on the command line: <Number of data points> <Number of query points> \n");
    MPI_Finalize();
    exit(0);
  }
  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &Q);

  // All ranks will have the input dataset, only the queries will be divided amongst each:
  const unsigned int localN = N, localQ = Q / size;
  // Allocating memory (and initializing to zero with calloc) for the array that stores the number of hits for each query:
  unsigned int *hitCountArray = (unsigned int *)calloc(localQ, sizeof(unsigned int));
  // Allocating memory for the data struct and making all ranks generate the input data:
  struct dataStruct *data = (struct dataStruct *)malloc(sizeof(struct dataStruct) * localN);
  generateData(data, localN);
  // Allocating memory for the query struct and making each rank generate a separate set of queries:
  struct queryStruct *queries = (struct queryStruct *)malloc(sizeof(struct queryStruct) * localQ);
  generateQueries(queries, localQ, rank);
  // Waiting for all ranks to reach completion of data and query generation:
  MPI_Barrier(MPI_COMM_WORLD);

  // Declaring variables to keep track of the different time measurements required:
  double startTime, endTime, localTotalTime, totalTime;

  // Collecting the start time prior to range query computation:
  MPI_Barrier(MPI_COMM_WORLD);
  startTime = MPI_Wtime();

  // Performing range queries (bounded by limits {xMin, yMin} and {xMax, yMax}) for each rank in a bruteforce approach:
  for(int i = 0; i < localQ; i++) 
  {
    unsigned int hits = 0;
    for(int j = 0; j < localN; j++) 
    {
      if((data[j].x >= queries[i].xMin && data[j].x <= queries[i].xMax) && (data[j].y >= queries[i].yMin && data[j].y <= queries[i].yMax)) 
      {
        hits++;
      }
    }
    hitCountArray[i] = hits;
  }

  // Collecting the time right after the computation is done on all the ranks:
  MPI_Barrier(MPI_COMM_WORLD);
  endTime = MPI_Wtime();

  // Computing the duration for the range computation for all ranks before applying a reduction (MPI_MAX) on them in rank 0:
  localTotalTime = endTime - startTime;
  MPI_Reduce(&localTotalTime, &totalTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if (!rank) fprintf(stdout, "Total time taken: %f\n", totalTime);

  // Declaring variables to compute the sums, computing the rank-specific sum and then performing a reduction (MPI_SUM) on those to obtain the global sum on rank 0:
  long unsigned int localSum = 0, globalSum;  
  for(int i = 0; i < localQ; i++) 
  {
    localSum += hitCountArray[i];
  }
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  if(!rank) fprintf(stdout, "The global sum: %lu\n", globalSum);

  // Deallocating memory for the used variables:
  free(data);
  free(queries);
  free(hitCountArray);

  MPI_Finalize();
  return 0;
}

// Function to generate data in the range [0, MAXVAL): 
void generateData(struct dataStruct *data, unsigned int localN) 
{
  srand(SEED);
  for(int i = 0; i < localN; i++)
  {
    data[i].x = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
    data[i].y = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
  }
}
// Function to generate different queries for each rank:
void generateQueries(struct queryStruct *data, unsigned int localQ, int rank) 
{
  srand(SEED + rank);
  for (int i = 0; i < localQ; i++) 
  {  // xMin and yMin are in that range, while xMax and yMax are incremented with [0, QUERYRNG):
    data[i].xMin = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
    data[i].yMin = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
    data[i].xMax = data[i].xMin + ((double)rand() / (double)(RAND_MAX)) * QUERYRNG;
    data[i].yMax = data[i].yMin + ((double)rand() / (double)(RAND_MAX)) * QUERYRNG;
  }
}