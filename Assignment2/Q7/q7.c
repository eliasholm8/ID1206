#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int num_threads = 0;
int array_size = 0;
int histogram_bins = 30;
double histogram_chunk_range = 0;

void *thread_func(void *arg); /* the thread function */

// Helper function to calculate elapsed time in nanoseconds
long nano_seconds(struct timespec *ts_start, struct timespec *ts_stop)
{
    return (ts_stop->tv_nsec - ts_start->tv_nsec) + (ts_stop->tv_sec - ts_start->tv_sec) * 1000000000;
}

int histogram_index(double value)
{
    int index = (int)(value / histogram_chunk_range);
    if (index >= histogram_bins)
        index = histogram_bins - 1;
    return index;
}

void print_histogram(int *histogram, int bins)
{
    // Dynamic histogram_increment
    int histogram_increment = array_size / histogram_bins / 50;
    printf("Histogram:\n");
    printf("'|' = %d\n", histogram_increment);
    for (int i = 0; i < bins; i++)
    {
        printf("Bin %2d:", i + 1);
        for (int j = 0; j < histogram[i]; j += histogram_increment) 
        {
            printf("|");
        }
        printf(" (%d)\n", histogram[i]);
    }
}

// Struct to hold data for each thread
typedef struct
{
    int id;
    int start_index;
    int end_index;
    double sum;
    int *local_histogram;
} thread_data;

// Global array to be summed
double *array;
int *histogram;

int main(int argc, char *argv[])
{
    // Check for correct number of arguments
    if (argc < 2)
    {
        printf("Missing argument for number of threads\n");
        exit(EXIT_FAILURE);
    }
    else if (argc < 3)
    {
        printf("Missing argument for array size\n");
        exit(EXIT_FAILURE);
    }

    // Parse the number of threads
    num_threads = atoi(argv[1]);
    array_size = atoi(argv[2]);

    // Initialize an array and histogram
    array = malloc(array_size * sizeof(double));
    histogram = calloc(histogram_bins, sizeof(int));
    histogram_chunk_range = 1.0 / histogram_bins;

    // Set random seed and fill the array with random doubles
    srand(time(NULL));
    for (int i = 0; i < array_size; i++)
    {
        array[i] = (double)rand() / (double)RAND_MAX;
    }

    /* Perform Serial Sum */
    double sum_serial = 0.0;
    struct timespec start_serial, end_serial;

    // Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_serial);

    // Summarize array elements
    for (int i = 0; i < array_size; i++)
    {
        sum_serial += array[i];
        histogram[histogram_index(array[i])]++;
    }

    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end_serial);

    // Timer End
    printf("Serial Sum = %.0f, time = %ld microseconds \n", sum_serial, nano_seconds(&start_serial, &end_serial) / 1000);

    print_histogram(histogram, histogram_bins);

    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
    struct timespec start_parallel, end_parallel;
    double sum_parallel = 0.0;

    // Calculate chunk size for each thread
    int array_chunk_size = array_size / num_threads;

    // Reset histogram for parallel computation
    free(histogram);
    histogram = calloc(histogram_bins, sizeof(int));

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
        t_data->local_histogram = calloc(histogram_bins, sizeof(int));

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
        for (int j = 0; j < histogram_bins; j++)
        {
            histogram[j] += t_data->local_histogram[j];
        }
        free(t_data->local_histogram);
        free(t_data);
    }

    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end_parallel);

    // Timer End
    printf("Parallel Sum = %.0f, time = %ld microseconds \n", sum_parallel, nano_seconds(&start_parallel, &end_parallel) / 1000);

    print_histogram(histogram, histogram_bins);

    /*free up resources properly */
    free(array);
    free(workers);
    free(histogram);
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
        t_data->local_histogram[histogram_index(array[i])]++;
    }

    printf("Thread %d sum = %f\n", my_id, t_data->sum);
    return t_data;
}