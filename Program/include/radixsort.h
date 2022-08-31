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
    @file radixsort.h
*/

// PURPOSE OF THE FILE: Prototypes Definitions of the functions used in radixsort.c

#ifndef RADIXSORT_H_ 
#define RADIXSORT_H_

#include "mpi.h"

void read_input_from_file(ELEMENT_TYPE *, int);

void write_output_to_file(ELEMENT_TYPE *, int);

int get_max(ELEMENT_TYPE *, int);

void localsort(ELEMENT_TYPE *, int, int);

void radixsort_naive(ELEMENT_TYPE *array, ELEMENT_TYPE *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others );

void radixsort(ELEMENT_TYPE *array, ELEMENT_TYPE *partition, int size , int nthread , int rank, int max , int last_rank , int size_last_rank , int size_others , int *sizes, int *occurrency, int *mat_occ, int *fill, int *read_from_pos);

void init_structures(ELEMENT_TYPE **, int, int);

void write_into_shared_file(MPI_File fh, int num_elem, ELEMENT_TYPE *partition, int read_from_pos, MPI_Offset disp, MPI_Status status);

#endif
