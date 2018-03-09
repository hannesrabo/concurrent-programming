/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 16  /* maximum number of workers */
#define MAX_NUMBER_OF_TIMES_MEAN 20

// int numWorkers;
// int numTimesMean;
int size;
int matrix[MAXSIZE][MAXSIZE];
double start_time, end_time;
double time_result[MAX_NUMBER_OF_TIMES_MEAN];
void *Worker(void *);

double matrixOperation() 
{
	// Thread private variables
	int j, _minI, _maxI, _min, _max, _total = 0, _temp = 0;

	// Shared variables
	int i, minI, maxI, min, max, total;
	minI = 0;
	maxI = 0;
	min  = matrix[0][0];
	max  = matrix[0][0];
	total = 0;

	int k;

	start_time = omp_get_wtime();

	#pragma omp parallel private(j, _temp, _total, _minI, _maxI, _min, _max) 
	{	
		#pragma omp for reduction(+ : total)
		for (i = 0; i < size; i++)
		{
			// We need to reset and define the local variables in here as thread might be executing again.
			_minI = 0;
			_maxI = 0;
			_min  = matrix[0][0];
			_max  = matrix[0][0];
			_total = 0;

			// Internal calculations (for one row)
			for (j = 0; j < size; j++)
			{
				_temp = matrix[i][j];

				_total += _temp;
				if (_temp < _min)
				{
					_min 	= _temp;
					_minI 	= i * size + j;
				}
				else if (_temp > _max) 
				{
					_max 	= _temp;
					_maxI 	= i * size + j;
				}
				// printf("%d (%d): %d \n", omp_get_thread_num(), i, _temp);
			}

			printf("%d (%d): %d %d %d\n", omp_get_thread_num(), i, _min, _max, _total);

			// Reduction state
			// We could check if the values are about to update before entering critical sector
			// This does on the other hand appear to have more overhead than just doing it the
			// regual and plain way.
			// The overhead of entering a critical section is also larger than the gain of making two
			// smaller here...#
		}

		#pragma omp critical
		{
			if (_min < min)
			{
				min 	= _min;
				minI 	= _minI;
			}

			if (_max > max)
			{
				max 	= _max;
				maxI 	= _maxI;
			}
			
			total += _total;
		}

	}

	// implicit barrier

	end_time = omp_get_wtime();

#if 1
	printf("the total is %d\n", total);
	printf("Min: %d at [%d, %d]\n", min, minI / size, minI % size);
	printf("Max: %d at [%d, %d]\n", max, maxI / size, maxI % size);
	printf("it took %g seconds\n", end_time - start_time);
#endif

	return end_time - start_time;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	int i, j;	
	double mean;
	char filename[120];
	FILE *fp;

	size = MAXSIZE;

	/* initialize the matrix */
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			matrix[i][j] = rand() % 99;
		}
	}

	int i_size, i_workers;

#if 1 // Debug

	omp_set_num_threads(1);
	size = 5;

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			printf("%d, ", matrix[i][j]);
		}
		printf("\n");
	}
	matrixOperation();

#else
	for (i_size = 1000; i_size <= 10000; i_size += 1000) {
		sprintf(filename, "result-s-%d", i_size);

		fp = fopen(filename, "w");

		for (i_workers = 1; i_workers <= 10; i_workers++) {
			omp_set_num_threads(i_workers);

			// To note here: We are not reinitializing the matrix each time which means that
			// caching effects might be different than if we had the entire matrix in a smaller
			// memory area
			size = i_size;

			for (i = 0; i < MAX_NUMBER_OF_TIMES_MEAN; i++)
			{
				time_result[i] = matrixOperation();
			}

			// Calculate mean value
			qsort((void *)time_result, MAX_NUMBER_OF_TIMES_MEAN, sizeof(double), cmpfunc);

			if ((MAX_NUMBER_OF_TIMES_MEAN % 2) == 0)
			{  // Even number of elements
				mean = (time_result[MAX_NUMBER_OF_TIMES_MEAN / 2 - 1] + time_result[MAX_NUMBER_OF_TIMES_MEAN / 2]) / 2;
			}
			else
			{  // Odd number of elements
				mean = time_result[MAX_NUMBER_OF_TIMES_MEAN / 2];
			}

			fprintf(fp, "%d %d %d %g\n", omp_get_max_threads(), size, MAX_NUMBER_OF_TIMES_MEAN, mean);

			printf(".");
			fflush(stdout);

		}

		fclose(fp);

		printf("\rWrote file for matrix size: %s\n", filename);
	}
#endif

}
