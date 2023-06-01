#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deque.h"

#define DATA_FILE   "scope.csv"
#define OUTPUT_FILE "output.csv"
#define FS          360

/* Filter parameters */
#define N         25  // Moving average window size for high-pass filter
#define S         7   // Triangle template matching parameter
#define L         5   // Moving average window size for Low-pass filter
#define BETA      2.5 // Dynamic threshold coefficient
#define M         150 // Moving average window size for dynamic threshold calculation

#define MIN_RR_DIST 0.272*FS

#define ABS(x) ((x) < 0 ? -(x) : (x))

void print_integer_deque(Deque *deque) {
    Node *node = deque->front;
    while (node != NULL) {
        printf("%d ", *(int *)(node->data));
        node = node->next;
    }
    printf("\n");
}

int main() {
    // Open the data and output files
    FILE *input_file = fopen(DATA_FILE, "r");
    FILE *output_file = fopen(OUTPUT_FILE, "w");

    unsigned short x_val;
    Deque *x = create_deque();

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%f,%*s", &x_val) != EOF) {
        push_front(x, &x_val, sizeof(unsigned short));
        if (deque_size(x) > 2*N + 1) {
            pop_rear(x);
        }
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}