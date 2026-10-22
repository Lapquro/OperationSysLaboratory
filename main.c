#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define RUNS 5

int *a;
int size;
int threads = 4;
int wait_sec = 0;
pthread_barrier_t barrier;

double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

void layer(int p, int k, int from, int to)
{
    int r = k % p;
    for (int x = from; x < to; x++) {
        if (x < r)
            continue;
        if ((x - r) % (2 * k) >= k)
            continue;
        if (x / (2 * p) != (x + k) / (2 * p))
            continue;
        if (a[x] > a[x + k]) {
            int tmp = a[x];
            a[x] = a[x + k];
            a[x + k] = tmp;
        }
    }
}

void sort_sequential(void)
{
    for (int p = 1; p < size; p *= 2)
        for (int k = p; k >= 1; k /= 2)
            layer(p, k, 0, size - k);
}

void *worker(void *arg)
{
    int id = (int)(long)arg;
    if (wait_sec > 0)
        sleep(wait_sec);
    for (int p = 1; p < size; p *= 2) {
        for (int k = p; k >= 1; k /= 2) {
            long long len = size - k;
            int from = (int)(len * id / threads);
            int to = (int)(len * (id + 1) / threads);
            layer(p, k, from, to);
            pthread_barrier_wait(&barrier);
        }
    }
    return NULL;
}

void sort_parallel(void)
{
    pthread_t *th = malloc(sizeof(pthread_t) * threads);
    pthread_barrier_init(&barrier, NULL, threads);
    for (int i = 0; i < threads; i++)
        pthread_create(&th[i], NULL, worker, (void *)(long)i);
    for (int i = 0; i < threads; i++)
        pthread_join(th[i], NULL);
    pthread_barrier_destroy(&barrier);
    free(th);
}

int main(int argc, char **argv)
{
    int n = 1000000;

    for (int i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "-t") == 0)
            threads = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-n") == 0)
            n = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-w") == 0)
            wait_sec = atoi(argv[i + 1]);
    }
    if (threads < 1 || n < 2) {
        printf("usage: %s -t threads -n size [-w seconds]\n", argv[0]);
        return 1;
    }

    size = 2;
    while (size < n)
        size *= 2;

    int *orig = malloc(sizeof(int) * size);
    int *result = malloc(sizeof(int) * size);
    a = malloc(sizeof(int) * size);

    srand(1);
    for (int i = 0; i < size; i++)
        orig[i] = i < n ? rand() : INT_MAX;

    double seq_total = 0;
    for (int r = 0; r < RUNS; r++) {
        memcpy(a, orig, sizeof(int) * size);
        double start = now_ms();
        sort_sequential();
        seq_total += now_ms() - start;
    }
    memcpy(result, a, sizeof(int) * size);
    double seq = seq_total / RUNS;

    if (wait_sec > 0) {
        printf("pid %d, waiting %d s\n", getpid(), wait_sec);
        fflush(stdout);
        memcpy(a, orig, sizeof(int) * size);
        sort_parallel();
        wait_sec = 0;
    }

    double par_total = 0;
    for (int r = 0; r < RUNS; r++) {
        memcpy(a, orig, sizeof(int) * size);
        double start = now_ms();
        sort_parallel();
        par_total += now_ms() - start;
    }
    double par = par_total / RUNS;

    for (int i = 1; i < size; i++) {
        if (a[i - 1] > a[i]) {
            printf("ERROR: array is not sorted\n");
            return 2;
        }
    }
    if (memcmp(a, result, sizeof(int) * size) != 0) {
        printf("ERROR: parallel result differs from sequential\n");
        return 2;
    }

    double speedup = seq / par;
    printf("n = %d (padded to %d)\n", n, size);
    printf("sequential: %.2f ms\n", seq);
    printf("threads: %d, time: %.2f ms, speedup: %.2f, efficiency: %.3f\n",
           threads, par, speedup, speedup / threads);
    return 0;
}
