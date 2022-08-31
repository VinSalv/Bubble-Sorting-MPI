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
    @file testradixsort_naive.c
*/

// PURPOSE OF THE FILE: Test the functions for the radixsort naive algorithm. 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "mpi.h"

void read_input_from_file(int *, int);

int get_max(int *, int);

void localsort(int *, int, int);

void radixsort_naive(int *array, int *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others );

void init_structures(int **, int, int);

void init_structure_test(int *array, int size, int threads);

void get_max_test(int *array, int size);

void radixsort_naive_test(int *array, int size);

/**
* @brief Function to read an unsorted array from a file.
* @param array    pointer to the vector to be sorted.
* @param size     size of array.
*/
void read_input_from_file(int *array, int size) {
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
void localsort(int *array, int size, int place) {
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
    // Deallocation
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
void radixsort_naive(int *array, int *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others ) {
    /* DEFINE AND INIT VARIABLES */
    int i,              // Index
    place;              // Placement (considered unit)

    /* RADIXSORT*/
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
* @brief Function to initializes all the data structures needed in the program.
* @param array      pointer to the vector to be sorted.
* @param size       size of array.
* @param threads    number of threads.
*/
void init_structures(int **array, int size, int threads){
    // Init variables
    int *temp_array;
    // Init array
    if (NULL == (temp_array = (int *) malloc(size * sizeof(int))))
        perror("Memory Allocation - array");
    // Read the array from file
    read_input_from_file(temp_array, size);
    *array = temp_array;
}

/**
 * @brief Function to test the initialization of the vector needed in the program.
 * @param array      pointer to the vector.
 * @param size       size of array.
 * @param threads    number of threads.
 */
void init_structure_test(int *array, int size, int threads) {

    int expected_array[50] = {226586,
                            828486,
                            77349,
                            41049,
                            731641,
                            625895,
                            441364,
                            364456,
                            842330,
                            151095,
                            839923,
                            25875,
                            663981,
                            735094,
                            242049,
                            258882,
                            872683,
                            740710,
                            607021,
                            722886,
                            396015,
                            793923,
                            359946,
                            81896,
                            437310,
                            864737,
                            322462,
                            55498,
                            126631,
                            172450,
                            681433,
                            641980,
                            327494,
                            587280,
                            226723,
                            301470,
                            873506,
                            690845,
                            235278,
                            408109,
                            440118,
                            892212,
                            377771,
                            976364,
                            13965,
                            558843,
                            417080,
                            559727,
                            439174,
                            754856};

    for (int i = 0; i < size; i++) {
        assert(array[i] == expected_array[i]);
    }
}

/**
 * @brief Function to test the max of the vector.
 * @param array     pointer to the vector.
 * @param size      size of array.
 */
void get_max_test(int *array, int size) {

    int max = get_max(array, size);

    int expected_max = 976364;

    assert(max == expected_max);
}

/**
 * @brief Function to test the radixsort algorithm.
 * @param array        pointer to the vector to be sorted.
 * @param size         size of array.
 */
void radixsort_naive_test(int *array, int size) {

    int expected_array[50] = {13965,
                            25875,
                            41049,
                            55498,
                            77349,
                            81896,
                            126631,
                            151095,
                            172450,
                            226586,
                            226723,
                            235278,
                            242049,
                            258882,
                            301470,
                            322462,
                            327494,
                            359946,
                            364456,
                            377771,
                            396015,
                            408109,
                            417080,
                            437310,
                            439174,
                            440118,
                            441364,
                            558843,
                            559727,
                            587280,
                            607021,
                            625895,
                            641980,
                            663981,
                            681433,
                            690845,
                            722886,
                            731641,
                            735094,
                            740710,
                            754856,
                            793923,
                            828486,
                            839923,
                            842330,
                            864737,
                            872683,
                            873506,
                            892212,
                            976364};

    for (int i = 0; i < size; i++)
        assert(array[i] == expected_array[i]);
    
}

int main(int argc, char *argv[]) {

    /* DEFINE AND INIT VARIABLES */
    int i,              // Index
    rank,               // ID thread
    nthread,            // Number of threads
    size = 50,          // Size of array to sort
    max,                // Max number of array to sort
    last_rank,          // ID last rank
    size_last_rank,     // Size of last rank's partition (could be more the others)
    size_others;        // Size of others ranks'partition

    int *array,        // Unsorted array
    *partition;                 // Partitioned array for each threads

    /* SET MPI ENVIRONMENT */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nthread);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* CHECK AND FETCH PARAMS */
    if (argc < 1) {
        if (rank == 0)
            printf("ERROR! Usage: ./main size");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    last_rank = nthread - 1;

    // Allocate array
    if (NULL == (array = malloc(size * sizeof(int))))
        exit(1);

    // Fetch unsorted array and perform its max
    if (rank == last_rank) {

        /* INIT STRUCTURE */
        init_structures(&array, size, nthread);
        // Test init structure
        init_structure_test(array, size, nthread);

        // Perform max
        max = get_max(array, size);
        // Test get_max
        get_max_test(array, size);
    }

    // Communicate the max to other ranks
    MPI_Bcast(&max, 1, MPI_INT, last_rank, MPI_COMM_WORLD);

    // Allocate partition of last rank
    size_last_rank = (size % nthread == 0) ? (size / nthread) : (size / nthread + size % nthread);
    if (rank == last_rank)
        if (NULL == (partition = malloc(size_last_rank * sizeof(int))))
            exit(1);

    // Allocate partition of others ranks
    size_others = size / nthread;
    if (rank != last_rank)
        if (NULL == (partition = malloc(size_others * sizeof(int))))
            exit(1);

    // Fill all partitions
    MPI_Scatter(array, size_others, MPI_INT, partition, size_others, MPI_INT, last_rank, MPI_COMM_WORLD);

    // Add eventual (size % nthread) unsorted array's elements to the last rank's partiton
    if (rank == last_rank)
        for (i = 0; i < (size % nthread); i++)
            partition[size_others + i] = array[size_others * nthread + i];

    // Radixsort naive
    radixsort_naive(array, partition, size, nthread, rank, max, last_rank, size_last_rank, size_others);

    if (rank == last_rank)
        radixsort_naive_test(array, size);

    free(partition);
    free(array);

    MPI_Finalize();

    exit(EXIT_SUCCESS);
}