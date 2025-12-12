#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]) {
    
    if (argc <= 1) exit(1);
    const int size = atoi(argv[1]);

    const int page_size = getpagesize();

    char *mem = malloc(size * page_size);
    if (mem == NULL) exit(1);
    
    memset(mem, 0, size*page_size);


    free(mem);

}