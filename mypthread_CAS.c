//
// Created by 方首智 on 11/2/22.
//
//
// Created by 方首智 on 11/2/22.
//
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "mypthread.h"

#define DEFAULT_THREAD_NUM 2
#define VECTOR_SIZE 3000000
#define MOD 100000

/* Global variables */
pthread_mutex_t   mutex;
int thread_num;
int** counter;
pthread_t *thread;
int r[VECTOR_SIZE];
int s[VECTOR_SIZE];
int res = 0;

void cas(void *arg){
    for (int i = 0; i < VECTOR_SIZE; i ++) {
        pthread_mutex_lock(&mutex);
        if(counter[0][i]<counter[1][i]){
            res ++;
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int verify() {
    int i = 0;
    res = 0;
    for (; i < VECTOR_SIZE; i ++) {
        if(counter[0][i]<counter[1][i]){
            res ++;
        }
    }
    printf("verified res is: %d\n", res);
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    int i = 0;

//    printf("%d", argc);
    if (argc == 1) {
        thread_num = DEFAULT_THREAD_NUM;
    } else {
        if (argv[1] < 1) {
            printf("enter a valid thread number\n");
            return 0;
        } else {
            thread_num = atoi(argv[1]);
        }
    }

    // initialize counter
    counter = (int**)malloc(thread_num*sizeof(int*));
    for (i = 0; i < thread_num; ++i)
        counter[i] = (int*) malloc(VECTOR_SIZE*sizeof (int));

    // initialize pthread_t
    thread = (pthread_t*)malloc(thread_num*sizeof(pthread_t));

    // initialize data array
    for (i = 0; i < VECTOR_SIZE; ++i) {
        counter[0][i] = rand() % MOD;
        counter[1][i] = rand() % MOD;
//        printf("x=%d, y=%d \n", counter[0][i], counter[1][i]);
    }


    pthread_mutex_init(&mutex, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < thread_num; ++i)
        pthread_create(&thread[i], NULL, cas, NULL);
//    pthread_create(&thread[0], NULL, &sort, &r);
//    pthread_create(&thread[1], NULL, &sort, &s);

    for (i = 0; i < thread_num; ++i)
        pthread_join(thread[i], NULL);

    clock_gettime(CLOCK_REALTIME, &end);
    printf("running time: %lu micro-seconds\n",
           (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
    printf("res is: %d\n", res/2);

    pthread_mutex_destroy(&mutex);
    verify();

    // Free memory on Heap
    free(thread);
    free(counter);
    return 0;
}
