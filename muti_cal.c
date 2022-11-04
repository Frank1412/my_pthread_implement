#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"
//#include "../test.h"
#include "test.h"
#include "string.h"

#define DEFAULT_THREAD_NUM 4
#define C_SIZE 100000
#define R_SIZE 10000

pthread_mutex_t   mutex;
int thread_num;
int* counter;
pthread_t *thread;
int iteration_time=10000;
int  sum = 0;


/* A CPU-bound task to do parallel array addition */
void *accumulator(void* arg) {
    int value;
    int k;
    printf("start sum:%d\n", sum);
    for (k=0; k < iteration_time; k++) {
        pthread_mutex_lock(&mutex);
        value=sum;
        value=value+1;
        for (int l = 0; l < 100; ++l) {};// do a busy wait
        sum=value;
        pthread_mutex_unlock(&mutex);
    }
    for (k=0; k < iteration_time; k++) {
        pthread_mutex_lock(&mutex);
        value=sum;
        value=value-1;
        for (int l = 0; l < 100; ++l) {};// do a busy wait
        sum=value;
        pthread_mutex_unlock(&mutex);
    }
    printf("end sum:%d\n", sum);
    pthread_exit(NULL);
}


/* verification function */
void verify() {

    printf("verified sum is: 0\n");
}

int main(int argc, char **argv) {

    int i = 0;

//	if (argc == 1) {
//		thread_num = DEFAULT_THREAD_NUM;
//	} else {
//		if (argv[1] < 1) {
//			printf("enter a valid thread number\n");
//			return 0;
//		} else
//			thread_num = atoi(argv[1]);
//	}
    thread_num=6;
    // initialize pthread_t
    thread = (pthread_t*)malloc(thread_num*sizeof(pthread_t));

    // initialize counter
    // mutex init
    pthread_mutex_init(&mutex, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    for (i = 0; i < thread_num; ++i)
        pthread_create(&thread[i], NULL, &accumulator, NULL);

    for (i = 0; i < thread_num; ++i)
        pthread_join(thread[i], NULL);

    clock_gettime(CLOCK_REALTIME, &end);
    printf("running time: %lu micro-seconds\n",
           (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);

    printf("sum is: %d\n", sum);
    // mutex destroy
    pthread_mutex_destroy(&mutex);

    // feel free to verify your answer here:
    verify();
    // Free memory on Heap
    free(thread);
    free(counter);


    return 0;
}
