#include "deque.h"
#include <string.h>

Deque *create_deque() {
    Deque *deque = (Deque *) malloc(sizeof(Deque));
    deque->front = NULL;
    deque->rear = NULL;
    deque->size = 0;
    return deque;
}

void delete_deque(Deque *deque) {
    while (deque->size > 0) {
        pop_front(deque);
    }
    free(deque);
}

void push_front(Deque *deque, const void *data, size_t dataSize) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->data = malloc(dataSize);
    memcpy(node->data, data, dataSize);

    node->next = deque->front;
    node->prev = NULL;

    if (deque->front != NULL) {
        deque->front->prev = node;
    }
    deque->front = node;

    if (deque->rear == NULL) {
        deque->rear = node;
    }
    deque->size++;
}

void push_rear(Deque *deque, const void *data, size_t dataSize) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->data = malloc(dataSize);
    memcpy(node->data, data, dataSize);

    node->next = NULL;
    node->prev = deque->rear;

    if (deque->rear != NULL) {
        deque->rear->next = node;
    }
    deque->rear = node;

    if (deque->front == NULL) {
        deque->front = node;
    }
    deque->size++;
}

void pop_front(Deque *deque) {
    if (deque->size == 0) {
        return;
    }

    Node *node = deque->front;

    deque->front = node->next;
    if (deque->front != NULL) {
        deque->front->prev = NULL;
    } else {
        deque->rear = NULL;
    }

    free(node->data);
    free(node);
    deque->size--;
}

void pop_rear(Deque *deque) {
    if (deque->size == 0) {
        return;
    }

    Node *node = deque->rear;

    deque->rear = node->prev;
    if (deque->rear != NULL) {
        deque->rear->next = NULL;
    } else {
        deque->front = NULL;
    }

    free(node->data);
    free(node);
    deque->size--;
}

void *peek_front(const Deque *deque) {
    return deque->front != NULL ? deque->front->data : NULL;
}

void *peek_rear(const Deque *deque) {
    return deque->rear != NULL ? deque->rear->data : NULL;
}

void *peek_middle(const Deque *deque) {
    if (deque->size == 0) {
        return NULL;
    }
    
    Node *node = deque->front;
    for(size_t i = 0; i < deque->size / 2; ++i) {
        node = node->next;
    }
    return node->data;
}

size_t deque_size(const Deque *deque) {
    return deque->size;
}
