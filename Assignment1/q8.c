#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>



int main(int argc, char *argv[]) {


    long nano_seconds(struct timespec *t_start, struct timespec *t_stop) {
    return (t_stop->tv_nsec - t_start->tv_nsec) + 
    (t_stop->tv_sec - t_start->tv_sec)*1000000000;
    }

    struct timespec start, end;

    
    int N = atoi(argv[1]);
    
    pid_t child1, child2;
    
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    
    double *numbers = malloc(N*sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < N; i++) {
        numbers[i] = (double)rand() / RAND_MAX;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    child1 = fork();

    if (child1 == 0) {
        double sum;

        for (int i = 0; i < N/2; i++) {
            sum += numbers[i];
        }
        close(pipe1[0]);
        write(pipe1[1], &sum, sizeof(sum));
        exit(EXIT_SUCCESS);
    }
    
    child2 = fork();

    if (child2 == 0) {
        double sum;

        for (int i = N/2; i < N; i++) {
            sum += numbers[i];
        }
        close(pipe2[0]);
        write(pipe2[1], &sum, sizeof(sum));
        exit(EXIT_SUCCESS);
    }


    double sum1, sum2;

    
    close(pipe1[1]);
    close(pipe2[1]);
    read(pipe1[0], &sum1, sizeof(sum1));
    read(pipe2[0], &sum2, sizeof(sum2));
    

    
    printf("%f\n", sum1+sum2);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("%d\n",nano_seconds(&start, &end)/1000);

}