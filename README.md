General
---
Includes my ventures into some fresh, quirky and interesting problems that require the use of distributed memory computing in MPI and C (along with a compute cluster, for problems that strictly involve high performance computing), with my approaches (code/explanations) open-sourced here. (tends to be a continuation along a similar train of exposure, following my previous [work](https://github.com/Anirban166/P-for-Parallel-Programming) on shared memory parallel programming)

Problems
---
<details>
<summary> Ping Pong</summary>

- Problem/Question: [Ping Pong](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/mpi_communication_0/#programming-activity-1)
  
- My solution: [PingPong.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/PingPong.c)
  
- Code explanation:

A fairly straightforward program, wherein after the initial setup of the MPI execution environment and required variables, I first do a parity check since the process count must be even for this scenario. I then declare some variables, namely ‘buffer’ (to store an integer element for a call to `MPI_Recv`), ‘localCounter’ (to store the ping pong count for each process rank) and ‘partnerRank’ (to store the process rank of the other process in a pair). Then within a for loop that iterates for the total number of ping pong communications that one desires for each process rank, I make calls to `MPI_Send` and `MPI_Recv` to send to the other process in a pair its rank itself (not the calling rank, but its ping pong partner rank), and then to receive that element by the same respectively. After such steps for each, the calling rank updates its local counter by the rank of its partner (which again, is the other process that forms their pair). Finally, the local counters of all the process ranks are printed out after the loop ends.
</details>

<details>
<summary> Ring</summary>

## Version 1.0 
  
- Problem/Question: [Ring Communication (blocking sends)](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/mpi_communication_2/#programming-activity-2)
  
- My solution: [Ring.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/Ring.c)
  
- Code explanation:
  
Following the topology of communication that it is indicative of, I created a ‘ring’ by sending and receiving from one process rank to the next and from one process rank to its former (as per the looped structure) in order respectively. The only special case (that deviates away from a logical chain of plus/minus one) to think of here is for the process with rank 0 (since it receives from the last element), for which I created a separate receive block at the end, set apart from the other process ranks. For the sends altogether (taking rank 0 into account), I used some simple modular arithmetic to ensure each process gets its immediately next rank in the ring. I used a variable called ‘token’ to be both the sending and receiving integer unit throughout the communications that take place, and then another variable for the local counter, which I set to 0 initially and then updated accordingly after receives. I first made a block for the case for when the process ranks other than 0 would be receiving the token from their immediately preceding ranks (`rank - 1`). Then I set the token to be the current rank and send that data via a call to `MPI_Send`. Notice that this part would go first for rank 0, and thus there is no deadlock in that case. The only remaining receive that has not been covered so far is rank 0, so I make a separate receive for that right after the generalized send, again to work without any deadlocks. For both of these cases, I increment my local counter for the process rank in play by the value of the token, which again is nothing but the received value from the former process rank with its value itself. This entire logic is put in a loop (which runs for the communication count desired), after which I print the local counters of each rank.
 
## Version 2.0 
  
- Problem/Question: [Ring Communication (non-blocking sends)](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/mpi_communication_2/#programming-activity-3)  

- My solution: [RingV2.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/RingV2.c)
  
- Code explanation:   
  
Given that I have to use `MPI_Isend`, I didn’t worry about the communication in the network being blocking in nature, as a forethought to coding out the solution for this. 
One change with respect to the former version was that I wrote my send calls for my token at the very beginning, instead of the case of emplacing them after the first conditional block which is for processes with ranks other than 0 (although completely possible to do so in the former, just wanted to make this approach slightly different). 
At this point, I also made the blocks for receiving an if-else conditional rather than two separate if branches (as I had to for incorporating the send in between earlier) as I avoided this in my former approach. For the receives, they are followed by calls to `MPI_Wait` with the usual request and status variables sent to keep track of the send-receive communications for each process.  
</details>

<details>
<summary> Random Communication</summary>

## Version 1.0 
  
- Problem/Question: [Random Communication](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/mpi_communication_3/#programming-activity-4)
  
- My solution: [Random.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/Random.c)
  
- Code explanation:
  
The key for my approach is to distribute the randomly generated rank (which I call to be the ‘target’ rank) among processes other than itself and rank 0, which in turn can be used to configure all other processes. That piece of datum is all that was needed to solve the missing piece of the puzzle, although it can be slightly tricky to figure parts for the rest of the functionality of the program, as but analyzing the data flow and affixing a strategy on pen and paper (nothing too complex!) helps. 

So the way I thought of this in general was that there are three possible states a given process is in: sender, target, and a listener who waits for a signal (like a broadcast). Rank 0 starts as the Sender, and all other ranks start as listeners, listening to 0 for the target rank it generates. From there, either they are that target rank, and thus have to receive the counter, or which I like to call the ‘payload’, add their rank to it, check if the total iteration limit has been reached, and send out the next broadcast (one after another in succession) or they aren't and they just listen to that target rank for the next signal. Thus at the start of the program, the process with rank 0 is in sender mode, and everything else in listener mode. Rank 0 generates the next sender, and broadcasts that rank to everyone. Every process that isn't this target doesn't change state - they're still broadcast listeners - but instead of listening for 0, they listen for the target that rank 0 just distributed. The process that is this target instead enters target mode, where it receives that payload and the current iteration from the sender. At this point there's a tiny change in logic. For rank 0, it just stops, since it will never be the next target, but any other sender would now enter the listener mode, listening to the target they previously broadcast themselves and the target, now that it has the payload and the total iteration count, increases the counter in the payload, and becomes the new sender. From here, it either broadcasts out the target rank that it randomly generated, and the process repeats, or every rank shuts down because the total iteration limit has been reached, depending on the value of iteration value passed around.

At the start, every process except the one with rank 0 initiates a read from rank 0, and 0 itself starts the counter payload at 0. From here, a step-wise process is repeated generically which begins when the current sender picks a random rank to send to, and it sends out this rank in a broadcast-sort of manner to all other ranks. The reason for me using ‘sort-of’ here is that I created a function that behaves like a broadcast (pseudo-broadcast would be another word for it I suppose), but it doesn’t send the information to the calling rank or to rank 0 itself, which as I found are unnecessary, thus making my function in fact better than MPI_Bcast for this case. Next up, if the rank received does not match the rank of the process that did the receiving, the process starts listening for a message from that rank. The rank that is equal to this target rank prepares to receive the payload from the current sender. And of course, the current sender sends the payload to this randomly chosen rank, prior to waiting for a message from that rank. The chosen rank receives the payload or the counter, processes it (in this case, just adds its rank to the existing counter value) and becomes the current sender. This whole process is done in an infinite loop that will just continue till the maximum number of desired iterations (10 as per requirement here) have been reached, in which case, the break statements I introduced for cases of the target rank (to be 0 or equal to the randomly generated rank) within the loop will get the ranks out.
 
## Version 2.0 
  
- Problem/Question: [Random Communication (using `MPI_ANY_SOURCE`)](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/mpi_communication_3/#programming-activity-5)
  
- My solution: [RandomV2.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/RandomV2.c)
  
- Code explanation: 
  
Much like in my previous program, everyone but rank 0 starts by listening for the incoming payload, and rank 0 just sends out the counter (and iteration count) of 0 to start with a randomly generated target rank. But the key part that makes it much easier hereafter from this point was that I did not have to worry about which rank to receive from (which is pretty much the whole point of this programming activity, or rather alternate version of the former I believe), as I could just use `MPI_ANY_SOURCE` within my receives. This also reduced the amount of times I had to call my pseudo-broadcast function, which I just hardcoded instead of making a function, since it’s just one loop. I used this loop to send the iteration limit to the other ranks (apart from the calling one, and apart from rank 0), and accordingly made changes to the iteration check with respect to the former version. Note that I have to use the limit as a variable declared in main instead of the using the macro from the `#define`, as it gets treated as an lvalue inside the call or as an argument to `MPI_Send`.  
</details>

<details>
<summary> Distance Matrix</summary>

## Version 1.0
  
- Problem/Question: [Row-wise Distance Matrix Computation](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/distance_matrix_1/#programming-activity-1)
  
- My solution: [DistanceMatrix.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/DistanceMatrix.c)
  
- Code explanation:
  
After the initial setup of the MPI execution environment, I declare pointers for the dataset (1D) and the distance matrix (2D), then take in as input three command line arguments apart from the executable name, (removed block size because it’s irrelevant and unused for this one) which are in order, the number of lines in the dataset N, the dimension of the dataset (which is always 90) and the name of the dataset file. I then allocate memory for the entire dataset, which is an array of N, with each array having space for 90 doubles.
Next up, I make rank 0 collect the time prior to computation (or even allocation of memory for the distance matrix) with a call to `MPI_Wtime()`. I then declare a pointer for an 1D array to hold the entire 1:N range, to be distributed and made specific to all the ranks soon, and another array of the same purpose to act as the receiving buffer element for my call to `MPI_Scatter()`, and thereafter as the range array itself, for each rank locally. This range again, is nothing but elements of the dataset, and thus I assign elements 0 to (N-1) after allocating memory for it.

Given N ranks, I assign the row size to be N divided by the number of ranks, with a separate special case for the last rank which I assign to have the remaining leftover rows if N doesn’t divide the process rank count evenly. For both cases, I allocate memory for my range array, and make my scatter call which distributes this to all ranks in order, so that each can now refer to their own set of data to work on based on the range provided. Again, for the case where leftover rows remain, I increase the range for the last rank to accommodate them. I had to do this after the call to scatter, given that scatter distributes the data evenly or with fair proportion.
I then allocate memory for my distance matrix based on the row size local to the running rank, as per set requirements. I then loop over the local row size times the number of columns or N, and then within that nested loop, loop over the dimensions. Within the innermost loop, I assign the index (to be used to access the distance matrix) for the rows (i.e., with respect to the outermost loop variable) to be the local range that is for the running rank. Thereafter, I compute the sum of elements inside of the square root for the distance equation with the column indexed element subtracted from the local row indexed element, and this being done 90 times for each dimension. Outside this loop, I simply get the square root of this value to obtain the required distance, which accounts for one spot in the distance matrix.

Following the completion of the distance matrix computation, I use rank 0 to print out the difference between the time obtained at that point (right after the triple-nested for loops) and the start time obtained earlier, to display the elapsed time as required. I then compute the local sum for each rank, summing up all the distances that particular rank computed. I follow up with a sum reduction using `MPI_Reduce()` to add up the local sums from each rank into a variable accounting for the global sum of distances, which I then print out using rank 0. Lastly, I deallocate memory for all the arrays that I used with calls to `free()`.

In order to validate that this parallelized MPI solution of mine is correct, I wrote the distance matrix elements (whitespace separated among rows) to a file for N = 100 and then performed a diff between my sequential reference implementation, which is nothing but a separate simple program that similarly used three loops (rows, columns, dimensions uptil N, N and 90 respectively) to compute the distance matrix from the dataset, and two loops (rows, columns) to write to another file. The diff showed no output, implying that there are no changes in between the two files, being indicative of my parallel solution being correct. While it was easy to dump the output to the sequential version, it was slightly tricky for the parallel version at the beginning, i.e. to make each rank write the part of distances it computed to the output file without any overlaps or in an organized or turn wise fashion. Race conditions are what one would expect, given that writing to a file is not thread or rank safe. I used append mode with both and read and write permissions (a+), and then sent an element to the next rank before writing the contents from the current rank, so as to initiate the ‘blocking’ behavior which makes the other rank wait its turn, being unable to print their set of elements (which again, would otherwise lead to the unorganized overlapped writes and a mess in general). I then receive on the next rank so that it can now get out of the wait and proceed to write its computed distances to the file for its turn. I did this for 2 process ranks, given that should be enough for a ‘parallel’ implementation, but this can be extended further (like rank 1 can send to rank 2 before it writes its elements, then rank 2 can send to rank 3 and so on, with the receives being in the immediately successive rank) for any set of ranks (could also use a broadcast strategy or anything that makes ranks wait its turn).
 
## Version 2.0
  
- Problem/Question: [Tiled Distance Matrix Computation](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/distance_matrix_2/#programming-activity-2)  

- My solution: [DistanceMatrixV2.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/DistanceMatrixV2.c)
  
- Code explanation:   
  
The core refactoring to be done here is for the nested loops that compute the distance matrix (keeping the rest of the code same/unaffected), which I’ll admit, required a bit of pen and paper to discern the breakdown for the tiling.

For any tile size, it will have dimensions of equal length, i.e. both the width and height will be the same, given that it's a square tile or block. I denote this length to be the ‘step size’ across both rows and columns (or directions across the x and y axes). I assign this step size is assigned to be the tile/block size that the user would input, and incrementally go forth. Since each rank has its own row size local to it, I needed to make sure that it doesn’t fall short on the length of the block size, thus, I impose the step size to be the local row size for the rank in the case it is strictly less than the block size, so as to stay within bounds for the row wise progression.

I then create loops to go through the rows (following increments of the step size) and then the columns (increments of the block size, which is the same as the step size, unless a lower local row count would change its value) for the tiles. Inside this nested loop, I first iterate based on the rows (i.e., with respect to the outermost loop’s index variable) until the constraints of being within the next step size (for the tile) and under the local row size (for the rank) are met. Then within that, I iterate through the columns, which again have the column-wise version of the tile and rank constraints.

Finally, I emplace the loop for the dimensions that iterates through 90 times. This part remains the exact same as my solution to the former row-wise version, as I assigned the index (to be used for accessing the distance matrix) for the rows to be the local range for the running rank and thereafter, I simply compute the sum of the elements (inside of the square root for the distance equation) with the column indexed element subtracted from the local row indexed element for each dimension. Again, I simply compute the square root of this summed value outside this innermost loop (for iterating through the dimensions) in order to obtain the required distance, which accounts for one element or distance in the distance matrix.  
</details>

<details>
<summary> Distribution Sort</summary>

## Version 1.0
  
- Problem/Question: [Distribution Sort on Uniformly Distributed Data](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/distribution_sort_1/#programming-activity-1)
  
- My solution: [DistributionSort.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/DistributionSort.c)
  
- Code explanation:
  
After the initial setup of the MPI execution environment, each rank is made to generate its own portion (equal to the total data points divided by the process count) of the uniform data to be sorted using the `generateData` function, and thereafter, memory is allocated for the send and receive buffers for each rank, and for the dataset in general. Now post this starter code, I declare variables to hold timings for the distribution, sorting and overall execution of the computations. I then use a sum reduction (`MPI_SUM`) to add up the local sums from each rank into a variable accounting for the global sum of all array elements across all ranks, which I then print out using rank 0. The value that I get is what I would compare while doing the same after the data has been sorted, as a required sanity check to ensure that the data is left unaffected or is correct. 

I emplace a barrier right after this to collect the start time for the overall computation's time measurement, ensuring that all ranks have reached up till this point. I proceed to allocate memory and then assign the ranges of data that each rank would get and work upon in a 2D array, wherein the first element of each 1D array denotes the starting position (with the interval size given by the ratio of `MAXVAL` is to the number of processes) and the second element denotes the end. I then broadcast these two values for all the ranks to know the ranges of the data. Now proceeding to the data distribution part, I first begin by collecting the starting time point for this metric, then I initialize a counter to hold the number of elements or size of the dataset for a rank. I then iterate over all the ranks to assign the appropriate portion of the data (less than the upper bound and equal to or greater than the lower bound, given by the two elements in the 2D array for each rank that was broadcasted) to each rank from the entire dataset if the data is on the range designated for the rank. If not, then I collect the data in a buffer and keep track of the size of it required to be sent by a counter which gets updated for the amount of data that lies in the range for the rank (same logic following the if-conditional up till this point). I then use two non-blocking asynchronous sends to send the required portion of data and the size (total number of elements) of it for each rank one by one through the iterations of the loop, and then for the size that is received, I use that number to iterate that many times and assign the correctly partitioned data for the rank, all before exiting the loop for ranks.  

Thereafter, I collect the time again which is the time point that denotes the end of the data distribution and the start of the sorting phase. I proceed with a call to qsort with the provided helper function to compare elements in its arguments. Right after, I collect the time again for the final time, which as a time point denotes the end of both the sorting and the total computation time for the core portion. I then compute the time measurements by getting out the differences between the appropriate start and end points. I follow with a reduction (`MPI_MAX`) on all three of these time measurements, and then proceed to print them. As the required sanity check, I compute the global sum of all the array elements in the data set across all ranks via a reduction of the local sums like I did previously (prior to data distribution).
Lastly, I deallocate memory for all the variables that I used with calls to `free()`.
 
## Version 2.0
  
- Problem/Question: [Distribution Sort on Exponentially Distributed Data](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/distribution_sort_2/#programming-activity-2)  

- My solution: [DistributionSortV2.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/DistributionSortV2.c)
  
- Code explanation:   
  
The only change here with respect to the version above is the introduction of the function that generates random values from the exponential distribution, and the use of it to generate the data (using a lambda or rate of change constant of 4 as passed on line 198 in DistributionSortV2.c) inside the function `generateData`.

## Version 3.0
  
- Problem/Question: [Distribution Sort on Exponentially Distributed Data with Histogram](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/distribution_sort_3/#programming-activity-3)  

- My solution: [DistributionSortV3.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/DistributionSortV3.c)
  
- Code explanation: 

The core refactor or rather addition here with respect to the version above was pre-computing the histogram or getting the frequency-based ranges for the ranks prior to data distribution. I start by declaring and allocating memory for two counters holding `MAXVAL` number of elements that keeps track of the frequency of a data in a rank and across all ranks globally, following with initialization of these arrays to zero for all the elements. I then make the ranks compute the frequency of each data element in their local counters and for the other ones other than rank 0, have them send these back to rank 0 for computing the values for the global counter. 

After I receive the local counters from each rank (inside a loop that iterates up till the process count, getting a receive from all the other ranks apart from 0 itself) at rank 0, I update my global counter array by incrementing it with the local ones (this is followed after the receives in the same loop, with the loop indices taking care of the appropriate counter indexing). I then declared a variable to hold the ratio of data distribution for each rank, and one to set the range for each rank in a loop. I also used two counters here, one for accounting and keeping up with the frequency values of the global counter, and one to set the range for each rank incrementally.

Now inside a loop that runs up till the global upper bound (`MAXVAL`) and until all the ranks have got their ranges, I set for each rank the range which is based on the distribution ratio and adjusted (or shifted) based on the frequencies of the data. I used a separate variable to keep track of the minimum range for each rank to start next with, and there is a corner case for the last rank in order to accommodate the leftover portion of data. After the adjusted ranges have been computed for all the ranks, I broadcast these and proceed just like I did for the versions above.
</details>

<details>
<summary> Range Queries</summary>

## Version 1.0
  
- Problem/Question: [Brute Force Approach for the Range Queries Computation](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/range_query_0/#programming-activity-1)
  
- My solution: [RangeQueries.c](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/RangeQueries.c)
  
- Code explanation:

After the initial setup of the MPI execution environment, each rank is made to generate the entire dataset and its own portion (equal to the total queries divided by the process count) of the total queries using th functions `generateData` and `generateQueries`. I then emplace a barrier to wait for all ranks to reach completion of this data and query generation phase. Thereafter, I collect the time which is the time point that denotes the start of the range query computation. I proceed to loop over the number of queries for the rank, then for each, I iterate through the entire dataset of points (since this *is* the brute force approach) and if the point lies in the rectangle bound by the minimum and maximum (x, y) values given by a particular query, then I increment my counter which stores the number of ‘hits’ or effectively the points with satisfy the range queries. This counter is reset for each query, but the hits are added to a buffer I initialized early on to keep track of the total number of successful hits for each rank. Right after, I collect the time again which as a time point denotes the end of the range query computation. I then compute the time measurements by getting out the differences between the appropriate start and end points. I follow with a reduction (`MPI_MAX`) on the time measurement and then print it. Next up, I compute the global sum of all the elements in my buffer storing the hits across all ranks via a reduction (`MPI_SUM`) of the local sums to obtain the total number of hits for the range queries for all ranks. Lastly, I deallocate memory for all the variables that I used with calls to `free()`.  
 
## Version 2.0
  
- Problem/Question: [Computing the Range Queries with an optimized apporach using R-trees](https://jan.ucc.nau.edu/mg2745/pedagogic_modules/courses/hpcdataintensive/range_query_1/#programming-activity-2)  

- My solution: [RangeQueriesV2.cpp](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/RangeQueriesV2.cpp)
  
- Code explanation:   
  
The core change that I made here with respect to the previous approach is the construction of the R-tree for each and every point of the dataset that gets generated across all ranks. I began by creating an object of the `RTree` class (refer to the [`Rtree.h`](https://github.com/Anirban166/High-Performance-Computing/blob/main/Programs/RTree.h) file for the underlying data structure's source code) and then iterating through the dataset to fill in the R-Tree with appropriate parameters, which include the minimum and maximum bounds for each point, as I supplied from the `Rect` struct. Note that since I'm dealing with points here (as opposed to polygons with four sides), the x and y coordinates for the bounds (min, max) are the same. After the construction of the R-Tree, I perform my range queries in each rank by searching through the constrained areas (minimized search space) given by the bounds that it creates. Here each rank iterates up till its query count and creates a `Rect` given by the specific query’s (x, y) limits (passed as arguments in order), and these minimum and maximum bounds are passed onto the search method for the `Rtree` object that I previously created. This function returns one (true for search) if the point satisfies the query limits, and this is then added to my buffer for keeping track of the ‘hits’ or successful range query satisfying points for a particular rank (a reduction is applied later to compute the global hits across all ranks, like previously). Another change here (wrt the former version) was for the timings, as I had to keep track of both the R-Tree creation time and the search time (or basically range query computation time) for the queries inside it, apart from the total time. For this, I just emplaced barriers appropriately (waiting for all ranks’ execution to reach those points) and used calls to `MPI_Wtime()` to collect the required time points, following up with reductions on them.
</details>

Compilation
---
```sh
> C:
mpicc <filename>.c -o <executablename>
> C++:
mpic++ <filename>.cpp -o <executablename> 
```

Execution
---
```sh
> Local:
mpirun -np <processcount> -hostfile <hostfilename>.<extension> ./<executablename> <arguments>
> Monsoon:
srun --nodes=<nodecount> --ntasks-per-node=1 -n<processcount> ./<executablename> <arguments>
```

Scheduling
---
```sh
> Slurm:
sbatch <runnerscriptname>.sh
```
