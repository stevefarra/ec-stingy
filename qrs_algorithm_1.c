#include <stdio.h>
#include <stdlib.h>

#define INPUT_FILE "scope.txt"
#define OUTPUT_FILE "filtered.txt"

#define BUFF_SIZE 1500
#define W1 35
#define W2 220
#define MOV_AVG_DELAY (W1 / 2)

float bandpass_b[] = {9.4131374e-04, 0, -2.8239412e-03, 0, 2.8239412e-03, 0, -9.4131374e-04};
float bandpass_a[] = {1.0000000, -5.4458894, 12.4912422, -15.445093, 10.858290, -4.1157299, 0.6572747};
int bandpass_order = 3;

void shift_buffer(float *buffer, int size) {
  for (int i = 1; i < size; i++) {
    buffer[i - 1] = buffer[i];
  }
}

float digital_filter(float *input, float *output, int idx, int filter_order, float *b, float *a) {
  float result = 0;
  for (int k = 0; k <= 2 * filter_order; k++) {
    if (idx - k >= 0) {
      if (k == 0) {
        result += b[k] * input[idx - k];
      } else {
        result += b[k] * input[idx - k] - a[k] * output[idx - k];
      }
    }
  }
  return result;
}

float mov_avg_filter(float *input, int idx, int W) {
  float sum = 0;
  int count = 0;
  int half_W = (W - 1) / 2;

  for (int i = idx - half_W; i <= idx + half_W; i++) {
    if (i >= 0 && i < BUFF_SIZE) {
      sum += input[i];
      count++;
    }
  }

  return sum / count;
}

int main() {
  FILE *input_file = fopen(INPUT_FILE, "r");
  FILE *output_file = fopen(OUTPUT_FILE, "w");

  if (input_file == NULL) {
    perror("Error opening input file");
    return EXIT_FAILURE;
  }
  if (output_file == NULL) {
    perror("Error opening output file");
    return EXIT_FAILURE;
  }

  float input[BUFF_SIZE] = {0};
  float bandpass[BUFF_SIZE] = {0};
  float squared[BUFF_SIZE] = {0};
  float qrs_mov_avg[BUFF_SIZE] = {0};
  float beat_mov_avg[BUFF_SIZE] = {0};

  float sample;

  int buffer_idx = 0;
  int mov_avg_idx = -MOV_AVG_DELAY;

  while (!feof(input_file)) {
    fscanf(input_file, "%f", &input[buffer_idx]);

    sample = digital_filter(input, bandpass, buffer_idx, bandpass_order, bandpass_b, bandpass_a);
    bandpass[buffer_idx] = sample;

    sample = sample * sample;
    squared[buffer_idx] = sample;

    if (mov_avg_idx >= 0) {
      sample = mov_avg_filter(squared, mov_avg_idx, W1);
      qrs_mov_avg[mov_avg_idx] = sample;

      fprintf(output_file, "%f\n", sample);

      sample = mov_avg_filter(squared, mov_avg_idx, W2);
      beat_mov_avg[mov_avg_idx] = sample;
    }

    if (buffer_idx == BUFF_SIZE - 1) {
      shift_buffer(input, BUFF_SIZE);
      shift_buffer(bandpass, BUFF_SIZE);
      shift_buffer(squared, BUFF_SIZE);
      shift_buffer(qrs_mov_avg, BUFF_SIZE);
    } else {
      buffer_idx++;
    }

    if (mov_avg_idx < BUFF_SIZE - 1) {
      mov_avg_idx++;
    }
  }

  fclose(input_file);
  fclose(output_file);

  return 0;
}
