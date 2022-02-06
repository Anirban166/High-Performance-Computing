General
---
Includes my ventures into some fresh, quirky and interesting problems that require the use of distributed memory computing in MPI and C (along with a compute cluster, for problems that strictly involve high performance computing), with my approaches (code/explanations) open-sourced here. (tends to be a continuation along a similar train of exposure, following my previous [work](https://github.com/Anirban166/P-for-Parallel-Programming) on shared memory parallel programming)

Compilation
---
```sh
mpicc <filename.c> -o <executablename>
```

Execution
---
```sh
// Local:
mpirun -np <processcount> -hostfile <hostfilename>.<extension> ./<executablename>
// Monsoon:
srun ...
```

Scheduling
---
```sh
// Slurm:
sbatch <runnerscriptname>.sh
```

> To be updated next on the 21<sup>st</sup> of February, '22.

