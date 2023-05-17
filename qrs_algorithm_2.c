#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define MIN_RR_DIST 0.272*FS

#define WINDOW(R) (2*(R) + 1)
#define ABS(x) ((x) < 0 ? -(x) : (x))

float movavg(float x[],
             float y[],
             float curr_val,
             const short x_idx,
             const short y_idx,
             const unsigned short window_radius) {

    const short i = x_idx,
                j = y_idx;

    const unsigned short R = window_radius,
                         W = WINDOW(R);

    if (i <= W) {
        curr_val += x[i] / W;
    } else {
        curr_val = curr_val + (x[j + R] - x[j - (R + 1)]) / W;
    }
    return curr_val;
}

float triangle(float x[],
               float y[],
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

int main() {

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
          th_val    = 0;

    // Indices
    short i,
          i_x           = 0,
          i_x_bar       = i_x     - (N + 1),
          i_t           = i_x_bar - (S + 1),
          i_l           = i_t     - (L + 1),
          i_ma          = i_l     - (M + 1),
          i_onset,
          i_offset,
          i_curr_max    = -1,
          i_prev_max    = -1,
          aoi           = 0,
          prev_aoi      = 0;

    // Open the data and output files
    FILE *input_file = fopen(DATA_FILE, "r");
    FILE *output_file = fopen(OUTPUT_FILE, "w");

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
        }
        x[i_x] = x_val;

        x_bar_val = movavg(x, x_bar, x_bar_val, i_x, i_x_bar, N);
        if (i_x_bar >= WINDOW(N)) {
            y_hat_val = x_val - x_bar_val;
            y_val     = ABS(y_hat_val);

            x_bar[i_x_bar] = x_bar_val;
            y    [i_x_bar] = y_val;
        }

        t_val = triangle(y, t, i_t, S);
        if (i_t >= 0) {
            t[i_t] = t_val;
        }

        l_val = movavg(t, l, l_val, i_t, i_l, L);
        if (i_l >= 0) {
            l[i_l] = l_val;
        }

        ma_val = movavg(l, ma, ma_val, i_l, i_ma, M);
        if (i_ma >= 0) {
            ma[i_ma] = ma_val;
            theta = ma_val / 4;
            th_val   = BETA*ma_val + theta;
        }

        if (i_x >= 300) {
            prev_aoi = aoi;
            aoi = l_val >= th_val ? 1 : 0;
        }

        if (aoi - prev_aoi == 1) {
            i_onset = i_x_bar;
        } else if (aoi - prev_aoi == -1) {
            i_offset = i_x_bar;
            i_prev_max = i_curr_max;
            i_curr_max = max_index(y, i_onset, i_offset);

            if (i_curr_max - i_prev_max <= MIN_RR_DIST) {
                if (y[i_curr_max] >= y[i_prev_max]) {
                    // discard previous peak
                } else {
                    // discard current peak
                }
            }
        }
        
        i_x++;
        i_x_bar++;
        i_t++;
        i_l++;
        i_ma++;

        fprintf(output_file,
                "%f,%f,%f,%f,%f,%f,%i\n",
                x_val,
                x_bar_val,
                y_val,
                t_val,
                l_val,
                th_val,
                aoi
        );
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}
