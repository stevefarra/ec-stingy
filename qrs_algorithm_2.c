#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DATA_FILE   "scope.csv"
#define OUTPUT_FILE "output.csv"
#define FS          360

/* Filter parameters */
#define BUFF_SIZE 720
#define N         25  // Moving average window size for high-pass filter
#define S         7   // Triangle template matching parameter
#define L         5   // Moving average window size for Low-pass filter
#define BETA      2.5 // Dynamic threshold coefficient
#define M         150 // Moving average window size for dynamic threshold calculation

typedef enum {
    FALSE,
    TRUE
} bool;

int main() {

    // Signal buffers
    float val,
          x    [BUFF_SIZE],
          x_bar[BUFF_SIZE],
          y_hat[BUFF_SIZE],
          y    [BUFF_SIZE];

    // Indices
    unsigned short i,
                   samples_read = 0,
                   x_idx        = 0,
                   highpass_idx = N;

    // Flags
    bool highpass = FALSE;

    // Moving average variables
    float sum;
    short j;

    // Open the data file
    FILE *input_file = fopen(DATA_FILE, "r");
    if (input_file == NULL) {
        perror("Unable to open the data file");
        return 1;
    }

    // Open the output file
    FILE *output_file = fopen(OUTPUT_FILE, "w");
    if (output_file == NULL) {
        perror("Unable to open the output file");
        fclose(input_file);
        return 1;
    }

    // Clear buffers
    for (i = 0; i < BUFF_SIZE; i++) {
        x[i]     = 0.0;
        x_bar[i] = 0.0;
        y_hat[i] = 0.0;
        y[i]     = 0.0;
    }

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%f,%*s", &val) != EOF) {

        // If the ECG buffer is full,
        if (x_idx == BUFF_SIZE) {

            // Shift the buffers
            for (i = 0; i < BUFF_SIZE - 1; i++) {
                x    [i] = x    [i + 1];
                x_bar[i] = x_bar[i + 1];
                y_hat[i] = y_hat[i + 1];
                y    [i] = y    [i + 1];
            }
            // Decrement the indices
            x_idx--;
            highpass_idx--;
        }
        // Add the sample to the buffer and increment the index
        i = x_idx++;
        x[i] = val;
        samples_read++;

        if (samples_read >= (2*N + 1) && samples_read < 2269 - (N + 1)) {
            highpass = TRUE;
        } else {
            highpass = FALSE;
        }
        if (highpass) {
            i = highpass_idx++;
            
            if (i == N) {
                sum = 0;

                for (j = -N; j <= N; j++) {
                    sum += x[i + j];
                }
                val = sum / (2*N + 1);
            } else {
                val = x_bar[i - 1] + (x[i + N] - x[i - (N + 1)])/(2*N + 1);
            }
            x_bar[i] = val;
            y_hat[i] = x[i] - x_bar[i];
            y[i] = fabs(y_hat[i]);
        }
        fprintf(output_file, "%f,%f,%f,%f\n", x[x_idx - 1],
                                              x_bar[highpass_idx - 1],
                                              y_hat[highpass_idx - 1],
                                              y[highpass_idx - 1]);
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}
