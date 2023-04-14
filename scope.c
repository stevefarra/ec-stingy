#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short dataType;

#define IN_FILE "scope_in.txt"
#define OUT_FILE "scope_out.txt"

int main() {
    FILE *fp;

    // Open the text file for reading
    fp = fopen(IN_FILE, "r");

    // Check for errors with opening the file
    if (fp == NULL) {
        printf("Error opening input file.\n");
        exit(EXIT_FAILURE);
    }

    dataType* in_array = NULL;
    size_t size = 0;
    char line[100];

    // Read each line of text from the file
    while (fgets(line, sizeof(line), fp) != NULL) {
        // Increment the size of the input array
        size++;

        // Allocate memory for the input array for the new line
        in_array = (dataType*) realloc(in_array, size * sizeof(dataType));

        // Store the integer in the input array
        in_array[size - 1] = atoi(line);
    }

    // Close the input file
    fclose(fp);

    // Now open the output file
    fp = fopen(OUT_FILE, "w");

    // Check for errors with opening the output file
    if (fp == NULL) {
        printf("Error opening output file.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the output array
    dataType* out_array = (dataType*) malloc(size * sizeof(dataType));

    // Write the contents of the array to the output file
    for (size_t i = 0; i < size; i++) {
        out_array[i] = in_array[i] + 1;
        fprintf(fp, "%hu\n", out_array[i]);
    }

    // Close the output file
    fclose(fp);

    // Free the memory allocated for the arrays;
    free(in_array);
    free(out_array);

    return 0;
}
