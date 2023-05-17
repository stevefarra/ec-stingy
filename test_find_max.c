#include <stdio.h>

unsigned short max_index(float arr[], unsigned short start_idx, unsigned short end_idx) {
    float max_value = arr[start_idx];
    unsigned short max_index = start_idx;

    for (unsigned short i = start_idx + 1; i <= end_idx; i++) {
        if (arr[i] > max_value) {
            max_value = arr[i];
            max_index = i;
        }
    }

    return max_index;
}

int main() {
    float arr[] = {1.2, 4.5, 3.7, 2.9, 6.8, 5.1};
    unsigned short start_idx = 1;
    unsigned short end_idx = 4;

    unsigned short max_index = max_index(arr, start_idx, end_idx);
    float max_value = arr[max_index];
    printf("The max value between indices %hu and %hu is: %f, found at index: %hu\n", start_idx, end_idx, max_value, max_index);

    return 0;
}
