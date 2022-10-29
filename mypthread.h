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
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "Queue.h"

typedef uint mypthread_t;

typedef struct exit_t_node {
    my_pthread_t tid;
    struct exit_t_node *next;
} exit_t_node;

/* add important states in a thread control block */
typedef struct thread_control_block {

    // YOUR CODE HERE
    // thread Id
    int tid;
    // thread status
    int status;
    // thread context
    ucontext_t *context;
    // thread stack  (uc_stack in ucontext_t)

    // thread priority  0-10
    int priority;
    // And more ...
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
    int yield_purpose;  // 0 for timeout
} thread_control_block;

typedef enum STATUS {
    Ready = 1, Running = 2, Wait = 3, Blocked = 4, Dead = 5
} STATUS;

/* mutex struct definition */
typedef struct mutex_waiting_queue_node {

    // YOUR CODE HERE
    thread_control_block *thread;
    uint mutex_lock;
    struct mutex_waiting_queue_node *next;
} mutex_waiting_queue_node;

typedef struct my_pthread_mutex_t {
    int initialized;
    my_pthread_t tid;
    int mutex_lock;
    uint mid;
}mypthread_mutex_t;

typedef struct join_waiting_queue_node {
    thread_control_block *tcb;
    mypthread_t tid;
    void **value_pointer;
    struct join_waiting_queue_node *next;
} join_waiting_queue_node;

// Feel free to add your own auxiliary data structures (linked list or queue etc...)
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

    //	The second wait queue is for threads waiting to join another thread
    join_waiting_queue_node *join_waiting_queue;
    //  The list contains pid of all finished thread
    exit_t_node *exit_thread_list;
    // current thread in execution state
    Node *current_thread;
} Scheduler;

// global variables
Scheduler *scheduler;
struct itimerval timer;
int scheduler_running; // binary semaphore
int modifying_queue; // binary semaphore
ucontext_t *return_function;
my_pthread_t thread_number;
uint mutex_id;


/* Function Declarations: */
Node *get_current_thread();

int get_highest_priority();

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

Scheduler *initial_scheduler();

Node *create_thread_node();

int add_node_into_queue(int schedulerStyle, Node *threadNode);

int add_to_mutex_waiting_queue(mutex_waiting_queue_node* node)

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
