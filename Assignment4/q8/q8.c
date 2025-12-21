#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define FILE_NAME "file_to_map.txt"
#define FILE_SIZE 1024*1024 // 1 MiB

int main() {

    int fd = open(FILE_NAME, O_RDWR);


    pid_t pid = fork();

    char *mapped_ptr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (pid == 0) {
        printf("Child process (pid=%d); mmap address: %p \n", getpid(), mapped_ptr);
    } else {
        printf("Parent process (pid=%d); mmap address: %p \n", getpid(), mapped_ptr);
    }

}