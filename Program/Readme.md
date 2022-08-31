# Contest-MPI

Course: High Performance Computing 2021/2022

Lecturer: Francesco Moscato	fmoscato@unisa.it

Group:
 - Lamberti      Martina     0622701476  m.lamberti61@studenti.unisa.it
 - Salvati       Vincenzo    0622701550  v.salvati10@studenti.unisa.it
 - Silvitelli    Daniele     0622701504  d.silvitelli@studenti.unisa.it
 - Sorrentino    Alex        0622701480  a.sorrentino120@studenti.unisa.it

    @file README.md

## Dependencies

* CMake 3.9+
* MPICH
* Python3

## How to run

In the main.c file there is a macro called "VERSION" that defines which algorithm is running:
   •	to run the first solution (naive version), set `VERSION 1`;
   •	to run the second solution (file and matrix of occurrences), set `VERSION 2` (default).

1. Create a build directory and launch cmake

   ```batch
   mkdir build
   cd build
   cmake ..
   ```

2. Generate executables with `make`
3. To generate measures run `make generate_measures`
4. To extract mean times and speedup curves from them run `make extract_measures`

Results can be found in the `measures/measure` directory, divided by problem size and the gcc optimization option used.

By default, speedup curves are generated taking the relative `program_seq_Ox` as reference, if you want `program_seq_O0` as reference for all speedup plots launch `make change_ref` before generating them with `make extract_measures`.

If you want execute the test case you should launch `mpicc test_radixsort.c -o test_radixsort && mpirun -np nthread ./test_radixsort` or `mpicc test_radixsort_naive.c -o test_radixsort_naive && mpirun -np nthread ./test_radixsort_naive` paying attention to execute it on the directory "./Contest-MPI/test".

With the command `make docs`, in the build directory, you can generate html files in order to see the auto generated doxygen documentation. In particular, you can open the file on a directory "./Contest-MPI/build/html/index.html". 