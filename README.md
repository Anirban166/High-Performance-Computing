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

Compilation
---
```sh
mpicc <filename.c> -o <executablename>
```

Execution
---
```sh
> Local
mpirun -np <processcount> -hostfile <hostfilename>.<extension> ./<executablename>
> Monsoon
srun -n<processcount> ./<executablename>
```

Scheduling
---
```sh
> Slurm:
sbatch <runnerscriptname>.sh
```
