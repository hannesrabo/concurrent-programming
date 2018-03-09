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
    int val_buffer[2];

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

        // Initialize min and max
        int min, max;
        min = max = localValue;
        val_buffer[0] = min;
        val_buffer[1] = max;

        // Master start chain
        if (rank == MASTER)
        {

            // Start chain
            MPI_Send(val_buffer,
                     2,
                     MPI_INT,
                     rank + 1,
                     0,
                     MPI_COMM_WORLD);

            // Receive global max
            MPI_Recv(val_buffer,
                     2,
                     MPI_INT,
                     size - 1,
                     0,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            min = val_buffer[0];
            max = val_buffer[1];

            // Send out global values to the rest.
            MPI_Send(val_buffer,
                     2,
                     MPI_INT,
                     rank + 1,
                     0,
                     MPI_COMM_WORLD);
        }
        else
        {

            // Receive values from previous.
            MPI_Recv(val_buffer,
                     2,
                     MPI_INT,
                     rank - 1,
                     0,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            if (val_buffer[0] > min)
                val_buffer[0] = min;
            if (val_buffer[1] < max)
                val_buffer[1] = max;

            // Pass the data on
            int receiver = (rank + 1) % size;
            MPI_Send(val_buffer,
                     2,
                     MPI_INT,
                     receiver,
                     0,
                     MPI_COMM_WORLD);

            // Receive the global max and min
            MPI_Recv(val_buffer,
                     2,
                     MPI_INT,
                     rank - 1,
                     0,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            // Pass global values to the next node
            MPI_Send(val_buffer,
                     2,
                     MPI_INT,
                     receiver,
                     0,
                     MPI_COMM_WORLD);

            min = val_buffer[0];
            max = val_buffer[1];
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