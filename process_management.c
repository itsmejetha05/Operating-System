#include <stdio.h>
#include <pthread.h>

int balance = 100;
pthread_mutex_t lock;

void *updateBalance(void *arg) {
    pthread_mutex_lock(&lock);
    balance += 10;
    printf("Thread %d updated balance to %d\n", *(int *)arg, balance);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    int id1 = 1, id2 = 2, id3 = 3;
    pthread_mutex_init(&lock, NULL);

    printf("=== Synchronized Balance Update (3 threads) ===\n");
    pthread_create(&t1, NULL, updateBalance, &id1);
    pthread_create(&t2, NULL, updateBalance, &id2);
    pthread_create(&t3, NULL, updateBalance, &id3);
    pthread_join(t1, NULL); pthread_join(t2, NULL); pthread_join(t3, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}