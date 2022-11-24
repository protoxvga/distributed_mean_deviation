# Distributed mean and deviation computation
Little program to compute the mean and the standard deviation of an array of size n filled with random integers between 0 and 50 with Open MPI
Program developed for an assignment during my fourth year in the Griffith College (Dublin)

## Installation

This program needs Open MPI to work.
Open MPI is a library for distributed system in C++.

Try to follow this steps to install open MPI on your system :
- **Windows**:  Sorry, cannot help you on this one :(
- **Mac OS**:   [Instructions](https://stackoverflow.com/questions/42703861/how-to-use-mpi-on-mac-os-x)
- **Linux**:    [Instructions](https://edu.itp.phys.ethz.ch/hs12/programming_techniques/openmpi.pdf)

## Usage

Feel free to upgrade it to give data through *.txt* or *.dat* file.

### Compilation
```mpic++ name_searcher.cpp -o <executable-name>```

### Execution

#### Default random array size
```mpiexec -n <number-of-nodes> <executable-name>```

example: ```mpiexec -n 4 ./a.out```

#### Defined random array size

```mpiexec -n <number-of-nodes> <executable-name> <random-array-size>```

example: ```mpiexec -n 4 ./a.out 100000```
