#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define HIGHPASS_DELAY  2*N + 1
#define TRIANGLE_DELAY  (HIGHPASS_DELAY + 2*S + 1)
#define LOWPASS_DELAY   (TRIANGLE_DELAY + 2*L + 1)
#define THRESHOLD_DELAY (LOWPASS_DELAY + 2*M + 1)

#define VOIDED -1

typedef enum {
    FALSE,
    TRUE
} bool;

int main() {
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

    // Declare signal buffers
    float val,
          ecg  [BUFF_SIZE],
          h_hat[BUFF_SIZE],
          h    [BUFF_SIZE],
          t    [BUFF_SIZE],
          l    [BUFF_SIZE],
          mean [BUFF_SIZE],
          ma   [BUFF_SIZE],
          th   [BUFF_SIZE];

    // Initialize indices
    unsigned short i,
                   samples_read = 0,
                   ecg_idx      = 0,
                   h_idx        = ecg_idx - N,
                   t_idx,
                   l_idx,
                   mean_idx,
                   th_idx;

    // Initialize flags
    bool highpass  = FALSE;
    bool triangle  = FALSE;
    bool lowpass   = FALSE;
    bool threshold = FALSE;

    // Clear buffers
    for (i = 0; i < BUFF_SIZE; i++) {
        ecg[i]   = 0;
        h_hat[i] = 0;
        h[i]     = 0;
        t[i]     = 0;
        l[i]     = 0;
        ma[i]    = 0;
        th[i]    = 0;
    }

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%f,%*s", &val) != EOF) {

        // If the ECG buffer is full,
        if (ecg_idx == BUFF_SIZE) {

            // Shift the buffers
            for (i = 1; i < BUFF_SIZE; i++) {
                ecg  [i - 1] = ecg  [i];
                h_hat[i - 1] = h_hat[i];
                h    [i - 1] = h    [i];
                t    [i - 1] = t    [i];
                l    [i - 1] = l    [i];
                mean [i - 1] = mean [i];
                ma   [i - 1] = ma   [i];
                th   [i - 1] = th   [i];
            }
            // Decrement the indices
            ecg_idx--;
            h_idx--;
            t_idx--;
            l_idx--;
            mean_idx--;
            th_idx--;
        }
        // Add the sample to the buffer and increment the index
        i = ecg_idx++;
        ecg[i] = val;

        // If the samples read counter is still in use, increment it
        if (samples_read != VOIDED) {
            samples_read++;
        }
        // if (samples_read == HIGHPASS_DELAY) {
        //     highpass = TRUE;
        //     h_idx = ecg_idx - N;
        // }
        // if (samples_read == TRIANGLE_DELAY) {
        //     triangle = TRUE;
        //     t_idx = h_idx - S;
        // }
        // if (samples_read == LOWPASS_DELAY) {
        //     lowpass = TRUE;
        //     l_idx = t_idx - L;
        // }
        // if (samples_read == THRESHOLD_DELAY) {
        //     threshold = TRUE;
        //     th_idx = l_idx - M;
        // }

        // h[i] = h_hat[i - 1] + ecg[i] - ecg[i - 1] + (ecg[i - (N + 1)] - ecg[i + N])/(2*N + 1)
        if (highpass) {
            i = h_idx++;
            val = 0.0; 

            if (i >= 0) {
                val += ecg[i];
                val -= ecg[i + N]/(2*N + 1);
            }
            if (i >= 1) {
                val += h_hat[i - 1];
                val -= ecg[i - 1];    
            }
            if (i >= N + 1) {
                val += ecg[i - (N + 1)]/(2*N + 1);
            }
            
            h_hat[i] = val;
            h[i] = abs(val);
        }
        // if (triangle) {
        //     i = t_idx++;
        //     val = (h[i] - h[i - S])*(h[i] - h[i + S]);
        //     t[i] = val;
        // }
        // if (lowpass) {
        //     i = l_idx++;
        //     val = l[i - 1] + (t[i + L] - t[i - L - 1])/((2*L + 1.0));
        //     l[i] = val;
        // }
        fprintf(output_file, "%f,%f,%f,%f,%f\n", ecg[i], h_hat[i], h[i], t[i], l[i]);
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}
