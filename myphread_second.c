//
// Created by JintongWang on 10/28/2022.
//

#include "mypthread.h"
#include <signal.h>
#include <sys/time.h>
#include "para.h"
#include "Queue.h"
#include "mypthread.c"

int mypthread_create(mypthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    // YOUR CODE HERE
//	int tid = pthread_create(thread, attr, function, arg);

    if (scheduler == NULL) {
        thread_number = 0;
        // create a Thread Control Block
        scheduler = initial_scheduler();
        // create and initialize the context of this thread
        Node *newNode = create_thread_node();
        while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
        add_node_into_queue(QUEUE_NUMBER, newNode);// TODO: about schedulerStyle, should be changed when changing test data
        __sync_lock_release(&modifying_queue);


        return_function = malloc(sizeof(ucontext_t));
        getcontext(return_function);
        return_function->uc_stack.ss_sp = malloc(65536);
        return_function->uc_stack.ss_size = 65536;
        makecontext(return_function, (void (*)(void)) &mypthread_exit, 1, arg);

        __sync_lock_release(&scheduler_running);
        __sync_lock_release(&modifying_queue);

        mutex_id = 0;
    }
    // set a timer to 25ms
    if(timer.it_interval.tv_usec==0){
        signal(SIGVTALRM, (void (*)(int))& swap_contexts);
        getitimer(ITIMER_VIRTUAL, &timer);
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = TIMER_PARA;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = TIMER_PARA;
        setitimer(ITIMER_VIRTUAL, &timer, NULL);
    }
    // initialize a new thread
    Node *new_thread= malloc(sizeof(Node));
    new_thread->tcb= malloc(sizeof(thread_control_block));
    new_thread->tcb->context= malloc(sizeof(ucontext_t));
    getcontext(new_thread->tcb->context);
    new_thread->tcb->context->uc_link=return_function;
    // initializes a stack for the new thread
    new_thread->tcb->context->uc_stack.ss_sp= malloc(65536);
    new_thread->tcb->context->uc_stack.ss_size=65536;
    new_thread->tcb->context->uc_stack.ss_flags=0;
    //set pid, status, return_value
    new_thread->tcb->tid=thread_number;
    new_thread->tcb->ret_val=NULL;
    new_thread->tcb->yield_purpose=0;
    *thread=thread_number;
    thread_number++;
    // change thread context to the function
    makecontext(new_thread->tcb->context, (void (*)(void ))function, 1, arg);
    //set new thread's priority
    new_thread->tcb->priority=100;

    // change the running queue
    while (__sync_lock_test_and_set(&modifying_queue,1)==1);
    // add thread into the queue
    add_node_into_queue(QUEUE_NUMBER, new_thread);
    __sync_lock_release(&modifying_queue);

    // TODO: unfinished......


    // allocate heap space for this thread's stack
    // after everything is all set, push this thread into the ready queue

//    return tid;

    return 0;
};