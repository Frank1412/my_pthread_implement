#include <stdio.h>
#include "test.h"


void *print_tid(void *arg){
    printf("thread working");
}

int main(){
    printf("0\n");
    pthread_t *new_thread;
    printf("1\n");
    int i=1;
    int tid=0;
    tid= pthread_create(&new_thread,NULL, &print_tid, NULL);
//    int a = mypthread_yield();
    return 0;

}

