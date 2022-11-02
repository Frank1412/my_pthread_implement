//
// Created by 方首智 on 11/2/22.
//
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
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

int cmp ( const void *a , const void *b )
{
    return *(int *)a - *(int *)b;
}

void mysort(void *arg){
    qsort(arg, VECTOR_SIZE, sizeof(int), cmp);
    pthread_exit(NULL);
}

int verify() {
    int i = 0;
    res = counter[0][0]==counter[1][0]?1:0;
    for (i = 1; i < VECTOR_SIZE; i += 1) {
        if(counter[0][i]==counter[1][i] && counter[0][i]>=counter[0][i-1]) res ++;
    }
    printf("verified res is: %d\n", res);
}

int groundTruth() {
    int i = 0;
    int ret = 1;
    for (i = 1; i < VECTOR_SIZE; i += 1) {
        if(counter[0][i]>=counter[0][i-1]) ret ++;
    }
    return ret;
}

int main(int argc, char **argv) {

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
        int x = rand() % MOD;
        counter[0][i] = x;
        counter[1][i] = x;
//        printf("x = %d\n", x);
    }


    pthread_mutex_init(&mutex, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    for (i = 0; i < thread_num; ++i)
        pthread_create(&thread[i], NULL, &mysort, counter[i]);
//    pthread_create(&thread[0], NULL, &sort, &r);
//    pthread_create(&thread[1], NULL, &sort, &s);

    for (i = 0; i < thread_num; ++i)
        pthread_join(thread[i], NULL);

    clock_gettime(CLOCK_REALTIME, &end);
    printf("running time: %lu micro-seconds\n",
           (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
    printf("res is: %d\n", groundTruth());

    pthread_mutex_destroy(&mutex);
    verify();

    // Free memory on Heap
    free(thread);
    free(counter);
    return 0;
}
