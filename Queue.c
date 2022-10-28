//
// Created by 方首智 on 10/23/22.
//

#include "Queue.h"

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