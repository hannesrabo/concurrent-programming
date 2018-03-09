#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#define MAX_NUMBER_OF_PEOPLE 32
#define MALE 0
#define FEMALE 1

#define MIN_SLEEP_TIME 5
#define VARIABLE_SLEEP_TIME 10

int number_of_people;
int number_of_times;

pthread_t humans[MAX_NUMBER_OF_PEOPLE];

sem_t mutex_lock, wait_queue_women, wait_queue_men;
int queue_size_women, queue_size_men;
int current_users_women, current_users_men;
int nr_in_a_row, max_users, max_in_a_row;

int getSex(long id)
{
    return id % 2;
}

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

int do_action(long id)
{
    char sex = (getSex(id) == MALE) ? 'M' : 'F';
#ifdef DEBUG
    printf("[%d](%c): entered\n", (int)id, sex);
#endif

    usleep(1000 * (MIN_SLEEP_TIME + rand() % VARIABLE_SLEEP_TIME));

#ifdef DEBUG
    printf("[%d](%c): done!\n", (int)id, sex);
#endif

    return 0;
}

/**
 * This will just wait a random amount of time before the process should
 * continue to try to get access to the resource.
 */
int wait_action(long id)
{
    usleep(1000 * (MIN_SLEEP_TIME + rand() % VARIABLE_SLEEP_TIME));

    return 0;
}

/**
 *  About fairness:
 * *******************************************************************************
 *  
 *  This code is fair in a way that one sex will not be able to occupy the bathroom
 *  for infinite time. It is not fair in a way that it will not garant them access
 *  in the same order as they try to access the resource. This increases throughput.
 *  
 *  The definition of fairness as defined in the assignment is that they should
 *  eventually get access to the shared resource.
 *  
 * 
 *  Algorithmic outline:
 * *******************************************************************************
 * As soon as we signal anyone we are also indirectly passing the mutex to them
 * this means that we loos the right to change things without requiring it back to
 * ourselves.
 * *******************************************************************************
 * 
 *  MUTEX LOCK
 *  If bathroom is full of men or has women in it
 *       Increase this waiting queue size
 *  
 *       MUTEX UNLOCKED
 *       <<<-----  Wait for signal for the queue of this sex
 *  
 *  If other sex has queue: increase nr of times in a row
 *  
 *  Number of free positions in bathroom is decreased
 *  
 *  If there are anyone in line for this queue and we havn't reach
 *  max for this sex yet (in a row)
 *  
 *       Remove from queue count
 *       Signal queue once (they will signal the next) ---->>>>
 *  else
 *       MUTEX UNLOCK
 *  
 *  Do bathroom stuff (this section is unlocked and may take a while)
 *  MUTEX LOCK
 *  
 *  Free positions in bathroom is increased
 *  
 *  If there are any on this sex waiting and we havn't reached max
 *  in a row for this sex yet
 *  
 *       decrease queue size
 *       signal this queue once ---->>>>
 *  
 *  Else if there is any of the other sex waiting
 *       nr in a row = 0 again
 *       decrease other queue size
 *       signal other queue once ---->>>>
 *  
 *  Else if (last one out)
 *       nr in row = 0
 *       MUTEX UNLOCK
 *  
 *  Else
 *       MUTEX UNLOCK
 * ******************************************************************************
 * 
 */
void *Worker(void *arg)
{
    // This is the worker thread (man or woman)
    long id = (long)arg;
    int sex = getSex(id);
    double *turn_around_time = (double *)malloc(number_of_times * sizeof(double));

    for (int turn = 0; turn < number_of_times; turn++)
    {
        turn_around_time[turn] = read_timer();

        if (sex == MALE)
        {
            // MUTEX LOCK
            sem_wait(&mutex_lock);

            // If bathroom is full of men or has women in it
            // If nr_in_a_row > 0 we can assume that it is because men are in
            // Else we would stop anyway.
            if (current_users_women > 0 || current_users_men >= max_users || nr_in_a_row >= max_in_a_row)
            {
                // Increase this waiting queue size
                queue_size_men++;
                // MUTEX UNLOCKED
                sem_post(&mutex_lock);
                // <<<-----  Wait for signal for the queue of this sex
                // PASS THE BATON MUTEX LOCK (we get the lock automatically)
                sem_wait(&wait_queue_men);
            }

            // If other sex has queue: increase nr of times in a row
            if (queue_size_women > 0)
            {
                nr_in_a_row++;
#ifdef DEBUG
                printf("[%d] time in a row: %d\n", (int)id, nr_in_a_row);
#endif
            }
#ifdef DEBUG
            else
            {
                printf("[%d] No queue for women\n", (int)id);
            }
#endif

            // Number of free positions in bathroom is decreased
            current_users_men++;

            // If there are anyone in line for this queue and we havn't reach
            // max for this sex yet (in a row)
            if (queue_size_men > 0 && nr_in_a_row < max_in_a_row && current_users_men < max_users)
            {
                // Remove from queue count
                queue_size_men--;
                // Signal queue once (they will signal the next) ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_men);
            }
            else
            {
                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }

                // Do bathroom stuff (this section is unlocked and may take a while)
#ifdef DEBUG
            printf("[%d]: Users -- M:[%d] F:[%d]\n", (int)id, current_users_men, current_users_women);
#endif

            // Record time as soon as we get the possibility to do anything
            turn_around_time[turn] = read_timer() - turn_around_time[turn];

            do_action(id);

            // MUTEX LOCK
            sem_wait(&mutex_lock);

            // Free positions in bathroom is increased
            current_users_men--;

            // If there are any on this sex waiting and we havn't reached max
            // in a row for this sex yet                         // This is redundant as we just decreased in crit sec.
            if (queue_size_men > 0 && nr_in_a_row < max_in_a_row /*&& current_users_men < max_users*/)
            {
                // decrease queue size
                queue_size_men--;
                // signal this queue once ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_men);
            }
            // Else if there is any of the other sex waiting and no users of this
            else if (current_users_men == 0 && queue_size_women > 0)
            {
                // nr in a row = 0 again
                nr_in_a_row = 0;
                // decrease other queue size
                queue_size_women--;
                // signal other queue once ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_women);
            }
            // Else (last one out)
            else if (current_users_men == 0)
            {
                // nr in row = 0
                nr_in_a_row = 0;

                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }
            else
            {
                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }
        }
        else /* sex == FEMALE */
        {
            // MUTEX LOCK
            sem_wait(&mutex_lock);

            // If bathroom is full of men or has women in it
            // If nr in a row is larger than 0 we assume it is because of women.
            // (else we would stop here anyway)
            if (current_users_men > 0 || current_users_women >= max_users || nr_in_a_row >= max_in_a_row)
            {
                // Increase this waiting queue size
                queue_size_women++;
                // MUTEX UNLOCKED
                sem_post(&mutex_lock);
                // <<<-----  Wait for signal for the queue of this sex
                // PASS THE BATON MUTEX LOCK (we get the lock automatically)
                sem_wait(&wait_queue_women);
            }

            // If other sex has queue: increase nr of times in a row
            if (queue_size_men > 0)
            {
                nr_in_a_row++;
#ifdef DEBUG
                printf("[%d] time in a row: %d\n", (int)id, nr_in_a_row);
#endif
            }
#ifdef DEBUG
            else
            {
                printf("[%d] No queue for men\n", (int)id);
            }
#endif

            // Number of free positions in bathroom is decreased
            current_users_women++;

            // If there are anyone in line for this queue and we havn't reach
            // max for this sex yet (in a row)
            if (queue_size_women > 0 && nr_in_a_row < max_in_a_row && current_users_women < max_users)
            {
                // Remove from queue count
                queue_size_women--;
                // Signal queue once (they will signal the next) ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_women);
            }
            else
            {
                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }

#ifdef DEBUG
            printf("[%d]: Users -- M:[%d] F:[%d]\n", (int)id, current_users_men, current_users_women);
#endif

            // Record time as soon as we get the possibility to do anything
            turn_around_time[turn] = read_timer() - turn_around_time[turn];

            // Do bathroom stuff (this section is unlocked and may take a while)
            do_action(id);

            // MUTEX LOCK
            sem_wait(&mutex_lock);

            // Free positions in bathroom is increased
            current_users_women--;

            // If there are any on this sex waiting and we havn't reached max
            // in a row for this sex yet
            // We don't need to check current users here as we just decreased
            // and are in a critical section.
            if (queue_size_women > 0 && nr_in_a_row < max_in_a_row)
            {
                // decrease queue size
                queue_size_women--;
                // signal this queue once ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_women);
            }
            // Else if there is any of the other sex waiting and no users of this
            else if (current_users_women == 0 && queue_size_men > 0)
            {
                // nr in a row = 0 again
                nr_in_a_row = 0;
                // decrease other queue size
                queue_size_men--;
                // signal other queue once ---->>>>
                // PASS THE BATON MUTEX UNLOCK
                sem_post(&wait_queue_men);
            }
            // Else (last one out)
            else if (current_users_women == 0)
            {
                // nr in row = 0
                nr_in_a_row = 0;

                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }
            else
            {
                // MUTEX UNLOCK
                sem_post(&mutex_lock);
            }
        } // end if (woman)

        wait_action(id);
    }

    pthread_exit(turn_around_time);
}

int main(int argc, char *argv[])
{
    double *result_vec;

    // Initialize the semaphores
    sem_init(&mutex_lock, 0, 1); // Only one process in the critical section
    sem_init(&wait_queue_women, 0, 0);
    sem_init(&wait_queue_men, 0, 0);

    // Initialize counters
    queue_size_women = 0;
    queue_size_men = 0;
    current_users_women = 0;
    current_users_men = 0;

    nr_in_a_row = 0;
    max_users = 2;
    max_in_a_row = 4;
    number_of_people = 32;

    // Initialize the number of workers
    // number_of_people = 20;
    number_of_times = 20; // How many times each will try

#ifndef DEBUG
    FILE *fp;
    fp = fopen("result.dat", "w");

    for (number_of_people = 4; number_of_people < MAX_NUMBER_OF_PEOPLE; number_of_people += 2)
    {

#endif
        double average[number_of_people];
        double total = 0;
        double total_avg = 0;
        // Create threads
        for (long i = 0; i < number_of_people; i++)
            pthread_create(&(humans[i]), NULL, Worker, (void *)i);

        for (long i = 0; i < number_of_people; i++)
        {
            pthread_join(humans[i], (void **)&result_vec);

            average[(int)i] = 0;
            for (int j = 0; j < number_of_times; j++)
                average[(int)i] += result_vec[j];

            total += average[(int)i];

            average[(int)i] /= (double)number_of_times;
        }

        total_avg = total / (double)(number_of_people * number_of_times);

#ifndef DEBUG
        fprintf(fp, "%d %f %f\n", number_of_people, total, total_avg);
#endif

        // Writing to console
        printf("\n\n");
        printf("Settings\n");
        printf("---------------------------\n");
        printf("Avg. time:          %d ms\n", ((MIN_SLEEP_TIME + VARIABLE_SLEEP_TIME / 2)));
        printf("Workers:            %d\n", number_of_people);
        printf("Times per worker:   %d\n", number_of_times);
        printf("Concurrent workers: %d\n", max_users);
        printf("Switch after max:   %d\n", max_in_a_row);
        printf("---------------------------\n\n");

        printf("Average turn around times: \n");
        printf("---------------------------\n");
        for (long i = 0; i < number_of_people; i++)
            printf("[%d]: %g \ts\n", (int)i, average[(int)i]);

        printf("---------------------------\n");
        printf("Total time   : %g\n", total);
        printf("Total average: %g\n", total_avg);

#ifndef DEBUG
    }

    fclose(fp);
#endif

    printf("Program exited sucessfully\n");

    return 0;
}