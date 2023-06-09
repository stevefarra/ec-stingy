#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Macros */
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define WINDOW(R) (2 * (R) + 1)

/* Data-related constants */
#define DATA_FILE   "scope.csv"
#define OUTPUT_FILE "output.csv"
#define FS          360

/* QRS algorithm parameters */
#define N           25  // Moving average window size for high-pass filter
#define S           7   // Triangle template matching parameter
#define L           5   // Moving average window size for Low-pass filter
#define M           150 // Moving average window size for dynamic threshold calculation
#define BETA        2.5 // Dynamic threshold coefficient
#define MIN_RR_DIST (0.272 * FS)

/* Buffer parameters */
#define X_SIZE (WINDOW(N) + 1)
#define H_SIZE (FS * 2)
#define T_SIZE (WINDOW(L) + 1)
#define L1_SIZE (WINDOW(M) + 1)

unsigned short max_index(unsigned short arr[],
                         unsigned short start_idx,
                         unsigned short end_idx) {

    unsigned short max_value = arr[start_idx];
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
    unsigned short x[X_SIZE];
    unsigned short h[H_SIZE];
    float t[T_SIZE];
    float l1[L1_SIZE];

    unsigned short x_val = 0;
    float x_bar_val = 0;
    unsigned short h_val = 0;
    float t_val = 0;
    float l1_val = 0;
    float l2_val = 0;
    float th_val = 0;
    float theta;

    unsigned short i;
    unsigned short i_x = 0;
    unsigned short i_h = 0;
    unsigned short i_t = 0;
    unsigned short i_l1 = 0;

    unsigned char prev_aoi;
    unsigned char aoi = 0;
    
    unsigned short i_onset;
    unsigned short i_offset;
    unsigned short i_cand_max;
    unsigned short i_curr_max = 0;
    unsigned short i_prev_max;

    float rr;
    float bpm;

    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%i,%*s", &x_val) != EOF) {
        if (i_x == X_SIZE) {
            for (i = 0; i < X_SIZE - 1; i++) {
                x[i] = x[i + 1];
            }
            i_x--;
        }
        x[i_x] = x_val;
        i_x++;

        x_bar_val += (float) x_val / WINDOW(N);
        if (i_x > WINDOW(N)) {
            x_bar_val -= (float) x[0] / WINDOW(N);
            h_val = ABS(x_val - x_bar_val);

            if (i_h == H_SIZE) {
                for (i = 0; i < H_SIZE - 1; i++) {
                    h[i] = h[i + 1];
                }
                i_h--;
                i_onset--;
                i_curr_max--;
                i_prev_max--;
            }
            h[i_h] = h_val;
            i_h++;

            if (i_h >= WINDOW(S)) {
                t_val = (h[(i_h - 1) - S] - h[(i_h - 1) - (2 * S)]) * (h[(i_h - 1) - S] - h[i_h - 1]);

                if (i_t == T_SIZE) {
                    for (i = 0; i < T_SIZE - 1; i++) {
                        t[i] = t[i + 1];
                    }
                    i_t--;
                }
                t[i_t] = t_val;
                i_t++;

                l1_val += t_val / WINDOW(L);
                if (i_t > WINDOW(L)) {
                    l1_val -= t[0] / WINDOW(L);

                    if (i_l1 == L1_SIZE) {
                        for (i = 0; i < L1_SIZE - 1; i++) {
                            l1[i] = l1[i + 1];
                        }
                        i_l1--;
                    }
                    l1[i_l1] = l1_val;
                    i_l1++;

                    l2_val += l1_val / WINDOW(M);
                    if (i_l1 > WINDOW(M)) {
                        l2_val -= l1[0] / WINDOW(M);
                    }
                    if (i_l1 > M + 1) {
                        theta = 0.25 * l2_val;
                        th_val = BETA*l2_val + theta;

                        prev_aoi = aoi;
                        aoi = l1_val >= th_val ? 1 : 0;

                        if (aoi - prev_aoi == 1) {
                            i_onset = i_h;
                        } else if (aoi - prev_aoi == -1) {
                            i_offset = i_h;
                            i_cand_max = max_index(h, i_onset, i_offset);

                            if (i_curr_max == 0) {
                                i_curr_max = i_cand_max;
                            } else {
                                if (i_cand_max - i_curr_max < MIN_RR_DIST) {
                                    if (h[i_cand_max] > h[i_curr_max]) {
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
                    }
                }
            }
        }
        fprintf(output_file, "%i,%i,%f,%f,%f,%i,%f\n", x_val, h_val, t_val, l1_val, th_val, aoi);
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}