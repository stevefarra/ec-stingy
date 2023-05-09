#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DATA_FILE   "scope.csv"
#define OUTPUT_FILE "output.csv"
#define FS          360

/* Filter parameters */
#define BUFF_SIZE 1080
#define N         25  // Moving average window size for high-pass filter
#define S         7   // Triangle template matching parameter
#define L         5   // Moving average window size for Low-pass filter
#define BETA      2.5 // Dynamic threshold coefficient
#define M         150 // Moving average window size for dynamic threshold calculation

typedef enum {
    FALSE,
    TRUE
} bool;

int compare(const void *a, const void *b) {
        float fa = *(const float *)a;
        float fb = *(const float *)b;
        return (fa > fb) - (fa < fb);
    }

float get_median_val(float arr[], unsigned short start_idx, unsigned short end_idx) {
    unsigned short len = end_idx - start_idx + 1;
    float *sorted_arr = malloc(len * sizeof(float));

    // Copy the relevant elements into the sorted_arr
    for (unsigned short i = 0; i < len; i++) {
        sorted_arr[i] = arr[start_idx + i];
    }

    // Sort the sorted_arr using the qsort function
    qsort(sorted_arr, len, sizeof(float), compare);

    // Calculate the median value
    float median;
    if (len % 2 == 0) {
        median = (sorted_arr[len / 2 - 1] + sorted_arr[len / 2]) / 2.0;
    } else {
        median = sorted_arr[len / 2];
    }

    // Free the allocated memory
    free(sorted_arr);

    return median;
}

int main() {

    // Signal buffers
    float x    [BUFF_SIZE],
          x_bar[BUFF_SIZE],
          y_hat[BUFF_SIZE],
          y    [BUFF_SIZE],
          t    [BUFF_SIZE],
          l    [BUFF_SIZE],
          ma   [BUFF_SIZE],
          th   [BUFF_SIZE];
    
    // Signal values
    float x_val     = 0,
          x_bar_val = 0,
          y_hat_val = 0,
          y_val     = 0,
          t_val     = 0,
          l_val     = 0,
          ma_val    = 0,
          theta,
          th_val    = 0;

    // Indices
    unsigned short i,
                   num_x_samples  = 0,
                   num_y_samples  = 0,
                   num_t_samples  = 0,
                   num_l_samples  = 0,

                   x_idx          = 0,
                   y_idx,
                   t_idx,
                   l_idx,
                   th_idx;

    // Flags
    bool counting_samples = TRUE,

         highpass  = FALSE,
         triangle  = FALSE,
         lowpass   = FALSE,
         threshold = FALSE,

         first_highpass  = TRUE,
         first_lowpass   = TRUE,
         first_threshold = TRUE;

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
        x[i]     = 0;
        x_bar[i] = 0;
        y_hat[i] = 0;
        y[i]     = 0;
        t[i]     = 0;
        l[i]     = 0;
        ma[i]    = 0;
        th[i]    = 0;
    }

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%f,%*s", &x_val) != EOF) {

        // If the ECG buffer is full,
        if (x_idx == BUFF_SIZE) {

            // Shift the buffers
            for (i = 0; i < BUFF_SIZE - 1; i++) {
                x    [i] = x    [i + 1];
                x_bar[i] = x_bar[i + 1];
                y_hat[i] = y_hat[i + 1];
                y    [i] = y    [i + 1];
                t    [i] = t    [i + 1];
                l    [i] = l    [i + 1];
                ma   [i] = ma   [i + 1];
                th   [i] = th   [i + 1];
            }
            // Decrement the indices
            x_idx--;
            y_idx--;
            t_idx--;
            l_idx--;
            th_idx--;
        }
        // Add the sample to the buffer and increment the index
        i = x_idx;
        x[i] = x_val;

        x_idx++;
        if (counting_samples) {
            num_x_samples++;

            if (num_x_samples == 2*N + 1) {
            highpass = TRUE;
            y_idx = x_idx - (N + 1);
            }
            if (num_y_samples == 2*S + 1) {
                triangle = TRUE;
                t_idx = y_idx - (S + 1);
            }
            if (num_t_samples == 2*L + 1) {
                lowpass = TRUE;
                l_idx = t_idx - (L + 1);
            }
            if (num_l_samples == 2*M + 1) {
                threshold = TRUE;
                th_idx = l_idx - (M + 1);
                theta = get_median_val(l, l_idx - num_l_samples, l_idx - 1) / 4;
            }
        }

        if (highpass) {
            i = y_idx;
            
            if (first_highpass) {
                first_highpass = FALSE;
                sum = 0;

                for (j = -N; j <= N; j++) {
                    sum += x[i + j];
                }
                x_bar_val = sum / (2*N + 1);
            } else {
                x_bar_val = x_bar[i - 1] + (x[i + N] - x[i - (N + 1)]) / (2*N + 1);
            }
            y_hat_val = x_val - x_bar_val;
            y_val = fabs(y_hat_val);

            x_bar[i] = x_bar_val;
            y_hat[i] = y_hat_val;
            y[i] = y_val;

            y_idx++;
            if (counting_samples) {
                num_y_samples++;
            }
        }
        if (triangle) {
            i = t_idx;

            t_val = (y[i] - y[i - S]) * (y[i] - y[i + S]);
            t[i] = t_val;

            t_idx++;
            num_t_samples++;
        }
        if (lowpass) {
            i = l_idx;
            
            if (first_lowpass) {
                first_lowpass = FALSE;
                sum = 0;

                for (j = -L; j <= L; j++) {
                    sum += t[i + j];
                }
                l_val = sum / (2*L + 1);
            } else {
                l_val = l[i - 1] + (t[i + L] - t[i - (L + 1)]) / (2*L + 1);
            }
            l[i] = l_val;

            l_idx++;
            if (counting_samples) {
                num_l_samples++;
            }
        }
        if (threshold) {
            i = th_idx;
            
            if (first_threshold) {
                counting_samples = FALSE;
                first_threshold = FALSE;
                sum = 0;

                for (j = -M; j <= M; j++) {
                    sum += l[i + j];
                }
                ma_val = sum / (2*M + 1);
            } else {
                ma_val = ma[i - 1] + (l[i + M] - l[i - (M + 1)])/(2*M + 1);
            }
            th_val = BETA*ma_val + theta;

            ma[i] = ma_val;
            th[i] = th_val;

            th_idx++;
        }

        fprintf(output_file,
                "%f,%f,%f,%f,%f,%f,%f\n",
                x_val,
                x_bar_val,
                y_hat_val,
                y_val,
                t_val,
                l_val,
                th_val
        );
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}
