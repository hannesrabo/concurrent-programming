#ifndef __REENTRANT
#define __REENTRANT
#endif

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define MASTER 0

int main(int argc, char *argv[])
{
    // Initializing MPI
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double startTime;
    int receive_buffer[size];

    srand(1000 * rank);
    int localValue = rand() % 100;

#ifdef DEBUG
    printf("P[%d]: %d\n", rank, localValue);
#endif
    int nrOfTimes = 1;
    for (int t = 0; t < nrOfTimes; t++)
    {

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == MASTER)
        {
            startTime = MPI_Wtime();
        }

        MPI_Allgather(&localValue,
                      1,
                      MPI_INT,
                      receive_buffer,
                      1,
                      MPI_INT,
                      MPI_COMM_WORLD);

        // Calculate min and max
        int min, max;
        min = max = localValue;

        for (int i = 0; i < size; i++)
        {
            if (receive_buffer[i] < min)
                min = receive_buffer[i];
            else if (receive_buffer[i] > max)
                max = receive_buffer[i];
        }

#ifdef DEBUG
        printf("P[%d]: min=%d max=%d\n", rank, min, max);
#endif

        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == MASTER)
        {
            double time = MPI_Wtime() - startTime;
            printf("%d %g\n", size, time);
        }
    }

    MPI_Finalize();
    return 0;
}