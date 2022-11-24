// Distributed Systems - 2022
// Pierre Perrin - protoxvga

#include <mpi.h>
#include <iostream>
#include <math.h>
#include <fstream>

int arr_size = 100;    // If we don't pass a file throgh the command line, this will be the size of the random array

void helper()
{
    std::cout << "The main goal of the program is to calculate the mean and the standard deviation of an array of size n filled sith random integers between 0 and 50" << std::endl;
    std::cout << "The default size of the array is: 100, but you can choose if by changing argument 1" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "1. Without custom array size: 'mpiexec -n #number_of_nodes #executable_name'" << std::endl;
    std::cout << "2. With custom array size: 'mpiexec -n #number_of_nodes #executable_name #custom size'" << std::endl;
}

// Method to print an array of integers (taking the array and the size of the array)
void printArray(int *arr, int size)
{
    std::cout << "---------------------------------------" << std::endl;                // Limit of the array
    for (int count = 0; count < size; count ++) {   // For each integer in array
        std::cout << arr[count];                    // Print the integer
        if (count != size - 1)                      // If we are not at the end of the tab
            std::cout << " ";                      // Print a coma between all the integers
    }
    std::cout << std::endl << "---------------------------------------" << std::endl;   // Limit of the array
}

// Method to compute the sum of an array of integers (taking the array and the size of the array)
int sum(int *arr, int size)
{
    int res = 0;

    for (int count = 0; count < size; count ++) {
        res += arr[count];
    }

    return (res);
}

// Method to compute the sum of differences (taking the array, the size of the array and the overall mean)
float sumDifferences(int *arr, int size, float mean)
{
    float res = 0;

    for (int count = 0; count < size; count++) {
        float temp = arr[count] - mean;
        res += pow(temp, 2);
    }

    return (res);
}

void coordinator(int node_rank, int world_size)
{
    int *arr = new int[arr_size];               // Creating the array with global variable 'array_size'
    int partition_size = arr_size / world_size; // Array size of each node
    int *partition = new int[partition_size];   // Array of int for the node partition of the global array
    float node_mean = 0;                        // Store the mean of the coordinator node
    float overall_mean_sum = 0;                 // Store the sum of all nodes average to perform global average after
    float overall_mean = 0;                     // Store the real average of the global array (from all nodes results)
    float node_sum_differences = 0;             // Store sum Differences from the coordinator node
    float overall_sum_difference = 0;           // Store the overall sum difference

    for (int count = 0; count < arr_size; count ++)     // Fill the array with randoms values between 0 and 50 (following seed 2)
        arr[count] = rand() % 50;

    MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);  // Send partition sizes (global array devided by number of processes) to all the nodes
    
    MPI_Scatter(arr, partition_size, MPI_INT, partition, partition_size, MPI_INT, 0, MPI_COMM_WORLD);   // Scatter the big array into partitions (thanks to 'partition_size' computed before) to all participants nodes
    node_mean = ((float) sum(partition, partition_size) / (float) partition_size);                      // Compute the mean of the coordinator node
    
    MPI_Reduce(&node_mean, &overall_mean_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);    // Make the sum of means of all the nodes (coordinator node included) and store the result on the 'overall_mean_sum' variable
    overall_mean = overall_mean_sum / world_size;
    
    MPI_Bcast(&overall_mean, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);                          // Send the overall mean to all participants node to compute the sum of differences after
    node_sum_differences = sumDifferences(partition, partition_size, overall_mean);     // Compute the coordinator Sum of differencies
    
    MPI_Reduce(&node_sum_differences, &overall_sum_difference, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);       // Make the sum of all sum of differencies from all nodes
    float standard_deviation = sqrt(overall_sum_difference / arr_size);                                         // Compute the standart deviation

    // Print
    printArray(arr, arr_size); // Print entry dataset
    std::cout << "The overall mean is: " << ceil(overall_mean * 100.0) / 100.0 << std::endl;  // Print overall mean
    std::cout << "The standard deviation is: " << ceil(standard_deviation * 100.0) / 100.0 << std::endl;  // Print standard deviation

    delete [] arr;
    delete [] partition;
}

void participant(int node_rank)
{
    int *partition;                 // Array of int for the node partition of the global array
    int partition_size = 0;         // Integer for the size of the partition of each node
    float node_mean = 0;            // Variable to store mean of the actual node
    float overall_mean = 0;         // Variable to store the overall mean sent after computing by the coordinator
    float node_sum_differences = 0; // Variable to store sum of differences computed at the end

    MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);  // All participants node receive the partition size sent by the coordiator and save the value in 'partition_size'
    partition = new int[partition_size];    // Create the partition (int array of each node) by the side received from coordinator
    
    MPI_Scatter(NULL, 0, MPI_INT, partition, partition_size, MPI_INT, 0, MPI_COMM_WORLD);   // Receive the partition array of the node and store it in 'partition'
    node_mean = ((float) sum(partition, partition_size) / (float) partition_size);          // Compute the node mean
    
    MPI_Reduce(&node_mean, NULL, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);     // Do the sum of node means and send it to coordinator
    
    MPI_Bcast(&overall_mean, 1, MPI_INT, 0, MPI_COMM_WORLD);                            // Receive the overall mean calculated by the coordinator
    node_sum_differences = sumDifferences(partition, partition_size, overall_mean);     // Compute the sum of differences thanks to the method
    MPI_Reduce(&node_sum_differences, NULL, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);  // send the node sum of differences to the coordinator
}

int main(int argc, char **argv)
{
    srand(2);               // Seeding the random number generator with 2 (as asked)

    float clock_result = 0; // Initialise clock result variable
    int node_rank = 0;      // Variable to store the process rank
    int world_size = 0;     // Variable to store the number of processes

    clock_t tStart = clock();   // Start the clock
    MPI_Init(NULL, NULL);       // Init the MPI library

    MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);      // Get the rank number of the actual node and store it in the node_rank variable
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);     // Get the number of processes running and store it in the world_size variable

    if (argc == 2) {
        if (argv[1][0] == '-' && argv[1][1] == 'h') {   // If the user ask for help
            if (node_rank == 0) {                       // If the node is the coordinator
                helper();                               // Print the helper
            }
            MPI_Finalize();                             // Finalize the MPI library
            return (0);                                 // Return control to the OS
        } else {
            arr_size = strtol(argv[1], NULL, 10);   // Convert argv 1 in long to store it as arr_size
        }
    }

    if (node_rank == 0) {
        coordinator(node_rank, world_size);     // If the node is node 0, he is the coordinator
    } else {
        participant(node_rank);                 // If the node is not the node 0, he is a participant
    }  

    MPI_Finalize();     // Close the MPI library
    clock_result = (float)(clock() - tStart) / CLOCKS_PER_SEC;  // Get the result of the clock in seconds
    //std::cout << "Time of the node " << node_rank << " is: " << clock_result << std::endl;    // Print the clock timer

    return (0);     // Return control to the OS
}