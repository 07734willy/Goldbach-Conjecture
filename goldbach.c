#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <math.h>

/*****************************************************
*                                                    *
*   Change This Directive To Match Your Cache Size   *
*                                                    *
*****************************************************/
#define BLOCK_SIZE (1 << 8)

struct thread_info {
    pthread_t thread_id;
    int  thread_num;
    int  thread_count;
    int  n;
    int  prime_count;
    int* primes;
    int* pair_counts;
};

int* sieve(int n) {
    int size = n >> 1;
    int* nonprimes = calloc(size, sizeof(int));
    int limit = (int)sqrt(n) + 1;

    int i;
    for (i = 1; i < limit; i++) {
        if (!nonprimes[i]) {
            int val = 2 * i + 1;

            int j = i + val;
            while (j < size) {
                nonprimes[j] = 1;
                j += val;
            }
        }
    }
    return nonprimes;
}

static void* decompose(void* arg) {
    struct thread_info* thread = arg;

    int i, j, jstart;
    for (jstart = thread->thread_num * BLOCK_SIZE; jstart < thread->prime_count; jstart += BLOCK_SIZE * thread->thread_count) {
        int block_end = jstart + BLOCK_SIZE;
        block_end = block_end < thread->prime_count ? block_end : thread->prime_count;
        for (i = jstart; i < thread->prime_count; i++) {
            int jend = block_end < i + 1 ? block_end : i + 1;
            int* pairs = thread->pair_counts + thread->primes[i];
            int pjmax = thread->n - thread->primes[i];
            for (j = jstart; (j < jend) && (thread->primes[j] < pjmax); j++) {
                pairs[thread->primes[j]]++;
            }
        }
    }
}

int main() {
    int n;

    printf("This code takes an even integer n greater than 2 and expresses all even numbers between 2 and n as a sum of two primes.\nEnter n: ");
    while ((scanf("%d", &n) != 1) || (n & 1)) {
        printf("You must enter an even number greater than 2.\nEnter n: ");
    }
    printf("OK, %d is a good number. Here is your decomposition: \n", n);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int* nonprimes = sieve(n);
    int prime_count = 0;

    int i;
    for (i = 0; i < n >> 1; i++) {
        if (!nonprimes[i]) { prime_count++; }
    }
    prime_count--;

    int* primes = malloc(prime_count * sizeof(int));
    int idx = 0;
    for (i = 1; i < n >> 1; i++) {
        if (!nonprimes[i]) {
            primes[idx++] = i;
        }
    }
    free(nonprimes);

    int cores = get_nprocs();
    #ifdef PHYSICAL_ONLY
    cores /= 2;
    #endif
    struct thread_info* threads = malloc(cores * sizeof(struct thread_info));

    for (i = 0; i < cores; i++) {
        threads[i].thread_num = i;
        threads[i].thread_count = cores;
        threads[i].n = n >> 1;
        threads[i].prime_count = prime_count;
        threads[i].primes = primes;
        threads[i].pair_counts = calloc((n >> 1) + 5, sizeof(int));
        if (i != 0) {
            pthread_create(&threads[i].thread_id, NULL, &decompose, &threads[i]);
        }
    }

    decompose((void *)&threads[0]);
    threads[0].pair_counts[1]++;

    void* res;
    for (i = 1; i < cores; i++) {
        pthread_join(threads[i].thread_id, &res);
    }

    int j;
    for (i = 4; i < n + 1; i += 2) {
        int count = 0;
        for (j = 0; j < cores; j++) {
            count += threads[j].pair_counts[(i >> 1) - 1];
        }
        /**************************************
        *                                     *
        *   Insert Your I/O Operations Here   *
        *                                     *
        **************************************/
        printf("%d, %d\n", i, count);
    }

    for (i = 0; i < cores; i++) {
        free(threads[i].pair_counts);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = end_time.tv_sec - start_time.tv_sec;
    elapsed += (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Time taken: %f\n", elapsed);
}
