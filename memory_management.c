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
// FIFO page replacement: evicts the oldest-loaded page
void fifo() {
    int frame[FRAMES], faults = 0, front = 0;
    for (int i = 0; i < FRAMES; i++) frame[i] = -1;

    for (int i = 0; i < n; i++) {
        int page = refs[i], hit = 0;
        for (int j = 0; j < FRAMES; j++) if (frame[j] == page) hit = 1;
        if (!hit) {
            frame[front] = page;          // replace oldest frame
            front = (front + 1) % FRAMES;
            faults++;
        }
        printf("Ref %d -> %s | Frames: ", page, hit ? "Hit " : "Fault");
        for (int j = 0; j < FRAMES; j++) printf("%d ", frame[j]);
        printf("\n");
    }
    printf("FIFO: Faults=%d Hits=%d HitRatio=%.2f\n\n", faults, n - faults, (float)(n - faults) / n);
}

int main() {
    printf("Page Size=%d, Frames=%d\n\n", PAGE_SIZE, FRAMES);
    printf("=== FIFO Page Replacement ===\n");
    fifo();
    return 0;
}