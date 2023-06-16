/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body. All of the user-generated code for
  	  	  	  	  	  this STM32CubeIDE project is contained in this file,
  	  	  	  	  	  which closely follows the program contained in the
  	  	  	  	  	  algorithm/algorithm.c file of this repository. Comments
  	  	  	  	  	  are provided primarily for implementation details
  	  	  	  	  	  unique to the STM32 version; more details can be found in
  	  	  	  	  	  the aforementioned C version.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/*
 * Used to print data to the serial buffer before it's transmitted through
 * USART.
 */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*
 * Included before the designated macro section since some pre-defined
 * constants make use of it.
 */
#define WINDOW(R) (2 * (R) + 1)

/*
 * The remaining predefined constants are identical to those found in
 * algorithm/algorithm.c.
 */
#define FS 360

#define N 25
#define S 7
#define L 5
#define M 150
#define BETA 2.5

#define NOTCHED_SIZE (WINDOW(N) + 1)
#define H_SIZE (FS * 2)
#define T_SIZE (WINDOW(L) + 1)
#define L1_SIZE (WINDOW(M) + 1)

#define MIN_RR_DIST (0.272 * FS)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ABS(x) ((x) < 0 ? -(x) : (x))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
static float B[] = {0.9576, -0.9864, 0.9576};
static float A[] = {0, -0.9864, 0.9153};

/*
 * Indicates whether the sample period has elapsed. Once triggered by an
 * interrupt, the superloop reads a sample and processes it.
 */
uint8_t fs_elapsed_flag = 0;

/*
 * Indicates whether a new R-peak has been detected. This triggers a bpm
 * reading transmitted through the serial port and flickers the LED.
 */
uint8_t r_peak_detected_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
/*
 * These two functions are identical in implementation to their algorithm.c
 * counterparts.
 */
uint16_t max_index(uint16_t arr[], uint16_t start_idx, uint16_t end_idx);
uint8_t rounded(float num);

/*
 * Callback function for the USART DMA.
 * Polling is too slow for our chosen sample rate.
 */
void DMATransferComplete(DMA_HandleTypeDef *hdma);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 * This project makes use of Charles Nicholson's nanoprintf library, the
 * smallest public printf implementation for its feature set. This version uses
 * a lot less memory and is considerably faster than the stdio version of
 * snprintf(), which is used to write to the USART buffer in this program.
 */
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	/* USART buffer. */
	char msg[20];

	uint16_t x[3];
	float y[3];
	uint16_t notched[NOTCHED_SIZE];
	uint16_t h[H_SIZE];
	float t[T_SIZE];
	float l1[L1_SIZE];

	memset(x, 0, sizeof(x));
	memset(y, 0, sizeof(y));

	uint16_t input;
	float y_val;
	uint16_t notched_val;
	float notched_bar_val = 0;
	short h_hat_val;
	uint16_t h_val;

	/*
	 * The triangle filter matching template computation runs into an overflow
	 * issue on this platform, so it's broken into two parts using these
	 * variables before they're multiplied together.
	 */
	float t_val1, t_val2;

	float t_val;
	float l1_val = 0;
	float l2_val = 0;
	float th_val;
	float theta;

	uint16_t i;
	uint16_t j;
	uint16_t i_notched = 0;
	uint16_t i_h = 0;
	uint16_t i_t = 0;
	uint16_t i_l1 = 0;

	uint8_t prev_aoi;
	uint8_t aoi = 0;

	uint16_t i_onset;
	uint16_t i_offset;
	uint16_t i_cand_max;
	uint16_t i_curr_max = 0;
	uint16_t i_prev_max;

	uint16_t rr;
	float bpm = 0;

	/*
	 * Keeps track of flicker duration for the LED that triggers when
	 * an R-peak has been detected.
	 */
	uint8_t led_counter = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  /* Required for USART DMA. */
  HAL_DMA_RegisterCallback(&hdma_usart2_tx, HAL_DMA_XFER_CPLT_CB_ID, &DMATransferComplete);

  /* Timer used for the super loop. Triggers every sample period. */
  HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (fs_elapsed_flag) {
		  /*
		   * Once the sample period corresponding to FS has triggered, immediately
		   * disable the flag.
		   */
		  fs_elapsed_flag = 0;

		  if (led_counter > 0) {
			  /*
			   * If led_counter has been incremented, then the R-peak detection flag
			   * was recently raised. Keep incrementing the counter until the threshold
			   * is reached, at which point the LED is turned off and the counter reset.
			   */
			  if (led_counter == (FS / 20) + 1) {
				  led_counter = 0;
				  HAL_GPIO_WritePin(Green_Led_GPIO_Port, Green_Led_Pin, GPIO_PIN_RESET);
			  } else {
				  led_counter++;
			  }
		  }

		  if (HAL_GPIO_ReadPin(AD8232_LOD_GPIO_Port, AD8232_LOD_Pin) == GPIO_PIN_SET) {
			  /*
			   * The AD8232's LOD pin indicates whether at least one of the leads are
			   * disconnected. In this case, the necessary memory buffers, moving
			   * average accumulators, buffer indices, flags, and outgoing signals
			   * are cleared until valid data is known to be available at the ADC.
			   */
			  memset(x, 0, sizeof(x));
			  memset(y, 0, sizeof(y));
			  notched_bar_val = 0;
			  l1_val = 0;
			  l2_val = 0;
			  i_notched = 0;
			  i_h = 0;
			  i_t = 0;
			  i_l1 = 0;
			  aoi = 0;
			  i_curr_max = 0;
			  bpm = 0;
		  } else {
			  /*
			   * Once the leads are connected, a sample is read by the ADC.
			   * Polling is plenty fast to meet the timing requirements set by
			   * this algorithm's sampling frequency and the additional overhead
			   * required for interfacing with DMA is avoided.
			   */
			  HAL_ADC_Start(&hadc1);
			  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
			  input = HAL_ADC_GetValue(&hadc1);

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

			  notched_val = (uint16_t) y_val;
			  if (i_notched == NOTCHED_SIZE) {
				  for (i = 0; i < NOTCHED_SIZE - 1; i++) {
					  notched[i] = notched[i + 1];
				  }
				  i_notched--;
			  }
			  notched[i_notched++] = notched_val;

			  notched_bar_val += (float) notched_val / WINDOW(N);
			  if (i_notched > WINDOW(N)) {
				  notched_bar_val -= (float) notched[0] / WINDOW(N);

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

					  /*
					   * As mentioned earlier, the triangle filter matching template
					   * computation requires two variables to be computed on this platform.
					   * the two factors of the expression are stored individually before
					   * being multiplied.
					   */
					  t_val1 = h[j - S] - h[j - (2 * S)];
					  t_val2 = h[j - S] - h[j];
					  t_val = t_val1 * t_val2;

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
										  /*
										   * Raise a flag to transmit the new bpm
										   * reading and trigger the LED.
										   */
										  r_peak_detected_flag = 1;

										  i_prev_max = i_curr_max;
										  i_curr_max = i_cand_max;
										  rr = i_curr_max - i_prev_max;
										  bpm = 60.0 * FS / (float) rr;
									  }
								  }
							  }
						  }
					  }
				  }
				  if (r_peak_detected_flag) {
					  /* Immediately disable the flag. */
					  r_peak_detected_flag = 0;

					  /*
					   * Trigger the LED and increment the counter. A non-zero
					   * counter value means the LED is on.
					   */
					  HAL_GPIO_WritePin(Green_Led_GPIO_Port, Green_Led_Pin, GPIO_PIN_SET);
					  led_counter++;

					  /*
					   * Transmit the new bpm reading in addition to the filtered
					   * ECG signal.
					   */
					  npf_snprintf(msg, 20, "%hi,%hu\r\n", h_hat_val, rounded(bpm));
					  huart2.Instance->CR3 |= USART_CR3_DMAT;
					  HAL_DMA_Start_IT(&hdma_usart2_tx, (uint32_t)msg, (uint32_t)&huart2.Instance->TDR, strlen(msg));
				  } else {
					  /*
					   * There is no new R-peak detected so the bpm reading is unchanged.
					   * Transmit only the ECG signal.
					   */
					  npf_snprintf(msg, 20, "%hi\r\n", h_hat_val);
					  huart2.Instance->CR3 |= USART_CR3_DMAT;
					  HAL_DMA_Start_IT(&hdma_usart2_tx, (uint32_t)msg, (uint32_t)&huart2.Instance->TDR, strlen(msg));
				  }
			  }
		  }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 421;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 422;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Green_Led_GPIO_Port, Green_Led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Green_Led_Pin */
  GPIO_InitStruct.Pin = Green_Led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Green_Led_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AD8232_LOD_Pin */
  GPIO_InitStruct.Pin = AD8232_LOD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(AD8232_LOD_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint16_t max_index(uint16_t arr[], uint16_t start_idx, uint16_t end_idx) {

    uint16_t max_value = arr[start_idx];
    uint16_t max_index = start_idx;

    for (uint16_t i = start_idx + 1; i <= end_idx; i++) {
        if (arr[i] > max_value) {
            max_value = arr[i];
            max_index = i;
        }
    }

    return max_index;
}
uint8_t rounded(float num) {
	if (num < 0.0f) {
		return 0;
	} else if (num > 255.0f) {
		return 255;
	} else {
		uint8_t int_part = (uint8_t) num;
		float frac_part = num - int_part;

		if (frac_part >= 0.5f) {
			int_part++;
		}
		return int_part;
	}
}

void DMATransferComplete(DMA_HandleTypeDef *hdma) {
  // Disable UART DMA mode
  huart2.Instance->CR3 &= ~USART_CR3_DMAT;
}

/* Triggers the main() superloop using a flag. */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		fs_elapsed_flag = 1;
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
