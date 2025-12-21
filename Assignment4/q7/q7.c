#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    off_t offset;
    size_t bytes;
} request;

typedef struct
{
    int thread_id;
    int start_index;
    int end_index;
    pthread_t thread;
    request *req_list;
} thread_data;

int num_of_bytes;
int num_of_threads;

int fd;
char *write_buffer;
char *read_buffer;

// @create two lists of 100 requests in the format of [offset, bytes]
request list1[100];
request list2[100];

// Helper function to calculate elapsed time in nanoseconds
long nano_seconds(struct timespec *ts_start, struct timespec *ts_stop)
{
    return (ts_stop->tv_nsec - ts_start->tv_nsec) + (ts_stop->tv_sec - ts_start->tv_sec) * 1000000000;
}

void *reader_thread_func(void *thread_data_arg)
{
    thread_data *t_data = (thread_data *)thread_data_arg;
    int thread_id = t_data->thread_id;
    int start_index = t_data->start_index;
    int end_index = t_data->end_index;
    request *req_list = t_data->req_list;

    // @Add code for reader threads
    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: read bytes_i from offset_i
    for (int i = start_index; i < end_index; i++)
    {
        pread(fd, read_buffer + req_list[i].offset, req_list[i].bytes, req_list[i].offset);
    }

    pthread_exit(0);
}

void *writer_thread_func(void *thread_data_arg)
{
    thread_data *t_data = (thread_data *)thread_data_arg;
    int thread_id = t_data->thread_id;
    int start_index = t_data->start_index;
    int end_index = t_data->end_index;
    request *req_list = t_data->req_list;

    // @Add code for writer threads
    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: write bytes_i to offset_i
    for (int i = start_index; i < end_index; i++)
    {
        pwrite(fd, write_buffer + req_list[i].offset, req_list[i].bytes, req_list[i].offset);
    }

    pthread_exit(0);
}

void run_experiment(int list_prefix, request *req_list)
{
    // @create a file for saving the data
    fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0)
    {
        printf("Error: Could not create file.\n");
        exit(1);
    }

    if (ftruncate(fd, num_of_bytes) != 0)
    {
        printf("Error: Could not set file size.\n");
        close(fd);
        exit(1);
    }

    // @start timing
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /* Create writer workers and pass in their portion of list1 */
    thread_data writers[num_of_threads];
    for (int i = 0; i < num_of_threads; i++)
    {
        writers[i].thread_id = i;
        writers[i].start_index = (i * 100) / num_of_threads;
        writers[i].end_index = ((i + 1) * 100) / num_of_threads;
        writers[i].req_list = req_list;
        pthread_create(&writers[i].thread, NULL, writer_thread_func, &writers[i]);
    }

    /* Wait for all writers to finish */
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_join(writers[i].thread, NULL);
    }

    // @close the file
    fsync(fd);
    close(fd);

    // @end timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    //@Print out the write bandwidth
    size_t data_written;
    if (list_prefix == 1)
        data_written = 16384 * 100;
    else
        data_written = 128 * 100;

    long elapsed_ns = nano_seconds(&start_time, &end_time);
    double elapsed_s = elapsed_ns / 1e9;
    double write_bandwidth = (data_written / (1024.0 * 1024.0)) / elapsed_s;
    printf("List %d: Write %zu bytes, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", list_prefix, data_written, num_of_threads, elapsed_s, write_bandwidth);

    // @reopen the file
    fd = open("output.txt", O_RDONLY);
    if (fd < 0)
    {
        printf("Error: Could not open file for reading.\n");
        exit(1);
    }

    // @start timing
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /* Create reader workers and pass in their portion of list1 */
    thread_data readers[num_of_threads];
    for (int i = 0; i < num_of_threads; i++)
    {
        readers[i].thread_id = i;
        readers[i].start_index = (i * 100) / num_of_threads;
        readers[i].end_index = ((i + 1) * 100) / num_of_threads;
        readers[i].req_list = req_list;
        pthread_create(&readers[i].thread, NULL, reader_thread_func, &readers[i]);
    }

    /* Wait for all reader to finish */
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_join(readers[i].thread, NULL);
    }

    // @close the file
    close(fd);

    // @end timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    //@Print out the read bandwidth
    size_t data_read;
    if (list_prefix == 1)
        data_read = 16384 * 100;
    else
        data_read = 128 * 100;
    elapsed_ns = nano_seconds(&start_time, &end_time);
    elapsed_s = elapsed_ns / 1e9;
    double read_bandwidth = (data_read / (1024.0 * 1024.0)) / elapsed_s;
    printf("List %d: Read %zu bytes, use %d threads, elapsed time %f s, read bandwidth: %f MB/s \n", list_prefix, data_read, num_of_threads, elapsed_s, read_bandwidth);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <num_of_bytes> <num_of_threads>\n", argv[0]);
        exit(1);
    }

    num_of_bytes = atoi(argv[1]);   // number of bytes
    num_of_threads = atoi(argv[2]); // number of threads

    if (num_of_bytes <= 0 || num_of_threads <= 0)
    {
        printf("Error: num_of_bytes and num_of_threads must be positive.\n");
        exit(1);
    }

    // INitalize random
    srand(time(NULL));

    // @allocate a buffer and initialize it
    write_buffer = (char *)malloc(num_of_bytes);
    read_buffer = (char *)malloc(num_of_bytes);

    if (write_buffer == NULL || read_buffer == NULL)
    {
        printf("Error: Unable to allocate memory for buffers.\n");
        exit(1);
    }

    char charData = 'a';
    for (int i = 0; i < num_of_bytes; i++)
    {
        read_buffer[i] = 0;
        write_buffer[i] = charData;
        charData = (charData == 'z') ? 'a' : (charData + 1);
    }

    // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
    // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
    // @ensure no overlapping among these requests.
    for (int i = 0; i < 100; i++)
    {
        list1[i].offset = i * 16384;
        list1[i].bytes = 16384;
    }

    // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
    // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
    // @ensure no overlapping among these requests.
    if (num_of_bytes < 4096 * 100)
    {
        printf("Error: Number of bytes is too small for list 2.\n");
        exit(1);
    }

    int num_of_pages = num_of_bytes / 4096;
    int used_pages[num_of_pages];
    for (int i = 0; i < num_of_pages; i++)
    {
        used_pages[i] = 0;
    }

    for (int i = 0; i < 100; i++)
    {
        int random_page;
        do
        {
            random_page = (rand() % num_of_pages);
        } while (used_pages[random_page] == 1);

        used_pages[random_page] = 1;

        list2[i].offset = random_page * 4096;
        list2[i].bytes = 128;
    }

    // Run experiment
    run_experiment(1, list1);
    run_experiment(2, list2);

    // @Repeat the write and read test now using List2

    /*free up resources properly */
    free(write_buffer);
    free(read_buffer);

    return 0;
}
