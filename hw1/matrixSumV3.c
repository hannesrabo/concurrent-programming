/* 	matrix summation using pthreads
	
	Hannes Rabo 2018-01-26

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 10 /* maximum number of workers */

int numWorkers; /* number of workers */
volatile int current_row;

/* timer */
double read_timer()
{
	static bool initialized = false;
	static struct timeval start;
	struct timeval end;
	if (!initialized)
	{
		gettimeofday(&start, NULL);
		initialized = true;
	}
	gettimeofday(&end, NULL);
	return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time;  /* start and end times */
int size, stripSize;		  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

// Datastructure for returning the thread answer.
struct partial_result
{
	int sum, min, minIndex, max, maxIndex;
};

typedef struct partial_result partial_result;

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	int i, j;
	long l; /* use long in case of a 64-bit system */
	pthread_attr_t attr;
	pthread_t workerid[MAXWORKERS];
	current_row = 0;
	int numTimes;

	/* set global thread attributes */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	/* read command line args if any */
	size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
	numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
	numTimes = (argc > 3) ? atoi(argv[3]) : 1;
	if (size > MAXSIZE)
		size = MAXSIZE;
	if (numWorkers > MAXWORKERS)
		numWorkers = MAXWORKERS;
	stripSize = size / numWorkers;

	/* initialize the matrix */
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			matrix[i][j] = rand() % 99;
		}
	}

		/* print the matrix */
#ifdef DEBUG
	for (i = 0; i < size; i++)
	{
		printf("[ ");
		for (j = 0; j < size; j++)
		{
			printf(" %d", matrix[i][j]);
		}
		printf(" ]\n");
	}

#endif

	/* do the parallel work: create the workers */
	start_time = read_timer();

#ifdef BENCHMARK
	for (int time = 0; time < numTimes; time++)
	{
#endif

		for (l = 0; l < numWorkers; l++)
			pthread_create(&workerid[l], &attr, Worker, (void *)l);

		// Collect the result from the threads
		long total, minVal, minIndex, maxVal, maxIndex;
		partial_result *w_result;
		// Initialize values.
		// We assume the first element is both the biggest and smallest at first.
		total = 0;
		minVal = matrix[0][0];
		maxVal = matrix[0][0];
		minIndex = 0;
		maxIndex = 0;

		// Waiting for threads to finish
		for (l = 0; l < numWorkers; l++)
		{
			pthread_join(workerid[l], (void **)&w_result);

			// We need to handle the result calculation immediately here
			// as the result should never be saved.
			total += w_result->sum;

			// Keep track of min and max
			if (w_result->min < minVal)
			{
				minVal = w_result->min;
				minIndex = w_result->minIndex;
			}

			if (w_result->max > maxVal)
			{
				maxVal = w_result->max;
				maxIndex = w_result->maxIndex;
			}

			// This is where we really should deallocate the memory.
			free(w_result);
		}
#ifdef BENCHMARK
	}
#endif
	/* get end time */
	end_time = read_timer();

#ifndef BENCHMARK // This will not run if we are doing a benchmark
	/* print results */
	printf("The total is %d\n", total);

	printf("The minimum is %d at [%d, %d]\n", minVal, (int)(minIndex / size), minIndex % size);
	printf("The maximum is %d at [%d, %d]\n", maxVal, (int)(maxIndex / size), maxIndex % size);
	printf("The execution time is %g sec\n", end_time - start_time);
#else
	printf("%d %d %g\n", size, numWorkers, end_time - start_time);
#endif

	// Thread finished successfully.
	return 0;
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
	long myId = (long)arg;
	int total, i, j, first, last, minVal, minIndex, maxVal, maxIndex;
	register int tempVal;

#ifdef DEBUG
	printf("worker %d (pthread id %d) has started\n", myId, pthread_self());
#endif

	// Initial values to the first element in the matrix
	total = 0;
	minVal = matrix[0][0];
	maxVal = matrix[0][0];
	minIndex = 0;
	maxIndex = 0;

	while (true)
	{
		// Atomic increase of variable
		int row = __sync_fetch_and_add(&current_row, 1);

		// printf("[%d] processing row: %d\n", myId, row);
		if (size > MAXSIZE)
			size = MAXSIZE;

		// Exit if we are done.
		if (row >= size)
		{
			// We first need to store the temporay data before exiting.
			partial_result *w_result = (partial_result *)malloc(sizeof(partial_result));
			w_result->min = minVal;
			w_result->max = maxVal;
			w_result->minIndex = minIndex;
			w_result->maxIndex = maxIndex;
			w_result->sum = total;

#ifdef DEBUG
			printf("The local minimum is %d at [%d, %d]\n", w_result->min, (int)(w_result->minIndex / size), w_result->minIndex % size);
			printf("The local maximum is %d at [%d, %d]\n", w_result->max, (int)(w_result->maxIndex / size), w_result->maxIndex % size);
			printf("The local sum is %d\n", w_result->sum);
#endif

			pthread_exit((void *)w_result);
		}

		// Doing the row calculation.
		for (j = 0; j < size; j++)
		{
			tempVal = matrix[row][j];

			total += tempVal;

			// Keep track of min and max
			if (tempVal < minVal)
			{
				minVal = tempVal;
				minIndex = row * size + j;
			}
			else if (tempVal > maxVal)
			{
				maxVal = tempVal;
				maxIndex = row * size + j;
			}
		}
	}
}
