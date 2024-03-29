//
// Created by JintongWang on 10/29/2022.
//

#include "mypthread.h"
// File:	mypthread.c


// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

// global variables
Scheduler *scheduler;
struct itimerval timer;
int scheduler_running; // binary semaphoreS
int modifying_queue; // binary semaphore
ucontext_t *return_function;
mypthread_t thread_number;
uint mutex_id;
int TIMER_PARA=250000;
int QUEUE_NUMBER=3;
int TIMER_PARA2=500000;



//
//void print_alart(int a){
//    i++;
//    printf("ALARM!!!");
//    return;
//}

/* create a new thread */
int mypthread_create(mypthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    // YOUR CODE HERE
//    while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
    if (scheduler == NULL) {
//        printf("scheduler is NULL");
        thread_number = 0;
        // create a Thread Control Block
        scheduler = initial_scheduler();
        // create and initialize the context of this thread
        Node *main_thread = create_thread_node();
        scheduler->current_thread=main_thread;
        while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
        add_to_run_queue(1,main_thread);
//        __sync_lock_release(&modifying_queue);


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
    if (timer.it_interval.tv_usec == 0) {
//        printf("timer ininininin \n");
        signal(SIGVTALRM, (void (*)(int)) &swap_context);
//        getitimer(ITIMER_VIRTUAL, &timer);
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = TIMER_PARA;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = TIMER_PARA;
//        signal(SIGVTALRM,print_alart);
//        timer.it_value.tv_sec = 0;
//        timer.it_value.tv_usec = 1;
//        timer.it_interval.tv_sec = 0;
//        timer.it_interval.tv_usec = 1;
        setitimer(ITIMER_VIRTUAL, &timer, NULL);

    }
//     initialize a new thread
    Node *new_thread = malloc(sizeof(Node));
    new_thread->tcb = malloc(sizeof(thread_control_block));
    new_thread->tcb->context = malloc(sizeof(ucontext_t));
    getcontext(new_thread->tcb->context);
    new_thread->tcb->context->uc_link = return_function;
    // initializes a stack for the new thread
    new_thread->tcb->context->uc_stack.ss_sp = malloc(65536);
    new_thread->tcb->context->uc_stack.ss_size = 65536;
    new_thread->tcb->context->uc_stack.ss_flags = 0;
    //set pid, status, return_value
    new_thread->tcb->tid = thread_number;
    new_thread->tcb->ret_val = NULL;
    new_thread->tcb->yield_purpose = 0;
    *thread = thread_number;
    thread_number++;
    // change thread context to the function
    makecontext(new_thread->tcb->context, (void (*)(void)) function, 1, arg);
    //set new thread's priority
    new_thread->tcb->priority = 100;

    // change the running queue
    while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
    // add thread into the queue
    add_to_run_queue(1, new_thread);
    __sync_lock_release(&modifying_queue);



    // allocate heap space for this thread's stack
    // after everything is all set, push this thread into the ready queue

//    return new_thread->tcb->tid;

    return 0;
}

/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield() {
    swap_context();
    return 0;
};


/* terminate a thread */
void mypthread_exit(void *value_ptr) {
    // preserve the return value pointer if not NULL
    // deallocate any dynamic memory allocated when starting this thread
     printf("\nthread %d exiting\n", get_current_thread()->tcb->tid);
    // lock queue
    while (__sync_lock_test_and_set(&modifying_queue, 1)==1);

    // join_waiting_queue may be not empty
    if (scheduler->join_waiting_queue != NULL) {
        join_waiting_queue_node *wait_ptr = scheduler->join_waiting_queue;
        while (wait_ptr != NULL) {
            // printf("thread %d:\n", wait_ptr->thread->pid);
            if (wait_ptr->tid == get_current_thread()->tcb->tid) {
                //save return value
                if (wait_ptr->value_pointer != NULL) {
                    *(wait_ptr->value_pointer) = value_ptr;
                }
            }
            wait_ptr = wait_ptr->next;
        }
    }
    // set yield_purpose = 1, thread exit
    get_current_thread()->tcb->yield_purpose = 1;
    // unlock queue
    __sync_lock_release(&modifying_queue);
    mypthread_yield();

};

/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
    // YOUR CODE HERE
//     printf("\n%d Joining by %d\n", get_current_thread()->tcb->tid, thread);
    // lock queue
//    if (__sync_lock_test_and_set(&modifying_queue, 1) == 1)
//    {
//        // printf("ERRIR: queue locked when joining\n");
//        return -1; // another thead locks the queue, should not happen
//    }
    while (__sync_lock_test_and_set(&modifying_queue, 1)==1);

    // check if the thread has already fiinshed
    exit_t_node *p = scheduler->exit_thread_list;
    while (p != NULL) {
        if (thread == p->tid) {
            // printf("thread already exit, return directly\n");
            __sync_lock_release(&modifying_queue);
            return 1;
        }
        p = p->next;
    }
    // create new waiting node
    // printf("Making new node\n");
    join_waiting_queue_node *new_node = (join_waiting_queue_node *) malloc(sizeof(join_waiting_queue_node));
    new_node->tcb = get_current_thread()->tcb;
    new_node->tid = thread;
    new_node->value_pointer = NULL;
    new_node->next = NULL;
    // printf("Setting value ptr\n");
    new_node->value_pointer = value_ptr;

    // printf("Finished making new node\n");
    // add to wait queue
    if (scheduler->join_waiting_queue == NULL) {
        scheduler->join_waiting_queue = new_node;
    } else {
        join_waiting_queue_node *ptr = scheduler->join_waiting_queue;
        while (ptr->next != NULL) {
            ptr = ptr->next;
        }
        ptr->next = new_node;
    }
    // set flag for scheduler
    get_current_thread()->tcb->yield_purpose = 2;
    // unlock queue mutex
    // printf("join finished, going to yield()\n");
    __sync_lock_release(&modifying_queue);
    mypthread_yield();
    //Wait for the other thread to finish executing

    return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
    // YOUR CODE HERE
    // set 1 means it has not been deleted
    mutex->initialized = 1;
    // open mutex lock
    __sync_lock_release(&(mutex->mutex_lock));
    // initial mutex id
    mutex->mid = mutex_id;
    mutex_id++;

    //initialize data structures for this mutex

    return 0;
};

/* aquire a mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
    // YOUR CODE HERE
    if (mutex->initialized != 1) {
        return -1;
    }

    // use the built-in test-and-set atomic function to test the mutex
    if (__sync_lock_test_and_set(&(mutex->mutex_lock), 1) == 0) {
        // if the mutex is acquired successfully, return
        mutex->tid = get_current_thread()->tcb->tid;
        return 0;
    }
    // if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
    // initialize a new mutex waiting queue node
    mutex_waiting_queue_node *wait_node = malloc(sizeof(mutex_waiting_queue_node));
    Node *current_thread = get_current_thread();
    wait_node->thread = current_thread->tcb;
//    printf("%d wait for %d\n", current_thread->tcb->tid, mutex->tid);
    current_thread->tcb->yield_purpose = 2;
    wait_node->mutex_lock = mutex->mid;
    wait_node->next = NULL;
    // add into waiting queue
    add_to_mutex_waiting_queue(wait_node);
    mypthread_yield();
    return 0;
};


/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
    // YOUR CODE HERE
    // if mutex is destroyed, return -1
    if (mutex->initialized != 1) {
        return -1;
    }
    if (get_current_thread()->tcb->tid == mutex->tid) {
        // update the mutex's metadata to indicate it is unlocked
        __sync_lock_release(&(mutex->mutex_lock));
        // thread start from 0
        mutex->tid = -1;
        mutex_waiting_queue_node *ptr = scheduler->mutex_waiting_queue;
        mutex_waiting_queue_node *pre = NULL;
        // remove all the node from mutex waiting queue that has relationship with mutex id
        while (ptr != NULL) {
            if (ptr->mutex_lock == mutex->mid) {
                if (pre == NULL) {
                    scheduler->mutex_waiting_queue = ptr->next;
                } else {
                    pre->next = ptr->next;
                }
                Node *node = malloc(sizeof(Node));
                node->tcb = ptr->thread;
                node->next = NULL;
                // add lock to modify queue
                while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
                // put the thread at the front of this mutex's blocked/waiting queue in to the run queue
                switch (QUEUE_NUMBER) {
                    case 1:{
                        add_to_run_queue(1, node);
                        break;
                    }
                    case 2:{
                        add_to_run_queue_waiting_time_based(scheduler->round_robin_queue_T1, node);
                        break;
                    }
                    case 3:{
                        add_to_run_queue_priority_based(node);
                        break;
                    }
                }
//                add_to_run_queue(1, node);
                __sync_lock_release(&modifying_queue);
                mutex_waiting_queue_node *tmp = ptr;
                ptr = ptr->next;
                free(tmp);
            } else {
                pre = ptr;
                ptr = ptr->next;
            }
        }

    }
    return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
    // YOUR CODE HERE
    // deallocate dynamic memory allocated during mypthread_mutex_init
    while (__sync_lock_test_and_set(&(mutex->mutex_lock), 1) == 1);
    mutex->initialized = 0;
//    free(mutex);

    return 0;
};

/* scheduler */
static void schedule() {
    switch (QUEUE_NUMBER){
        case 1:
            sched_RR();
            break;
        case 2:
            sched_PSJF();
            break;
        case 3:
            sched_MLFQ();
            break;
        default:
            sched_RR();
            break;
    }

    // each time a timer signal occurs your library should switch in to this context

    // be sure to check the SCHED definition to determine which scheduling algorithm you should run
    //   i.e. RR, PSJF or MLFQ

    return;
}

/* Round Robin scheduling algorithm */
static void sched_RR() {
    // YOUR CODE HERE
//    Node *ptr=scheduler->round_robin_queue_T1->head->next;
    Queue *current_running_queue=scheduler->round_robin_queue_T1;
    Node *ptr= removeFront(current_running_queue);
//    age();
//    thread_handle(ptr);
//    addBack(current_running_queue, ptr);
    yield_handler_RR(ptr);
    scheduler->current_queue_number=1;
    getitimer(ITIMER_VIRTUAL, &timer);
    timer.it_interval.tv_usec=TIMER_PARA;
    timer.it_interval.tv_usec=TIMER_PARA;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    if (ptr->tcb->tid==scheduler->round_robin_queue_T1->head->next->tcb->tid){
        // this is the only thread
        __sync_lock_release(&scheduler_running);
        __sync_lock_release(&modifying_queue);
        return;
    }
    if (ptr->tcb->yield_purpose==1 && ptr->tcb->tid!=0){
        free(ptr->tcb->context->uc_stack.ss_sp);
        free(ptr->tcb->context);
        free(ptr->tcb);
        free(ptr);
        __sync_lock_release(&scheduler_running);
        __sync_lock_release(&modifying_queue);
        setcontext(scheduler->round_robin_queue_T1->head->next->tcb->context);
    }
    __sync_lock_release(&scheduler_running);
    __sync_lock_release(&modifying_queue);
    scheduler->current_thread=current_running_queue->head->next;
    swapcontext(ptr->tcb->context, current_running_queue->head->next->tcb->context);
    // Your own implementation of RR
    // (feel free to modify arguments and return types)
    return;
}

int yield_handler_RR(Node *ptr){
    switch (ptr->tcb->yield_purpose) {
        case 1: {
            mypthread_t exit_tid=ptr->tcb->tid;
            join_waiting_queue_node *wait_prev=NULL;
            join_waiting_queue_node *wait_ptr=scheduler->join_waiting_queue;
            while (wait_ptr!=NULL){
                if (wait_ptr->tid==exit_tid){// move father thread back to T1 queue
                    Node *new_node= malloc(sizeof(Node));
                    new_node->next=NULL;
                    new_node->prev=NULL;
                    new_node->tcb=wait_ptr->tcb;
//                    printf("thread %d comes back", new_node->tcb->tid);
                    add_to_run_queue(1, new_node);
                    if (wait_prev==NULL){
                        scheduler->join_waiting_queue=wait_ptr->next;
                    } else{
                        wait_prev->next=wait_ptr->next;
                    }
                    join_waiting_queue_node *tmp=wait_ptr;
                    wait_ptr=wait_ptr->next;
                    if (tmp->tid<=thread_number){
                        free(tmp);
                    }
                } else{
                    wait_prev=wait_ptr;
                    wait_ptr=wait_ptr->next;
                }

            }
            // move exit node to exit node list
            exit_t_node *exit_node=scheduler->exit_thread_list;
            if (exit_node==NULL){
                scheduler->exit_thread_list= malloc(sizeof(exit_t_node));
                scheduler->exit_thread_list->tid=exit_tid;
                scheduler->exit_thread_list->next=NULL;
            } else{
                while (exit_node->next!=NULL){
                    exit_node=exit_node->next;
                }
                exit_node->next= malloc(sizeof(exit_t_node));
                exit_node->next->tid=exit_tid;
                exit_node->next->next=NULL;
            }
            break;
        }
        case 2:{
            ptr->tcb->yield_purpose=0;
            break;
        }
        default:{
            ptr->tcb->yield_purpose=0;
            addBack(scheduler->round_robin_queue_T1, ptr);
        }
    }
}

/* Preemptive PSJF (STCF) scheduling algorithm */
static void sched_PSJF() {
    // YOUR CODE HERE
    Queue *current_running_queue=scheduler->round_robin_queue_T1;
    Node *ptr=removeFront(current_running_queue);
//    removeNode(current_running_queue, ptr);
//    ptr->tcb->waiting_time=0;
    add_waiting_time();
//    age();
//    thread_handle(ptr);
    yield_handler_PSJF(ptr);
    scheduler->current_queue_number=1;
    getitimer(ITIMER_VIRTUAL, &timer);
    timer.it_interval.tv_usec=TIMER_PARA;
    timer.it_interval.tv_usec=TIMER_PARA;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    if (ptr->tcb->tid==scheduler->round_robin_queue_T1->head->next->tcb->tid){
        // this is the only thread
        __sync_lock_release(&scheduler_running);
        __sync_lock_release(&modifying_queue);
        return;
    }
    if (ptr->tcb->yield_purpose==1 && ptr->tcb->tid!=0){
        free(ptr->tcb->context->uc_stack.ss_sp);
        free(ptr->tcb->context);
        free(ptr->tcb);
        free(ptr);
        __sync_lock_release(&scheduler_running);
        __sync_lock_release(&modifying_queue);
        setcontext(scheduler->round_robin_queue_T1->head->next->tcb->context);
    }
    __sync_lock_release(&scheduler_running);
    __sync_lock_release(&modifying_queue);
    scheduler->current_thread=current_running_queue->head->next;
    swapcontext(ptr->tcb->context, current_running_queue->head->next->tcb->context);
    // Your own implementation of PSJF (STCF)
    // (feel free to modify arguments and return types)
    return;
}

int yield_handler_PSJF(Node *ptr){
    switch (ptr->tcb->yield_purpose) {
        case 1: {
            mypthread_t exit_tid=ptr->tcb->tid;
            join_waiting_queue_node *wait_prev=NULL;
            join_waiting_queue_node *wait_ptr=scheduler->join_waiting_queue;
            while (wait_ptr!=NULL){
                if (wait_ptr->tid==exit_tid){// move father thread back to T1 queue
                    Node *new_node= malloc(sizeof(Node));
                    new_node->next=NULL;
                    new_node->prev=NULL;
                    new_node->tcb=wait_ptr->tcb;
                    new_node->tcb->waiting_time=wait_ptr->waiting_time;
//                    printf("thread %d comes back", new_node->tcb->tid);
                    add_to_run_queue_waiting_time_based(scheduler->round_robin_queue_T1, new_node);
                    if (wait_prev==NULL){
                        scheduler->join_waiting_queue=wait_ptr->next;
                        wait_prev=NULL;
                    } else{
                        wait_prev->next=wait_ptr->next;
                    }
                    join_waiting_queue_node *tmp=wait_ptr;
                    wait_ptr=wait_ptr->next;
                    if(tmp->tid<=thread_number){
                        free(tmp);
                    }
                }else{
                    wait_prev=wait_ptr;
                    wait_ptr=wait_ptr->next;
                }

            }
            // move exit node to exit node list
            exit_t_node *exit_node=scheduler->exit_thread_list;
            if (exit_node==NULL){
                scheduler->exit_thread_list= malloc(sizeof(exit_t_node));
                scheduler->exit_thread_list->tid=exit_tid;
                scheduler->exit_thread_list->next=NULL;
            } else{
                while (exit_node->next!=NULL){
                    exit_node=exit_node->next;
                }
                exit_node->next= malloc(sizeof(exit_t_node));
                exit_node->next->tid=exit_tid;
                exit_node->next->next=NULL;
            }
            break;
        }
        case 2:{
            ptr->tcb->yield_purpose=0;
            break;
        }
        default:{
            ptr->tcb->yield_purpose=0;
            add_to_run_queue_waiting_time_based(scheduler->round_robin_queue_T1, ptr);
        }
    }
}

int add_to_run_queue_waiting_time_based(Queue *current_queue, Node *new_node){
    Node *ptr=current_queue->head->next;
    while (ptr!=current_queue->rear){
        if (new_node->tcb->waiting_time>ptr->tcb->waiting_time){
            new_node->prev=ptr->prev;
            new_node->next=ptr;
            ptr->prev->next=new_node;
            ptr->prev=new_node;
            break;
        }
        ptr=ptr->next;
    }
    if (ptr==current_queue->rear){
        new_node->next=ptr;
        new_node->prev=ptr->prev;
        ptr->prev->next=new_node;
        ptr->prev=new_node;
    }
    current_queue->size++;
    return 0;
}

/* Preemptive MLFQ scheduling algorithm */
/* Graduate Students Only */
static void sched_MLFQ() {

    // printf("preparing to handle yield(), running queue is %d\nprinting current tcb:", scheduler->current_queue_number);
    // read_queues();
    Node *ptr = get_current_thread();
    switch (scheduler->current_queue_number) {
        // move the thread in first queue to second queue
        case 1:
            removeNode(scheduler->round_robin_queue_T1, ptr);
            age();
            thread_handle(ptr);
            break;

        case 2: //  move the thread in second queue to third queue
            removeNode(scheduler->round_robin_queue_T2, ptr);
            age();
            thread_handle(ptr);
            break;
        case 3: //		If a thread in the third running queue was running, do round-robin
            removeNode(scheduler->round_robin_queue_T3, ptr);
            age();
            thread_handle(ptr);
            break;

        default: //	If none of the above, unlock
            __sync_lock_release(&scheduler_running);
            __sync_lock_release(&modifying_queue);
            return ;
    }
    //	Depending on which queue has the highest first priority, switch the context to run that thread

    // choose the first node from the non-empty highest level running queue
    switch (get_highest_priority()) {
        //	no thread, unlock
        case 0:
            __sync_lock_release(&scheduler_running);
            __sync_lock_release(&modifying_queue);
            break;
        case 1: //	If the first queue is not empty, switch to first thread node
            scheduler->current_queue_number = 1;
            getitimer(ITIMER_VIRTUAL, &timer);
            timer.it_value.tv_usec = TIMER_PARA;
            timer.it_interval.tv_usec = TIMER_PARA;
            setitimer(ITIMER_VIRTUAL, &timer, NULL);
            if (ptr->tcb->tid == scheduler->round_robin_queue_T1->head->next->tcb->tid) { // this is the only thread
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                // setcontext(ptr->thread->context);
                return ;
            }
            // free space
            if (ptr->tcb->yield_purpose == 1 && ptr->tcb->tid != 0) {
                free(ptr->tcb->context->uc_stack.ss_sp);
                free(ptr->tcb->context);
                free(ptr->tcb);
                free(ptr);
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                setcontext(scheduler->round_robin_queue_T1->head->next->tcb->context);
            }
            scheduler->current_thread = scheduler->round_robin_queue_T1->head->next;
            scheduler->current_queue_number=1;
            __sync_lock_release(&scheduler_running);
            __sync_lock_release(&modifying_queue);
//            swapcontext(get_current_thread()->tcb->context, scheduler->round_robin_queue_T1->head->next->tcb->context);
            swapcontext(ptr->tcb->context, scheduler->round_robin_queue_T1->head->next->tcb->context);
            break;
            //		If the second queue has the highest priority thread, switch to that one.
        case 2: //	If second queue is not empty, switch to first thread node
            scheduler->current_queue_number = 2;
            getitimer(ITIMER_VIRTUAL, &timer);
            timer.it_value.tv_usec = TIMER_PARA2;
            timer.it_interval.tv_usec = TIMER_PARA2;
            setitimer(ITIMER_VIRTUAL, &timer, NULL);
            if (ptr->tcb->tid == scheduler->round_robin_queue_T2->head->next->tcb->tid) { // this is the only thread
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                // setcontext(ptr->thread->context);
                return ;
            }
            // free space
            if (ptr->tcb->yield_purpose == 1 && ptr->tcb->tid != 0) {
                free(ptr->tcb->context->uc_stack.ss_sp);
                free(ptr->tcb->context);
                free(ptr->tcb);
                free(ptr);
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                setcontext(scheduler->round_robin_queue_T2->head->next->tcb->context);
            }
            scheduler->current_thread = scheduler->round_robin_queue_T2->head->next;
            scheduler->current_queue_number=2;
            __sync_lock_release(&scheduler_running);
            __sync_lock_release(&modifying_queue);
//            swapcontext(get_current_thread()->tcb->context, scheduler->round_robin_queue_T2->head->next->tcb->context);
            swapcontext(ptr->tcb->context, scheduler->round_robin_queue_T2->head->next->tcb->context);
            break;
        case 3: //	If the third queue is not empty, switch to first thread node
            scheduler->current_queue_number = 3;
            getitimer(ITIMER_VIRTUAL, &timer);
            timer.it_value.tv_usec = 0;
            timer.it_interval.tv_usec = 0;
            setitimer(ITIMER_VIRTUAL, &timer, NULL);
            if (ptr->tcb->tid == scheduler->round_robin_queue_T3->head->next->tcb->tid) { // this is the only thread
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                // setcontext(ptr->thread->context);
                return ;
            }
            // free space
            if (ptr->tcb->yield_purpose == 1 && ptr->tcb->tid != 0) {
                free(ptr->tcb->context->uc_stack.ss_sp);
                free(ptr->tcb->context);
                free(ptr->tcb);
                free(ptr);
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
                setcontext(scheduler->round_robin_queue_T3->head->next->tcb->context);
            }
            if (scheduler->current_queue_number!=3){
                scheduler->current_thread = scheduler->round_robin_queue_T3->head->next;
                scheduler->current_queue_number=3;
                __sync_lock_release(&scheduler_running);
                __sync_lock_release(&modifying_queue);
//            swapcontext(get_current_thread()->tcb->context, scheduler->round_robin_queue_T3->head->next->tcb->context);
                swapcontext(ptr->tcb->context, scheduler->round_robin_queue_T3->head->next->tcb->context);
            }
            break;
        default:
            //	unlock
            __sync_lock_release(&scheduler_running);
            __sync_lock_release(&modifying_queue);
            return ;
    }

}

// check whether there is a thread
int check_queue_is_empty() {
    if (scheduler->round_robin_queue_T1 == NULL && scheduler->round_robin_queue_T2 == NULL &&
        scheduler->round_robin_queue_T3 == NULL) {
        return 1;
    }
    return 0;
}

// YOUR CODE HERE

int add_to_run_queue(int num, Node *node) {
    //	If there are no running threads in the given run queue, make the thread the beginning of the queue
    if (num == 1) {
        if (scheduler->round_robin_queue_T1 != NULL) {
            addBack(scheduler->round_robin_queue_T1, node);
            __sync_lock_release(&modifying_queue);
            return 0;
        }
    }
    if (num == 2) {
        if (scheduler->round_robin_queue_T2 != NULL) {
            addBack(scheduler->round_robin_queue_T2, node);
            __sync_lock_release(&modifying_queue);
            return 0;
        }
    }
    if (num == 3) {
        if (scheduler->round_robin_queue_T2 != NULL) {
            addBack(scheduler->round_robin_queue_T2, node);
            __sync_lock_release(&modifying_queue);
            return 0;
        }
    }
    return 0;
}

// increase priority by 1
int age() {
    Node *ptr = scheduler->round_robin_queue_T1->head->next, *end = scheduler->round_robin_queue_T1->rear;
    while (ptr != end) {
        ptr->tcb->priority += 1;
        ptr = ptr->next;
    }
    // printf("Finished with first queue\n");
    ptr = scheduler->round_robin_queue_T2->head->next, end = scheduler->round_robin_queue_T2->rear;
    while (ptr != end) {
        ptr->tcb->priority += 1;
        ptr = ptr->next;
    }
    // printf("Finished with second queue\n");
    ptr = scheduler->round_robin_queue_T3->head->next, end = scheduler->round_robin_queue_T3->rear;
    while (ptr != end) {
        ptr->tcb->priority += 1;
        ptr = ptr->next;
    }
    return 0;
}

// MLFQ handle
int thread_handle(Node *ptr) {
    switch (ptr->tcb->yield_purpose) {
        case 1: {
            mypthread_t exit_tid = ptr->tcb->tid;
            // remove the thread
            // free(ptr->thread);
            // iterate through waiting queue, move threads waiting for exit thread to running queue
            join_waiting_queue_node *wait_prev = NULL;
            join_waiting_queue_node *wait_ptr = scheduler->join_waiting_queue;
            while (wait_ptr != NULL) {
                if (wait_ptr->tid == exit_tid) {// printf("adding thread %u to run queue ... ", wait_ptr->thread->pid);
                    // add node to run queue
                    Node *new_node = (Node *) malloc(sizeof(Node));
                    new_node->next = NULL;
                    new_node->prev = NULL;
                    new_node->tcb = wait_ptr->tcb;
                    add_to_run_queue_priority_based(new_node);
//                    printf("thread %d comes back", new_node->tcb->tid);
                    // remove node from wait queue
                    if (wait_ptr!=NULL){
                        scheduler->join_waiting_queue = wait_ptr->next;
                    } else{
                        wait_prev->next=wait_ptr->next;
                    }
                    join_waiting_queue_node *tmp = wait_ptr;
                    wait_ptr = wait_ptr->next;
                    if (tmp->tid<=thread_number){
                        free(tmp);
                    }
                } else{
                    wait_prev=wait_ptr;
                    wait_ptr=wait_ptr->next;
                }

            }
            // add pid to finished list
            exit_t_node *finished_ptr = scheduler->exit_thread_list;
            if (finished_ptr == NULL) {
                scheduler->exit_thread_list = malloc(sizeof(exit_t_node));
                scheduler->exit_thread_list->tid = exit_tid;
                scheduler->exit_thread_list->next = NULL;
            } else {
                while (finished_ptr->next != NULL) {
                    finished_ptr = finished_ptr->next;
                }
                finished_ptr->next = malloc(sizeof(exit_t_node));
                finished_ptr->next->tid = exit_tid;
                finished_ptr->next->next = NULL;
            }
            break;
        }
        case 2: {
            // printf("join() or mutex_lock() happened, handling ... \n");
            // join and mutex_lock()
            ptr->tcb->yield_purpose = 0;
            break;
        }
        case 3: {
            // printf("yield() happened, handling ... \n");
            //yield()
            Node *first_node;
            switch (scheduler->current_queue_number) {
                case 1:
                    first_node = scheduler->round_robin_queue_T1->head->next;
                    break;
                case 2:
                    first_node = scheduler->round_robin_queue_T2->head->next;
                    break;
                case 3:
                    first_node = scheduler->round_robin_queue_T3->head->next;
                    break;
            }
//            if (ptr->next != NULL)
//            {
//                first_node = ptr->next;
//                first_node->next = ptr;
//                ptr->next = ptr->next->next;
//            }
            if(first_node!=NULL){
                ptr->next = first_node->next;
                ptr->prev = first_node;
                ptr->next->prev = first_node;
                first_node->next = ptr;
            }
            ptr->tcb->yield_purpose = 0;
        }
        default: {
            // printf("timeout happened, handling ... \n");
            switch (scheduler->current_queue_number) {
                case 1:
                    ptr->tcb->priority = 50;
                    add_to_run_queue(2, ptr);
                    break;
                case 2:
                    ptr->tcb->priority = 1;
                    add_to_run_queue(3, ptr);
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

void add_to_run_queue_priority_based(Node *node) {
//    Node *ptr = scheduler->round_robin_queue_T1->head->next, *end = scheduler->round_robin_queue_T1->rear;
//    //	Iterate through the first run queue until you reach the end or a thread with lower priority is found
//    while (ptr != end) {
//        if (ptr->tcb->priority < node->tcb->priority) {
//            //			If prev isn't next, insert the node between ptr and prev
//            insertBefore(scheduler->round_robin_queue_T1, node, ptr);
//            return;
//        }
//        ptr = ptr->next;
//    }
//    //	If no threads have lower priority, then the thread must be inserted at the end
//    insertBefore(scheduler->round_robin_queue_T1, node, end);
    node->tcb->priority=100;
    add_to_run_queue(1, node);
}

int get_highest_priority() {
    //	If the queue is already being modified, wait for the operation to finish, then continue

    //Don't need to lock, the only time this function is called is inside the scheduler
    /*while (__sync_lock_test_and_set(&modifying_queue, 1) == 1) {
        int placeholder = 0;
    }*/
    //	If no queue has any elements, return 0
    int highest_priority = 0;
    int highest_priority_queue = 0;
    //	If the first queue isn't empty, then it is the highest so far
    if (scheduler->round_robin_queue_T1->size != 0) {
        highest_priority = scheduler->round_robin_queue_T1->head->next->tcb->priority;
        highest_priority_queue = 1;
    }
    //	same logic as first queue
    if (scheduler->round_robin_queue_T2->size != 0) {
        if (scheduler->round_robin_queue_T2->head->next->tcb->priority > highest_priority) {
            highest_priority = scheduler->round_robin_queue_T2->head->next->tcb->priority;
            highest_priority_queue = 2;
        }
    }
    //	same logic as first queue
    if (scheduler->round_robin_queue_T3->size != 0) {
        if (scheduler->round_robin_queue_T3->head->next->tcb->priority > highest_priority) {
            highest_priority = scheduler->round_robin_queue_T3->head->next->tcb->priority;
            highest_priority_queue = 3;
        }
    }
    //	Return the highest priority queue number
    //__sync_lock_release(&modifying_queue);
    return highest_priority_queue;
}

int swap_context() {
//    printf("This is swap context!!!!!! \n");
//    printf("current queue:%d\n", scheduler->current_queue_number);
//    print_queue_tid();
//    printf("\n");
    //If no running threads, exit
    if (check_queue_is_empty() == 1) {
        return 0;
    }
    // printf("\nswap contexts\n");
    //	add queue lock
    if (__sync_lock_test_and_set(&modifying_queue, 1) == 1)
    {
        // printf("someone modifying the queue, return for now, come back soon\n");
        timer.it_interval.tv_usec = 1000;
        return 0;
    }
    //	add scheduler lock
    if (__sync_lock_test_and_set(&scheduler_running, 1) == 1)
    {
        // printf("scheduler is running, return\n");
        return 0;
    }
    schedule();
    __sync_lock_release(&scheduler_running);
    __sync_lock_release(&modifying_queue);

    return 0;
}

Node *get_current_thread() {
    if (QUEUE_NUMBER==3){
        if (scheduler->current_queue_number==1){
            return scheduler->round_robin_queue_T1->head->next;
        } else if (scheduler->current_queue_number==2){
            return scheduler->round_robin_queue_T2->head->next;
        } else if (scheduler->current_queue_number==3){
            return scheduler->round_robin_queue_T3->head->next;
        }
    }
    return scheduler->round_robin_queue_T1->head->next;
}

Scheduler *initial_scheduler() {
    while (__sync_lock_test_and_set(&modifying_queue, 1) == 1);
    scheduler = malloc(sizeof(Scheduler));
    scheduler->round_robin_queue_T1 = initQueue();
    scheduler->round_robin_queue_T2 = initQueue();
    scheduler->round_robin_queue_T3 = initQueue();

    scheduler->current_queue_number = 1;
    scheduler->mutex_waiting_queue = NULL;
    scheduler->join_waiting_queue = NULL;
    scheduler->exit_thread_list = NULL;
    __sync_lock_release(&modifying_queue);
    return scheduler;
}

Node *create_thread_node() {
    // create new Node
    Node *threadNode = malloc(sizeof(Node));
    threadNode->next = NULL;
    threadNode->prev = NULL;
    // create new thread node
    thread_control_block *newThread = malloc(sizeof(thread_control_block));
    // create and initialize the context of this thread
    newThread->context = malloc(sizeof(ucontext_t));
    getcontext(newThread->context);
    newThread->priority = 100;
    newThread->tid = thread_number;
    newThread->ret_val = NULL;
    newThread->yield_purpose = 0;
    // put new thread container into thread node
    threadNode->tcb = newThread;
    // add thread number
    thread_number++;
    return threadNode;
}



int add_to_mutex_waiting_queue(mutex_waiting_queue_node *node) {
    if (scheduler->mutex_waiting_queue == NULL) {
        scheduler->mutex_waiting_queue = node;
        return 0;
    }
    mutex_waiting_queue_node *ptr = scheduler->mutex_waiting_queue;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = node;
    return 0;
}

Node *get_most_waiting_time_node(Queue *queue){
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
    Node *ptr=queue->head->next;
    queue->head->next=queue->head->next->next;
    queue->size--;
    return ptr;
}

void add_waiting_time(){
    int iteration_time=timer.it_interval.tv_usec;
    Queue *current_running_queue=scheduler->round_robin_queue_T1;
    Node *ptr=current_running_queue->head->next;
    while (ptr!=current_running_queue->rear){
        ptr->tcb->waiting_time+=iteration_time;
        ptr=ptr->next;
    }
    return;
}


Queue *initQueue(){
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->head = (Node*) malloc(sizeof(Node));
    queue->rear = (Node*) malloc(sizeof(Node));
    queue->head->next = queue->rear;
    queue->head->prev = NULL;
    queue->rear->prev = queue->head;
    queue->rear->next = NULL;
    queue->size = 0;
    return queue;
}

int isEmpty(Queue *queue){
    return queue->size == 0 ;
}

void addFront(Queue *queue, Node *node){
    node->next = queue->head->next;
    node->next->prev = node;
    node->prev = queue->head;
    queue->head->next = node;
    queue->size ++;
}

int addBack(Queue *queue, Node *node){
    node->next = queue->rear;
    node->prev = queue->rear->prev;
    queue->rear->prev = node;
    node->prev->next = node;
    queue->size ++;
}

void removeNode(Queue *queue, Node *node){
    Node *prev = node->prev, *next=node->next;
    prev->next = next;
    next->prev = prev;
    node->next = NULL;
    node->prev = NULL;
    queue->size--;
}

void insertBefore(Queue* queue, Node *node, Node *pivot){
    Node *prev = pivot->prev;
    node->prev = prev;
    node->next = pivot;
    pivot->prev = node;
    prev->next = node;
    queue->size ++;
}

Node* removeFront(Queue *queue){
    Node* ret = queue->head->next;
    queue->head->next = ret->next;
    ret->next->prev = queue->head;
    ret->prev = NULL;
    ret->next = NULL;
    queue->size--;
    return ret;
}

Node* removeBack(Queue *queue){
    Node* ret = queue->rear->prev;
    queue->rear->prev = ret->prev;
    ret->prev->next = queue->rear;
    ret->prev = NULL;
    ret->next = NULL;
    queue->size--;
    return ret;
}

//my test debug function
void print_queue_tid(){
    Queue *queue;
    Node *ptr;
    if (QUEUE_NUMBER==3){
        printf("round_robin_queue_T1:");
        queue=scheduler->round_robin_queue_T1;
        ptr=queue->head->next;
        while (ptr!=queue->rear){
            printf("%d\t",ptr->tcb->tid);
            ptr=ptr->next;
        }
        printf("\n");
        printf("round_robin_queue_T2:");
        queue=scheduler->round_robin_queue_T2;
        ptr=queue->head->next;
        while (ptr!=queue->rear){
            printf("%d\t",ptr->tcb->tid);
            ptr=ptr->next;
        }
        printf("\n");
        printf("round_robin_queue_T3:");
        queue=scheduler->round_robin_queue_T3;
        ptr=queue->head->next;
        while (ptr!=queue->rear){
            printf("%d\t",ptr->tcb->tid);
            ptr=ptr->next;
        }
        printf("\n");
        return;
    }
    queue=scheduler->round_robin_queue_T1;
    ptr=queue->head->next;
    while (ptr!=queue->rear){
        printf("%d\t",ptr->tcb->tid);
        ptr=ptr->next;
    }
}