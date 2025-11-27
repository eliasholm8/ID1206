#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int num_threads = 0;

void *thread_func(void *arg); /* the thread function */

// Helper function to calculate elapsed time in nanoseconds
long nano_seconds(struct timespec *ts_start, struct timespec *ts_stop)
{
    return (ts_stop->tv_nsec - ts_start->tv_nsec) + (ts_stop->tv_sec - ts_start->tv_sec) * 1000000000;
}

// Struct to hold data for each thread
typedef struct
{
    int id;
    int start_index;
    int end_index;
    float sum;
} thread_data;

// Global array to be summed
float *array;

int main(int argc, char *argv[])
{
    // Check for correct number of arguments
    if (argc != 2)
    {
        printf("Missing argument for number of threads\n");
        exit(EXIT_FAILURE);
    }

    // Parse the number of threads
    num_threads = atoi(argv[1]);

    /* Initialize an array of random values */
    int array_size = 1000000;
    array = malloc(array_size * sizeof(float));

    // Set random seed and fill the array with random floats
    srand(time(NULL));
    for (int i = 0; i < array_size; i++)
    {
        array[i] = (float)rand() / (float)RAND_MAX;
    }

    /* Perform Serial Sum */
    float sum_serial = 0.0;
    struct timespec start_serial, end_serial;

    // Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_serial);

    // Summarize array elements
    for (int i = 0; i < array_size; i++)
    {
        sum_serial += array[i];
    }

    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end_serial);

    // Timer End
    printf("Serial Sum = %.0f, time = %ld microseconds \n", sum_serial, nano_seconds(&start_serial, &end_serial) / 1000);

    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
    struct timespec start_parallel, end_parallel;
    double sum_parallel = 0.0;

    // Calculate chunk size for each thread
    int array_chunk_size = array_size / num_threads;

    // Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_parallel);

    // Create threads and their respective data
    for (int i = 0; i < num_threads; i++)
    {
        thread_data *t_data = malloc(sizeof(thread_data));
        t_data->id = i;
        t_data->start_index = i * array_chunk_size;
        t_data->end_index = (i == num_threads - 1) ? array_size : (i + 1) * array_chunk_size;
        t_data->sum = 0.0;

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&workers[i], &attr, thread_func, t_data);
    }

    // Join the threads and sum their results
    for (int i = 0; i < num_threads; i++)
    {
        thread_data *t_data;
        pthread_join(workers[i], (void **)&t_data);
        sum_parallel += t_data->sum;
        free(t_data);
    }

    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end_parallel);

    // Timer End
    printf("Parallel Sum = %.0f, time = %ld microseconds \n", sum_parallel, nano_seconds(&start_parallel, &end_parallel) / 1000);

    /*free up resources properly */
    free(array);
    free(workers);
}

void *thread_func(void *arg)
{
    thread_data *t_data = (thread_data *)arg;

    /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
    int my_id = t_data->id;

    /* Perform Partial Parallel Sum Here */
    for (int i = t_data->start_index; i < t_data->end_index; i++)
    {
        t_data->sum += array[i];
    }

    printf("Thread %d sum = %f\n", my_id, t_data->sum);
    return t_data;
}