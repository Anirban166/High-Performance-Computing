/*-------------------------
  Author: Anirban
  Email:  ac4743@nau.edu
--------------------------*/
// Compilation: mpicc <filename>.c -o <executablename>
// Execution: mpirun -np <processcount> -hostfile <hostfilename>.<hostfileextension> ./<executablename>
// Example: mpicc -O3 Kmeans.c -lm -o Kmeans && mpirun -np 2 -hostfile myhostfile.txt ./Kmeans 5159737 2 2 iono_57min_5.16Mpts_2D.txt
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define KMEANSITERS 10

int importDataset(char *fname, int N, double **dataset);
double computeDistance(double *centroid, double *point, int dim);

int main(int argc, char *argv[]) 
{
  // Declaring variables for holding the unique ranks (assigned to each process in a communicator) and process count:
  int rank, size;
  // Initializing the MPI execution environment, along with the ranks and the count of them:
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Process command-line arguments:
  int N, DIM, KMEANS;
  char inputFilename[500];
  if(argc != 5) 
  {
    fprintf(stderr, "Please provide the following on the command line: N (number of lines in the file), dimensionality (number of coordinates per point/feature vector), K (number of means), dataset filename.");
    MPI_Finalize();
    exit(0);
  }
  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &DIM);
  sscanf(argv[3], "%d", &KMEANS);
  strcpy(inputFilename, argv[4]);

  // Declare a pointer to the entire dataset and take care of typical edge cases:
  double **dataset;
  if(N < 1 || DIM < 1 || KMEANS < 1) 
  {
    fprintf(stderr, "\nOne of the following are invalid: N, DIM, K(MEANS)\n");
    MPI_Finalize();
    exit(0);
  }
  else
  {
    if(!rank) 
    {
      fprintf(stderr, "\nNumber of lines (N): %d, Dimensionality: %d, KMEANS: %d, Filename: %s\n", N, DIM, KMEANS, inputFilename);
    }
    // Making all ranks allocate memory for the dataset and then importing it:
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

  MPI_Barrier(MPI_COMM_WORLD);

  // Declaring/Initializing variables to hold the timings and collecting the start time point:
  double startTime, endTime, localDistanceCalculationTime = 0.0, localCentroidUpdateTime = 0.0, localTotalTime, globalDistanceCalculationTime, globalCentroidUpdateTime, globalTotalTime, timePointOne, timePointTwo, timePointThree;
  startTime = MPI_Wtime();

  // Declaring and allocating memory for the variables to hold the ranges (start, end) to work upon:
  int startRange, endRange;
  int *startRanges = (int *)malloc(sizeof(int) * size);
  int *endRanges = (int *)malloc(sizeof(int) * size);
  // Making rank 0 assign the data ranges to all the ranks:
  if(!rank) 
  {
    for(int i = 0; i < size; i++) 
    {
      startRanges[i] = N / size * i;
      if(N % size != 0 && i == size - 1) 
      {
        endRanges[i] = startRanges[i] + N / size + N % size;
      } 
      else 
      {
        endRanges[i] = startRanges[i] + N / size;
      }
    }
  }
  MPI_Scatter(startRanges, 1, MPI_INT, &startRange, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Scatter(endRanges, 1, MPI_INT, &endRange, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Allocating memory for the centroids and assigning them with the first k values:
  double **centroids = (double **)malloc(sizeof(double *) * KMEANS);
  for(int i = 0; i < KMEANS; i++) 
  {
    centroids[i] = (double *)malloc(sizeof(double) * DIM);
    centroids[i] = dataset[i];
  }
  double **centroidsToPlot = (double **)malloc(sizeof(double *) * KMEANS);
  for(int i = 0; i < KMEANS; i++) 
  {
    centroidsToPlot[i] = (double *)malloc(sizeof(double) * DIM);
  }  

  // Declaring and allocating memory for point clusters and a counter for the global count of them:
  int *clusters = (int *)calloc(sizeof(int), N);
  int *globalClusterCount = (int *)calloc(sizeof(int), KMEANS);

  // Allocating memory for partial means:S
  double **partialMean = (double **)malloc(sizeof(double *) * KMEANS);
  for(int i = 0; i < KMEANS; i++) 
  {
    partialMean[i] = (double *)calloc(sizeof(double), DIM);
  }

  // Declaring and allocating memory for the local cluster count:
  int *localClusterCount = (int *)calloc(sizeof(int), KMEANS);

  for(int z = 0; z < KMEANSITERS; z++) 
  {
    // Initializing partial means and the local cluster count to 0 in each iteration:
    for(int i = 0; i < KMEANS; i++) 
    {
      for(int j = 0; j < DIM; j++) 
      {
        partialMean[i][j] = 0.0;
      }
      localClusterCount[i] = 0;
    }

    // Collecting the distance computation start time:
    MPI_Barrier(MPI_COMM_WORLD);
    timePointOne = MPI_Wtime();

    for(int i = startRange; i < endRange; i++) 
    {
      double *distances = (double *)calloc(sizeof(double), KMEANS);
      for(int j = 0; j < KMEANS; j++) 
      {
        double *centroid = centroids[j];
        double *point = dataset[i];
        distances[j] = computeDistance(centroid, point, DIM);
      }

      // Collecting the minimum distance and assigning point to the cluster associated with the centroid:
      double minDistance = distances[0];
      int clusterIndex = 0;
      for(int x = 0; x < KMEANS; x++) 
      {
        if(distances[x] < minDistance) 
        {
          minDistance = distances[x];
          clusterIndex = x;
        }
      }
      // Assigning cluster ID to point:
      clusters[i] = clusterIndex;
      localClusterCount[clusterIndex] += 1;
    }

    // Collecting the end time point for the distance calculation: (also the start timepoint for centroid updation)
    MPI_Barrier(MPI_COMM_WORLD);
    timePointTwo = MPI_Wtime();
    // Computing the local distance calculation time as the difference between its start and end timepoints:
    localDistanceCalculationTime += timePointTwo - timePointOne;

    // Performing a reduction (MPI_SUM) on the local cluster count to get the global cluster count:
    MPI_Allreduce(localClusterCount, globalClusterCount, KMEANS, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Calculating the partial means from points in a cluster:
    for(int i = startRange; i < endRange; i++) 
    {
      for(int j = 0; j < KMEANS; j++) 
      {
        if(clusters[i] == j) 
        {
          for(int k = 0; k < DIM; k++) 
          {
            partialMean[j][k] += dataset[i][k] / globalClusterCount[j];
          }
        }
      }
    }

    // Performing a reduction on the partial means (MPI_SUM) to get the centroids:
    for(int x = 0; x < KMEANS; x++) 
    {
      MPI_Allreduce(partialMean[x], centroids[x], DIM, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    }

    // Computing the local centroid updation time, after collecting the time point signifying its end:
    MPI_Barrier(MPI_COMM_WORLD);
    timePointThree = MPI_Wtime();
    localCentroidUpdateTime += timePointThree - timePointTwo;
    
    // Collecting the centroids: (for the validation plots)
    if(z == KMEANSITERS - 2) 
    {
      for(int i = 0; i < KMEANS; i++) 
      {
        for(int j = 0; j < DIM; j++) 
        {
          centroidsToPlot[i][j] = centroids[i][j];
        }
      }
    }
  }

  // Collecting the time point post the entire process and computing the local time taken by each rank:
  MPI_Barrier(MPI_COMM_WORLD);
  endTime = MPI_Wtime();
  localTotalTime = endTime - startTime;

  // Performing reductions (MPI_MAX) with the local time data (distance calculation time, centroid updation time and total time) on rank 0:
  MPI_Allreduce(&localDistanceCalculationTime, &globalDistanceCalculationTime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(&localCentroidUpdateTime, &globalCentroidUpdateTime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  MPI_Allreduce(&localTotalTime, &globalTotalTime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

  if(!rank) 
  { // Printing to stderr to not mess with the centroids output: (to be extracted for the plot)
    fprintf(stderr, "Total time taken: %f\n", globalTotalTime);  
    fprintf(stderr, "Distance calculation time: %f\n", globalDistanceCalculationTime);
    fprintf(stderr, "Centroid updation time: %f\n", globalCentroidUpdateTime);
    // Printing the centroids for plotting and validation:
    fprintf(stdout, "\"x\", \"y\"\n"); // To get the header/columns set for the csv files.
    for(int i = 0; i < KMEANS; i++) 
    {
      for(int j = 0; j < DIM; j++) 
      { // Printing the point data (comma + whitespace separated for x and y coordinates per row) written to a file (csv format) while using Monsoon:
        if(j != DIM - 1) fprintf(stdout, "%f, ", centroidsToPlot[i][j]);
        else             fprintf(stdout, "%f\n", centroidsToPlot[i][j]);
      }
    }
  }  

  // Deallocating memory for the data set and other used variables:
  for(int i = 0; i < N; i++) 
  {
    free(dataset[i]);
  }
  free(dataset);
  free(startRanges);
  free(endRanges);
  for(int i = 0; i < KMEANS; i++) 
  {
    free(partialMean[i]);
  }
  free(partialMean);
  free(globalClusterCount);
  free(localClusterCount);
  free(clusters);

  MPI_Finalize();
  return 0;
}

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

double computeDistance(double *centroid, double *point, int dimension) 
{
  double dist = 0.00;
  for(int x = 0; x < dimension; x++) 
  {
    dist += (centroid[x] - point[x]) * (centroid[x] - point[x]);
  }
  return sqrt(dist);
}