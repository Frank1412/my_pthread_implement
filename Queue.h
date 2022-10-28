//
// Created by 方首智 on 10/23/22.
//

#ifndef MYPTHREAD_QUEUE_H
#define MYPTHREAD_QUEUE_H

#include<stdlib.h>
#include <stdlib.h>
#include "mypthread.h"

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

Queue *initQueue();

int isEmpty(Queue *queue);

void addFront(Queue *queue, Node *node);

int addBack(Queue *queue, Node *node);

Node* removeFront(Queue *queue);

Node* removeBack(Queue *queue);

#endif //MYPTHREAD_QUEUE_H

