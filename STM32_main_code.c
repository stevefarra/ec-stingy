  /* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

/* USER CODE BEGIN PD */
#define TIM6_FREQ 1E6
#define FS 360.0

#define BUFF_SIZE 1080
#define N         25  // Moving average window size for high-pass filter
#define S         7   // Triangle template matching parameter
#define L         5   // Moving average window size for Low-pass filter
#define BETA      2.5 // Dynamic threshold coefficient
#define M         150 // Moving average window size for dynamic threshold calculation

#define MIN_RR_DIST 0.272*FS

#define ABS(x) ((x) < 0 ? -(x) : (x))

/* USER CODE BEGIN PFP */
float movavg(float curr_val,
             float x[],
             const short x_idx,
             const unsigned short window_radius);

float triangle(float x[],
               const short y_idx,
               const unsigned short window_radius);

unsigned short max_index(float arr[],
                         unsigned short start_idx,
                         unsigned short end_idx);

int rounded(float value);

  /* USER CODE BEGIN 1 */
  uint16_t tim6_val;
  char msg[10];

  float x[BUFF_SIZE];

  float x_val = 0;

  short i = -1,
		i_x = 0;

  /* USER CODE BEGIN Init */
  for (i = 0; i < BUFF_SIZE; i++) {
	  x[i] = 0;
  }
  
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (__HAL_TIM_GET_COUNTER(&htim6) - tim6_val >= TIM6_FREQ / FS) {
		  HAL_ADC_Start(&hadc1);
		  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		  x_val = HAL_ADC_GetValue(&hadc1);

		  if (i_x == BUFF_SIZE) {
			  for (i = 0; i < BUFF_SIZE - 1; i++) {
				  x[i] = x[i + 1];
			  }
			  i_x--;
		  }
		  x[i_x] = x_val;

		  sprintf(msg, "%f\r\n", x_val);
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

		  tim6_val = __HAL_TIM_GET_COUNTER(&htim6);
	  }

      /* USER CODE BEGIN 4 */
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