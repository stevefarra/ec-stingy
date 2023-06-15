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
#define MIN_RR_DIST (0.272 * FS) // Minimum distance between two RR peaks in seconds

/* Buffer parameters */
#define NOTCHED_SIZE (WINDOW(N) + 1)
#define H_SIZE (FS * 2)
#define T_SIZE (WINDOW(L) + 1)
#define L1_SIZE (WINDOW(M) + 1)

/* Notch filter coefficients */
static float B[] = {0.9175, -0.9451, 0.9175};
static float A[] = {0, -0.9451, 0.8350};

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
    unsigned short x[3];
    float y[3];
    unsigned short notched[NOTCHED_SIZE];
    unsigned short h[H_SIZE];
    float t[T_SIZE];
    float l1[L1_SIZE];

    // Buffer initializations
    memset(x, 0, sizeof(x));
    memset(y, 0, sizeof(y));

    // Signal values
    unsigned short input;
    float y_val;
    unsigned short notched_val;
    float notched_bar_val = 0;
    short h_hat_val = 0;
    unsigned short h_val = 0;
    float t_val = 0;
    float l1_val = 0;
    float l2_val = 0;
    float th_val = 0;
    float theta;

    // Buffer indices
    unsigned short i;
    unsigned short j;
    unsigned short i_notched = 0;
    unsigned short i_h = 0;
    unsigned short i_h_hat = 0;
    unsigned short i_ecg = 0;
    unsigned short i_t = 0;
    unsigned short i_l1 = 0;

    // Area of interest flags
    unsigned char prev_aoi;
    unsigned char aoi = 0;
    
    // Peak detection indices
    unsigned short i_onset;
    unsigned short i_offset;
    unsigned short i_cand_max;
    unsigned short i_curr_max = 0;
    unsigned short i_prev_max;

    float rr; // Distance (in samples) between most recent two RR peaks
    float bpm = 0; // beats per minute reading
    
    // Read data from file and store in buffer, then write to output file
    while (fscanf(input_file, "%i,%*s", &input) != EOF) {

        for (i = 0; i < 2; i++) {
            x[i] = x[i + 1];
            y[i] = y[i + 1];
        }
        x[2] = input;
    
        y_val = 0;
        for (i = 0; i < 3; i++) {
            y_val += B[i] * x[2 - i];
            y_val -= A[i] * y[2 - i];
        }
        y[2] = y_val;

        notched_val = (unsigned short) y_val;
        if (i_notched == NOTCHED_SIZE) {
            for (i = 0; i < NOTCHED_SIZE - 1; i++) {
                notched[i] = notched[i + 1];
            }
            i_notched--;
        }
        notched[i_notched++] = notched_val;

        notched_bar_val += (float) notched_val / WINDOW(N);
        if (i_notched > WINDOW(N)) {
            notched_bar_val -= (float) x[0] / WINDOW(N);

            h_hat_val = notched_val - notched_bar_val;

            h_val = ABS(h_hat_val);
            if (i_h == H_SIZE) {
                for (i = 0; i < H_SIZE - 1; i++) {
                    h[i] = h[i + 1];
                }
                i_h--;
                i_onset--;
                i_curr_max--;
                i_prev_max--;
            }
            h[i_h++] = h_val;

            if (i_h >= WINDOW(S)) {
                j = i_h - 1;
                t_val = (h[j - S] - h[j - (2 * S)]) * (h[j - S] - h[j]);

                if (i_t == T_SIZE) {
                    for (i = 0; i < T_SIZE - 1; i++) {
                        t[i] = t[i + 1];
                    }
                    i_t--;
                }
                t[i_t++] = t_val;

                l1_val += t_val / WINDOW(L);
                if (i_t > WINDOW(L)) {
                    l1_val -= t[0] / WINDOW(L);

                    if (i_l1 == L1_SIZE) {
                        for (i = 0; i < L1_SIZE - 1; i++) {
                            l1[i] = l1[i + 1];
                        }
                        i_l1--;
                    }
                    l1[i_l1++] = l1_val;

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
                                    bpm = 60.0 * FS / (float) rr;
                                    printf("bpm: %f\n", bpm);
                                }
                            }
                        }           
                    }
                }
            }
        }
        fprintf(output_file, "%i,%i,%f,%f,%f,%i\n", notched_val, h_val, t_val, l1_val, th_val, aoi);
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}