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
    int *receive_buffer = NULL;

    // Get a local value
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

            // Initialize buffer to receive the values from slaves
            receive_buffer = malloc(size * sizeof(int));
        }

        // Gather all random values
        MPI_Gather((void *)&localValue,
                   1,
                   MPI_INT,
                   receive_buffer,
                   1,
                   MPI_INT,
                   MASTER,
                   MPI_COMM_WORLD);

        int min_max_buffer[2];

        if (rank == MASTER)
        {

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

            min_max_buffer[0] = min;
            min_max_buffer[1] = max;
        }

        // Push this data to slaves
        MPI_Bcast(min_max_buffer, 2, MPI_INT, MASTER, MPI_COMM_WORLD);
#ifdef DEBUG
        printf("P[%d]: min=%d max=%d\n", rank, min_max_buffer[0], min_max_buffer[1]);
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