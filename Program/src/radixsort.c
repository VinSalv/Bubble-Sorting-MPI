/* 
* Course: High Performance Computing 2021/2022
* 
* Lecturer: Francesco Moscato   fmoscato@unisa.it
*
* Group:
* Lamberti      Martina     0622701476  m.lamberti61@studenti.unisa.it
* Salvati       Vincenzo    0622701550  v.salvati10@studenti.unisa.it
* Silvitelli    Daniele     0622701504  d.silvitelli@studenti.unisa.it
* Sorrentino    Alex        0622701480  a.sorrentino120@studenti.unisa.it
*
* Copyright (C) 2021 - All Rights Reserved
*
* This file is part of EsameHPC.
*
* Contest-MPI is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Contest-MPI is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Contest-MPI.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
    @file radixsort.c
*/

// PURPOSE OF THE FILE: Implementation of the functions for the radix sort algorithms. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "radixsort.h"

#define MAX 10 // number of plates

/**
* @brief Function to read an unsorted array from a file.
* @param array    pointer to the vector to be sorted.
* @param size     size of array.
*/
void read_input_from_file(ELEMENT_TYPE *array, int size) {
    // Init variables
    FILE *fd;
    char buf[10];
    char *res;
    int i = 0;
    // Check the file
    fd = fopen("random_numbers.txt", "r");
    if (fd == NULL) {
        perror("Error opening file");
        exit(1);
    }
    // Read from the file to fill the array
    while (i < size) {
        res = fgets(buf, 10, fd);
        array[i++] = atoi(buf);
    }
    fclose(fd);
}

/**
* @brief Function to write an ordered array into a file.
* @param array    pointer to the sorted vector.
* @param size     size of array.
*/
void write_output_to_file(ELEMENT_TYPE *array, int size) {
    // Init variables
    FILE *fptr;
    fptr = fopen("sorted_array.txt", "w");
    // Check the file
    if (fptr == NULL) {
        printf("Error!");
        exit(1);
    }
    // Write into the file to fill the array
    for (int i = 0; i < size; ++i) {
        fprintf(fptr, "%d\n", array[i]);
    }
    fclose(fptr);
}

/**
* @brief Function to get the largest element from an array.
* @param array    pointer to the vector to be sorted.
* @param size     size of array.
*/
int get_max(int *array, int size) {
    // Init variables
    int max = array[0];
    // Find max
    for (int i = 1; i < size; i++)
        if (array[i] > max)
            max = array[i];
    return max;
}

/**
* @brief Function to execute localsort.
* @param array    pointer to the vector to be sorted.
* @param size     size of array.
* @param place    current digit which it is considered (units, tens, hundreds, ...).
*/
void localsort(ELEMENT_TYPE *array, int size, int place) {
    // Init variables
    int *output = malloc((size + 1) * sizeof(int)); // Output vector + TAPE
    int count[10];
    for (int i = 0; i < 10; ++i)
        count[i] = 0;
    // Calculate count of elements
    for (int i = 0; i < size; i++)
        count[(array[i] / place) % 10]++;
    // Calculate cumulative count
    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];
    // Place the elements in sorted order
    for (int i = size - 1; i >= 0; i--) {
        output[count[(array[i] / place) % 10] - 1] = array[i];
        count[(array[i] / place) % 10]--;
    }
    for (int i = 0; i < size; i++)
        array[i] = output[i];
    // De-allocation
    free(output);
}

/**
* @brief Function to execute radixsort_naive.
* @param array             pointer to the vector to be sorted.
* @param partition         partitioned array for each thread.
* @param size              size of array.
* @param rank              ID rank.
* @param nthread           number of threads.
* @param max               max number of array to sort.
* @param last_rank         ID last rank.
* @param size_last_rank    size of last rank's partition (could be more the others).
* @param size_others       size of others ranks'partition.
*/
void radixsort_naive(ELEMENT_TYPE *array, ELEMENT_TYPE *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others ) {
    /* DEFINE AND INIT VARIABLES */
    int i,              // Index
    place;              // Placement (considered unit)

    /* RADIXSORT */
    for (place = 1; max / place > 0; place *= 10)
        (rank == last_rank) ? localsort(partition, size_last_rank, place) : localsort(partition, size_others, place);

    // Use gather to send all partitions to last_rank
    MPI_Gather(partition, size_others, MPI_INT, array, size_others, MPI_INT, last_rank, MPI_COMM_WORLD);

    // Add eventual (size % nthread) last rank's elements to the array and perform last radixsort
    if (rank == last_rank) {
        for (i = 0; i < (size % nthread); i++)
            array[size_others * nthread + i] = partition[size_others + i];
        for (place = 1; max / place > 0; place *= 10)
            localsort(array, size, place);
    }
}

/**
* @brief Function to execute radixsort.
* @param array             pointer to the vector to be sorted.
* @param partition         partitioned array for each thread.
* @param size              size of array.
* @param rank              ID rank.
* @param nthread           number of threads.
* @param max               max number of array to sort.
* @param last_rank         ID last rank.
* @param size_last_rank    size of last rank's partition (could be more the others).
* @param size_others       size of others ranks' partition.
* @param sizes             partitions' sizes of all threads.
* @param occurrence        partition's occurrences.
* @param mat_occurrences   matrix of all partitions' occurrences.
* @param fill              indicator of the filling array.
* @param read_from_pos     indicator of the array's element next to send.
*/
void radixsort(ELEMENT_TYPE *array, ELEMENT_TYPE *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others , int *sizes, int *occurrence, int *mat_occurrences, int *fill, int *read_from_pos) {
    /* DEFINE AND INIT VARIABLES */
    int i,              // Index
    j,                  // Index
    place,              // Placement (considered unit)
    displacement,       // Local pointer of a thread to write/read into a file by a collective function
    r,                  // Index of row
    c,                  // Index of column
    rem,                // Occurences of the matrix that remains after a writement in a shared file (remaining)
    dst_rank,           // Rank which must receive a number (destination rank)
    free_space;         // Occurences of the matrix that could be write

    char fname[] = "arr.bin";   // Shared file
    MPI_File fh;                // File
    MPI_Offset disp;            // Offset of file
    MPI_Status status;          // MPI operation's status
    
    // Open shared file useful for radixsort
    MPI_File_open(MPI_COMM_WORLD, fname, MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE | MPI_MODE_RDWR, MPI_INFO_NULL, &fh);

    /* RADIXSORT */
    for (place = 1; max / place > 0; place *= 10) {
        //Init variables
        dst_rank = 0;
        for (i = 0; i < nthread * 10; i++) {
            if (i < 10)
                occurrence[i] = 0;
            if (i < nthread) {
                fill[i] = 0;
                read_from_pos[i] = 0;
            }
            mat_occurrences[i] = 0;
        }

        // Radixsort for each partition
        localsort(partition, sizes[rank], place);

        // Counting occurences
        for (i = 0; i < sizes[rank]; i++)
            occurrence[(partition[i] / place) % 10] += 1;

        // Distribution of occurrences
        MPI_Allgather(occurrence, 10, MPI_INT, mat_occurrences, 10, MPI_INT, MPI_COMM_WORLD);

        // Analysis of the matrix's occurrences and write into the file
        for (c = 0; c < 10; c++) { // Fix columns
            for (r = 0; r < nthread; r++) { // Scroll rows
                rem = mat_occurrences[r * 10 + c];
                while (rem > 0) {
                    if (fill[dst_rank] == sizes[dst_rank]) // Define the rank to be filled
                        dst_rank++;
                    free_space = (sizes[dst_rank] - fill[dst_rank]); // Define allocable space
                    displacement = 0;
                    if (rank == dst_rank) { // The occurences belong to the current rank

                        if (rank == r) { // The current rank already has the occurrences for itself
                            if (rem <=
                                free_space) { // The current rank has enough space in order to take all occurrences
                                // Calculate displacement
                                for (i = 0; i < nthread; i++)
                                    displacement += fill[i];
                                disp = displacement * sizeof(int);
                                // Write into the file
                                write_into_shared_file(fh, rem, partition, read_from_pos[rank], disp, status);
                                // Set counters
                                read_from_pos[rank] += rem;
                                fill[dst_rank] += rem;
                                rem = 0;
                            } else { // The current rank does not have enough space and take part of the occurrences
                                // Calculate displacement
                                for (i = 0; i < nthread; i++)
                                    displacement += fill[i];
                                disp = displacement * sizeof(int);
                                // Write into the file
                                write_into_shared_file(fh, free_space, partition, read_from_pos[rank], disp, status);
                                // Set counters
                                read_from_pos[r] += free_space;
                                fill[dst_rank] += free_space;
                                rem -= free_space;
                            }
                        } else { // The current rank does not have already the occurences (rank r has them) - the counters must be updated
                            if (rem <= free_space) { // The current rank has enough space in order to take all occurrences
                                // Set counters
                                read_from_pos[r] += rem;
                                fill[dst_rank] += rem;
                                rem = 0;
                            } else { // The current rank does not have enough space and takes part of the occurrences
                                // Set counters
                                read_from_pos[r] += free_space;
                                fill[dst_rank] += free_space;
                                rem -= free_space;
                            }
                        }

                    } else { // The occurences do not belong to the current rank

                        if (rank == r) { // The current rank has the occurrences to give
                            if (rem <= free_space) { // dst_rank has enough space in order to take all occurrences
                                // Calculate displacement
                                for (i = 0; i < nthread; i++)
                                    displacement += fill[i];
                                disp = displacement * sizeof(int);
                                // Write into the file
                                write_into_shared_file(fh, rem, partition, read_from_pos[rank], disp, status);
                                // Set counters
                                read_from_pos[rank] += rem;
                                fill[dst_rank] += rem;
                                rem = 0;
                            } else { // dst_rank does not have enough space and take part of the occurrences
                                // Calculate displacement
                                for (i = 0; i < nthread; i++)
                                    displacement += fill[i];
                                disp = displacement * sizeof(int);
                                // Write into the file
                                write_into_shared_file(fh, free_space, partition, read_from_pos[rank], disp, status);
                                // Set counters
                                read_from_pos[r] += free_space;
                                fill[dst_rank] += free_space;
                                rem -= free_space;
                            }
                        } else { // The current rank does not have the occurences (rank r has them) - the counters must be update
                            if (rem <= free_space) { // dst_rank has enough space in order to take all occurrences
                                // Set counters
                                read_from_pos[r] += rem;
                                fill[dst_rank] += rem;
                                rem = 0;
                            } else { // dst_rank does not have enough space and takes part of the occurrences
                                // Set counters
                                read_from_pos[r] += free_space;
                                fill[dst_rank] += free_space;
                                rem -= free_space;
                            }
                        }

                    }
                }
            }
        }

        /* BARRIER */
        MPI_Barrier(MPI_COMM_WORLD);

        // Update partitions
        disp = rank * size_others * sizeof(int); // Calculate specific displacement for each thread
        MPI_File_read_at(fh, disp, partition, sizes[rank], MPI_INT, &status); // Read partitions from file for each thread
    }

    // Use gather to send all partitions to last_rank
    MPI_Gather(partition, size_others, MPI_INT, array, size_others, MPI_INT, last_rank, MPI_COMM_WORLD);

    // Add eventual (size % nthread) last rank's elements to the array
    if (rank == last_rank) {
        for (i = 0; i < (size % nthread); i++)
            array[size_others * nthread + i] = partition[size_others + i];
    }

    MPI_File_close(&fh);
}

/**
* @brief Function to initializes all the data structures needed in the program.
* @param array      pointer to the vector to be sorted.
* @param size       size of array.
* @param threads    number of threads.
*/
void init_structures(ELEMENT_TYPE **array, int size, int threads){
    // Init variables
    ELEMENT_TYPE *temp_array;
    // Init array
    if (NULL == (temp_array = (ELEMENT_TYPE *) malloc(size * sizeof(ELEMENT_TYPE))))
        perror("Memory Allocation - array");
    // Read the array from file
    read_input_from_file(temp_array, size);
    *array = temp_array;
}

/**
* @brief Function to write specified elements into a file.
* @param fh               file.
* @param num_elem         elements to write into a file.
* @param partition        partitioned array for each thread.
* @param read_from_pos    indicator of the array's element next to send.
* @param disp             offset of file.
* @param status           MPI operation's status.
*/
void write_into_shared_file(MPI_File fh, int num_elem, ELEMENT_TYPE *partition, int read_from_pos, MPI_Offset disp, MPI_Status status) {
    // Init variables
    int *array = malloc(num_elem * sizeof(int));
    for (int j = 0; j < num_elem; j++) {
        array[j] = partition[read_from_pos + j];
    }
    // Write into the file
    MPI_File_write_at(fh, disp, array, num_elem, MPI_INT, &status);
    // De-allocation
    free(array);
}
