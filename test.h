// File:	mypthread_t.h

// List all group members' names:
// iLab machine tested on:

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

/* in order to use the built-in Linux pthread library as a control for benchmarking, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>


typedef uint mypthread_t;

typedef struct Node {
    struct Node *prev;
    struct Node *next;
    struct thread_control_block *tcb;
} Node;

typedef struct Queue {
    struct Node *head;
    struct Node *rear;
    int size;
} Queue;


typedef struct exit_t_node {
    mypthread_t tid;    // thread ID
    struct exit_t_node *next;   // points to next thread
} exit_t_node;

/* add important states in a thread control block */
typedef struct thread_control_block {

    // thread Id
    int tid;

    // thread status
    int status;

    // thread context
    ucontext_t *context; // contains thread stack

    // thread priority  0-10
    int priority;

    // waiting time
    double waiting_time;

    // quantum count
    int quantum_count;

    // execution time:
    double execution_time;

    // return value from thread(maybe!!!)
    void *ret_val;

    /**
     * 0 for timeout
     * 1 for exit()
     * 2 for join() and mutex_lock()
     * 3 for yield()
     */
    int yield_purpose;
} thread_control_block;

/* mutex struct definition */
typedef struct mutex_waiting_queue_node {
    thread_control_block *thread;
    uint mutex_lock;
    struct mutex_waiting_queue_node *next;  // points to next mutex waiting thread
} mutex_waiting_queue_node;

typedef struct my_pthread_mutex_t {
    int initialized;
    mypthread_t tid;
    int mutex_lock;
    uint mid;
} mypthread_mutex_t;

/* thread join waiting queue */
typedef struct join_waiting_queue_node {
    thread_control_block *tcb;
    mypthread_t tid;
    void **value_pointer;
    struct join_waiting_queue_node *next;
    double waiting_time;
} join_waiting_queue_node;

typedef struct Scheduler {
    //	The first run queue is round robin with a time quantum of 25 ms
    Queue *round_robin_queue_T1;
    //	The second run queue is round robin with a time quantum of 50 ms
    Queue *round_robin_queue_T2;
    // The fourth running queue is MLFQ
    Queue *round_robin_queue_T3;
    //	Stores which queue is currently running
    int current_queue_number;
    //	The first wait queue is for threads waiting for a mutex lock
    mutex_waiting_queue_node *mutex_waiting_queue;
    //	The join wait queue is for threads waiting to join another thread
    join_waiting_queue_node *join_waiting_queue;
    //  The list contains pid of all finished thread to exit
    exit_t_node *exit_thread_list;
    // current thread in execution state
    Node *current_thread;
} Scheduler;

//// global variables
//Scheduler *scheduler;
//struct itimerval timer;
//int scheduler_running; // binary semaphore
//int modifying_queue; // binary semaphore
//ucontext_t *return_function;
//mypthread_t thread_number;
//uint mutex_id;
//int TIMER_PARA=25000;
//int QUEUE_NUMBER=1;


/* Function Declarations: */

// get current thread
Node *get_current_thread();

// get queue number with the highest level and non-empty
int get_highest_priority();

// add new_node to first queue in a proper place
void add_to_run_queue_priority_based(Node *new_node);

// swap thread context
int swap_context();

/* create a new thread */
int mypthread_create(mypthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg);

/* current thread voluntarily surrenders its remaining runtime for other threads to use */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initialize a mutex */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire a mutex (lock) */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release a mutex (unlock) */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy a mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

// initialize a scheduler
Scheduler *initial_scheduler();

// create a thread Node
Node *create_thread_node();

// MLFQ handle thread node ptr
int thread_handle(Node *ptr);

// increase priority by 1
int age();

// add thread to mutex waiting queue
int add_to_mutex_waiting_queue(mutex_waiting_queue_node *node);

// scheduling by round-robin
static void sched_RR();

// scheduling by shortest job first
static void sched_PSJF();

// scheduling by multi-level feedback queue
static void sched_MLFQ();

// get the thrr=ead node with the largest waiting time
Node *get_most_waiting_time_node(Queue *queue);

// add waiting time to all thread
void add_waiting_time();

// add node to specific queue number
int add_to_run_queue(int num, Node *node);

// initialize a empty queue
Queue *initQueue();

// check whether queue is empty
int isEmpty(Queue *queue);

// add node to the head of the queue
void addFront(Queue *queue, Node *node);

// add node from the rear of the queue
int addBack(Queue *queue, Node *node);

// remove the node from queue
void removeNode(Queue *queue, Node *node);

// insert node into queue before pivot
void insertBefore(Queue *queue, Node *node, Node *pivot);

// remove the first node from queue
Node *removeFront(Queue *queue);

// remove the last node from queue
Node *removeBack(Queue *queue);

// handle Node ptr when round-robin
int yield_handler_RR(Node *ptr);

// handle Node ptr when SJF
int yield_handler_PSJF(Node *ptr);

// insert new_node into current_queue ib a proper place base on waiting time
int add_to_run_queue_waiting_time_based(Queue *current_queue, Node *new_node);

// test function
void print_queue_tid();


#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif


#endif