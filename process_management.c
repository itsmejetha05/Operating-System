#include <stdio.h>
#include <pthread.h>

int balance = 100;

int main() {
    pthread_t t1, t2, t3;
    int id1 = 1, id2 = 2, id3 = 3;

    printf("=== Synchronized Balance Update (3 threads) ===\n");
    // Thread callbacks will be added here
    
    return 0;
}