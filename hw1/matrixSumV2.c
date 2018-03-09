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

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	int i, j;
	long l; /* use long in case of a 64-bit system */
	pthread_attr_t attr;
	pthread_t workerid[MAXWORKERS];
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
		struct partial_result *thread_result;
		long total, minVal, minIndex, maxVal, maxIndex;

		// Initialize values.
		// We assume the first element is both the biggest and smallest at first.
		total = 0;
		minVal = matrix[0][0];
		maxVal = matrix[0][0];
		minIndex = 0;
		maxIndex = 0;

		for (l = 0; l < numWorkers; l++)
		{
			pthread_join(workerid[l], (void **)&thread_result);

			// We need to handle the result calculation immediately here
			// as the result should never be saved.
			total += thread_result->sum;

			// Keep track of min and max
			if (thread_result->min < minVal)
			{
				minVal = thread_result->min;
				minIndex = thread_result->minIndex;
			}

			if (thread_result->max > maxVal)
			{
				maxVal = thread_result->max;
				maxIndex = thread_result->maxIndex;
			}
			free(thread_result);
		}

#ifdef BENCHMARK
	}
#endif

	/* get end time */
	end_time = read_timer();

#ifndef BENCHMARK
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

	/* determine first and last rows of my strip */
	first = myId * stripSize;
	last = (myId == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

	/* sum values in my strip, also find min and max */
	total = 0;
	minVal = matrix[first][0]; // Initial values are the first of the strip.
	maxVal = matrix[first][0];
	minIndex = first * size;
	maxIndex = minIndex;
	for (i = first; i <= last; i++)
		for (j = 0; j < size; j++)
		{
			tempVal = matrix[i][j];

			total += tempVal;

			// Keep track of min and max
			if (tempVal < minVal)
			{
				minVal = tempVal;
				minIndex = i * size + j;
			}
			else if (tempVal > maxVal)
			{
				maxVal = tempVal;
				maxIndex = i * size + j;
			}
		}

	// Create the result struct
	struct partial_result *result_struct = malloc(sizeof(struct partial_result));
	result_struct->sum = total;
	result_struct->min = minVal;
	result_struct->minIndex = minIndex;
	result_struct->max = maxVal;
	result_struct->maxIndex = maxIndex;

#ifdef DEBUG
	printf("The local minimum is %d at [%d, %d]\n", result_struct->min, (int)(result_struct->minIndex / size), result_struct->minIndex % size);
	printf("The local maximum is %d at [%d, %d]\n", result_struct->max, (int)(result_struct->maxIndex / size), result_struct->maxIndex % size);
#endif

	pthread_exit((void *)(result_struct));
}
