#include <stdio.h>
#include <pthread.h>

int balance = 100;
pthread_mutex_t lock, lockA, lockB;

// Synchronization mechanism: mutex protects the shared balance (handles race condition)
void *updateBalance(void *arg) {
    pthread_mutex_lock(&lock);
    balance += 10;
    printf("Thread %d updated balance to %d\n", *(int *)arg, balance);
    pthread_mutex_unlock(&lock);
    return NULL;
}

// Deadlock prevention: every thread locks A before B, so no circular wait can form
void *safeThread(void *arg) {
    pthread_mutex_lock(&lockA);
    pthread_mutex_lock(&lockB);
    printf("Thread %d acquired both locks safely\n", *(int *)arg);
    pthread_mutex_unlock(&lockB);
    pthread_mutex_unlock(&lockA);
    return NULL;
}

// Round-robin scheduler simulation using arrays
void roundRobin() {
    int burst[3] = {6, 4, 8}, rem[3] = {6, 4, 8}, q = 3, t = 0, done = 0;
    printf("\nRound-Robin Scheduler (quantum=%d)\n", q);
    while (done < 3) {
        for (int i = 0; i < 3; i++) {
            if (rem[i] > 0) {
                int s = rem[i] < q ? rem[i] : q;
                printf("P%d: t=%d-%d\n", i + 1, t, t + s);
                t += s;
                rem[i] -= s;
                if (rem[i] == 0) { done++; printf("P%d completed\n", i + 1); }
            }
        }
    }
}

int main() {
    pthread_t t1, t2, t3;
    int id1 = 1, id2 = 2, id3 = 3;
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lockA, NULL);
    pthread_mutex_init(&lockB, NULL);

    printf("=== Synchronized Balance Update (3 threads) ===\n");
    pthread_create(&t1, NULL, updateBalance, &id1);
    pthread_create(&t2, NULL, updateBalance, &id2);
    pthread_create(&t3, NULL, updateBalance, &id3);
    pthread_join(t1, NULL); pthread_join(t2, NULL); pthread_join(t3, NULL);

    printf("\n=== Deadlock Prevention (consistent lock order) ===\n");
    pthread_create(&t1, NULL, safeThread, &id1);
    pthread_create(&t2, NULL, safeThread, &id2);
    pthread_join(t1, NULL); pthread_join(t2, NULL);

    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lockA);
    pthread_mutex_destroy(&lockB);

    roundRobin();
    return 0;
}