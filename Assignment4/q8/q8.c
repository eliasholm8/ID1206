#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define FILE_NAME "file_to_map.txt"
#define FILE_SIZE 1024*1024 // 1 MiB

int main() {

    int fd = open(FILE_NAME, O_RDWR);


    pid_t pid = fork();

    char *mapped_ptr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (pid == 0) {
        printf("Child process (pid=%d); mmap address: %p \n", getpid(), mapped_ptr);

        memcpy(mapped_ptr, "01234", 5);

        char buffer[6];
        memcpy(buffer, mapped_ptr + 4096, 5);
        buffer[5] = '\0';
        
        printf("Child process (pid=%d); read from mmaped_ptr [4096]: %s\n", getpid(), buffer);

    } else {
        printf("Parent process (pid=%d); mmap address: %p \n", getpid(), mapped_ptr);

        memcpy(mapped_ptr + 4096, "56789", 5);

        
        char buffer[6];
        memcpy(buffer, mapped_ptr, 5);
        buffer[5] = '\0';
        
        printf("Parent process (pid=%d); read from mmaped_ptr [0]: %s\n", getpid(), buffer);

        wait(NULL);
        
    }

}