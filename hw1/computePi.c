/**
 * This program approximates PI given a epsilon using n nr of threads
 * 
 * Hannes Rabo 2018-01-29
 */

// #ifndef _REENTRANT
// #define _REENTRANT
// #endif

#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAXWORKERS 10
#define DEFAULT_EPSILON 0.01L

pthread_t workers[MAXWORKERS];
long double workerResult[MAXWORKERS];
int numWorkers;
long double epsilon;
long double sliceSize;
const long double PI = 3.1415926535897932384626433832795028841971693993751058209749445923L;

/*
 * Read the current time. Used to calculate execution time.
 */
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

// TODO: Make it inline
long double circle_function(long double x)
{
    return sqrtl(1.0L - powl(x, 2.0L));
}

// Recursively calculate integral
long double calculateArea(long double left, long double right, long double left_v, long double right_v, long double area)
{
    long double middle = (right + left) / 2;
    long double middle_v = circle_function(middle);

    // Approximating areas
    long double left_a = (left_v + middle_v) * (middle - left) / 2.0L;
    long double right_a = (middle_v + right_v) * (right - middle) / 2.0L;

    // printf("l: %Lf %Lf\n", left, left_a);
    // printf("r: %Lf %Lf\n", right, right_a);

    // Continue recusing which the change is sufficiently large
    if (fabsl((left_a + right_a) - area) > epsilon)
    {
        left_a = calculateArea(left, middle, left_v, middle_v, left_a);
        right_a = calculateArea(middle, right, middle_v, right_v, right_a);
    }

    return (left_a + right_a);
}

void *Worker(void *arg)
{
    long id = (long)arg;

    // Divide area in two and recurse both ways.
    long double left = sliceSize * id;
    long double right = sliceSize * (id + 1);
    long double left_v = circle_function(left);
    long double right_v = circle_function(right);
    long double area_approximation = (left_v + right_v) * (right - left) / 2.0L;

    workerResult[id] = calculateArea(left, right, left_v, right_v, area_approximation);

#ifdef DEBUG
    printf("[%ld]: %Lf\n", id, workerResult[id]);
#endif

    return NULL;
}

int main(int argc, char *argv[])
{
    // Reading command line parameters
    if (argc > 1)
    {
        sscanf(argv[1], "%Lf", &epsilon);
    }
    else
    {
        epsilon = DEFAULT_EPSILON;
    }

    numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;

    if (numWorkers > MAXWORKERS)
        numWorkers = MAXWORKERS;

    // Dividing the interval in the same number as the workers
    sliceSize = 1.0L / (long double)numWorkers;

    // And starting workers
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    double time_start, time_end;

    time_start = read_timer();
    for (long i = 0; i < numWorkers; i++)
        if (pthread_create(&workers[i], &attr, Worker, (void *)i))
            exit(-1);

    // Await threads and calculate the result.
    long double pi = 0;
    for (long i = 0; i < numWorkers; i++)
    {
        pthread_join(workers[i], NULL);
        pi += workerResult[i];
    }

    pi *= 4.0L;
    time_end = read_timer();

    // Calculate how many decimals correct this is.
    long double d = fabsl(pi - PI);
    int diff = 0;
    while ((int)(d *= 10) == 0)
        diff++;

#ifndef BENCHMARK
    printf("Pi calculated correctly with %d decimals\n\n", diff);

    printf("PI == %.63Lf\n", PI);
    printf("PI ~= %.63Lf\n", pi);

    printf("Time: %g sec\n", time_end - time_start);
#else
    printf("%d %g %d\n", numWorkers, time_end - time_start, diff);
#endif
    // Each worker will recuse by itself on that interval.

    // Done when each sub interval has been calculated and we have collected the result.
    // TODO: Can we collect any of the worker threads (no order?).

    return 0;
}