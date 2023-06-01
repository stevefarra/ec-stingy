#ifndef DEQUE_H
#define DEQUE_H

#include <stdlib.h>

typedef struct Node {
    void *data;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct Deque {
    Node *front;
    Node *rear;
    size_t size;
} Deque;

Deque *create_deque();
void delete_deque(Deque *deque);
void push_front(Deque *deque, const void *data, size_t dataSize);
void push_rear(Deque *deque, const void *data, size_t dataSize);
void pop_front(Deque *deque);
void pop_rear(Deque *deque);
void *peek_front(const Deque *deque);
void *peek_rear(const Deque *deque);
void *peek_middle(const Deque *deque);
size_t deque_size(const Deque *deque);

#endif // DEQUE_H
