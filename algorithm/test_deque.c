#include <stdio.h>
#include "deque.h"

void print_integer_deque(Deque *deque) {
    Node *node = deque->front;
    while (node != NULL) {
        printf("%d ", *(int *)(node->data));
        node = node->next;
    }
    printf("\n");
}

int main() {
    Deque *deque = create_deque();

    for (int i = 1; i <= 7; ++i) {
        push_front(deque, &i, sizeof(int));
    }
    print_integer_deque(deque);

    int front_num = *(int *)peek_front(deque);
    int rear_num = *(int *)peek_rear(deque);
    int mid_num = *(int *)peek_middle(deque);
    printf("Front of deque: %d\n", front_num);
    printf("Rear of deque: %d\n", rear_num);
    printf("Middle of deque: %d\n", mid_num);
    printf("Size of deque: %zu\n", deque_size(deque));

    pop_front(deque);
    print_integer_deque(deque);

    pop_rear(deque);
    print_integer_deque(deque);

    delete_deque(deque);

    return 0;
}
