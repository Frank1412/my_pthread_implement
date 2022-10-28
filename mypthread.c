// File:	mypthread.c

// List all group members' names:
// iLab machine tested on:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
	   // YOUR CODE HERE	
//	int tid = pthread_create(thread, attr, function, arg);

       if(scheduler==NULL){

           thread_number=0;
           // create a Thread Control Block
           scheduler=InitialScheduler();
           // create and initialize the context of this thread
           Node *newNode=InitialThreadContainer();
           while (__sync_lock_test_and_set(&modifying_queue, 1)==1);
           AddNodeIntoRunningQueue(1, newNode);// TODO: about schedulerStyle, should be changed when changing test data
           __sync_lock_release(&modifying_queue);


           return_function= malloc(sizeof(ucontext_t));
           getcontext(return_function);
           return_function->uc_stack.ss_sp= malloc(65536);
           return_function->uc_stack.ss_size= 65536;
           makecontext(return_function, (void (*)(void))&mypthread_exit, 1,arg);

           __sync_lock_release(&scheduler_running);
           __sync_lock_release(&modifying_queue);

           mutex_id=0;
       }
       // TODO: unfinished......


	   // allocate heap space for this thread's stack
	   // after everything is all set, push this thread into the ready queue
    
//    return tid;

	return 0;
};

/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield()
{
	// YOUR CODE HERE
	
	// change current thread's state from Running to Ready
	// save context of this thread to its thread control block
	// switch from this thread's context to the scheduler's context

	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr)
{
	// YOUR CODE HERE

	// preserve the return value pointer if not NULL
	// deallocate any dynamic memory allocated when starting this thread
	
	return;
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr)
{
	// YOUR CODE HERE
    pthread_join(thread, value_ptr);
	// wait for a specific thread to terminate
	// deallocate any dynamic memory created by the joining thread

	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	// YOUR CODE HERE
	
	//initialize data structures for this mutex

	return 0;
};

/* aquire a mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex)
{
		// YOUR CODE HERE
	
		// use the built-in test-and-set atomic function to test the mutex
		// if the mutex is acquired successfully, return
		// if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
		
		return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex)
{
	// YOUR CODE HERE	
	
	// update the mutex's metadata to indicate it is unlocked
	// put the thread at the front of this mutex's blocked/waiting queue in to the run queue

	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex)
{
	// YOUR CODE HERE
	
	// deallocate dynamic memory allocated during mypthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule()
{
	// YOUR CODE HERE
	
	// each time a timer signal occurs your library should switch in to this context
	
	// be sure to check the SCHED definition to determine which scheduling algorithm you should run
	//   i.e. RR, PSJF or MLFQ

	return;
}

/* Round Robin scheduling algorithm */
static void sched_RR()
{
	// YOUR CODE HERE
	
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
	
	return;
}

/* Preemptive PSJF (STCF) scheduling algorithm */
static void sched_PSJF()
{
	// YOUR CODE HERE

	// Your own implementation of PSJF (STCF)
	// (feel free to modify arguments and return types)

	return;
}

/* Preemptive MLFQ scheduling algorithm */
/* Graduate Students Only */
static void sched_MLFQ() {
	// YOUR CODE HERE
	
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	return;
}

// Feel free to add any other functions you need

// YOUR CODE HERE

tcb *InitialScheduler(){
    while (__sync_lock_test_and_set(&modifying_queue,1)==1);
    scheduler= malloc(sizeof (tcb));
    scheduler->round_robin_queue_T1= malloc(sizeof(Queue));
    scheduler->round_robin_queue_T1->head=NULL;
    scheduler->round_robin_queue_T1->rear=NULL;
    scheduler->round_robin_queue_T1->size=0;
    scheduler->round_robin_queue_T2= malloc(sizeof(Queue));
    scheduler->round_robin_queue_T2->head=NULL;
    scheduler->round_robin_queue_T2->rear=NULL;
    scheduler->round_robin_queue_T2->size=0;
    scheduler->SJF_queue=malloc(sizeof(Queue));
    scheduler->SJF_queue->head=NULL;
    scheduler->SJF_queue->rear=NULL;
    scheduler->SJF_queue->size=0;
    // ??
    // TODO: What should this part like?
    scheduler->MLFQ_queue=NULL;
    // ??

    scheduler->current_queue_number=1;
    scheduler->mutex_waiting_queue=NULL;
    scheduler->join_waiting_queue=NULL;
    scheduler->exit_thread_list=NULL;
    __sync_lock_release(&modifying_queue);
    return scheduler;
}

Node *InitialThreadContainer(){
    // create new Node
    Node *threadNode= malloc(sizeof(thread_container));
    threadNode->next=NULL;
    threadNode->prev=NULL;
    // create new thread container
    thread_control_block *newThread= malloc(sizeof(thread_control_block));
    // create and initialize the context of this thread
    newThread->context= malloc(sizeof(ucontext_t));
    getcontext(newThread->context);
    newThread->priority=100;
    newThread->tid=thread_number;
    newThread->ret_val=NULL;
    newThread->yield_purpose=0;
    // put new thread container into thread node
    threadNode->tcb=newThread;
    // add thread number
    thread_number++;
    return threadNode;
}

int AddNodeIntoRunningQueue(int schedulerStyle, Node *threadNode){
    Node *ptrNode;
    Queue *ptrQueue;
    if(schedulerStyle==0){// round-robin with 25ms
        if(scheduler->round_robin_queue_T1->head==NULL){
            scheduler->round_robin_queue_T1->rear=threadNode;
            scheduler->round_robin_queue_T1->head=threadNode;
            scheduler->round_robin_queue_T1->size++;
            __sync_lock_release(&modifying_queue);
            return 0;
        }
        ptrNode=scheduler->round_robin_queue_T1->head;
        ptrQueue=scheduler->round_robin_queue_T1;
    } else if(schedulerStyle==1){// round-robin with 50ms
        if(scheduler->round_robin_queue_T2->head==NULL){
            scheduler->round_robin_queue_T2->rear=threadNode;
            scheduler->round_robin_queue_T2->head=threadNode;
            scheduler->round_robin_queue_T2->size++;
            __sync_lock_release(&modifying_queue);
            return 0;
        }
        ptrNode=scheduler->round_robin_queue_T2->head;
        ptrQueue=scheduler->round_robin_queue_T2;
    } else if(schedulerStyle==2){// SJF
        if(scheduler->SJF_queue->head==NULL){
            scheduler->SJF_queue->rear=threadNode;
            scheduler->SJF_queue->head=threadNode;
            scheduler->SJF_queue->size++;
            __sync_lock_release(&modifying_queue);
            return 0;
        }
        ptrNode=scheduler->SJF_queue->head;
        ptrQueue=scheduler->SJF_queue;
    } else{// MLFQ
        // TODO: structure of MLFQ is unknown.
    }
    while (ptr->next!=NULL){
        ptrNode=ptrNode->next;
    }
    ptrNode->next=threadNode;
    threadNode->prev=ptrNode;
    ptrQueue->rear=ptrNode;
    ptrQueue->size++;
    return 0;
}