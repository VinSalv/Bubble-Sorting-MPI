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
    @file main.c
*/

#include <stdlib.h>
#include <stdio.h>
#include "radixsort.h"
#include "mpi.h"

#define VERSION 2

int main(int argc, char *argv[]) {
    /* DEFINE AND INIT VARIABLES */
    int i,              // Index
    rank,               // ID thread
    nthread,            // Number of threads
    size,               // Size of array to sort
    max,                // Max number of array to sort
    last_rank,          // ID last rank
    size_last_rank,     // Size of last rank's partition (could be more the others)
    size_others;        // Size of others ranks'partition

    ELEMENT_TYPE *array,        // Unsorted array
    *partition;                 // Partitioned array for each threads

    double time_init_start = 0.0,   // Start init time
    time_radixsort_start = 0.0,     // Start radixsort time
    time_init_end = 0.0,            // End init time
    time_radixsort_end = 0.0;       // End radixsort time

    /* SET MPI ENVIRONMENT */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nthread);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* CHECK AND FETCH PARAMS */
    if (argc < 2) {
        if (rank == 0)
            printf("ERROR! Usage: ./main size");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    size = atoi(argv[1]);
    last_rank = nthread - 1;

    // Allocate array
    if (NULL == (array = malloc(size * sizeof(int))))
        exit(1);

    // Fetch unsorted array and perform its max
    if (rank == last_rank) {
        /* START TIME OF INIT STRUCTURE */
        time_init_start = MPI_Wtime();

        /* INIT STRUCTURE */
        init_structures(&array, size, nthread);

        /* END TIME OF INIT STRUCTURE */
        time_init_end = MPI_Wtime();

        // Perform max
        max = get_max(array, size);
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

#if VERSION == 1
    /* START TIME OF RADIXSORT */
    time_radixsort_start = MPI_Wtime();

    // Radixsort naive
    radixsort_naive(array, partition, size , nthread , rank, max , last_rank , size_last_rank , size_others );

    /* END TIME OF RADIXSORT */
    time_radixsort_end = MPI_Wtime();

#elif VERSION == 2
    int *sizes,         // Partitions'sizes of all threads
    *occurrence,        // Partition's occurences
    *mat_occ,           // Matrix of all partitions's occurences
    *fill,              // Indicator of the arrys fillment
    *read_from_pos;     // Indicator of the array's element next to send

    // Dimension reserved for each threads'partition
    if (NULL == (sizes = (int *) malloc(nthread * sizeof(int))))
        exit(1);
    for (i = 0; i < nthread; i++) {
        if (i == last_rank)
            sizes[i] = size_last_rank;
        else
            sizes[i] = size_others;
    }

    // Allocation arrays for radixsort
    if (NULL == (occurrence = malloc(10 * sizeof(int))))
        exit(1);
    if (NULL == (mat_occ = malloc((nthread * 10) * sizeof(int))))
        exit(1);
    if (NULL == (fill = malloc(nthread * sizeof(int))))
        exit(1);
    if (NULL == (read_from_pos = malloc(nthread * sizeof(int))))
        exit(1);

    // RadixSort
    /* START TIME OF RADIXSORT */
    time_radixsort_start = MPI_Wtime();

    // Radixsort
    radixsort(array, partition, size, nthread, rank, max, last_rank, size_last_rank, size_others, sizes, occurrence,
              mat_occ, fill, read_from_pos);

    /* END TIME OF RADIXSORT */
    time_radixsort_end = MPI_Wtime();

    /* DE-ALLOCATION */
    free(sizes);
    free(occurrence);
    free(mat_occ);
    free(fill);
    free(read_from_pos);

#endif

    /* PRINT OUTPUT */
    if (rank == last_rank)
        write_output_to_file(array, size);

    /* PRINT TIMES */
    printf("%d;%d;%f;%f\n", size, nthread, time_init_end - time_init_start, time_radixsort_end - time_radixsort_start);

    /* DE-ALLOCATION */
    free(partition);
    free(array);

    MPI_Finalize();

    return 0;
}