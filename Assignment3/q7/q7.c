#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

long nano_seconds(struct timespec *ts_start, struct timespec *ts_stop)
{
    return (ts_stop->tv_nsec - ts_start->tv_nsec) + (ts_stop->tv_sec - ts_start->tv_sec) * 1000000000;
}

int main(int argc, char** argv){

  if (argc != 3) {
    printf("Usage: %s <num_pages> <use_huge_pages (0 or 1)>\n", argv[0]);
    exit(1);
  }
  
  int num_pages = atoi(argv[1]);
  int page_size = getpagesize();

  int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;

  bool huge_pages = atoi(argv[2]) != 0;

  if (huge_pages) {
      mmap_flags |= MAP_HUGETLB;
  }
 
  printf("Allocating %d pages of %d bytes \n", num_pages, page_size);

  size_t bytes_to_write = (size_t)num_pages * (size_t)page_size;

  // Allocate virtual memory.
  char *addr = mmap(NULL, bytes_to_write, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);
  
  if (addr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  struct timespec start_ts, stop_ts;

  // @Add the start of Timer here
  clock_gettime(CLOCK_MONOTONIC, &start_ts);

  //the code below updates the pages
  char c = 'a';
  for(int i=0; i<num_pages; i++){
    addr[i*page_size] = c;
    c ++;
  }

  // @Add the end of Timer here
  clock_gettime(CLOCK_MONOTONIC, &stop_ts);

  // @Add printout of elapsed time in cycles
  long elapsed_nanoseconds = nano_seconds(&start_ts, &stop_ts);
  printf("Time taken to write to %d pages: %ld nanoseconds\n", num_pages, elapsed_nanoseconds);  
  
  for(int i=0; (i<num_pages && i<16); i++){
    printf("%c ", addr[i*page_size]);
  }
  printf("\n");
  
  munmap(addr, page_size*num_pages);
  
}
