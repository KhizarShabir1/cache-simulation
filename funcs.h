#include <iostream>
#include <string.h>

using namespace std;

bool parseParams(int argc, char *argv[ ], int& cache_capacity, int& cache_blocksize, int& cache_associativity, int& cache_spilt, int& write_back, int& write_through, int& write_allocate, int& write_no_allocate);