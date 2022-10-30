#include <stdio.h>
#include "test.h"


void *print_tid(void *arg){
    printf("thread working");
}

int main(){
    pthread_t *new_thread;
    int tid=0;
    tid= pthread_create(&new_thread,NULL, &print_tid, NULL);
//    int a = mypthread_yield();
//    while (1);
    return 0;

}

