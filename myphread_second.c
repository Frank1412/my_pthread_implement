////
//// Created by JintongWang on 10/28/2022.
////
//
//#include "mypthread.h"
//#include <signal.h>
//#include <sys/time.h>
//#include "para.h"
//#include "Queue.h"
//#include "mypthread.c"
//
//int mypthread_create(mypthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
//    // YOUR CODE HERE
////	int tid = pthread_create(thread, attr, function, arg);
//
//    if (scheduler == NULL) {
//        thread_number = 0;
//        // create a Thread Control Block
//        scheduler = initial_scheduler();
//        // create and initialize the context of this thread
//        Node *newNode = create_thread_node();
//        while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
//        add_node_into_queue(QUEUE_NUMBER, newNode);// TODO: about schedulerStyle, should be changed when changing test data
//        __sync_lock_release(&modifying_queue);
//
//
//        return_function = malloc(sizeof(ucontext_t));
//        getcontext(return_function);
//        return_function->uc_stack.ss_sp = malloc(65536);
//        return_function->uc_stack.ss_size = 65536;
//        makecontext(return_function, (void (*)(void)) &mypthread_exit, 1, arg);
//
//        __sync_lock_release(&scheduler_running);
//        __sync_lock_release(&modifying_queue);
//
//        mutex_id = 0;
//    }
//    // set a timer to 25ms
//    if(timer.it_interval.tv_usec==0){
//        signal(SIGVTALRM, (void (*)(int))& swap_contexts);
//        getitimer(ITIMER_VIRTUAL, &timer);
//        timer.it_value.tv_sec = 0;
//        timer.it_value.tv_usec = TIMER_PARA;
//        timer.it_interval.tv_sec = 0;
//        timer.it_interval.tv_usec = TIMER_PARA;
//        setitimer(ITIMER_VIRTUAL, &timer, NULL);
//    }
//    // initialize a new thread
//    Node *new_thread= malloc(sizeof(Node));
//    new_thread->tcb= malloc(sizeof(thread_control_block));
//    new_thread->tcb->context= malloc(sizeof(ucontext_t));
//    getcontext(new_thread->tcb->context);
//    new_thread->tcb->context->uc_link=return_function;
//    // initializes a stack for the new thread
//    new_thread->tcb->context->uc_stack.ss_sp= malloc(65536);
//    new_thread->tcb->context->uc_stack.ss_size=65536;
//    new_thread->tcb->context->uc_stack.ss_flags=0;
//    //set pid, status, return_value
//    new_thread->tcb->tid=thread_number;
//    new_thread->tcb->ret_val=NULL;
//    new_thread->tcb->yield_purpose=0;
//    *thread=thread_number;
//    thread_number++;
//    // change thread context to the function
//    makecontext(new_thread->tcb->context, (void (*)(void ))function, 1, arg);
//    //set new thread's priority
//    new_thread->tcb->priority=100;
//
//    // change the running queue
//    while (__sync_lock_test_and_set(&modifying_queue,1)==1);
//    // add thread into the queue
//    add_node_into_queue(QUEUE_NUMBER, new_thread);
//    __sync_lock_release(&modifying_queue);
//
//
//
//    // allocate heap space for this thread's stack
//    // after everything is all set, push this thread into the ready queue
//
////    return tid;
//
//    return 0;
//};
//
//int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
//    // YOUR CODE HERE
//    // set 1 means it has not been deleted
//    mutex->initialized=1;
//    // open mutex lock
//    __sync_lock_release(&(mutex->mutex_lock));
//    // initial mutex id
//    mutex->mid=mutex_id;
//    mutex_id++;
//
//    //initialize data structures for this mutex
//
//    return 0;
//};
//int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
//    // YOUR CODE HERE
//    if(mutex->initialized!=1){
//        return -1;
//    }
//
//    // use the built-in test-and-set atomic function to test the mutex
//    if(__sync_lock_test_and_set(&(mutex->mutex_lock),1)==0){
//        // if the mutex is acquired successfully, return
//        mutex->tid=get_current_thread()->tcb->tid;
//        return 0;
//    }
//    // if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
//    // initialize a new mutex waiting queue node
//    mutex_waiting_queue_node *wait_node= malloc(sizeof(mutex_waiting_queue_node));
//    Node *current_thread=get_current_thread();
//    wait_node->thread=current_thread->tcb;
//    current_thread->tcb->yield_purpose=2;
//    wait_node->mutex_lock=mutex->mid;
//    wait_node->next=NULL;
//    // add into waiting queue
//    add_to_mutex_waiting_queue(wait_node);
//    mypthread_yield();
//    return 0;
//};
//
//int add_to_mutex_waiting_queue(mutex_waiting_queue_node* node){
//    if(scheduler->mutex_waiting_queue==NULL){
//        scheduler->mutex_waiting_queue=node;
//        return 0;
//    }
//    mutex_waiting_queue_node *ptr=scheduler->mutex_waiting_queue;
//    while (ptr->next!=NULL){
//        ptr=ptr->next;
//    }
//    ptr->next=node;
//    return 0;
//}
//
//static void sched_RR() {
//    // YOUR CODE HERE
////    Node *ptr=scheduler->round_robin_queue_T1->head->next;
//    Queue *current_running_queue=scheduler->round_robin_queue_T1;
//    Node *ptr= removeFront(current_running_queue);
//    age();
//    thread_handle(ptr);
//    // Your own implementation of RR
//    // (feel free to modify arguments and return types)
//
//    return;
//}
//static void sched_PSJF() {
//    // YOUR CODE HERE
//    Queue *current_running_queue=scheduler->round_robin_queue_T1;
//    Node *ptr=get_most_waiting_time_node(current_running_queue);
//    removeNode(current_running_queue, ptr);
//    ptr->tcb->waiting_time=0;
//    add_waiting_time();
//    age();
//    thread_handle(ptr);
//    // Your own implementation of PSJF (STCF)
//    // (feel free to modify arguments and return types)
//
//    return;
//}
//
//Node *get_most_waiting_time_node(Queue *queue){
//    Node *most_waiting_time_node;
//    int most_waiting_time=0;
//    Node *ptr=queue->head->next;
//    while (ptr->next!=NULL){
//        if (most_waiting_time<ptr->tcb->waiting_time){
//            most_waiting_time=ptr->tcb->waiting_time;
//            most_waiting_time_node=ptr;
//        }
//        ptr=ptr->next;
//    }
//    return most_waiting_time_node;
//}
//
//void add_waiting_time(){
//    int iteration_time=timer.it_interval.tv_usec;
//    Queue *current_running_queue=scheduler->round_robin_queue_T1;
//    Node *ptr=current_running_queue->head->next;
//    while (ptr->next!=NULL){
//        ptr->tcb->waiting_time+=iteration_time;
//    }
//    return;
//}
//
////int add_node_into_queue(int running_queue_number, Node *threadNode) {
////
////    if (running_queue_number == 0) {// round-robin with 25ms
////        addFront(scheduler->round_robin_queue_T1, threadNode);
////    } else if (running_queue_number == 1) {// round-robin with 50ms
////        addFront(scheduler->round_robin_queue_T1, threadNode);
////    } else if (running_queue_number == 2) {// SJF
////        addFront(scheduler->round_robin_queue_T1, threadNode);
////    } else {// MLFQ
////
////
////    }
////    return 0;
////}