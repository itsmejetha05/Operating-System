#include <stdio.h>

#define MEM_SIZE 12          // total memory size (units)
#define PAGE_SIZE 4           // configurable page size
#define FRAMES (MEM_SIZE / PAGE_SIZE)

int refs[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};   // page reference stream
int n = sizeof(refs) / sizeof(refs[0]);

int main() {
    printf("Page Size=%d, Frames=%d\n\n", PAGE_SIZE, FRAMES);
    return 0;
}