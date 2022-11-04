#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "string.h"

#include <pthread.h>
#include "../mypthread.h"
//#include "test.h"
#define DEFAULT_THREAD_NUM 2
#define RAM_SIZE 160
#define RECORD_NUM 10
#define RECORD_SIZE 4194304

/* Global variables */
pthread_mutex_t   mutex;
int thread_num;
int* counter;
pthread_t *thread;
int *mem = NULL;
int sum = 0;
int itr = RECORD_SIZE / 16;
int iterate_number=1000;

void *external_calculate(void* arg) {

    char path[50] = "./record_file/1";
    strcpy(path, "/common/home/jw1419/cs518/pro1/record_file/1");


    for (int i = 0; i < iterate_number; i++) {
        pthread_mutex_lock(&mutex);
        FILE *f;
        f = fopen(path, "a");
        if (!f) {
            printf("failed to open file %s\n", path);
            exit(0);
        }
        fputs("1", f);
        fclose(f);
        pthread_mutex_unlock(&mutex);
    }
	pthread_exit(NULL);
}

void init_file(){
    char path[50] = "./record_file/1";
    strcpy(path, "/common/home/jw1419/cs518/pro1/record_file/1");
    FILE *f;
    f = fopen(path, "w");
    if (!f) {
        printf("failed to open file %s\n", path);
        exit(0);
    }
    fclose(f);
    printf("finish initializing");
}

void verify() {


    printf("verified sum is: %d\n", thread_num*iterate_number);


}


void sig_handler(int signum) {
	printf("%d\n", signum);
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

    init_file();

	pthread_mutex_init(&mutex, NULL);

	struct timespec start, end;
        clock_gettime(CLOCK_REALTIME, &start);
 
	for (i = 0; i < thread_num; ++i)
		pthread_create(&thread[i], NULL, external_calculate, &counter[i]);
	
	signal(SIGABRT, sig_handler);
	signal(SIGSEGV, sig_handler);

	for (i = 0; i < thread_num; ++i)
		pthread_join(thread[i], NULL);

	clock_gettime(CLOCK_REALTIME, &end);
        printf("running time: %lu micro-seconds\n", 
	       (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);


    char path[50] = "./record_file/1";
    strcpy(path, "/common/home/jw1419/cs518/pro1/record_file/1");
    FILE *f;
    f = fopen(path, "r");
    if (!f) {
        printf("failed to open file %s\n", path);
        exit(0);
    }
    while (fgetc(f)!=EOF){
        sum++;
    }
    fclose(f);

	printf("sum is: %d\n", sum);
	pthread_mutex_destroy(&mutex);

	// feel free to verify your answer here:
//    printf("start");
	verify();
    printf("end");
//	free(mem);
//	free(thread);
//	free(counter);

	return 0;
}