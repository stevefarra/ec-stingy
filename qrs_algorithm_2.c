#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE   "scope.csv"
#define OUTPUT_FILE "output.csv"
#define FS          360

/* Filter parameters */
#define BUFF_SIZE 1000
#define N         25  // Moving average window size for high-pass filter
#define S         7   // Triangle template matching parameter
#define L         5   // Moving average window size for Low-pass filter
#define BETA      2.5 // Dynamic threshold coefficient
#define M         150 // Moving average window size for dynamic threshold calculation

#define MIN_RR_DIST 0.272*FS

#define ABS(x) ((x) < 0 ? -(x) : (x))

float movavg(float curr_val,
             float x[],
             const short x_idx,
             const unsigned short window_radius) {

    const unsigned short R = window_radius,
                         W = 2*R + 1;

    const short i = x_idx,
                j = x_idx - (R + 1);

    if (i <= W) {
        curr_val += x[i] / W;
    } else {
        curr_val = curr_val + (x[j + R] - x[j - (R + 1)]) / W;
    }
    return curr_val;
}

float triangle(float x[],
               const short y_idx,
               const unsigned short window_radius) {
    
    const short j = y_idx;

    const unsigned short R = window_radius;

    if (j < 0) {
        return 0;
    } else if (j < R) {
        return x[j] * (x[j] - x[j + R]);
    } else {
        return (x[j] - x[j - R]) * (x[j] - x[j + R]);
    }
}

unsigned short max_index(float arr[],
                         unsigned short start_idx,
                         unsigned short end_idx) {

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

int rounded(float value) {
    if (value >= 0) {
        return (int)(value + 0.5f);
    } else {
        return (int)(value - 0.5f);
    }
}

int main() {
    // Open the data and output files
    FILE *input_file = fopen(DATA_FILE, "r");
    FILE *output_file = fopen(OUTPUT_FILE, "w");

    // Signal buffers
    float x    [BUFF_SIZE],
          x_bar[BUFF_SIZE],
          y    [BUFF_SIZE],
          t    [BUFF_SIZE],
          l    [BUFF_SIZE],
          ma   [BUFF_SIZE];
    
    // Signal values
    float x_val     = 0,
          x_bar_val = 0,
          y_hat_val = 0,
          y_val     = 0,
          t_val     = 0,
          l_val     = 0,
          ma_val    = 0,
          theta     = 0,
          th_val    = 0,
          bpm       = -1;

    // Indices
    short i             = -1,
          i_x           = 0,
          i_x_bar       = i_x     - (N + 1),
          i_t           = i_x_bar - (S + 1),
          i_l           = i_t     - (L + 1),
          i_ma          = i_l     - (M + 1),

          aoi           = 0,
          prev_aoi      = 0,
          i_onset       = -1,
          i_offset      = -1,
          i_cand_max    = -1,
          i_curr_max    = -1,
          i_prev_max    = -1,
          rr            = -1;

    // Clear buffers
    for (i = 0; i < BUFF_SIZE; i++) {
        x[i]     = 0;
        x_bar[i] = 0;
        y[i]     = 0;
        t[i]     = 0;
        l[i]     = 0;
        ma[i]    = 0;
    }

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%f,%*s", &x_val) != EOF) {

        // If the ECG buffer is full,
        if (i_x == BUFF_SIZE) {

            // Shift the buffers
            for (i = 0; i < BUFF_SIZE - 1; i++) {
                x    [i] = x    [i + 1];
                x_bar[i] = x_bar[i + 1];
                y    [i] = y    [i + 1];
                t    [i] = t    [i + 1];
                l    [i] = l    [i + 1];
                ma   [i] = ma   [i + 1];
            }
            // Decrement the indices
            i_x--;
            i_x_bar--;
            i_t--;
            i_l--;
            i_ma--;
            i_onset--;
            i_curr_max--;
            i_prev_max--;
        }
        x[i_x] = x_val;

        x_bar_val = movavg(x_bar_val, x, i_x, N);
        if (i_x_bar >= N) {
            y_hat_val = x_val - x_bar_val;
            y_val     = ABS(y_hat_val);

            x_bar[i_x_bar] = x_bar_val;
            y    [i_x_bar] = y_val;
        }

        t_val = triangle(y, i_t, S);
        if (i_t >= 0) {
            t[i_t] = t_val;
        }

        l_val = movavg(l_val, t, i_t, L);
        if (i_l >= 0) {
            l[i_l] = l_val;
        }

        ma_val = movavg(ma_val, l, i_l, M);
        if (i_ma >= 0) {
            ma[i_ma] = ma_val;

            theta = ma_val / 4;
            th_val   = BETA*ma_val + theta;

            prev_aoi = aoi;
            aoi = l_val >= th_val ? 1 : 0;
        }

        if (aoi - prev_aoi == 1) {
            i_onset = i_x_bar;
        } else if (aoi - prev_aoi == -1) {
            i_offset = i_x_bar;
            i_cand_max = max_index(y, i_onset, i_offset);

            if (i_curr_max < 0) {
                i_curr_max = i_cand_max;
            } else {
                if (i_cand_max - i_curr_max < MIN_RR_DIST) {
                    if (y[i_cand_max] > y[i_curr_max]) {
                        i_curr_max = i_cand_max;
                    } 
                } else {
                    i_prev_max = i_curr_max;
                    i_curr_max = i_cand_max;
                    rr = i_curr_max - i_prev_max;
                    bpm = 60.0 * FS / rr;
                    printf("%f\n", bpm);
                }
            }
        }
        fprintf(output_file,"%f,%f,%f,%f,%f,%i\n",x_val,y_val,t_val,l_val,th_val,aoi);

        i_x++;
        i_x_bar++;
        i_t++;
        i_l++;
        i_ma++;
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}