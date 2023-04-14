/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

#include <string.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef uint16_t dataType;
typedef enum {false, true} bool;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MHZ_TIMER &htim6
#define ECG_ADC &hadc1
#define SIG_UART &huart2

#define WINDOWSIZE 20   // Integrator window size, in samples. The article recommends 150ms. So, FS*0.15.
						// However, you should check empirically if the waveform looks ok.
#define NOSAMPLE -32000 // An indicator that there are no more samples to read. Use an impossible value for a sample.
#define FS 360          // Sampling frequency.
#define BUFFSIZE 600    // The size of the buffers (in samples). Must fit more than 1.66 times an RR interval, which
                        // typically could be around 1 second.

#define DELAY 22		// Delay introduced by the filters. Filter only output samples after this one.
						// Set to 0 if you want to keep the delay. Fixing the delay results in DELAY less samples
						// in the final end result.

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint16_t loop_counter;

char msg[10];
dataType data;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

void init_loop_counter(void);
bool loop_triggered(void);
void reset_loop_counter(void);

dataType input(void);
void output(dataType data);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// The signal array is where the most recent samples are kept. The other arrays are the outputs of each
// filtering module: DC Block, low pass, high pass, integral etc.
// The output is a buffer where we can change a previous result (using a back search) before outputting.
dataType signal[BUFFSIZE], dcblock[BUFFSIZE], lowpass[BUFFSIZE], highpass[BUFFSIZE], derivative[BUFFSIZE], squared[BUFFSIZE], integral[BUFFSIZE], outputSignal[BUFFSIZE];

// rr1 holds the last 8 RR intervals. rr2 holds the last 8 RR intervals between rrlow and rrhigh.
// rravg1 is the rr1 average, rr2 is the rravg2. rrlow = 0.92*rravg2, rrhigh = 1.08*rravg2 and rrmiss = 1.16*rravg2.
// rrlow is the lowest RR-interval considered normal for the current heart beat, while rrhigh is the highest.
// rrmiss is the longest that it would be expected until a new QRS is detected. If none is detected for such
// a long interval, the thresholds must be adjusted.
int rr1[8], rr2[8], rravg1, rravg2, rrlow = 0, rrhigh = 0, rrmiss = 0;

// i and j are iterators for loops.
// sample counts how many samples have been read so far.
// lastQRS stores which was the last sample read when the last R sample was triggered.
// lastSlope stores the value of the squared slope when the last R sample was triggered.
// currentSlope helps calculate the max. square slope for the present sample.
// These are all long unsigned int so that very long signals can be read without messing the count.
long unsigned int i, j, sample = 0, lastQRS = 0, lastSlope = 0, currentSlope = 0;

// This variable is used as an index to work with the signal buffers. If the buffers still aren't
// completely filled, it shows the last filled position. Once the buffers are full, it'll always
// show the last position, and new samples will make the buffers shift, discarding the oldest
// sample and storing the newest one on the last position.
int current;

// There are the variables from the original Pan-Tompkins algorithm.
// The ones ending in _i correspond to values from the integrator.
// The ones ending in _f correspond to values from the DC-block/low-pass/high-pass filtered signal.
// The peak variables are peak candidates: signal values above the thresholds.
// The threshold 1 variables are the threshold variables. If a signal sample is higher than this threshold, it's a peak.
// The threshold 2 variables are half the threshold 1 ones. They're used for a back search when no peak is detected for too long.
// The spk and npk variables are, respectively, running estimates of signal and noise peaks.
dataType peak_i = 0, peak_f = 0, threshold_i1 = 0, threshold_i2 = 0, threshold_f1 = 0, threshold_f2 = 0, spk_i = 0, spk_f = 0, npk_i = 0, npk_f = 0;

// qrs tells whether there was a detection or not.
// regular tells whether the heart pace is regular or not.
// prevRegular tells whether the heart beat was regular before the newest RR-interval was calculated.
// firstPeakOutput tells whether a detection is being sent to the output for the first time.
bool qrs, regular = true, prevRegular, firstPeakOutput = true;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	// Initializing the RR averages
	for (i = 0; i < 8; i++)
	{
		rr1[i] = 0;
		rr2[i] = 0;
	}

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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  init_loop_counter();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (loop_triggered()) {
		  reset_loop_counter();

		  // Test if the buffers are full.
		  // If they are, shift them, discarding the oldest sample and adding the new one at the end.
		  // Else, just put the newest sample in the next free position.
		  // Update 'current' so that the program knows where's the newest sample.
		  if (sample >= BUFFSIZE)
		  {
		  	for (i = 0; i < BUFFSIZE - 1; i++)
		  	{
		  		signal[i] = signal[i+1];
		  		dcblock[i] = dcblock[i+1];
		  		lowpass[i] = lowpass[i+1];
		  		highpass[i] = highpass[i+1];
		  		derivative[i] = derivative[i+1];
		  		squared[i] = squared[i+1];
		  		integral[i] = integral[i+1];
		  		outputSignal[i] = outputSignal[i+1];
		  	}
		  	current = BUFFSIZE - 1;
		  }
		  else
		  {
		  	current = sample;
		  }
		  signal[current] = input();

		  sprintf(msg, "%hu\r\n", signal[current]);
		  HAL_UART_Transmit(SIG_UART, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

//		  // DC Block filter
//		  // This was not proposed on the original paper.
//		  // It is not necessary and can be removed if your sensor or database has no DC noise.
//		  if (current >= 1)
//		  	dcblock[current] = signal[current] - signal[current-1] + 0.995*dcblock[current-1];
//		  else
//		  	dcblock[current] = 0;
//
//		  // Low Pass filter
//		  // Implemented as proposed by the original paper.
//		  // y(nT) = 2y(nT - T) - y(nT - 2T) + x(nT) - 2x(nT - 6T) + x(nT - 12T)
//		  // Can be removed if your signal was previously filtered, or replaced by a different filter.
//		  lowpass[current] = dcblock[current];
//		  if (current >= 1)
//		  	lowpass[current] += 2*lowpass[current-1];
//		  if (current >= 2)
//		  	lowpass[current] -= lowpass[current-2];
//		  if (current >= 6)
//		  	lowpass[current] -= 2*dcblock[current-6];
//		  if (current >= 12)
//		  	lowpass[current] += dcblock[current-12];
//
//		  // High Pass filter
//		  // Implemented as proposed by the original paper.
//		  // y(nT) = 32x(nT - 16T) - [y(nT - T) + x(nT) - x(nT - 32T)]
//		  // Can be removed if your signal was previously filtered, or replaced by a different filter.
//		  highpass[current] = -lowpass[current];
//		  if (current >= 1)
//		  	highpass[current] -= highpass[current-1];
//		  if (current >= 16)
//		  	highpass[current] += 32*lowpass[current-16];
//		  if (current >= 32)
//		  	highpass[current] += lowpass[current-32];
//
//		  // Derivative filter
//		  // This is an alternative implementation, the central difference method.
//		  // f'(a) = [f(a+h) - f(a-h)]/2h
//		  // The original formula used by Pan-Tompkins was:
//		  // y(nT) = (1/8T)[-x(nT - 2T) - 2x(nT - T) + 2x(nT + T) + x(nT + 2T)]
//		  derivative[current] = highpass[current];
//		  if (current > 0)
//		  	derivative[current] -= highpass[current-1];
//
//		  // This just squares the derivative, to get rid of negative values and emphasize high frequencies.
//		  // y(nT) = [x(nT)]^2.
//		  squared[current] = derivative[current]*derivative[current];
//
//		  // Moving-Window Integration
//		  // Implemented as proposed by the original paper.
//		  // y(nT) = (1/N)[x(nT - (N - 1)T) + x(nT - (N - 2)T) + ... x(nT)]
//		  // WINDOWSIZE, in samples, must be defined so that the window is ~150ms.
//
//		  integral[current] = 0;
//		  for (i = 0; i < WINDOWSIZE; i++)
//		  {
//		  	if (current >= (dataType)i)
//		  		integral[current] += squared[current - i];
//		  	else
//		  		break;
//		  }
//		  integral[current] /= (dataType)i;
//
//		  qrs = false;
//
//		  // If the current signal is above one of the thresholds (integral or filtered signal), it's a peak candidate.
//		  if (integral[current] >= threshold_i1 || highpass[current] >= threshold_f1)
//		  {
//		  	peak_i = integral[current];
//		  	peak_f = highpass[current];
//		  }
//
//		  // If both the integral and the signal are above their thresholds, they're probably signal peaks.
//		  if ((integral[current] >= threshold_i1) && (highpass[current] >= threshold_f1))
//		  {
//		  	// There's a 200ms latency. If the new peak respects this condition, we can keep testing.
//		  	if (sample > lastQRS + FS/5)
//		  	{
//		  		// If it respects the 200ms latency, but it doesn't respect the 360ms latency, we check the slope.
//		  		if (sample <= lastQRS + (long unsigned int)(0.36*FS))
//		  		{
//		  			// The squared slope is "M" shaped. So we have to check nearby samples to make sure we're really looking
//		  			// at its peak value, rather than a low one.
//		  			currentSlope = 0;
//		  			for (j = current - 10; j <= current; j++)
//		  				if (squared[j] > currentSlope)
//		  					currentSlope = squared[j];
//
//		  			if (currentSlope <= (dataType)(lastSlope/2))
//		  			{
//		  				qrs = false;
//		  			}
//
//		  			else
//		  			{
//		  				spk_i = 0.125*peak_i + 0.875*spk_i;
//		  				threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
//		  				threshold_i2 = 0.5*threshold_i1;
//
//		  				spk_f = 0.125*peak_f + 0.875*spk_f;
//		  				threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
//		  				threshold_f2 = 0.5*threshold_f1;
//
//		  				lastSlope = currentSlope;
//		  				qrs = true;
//		  			}
//		  		}
//		  		// If it was above both thresholds and respects both latency periods, it certainly is a R peak.
//		  		else
//		  		{
//		  			currentSlope = 0;
//		  			for (j = current - 10; j <= current; j++)
//		  				if (squared[j] > currentSlope)
//		  					currentSlope = squared[j];
//
//		  			spk_i = 0.125*peak_i + 0.875*spk_i;
//		  			threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
//		  			threshold_i2 = 0.5*threshold_i1;
//
//		  			spk_f = 0.125*peak_f + 0.875*spk_f;
//		  			threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
//		  			threshold_f2 = 0.5*threshold_f1;
//
//		  			lastSlope = currentSlope;
//		  			qrs = true;
//		  		}
//		  	}
//		  	// If the new peak doesn't respect the 200ms latency, it's noise. Update thresholds and move on to the next sample.
//		  	else
//		  	{
//		  		peak_i = integral[current];
//		  		npk_i = 0.125*peak_i + 0.875*npk_i;
//		  		threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
//		  		threshold_i2 = 0.5*threshold_i1;
//		  		peak_f = highpass[current];
//		  		npk_f = 0.125*peak_f + 0.875*npk_f;
//		  		threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
//		  		threshold_f2 = 0.5*threshold_f1;
//		  		qrs = false;
//		  		outputSignal[current] = qrs;
//		  		if (sample > DELAY + BUFFSIZE)
//		  			output(outputSignal[0]);
//		  		continue;
//		  	}
//
//		  }
//
//		  // If a R-peak was detected, the RR-averages must be updated.
//		  if (qrs)
//		  {
//		  	// Add the newest RR-interval to the buffer and get the new average.
//		  	rravg1 = 0;
//		  	for (i = 0; i < 7; i++)
//		  	{
//		  		rr1[i] = rr1[i+1];
//		  		rravg1 += rr1[i];
//		  	}
//		  	rr1[7] = sample - lastQRS;
//		  	lastQRS = sample;
//		  	rravg1 += rr1[7];
//		  	rravg1 *= 0.125;
//
//		  	// If the newly-discovered RR-average is normal, add it to the "normal" buffer and get the new "normal" average.
//		  	// Update the "normal" beat parameters.
//		  	if ( (rr1[7] >= rrlow) && (rr1[7] <= rrhigh) )
//		  	{
//		  		rravg2 = 0;
//		  		for (i = 0; i < 7; i++)
//		  		{
//		  			rr2[i] = rr2[i+1];
//		  			rravg2 += rr2[i];
//		  		}
//		  		rr2[7] = rr1[7];
//		  		rravg2 += rr2[7];
//		  		rravg2 *= 0.125;
//		  		rrlow = 0.92*rravg2;
//		  		rrhigh = 1.16*rravg2;
//		  		rrmiss = 1.66*rravg2;
//		  	}
//
//		  	prevRegular = regular;
//		  	if (rravg1 == rravg2)
//		  	{
//		  		regular = true;
//		  	}
//		  	// If the beat had been normal but turned odd, change the thresholds.
//		  	else
//		  	{
//		  		regular = false;
//		  		if (prevRegular)
//		  		{
//		  			threshold_i1 /= 2;
//		  			threshold_f1 /= 2;
//		  		}
//		  	}
//		  }
//		  // If no R-peak was detected, it's important to check how long it's been since the last detection.
//		  else
//		  {
//		  	// If no R-peak was detected for too long, use the lighter thresholds and do a back search.
//		  	// However, the back search must respect the 200ms limit and the 360ms one (check the slope).
//		  	if ((sample - lastQRS > (long unsigned int)rrmiss) && (sample > lastQRS + FS/5))
//		  	{
//		  		for (i = current - (sample - lastQRS) + FS/5; i < (long unsigned int)current; i++)
//		  		{
//		  			if ( (integral[i] > threshold_i2) && (highpass[i] > threshold_f2))
//		  			{
//		  				currentSlope = 0;
//		  				for (j = i - 10; j <= i; j++)
//		  					if (squared[j] > currentSlope)
//		  						currentSlope = squared[j];
//
//		  				if ((currentSlope < (dataType)(lastSlope/2)) && (i + sample) < lastQRS + 0.36*lastQRS)
//		  				{
//		  					qrs = false;
//		  				}
//		  				else
//		  				{
//		  					peak_i = integral[i];
//		  					peak_f = highpass[i];
//		  					spk_i = 0.25*peak_i+ 0.75*spk_i;
//		  					spk_f = 0.25*peak_f + 0.75*spk_f;
//		  					threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
//		  					threshold_i2 = 0.5*threshold_i1;
//		  					lastSlope = currentSlope;
//		  					threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
//		  					threshold_f2 = 0.5*threshold_f1;
//		  					// If a signal peak was detected on the back search, the RR attributes must be updated.
//		  					// This is the same thing done when a peak is detected on the first try.
//		  					//RR Average 1
//		  					rravg1 = 0;
//		  					for (j = 0; j < 7; j++)
//		  					{
//		  						rr1[j] = rr1[j+1];
//		  						rravg1 += rr1[j];
//		  					}
//		  					rr1[7] = sample - (current - i) - lastQRS;
//		  					qrs = true;
//		  					lastQRS = sample - (current - i);
//		  					rravg1 += rr1[7];
//		  					rravg1 *= 0.125;
//
//		  					//RR Average 2
//		  					if ( (rr1[7] >= rrlow) && (rr1[7] <= rrhigh) )
//		  					{
//		  						rravg2 = 0;
//		  						for (i = 0; i < 7; i++)
//		  						{
//		  							rr2[i] = rr2[i+1];
//		  							rravg2 += rr2[i];
//		  						}
//		  						rr2[7] = rr1[7];
//		  						rravg2 += rr2[7];
//		  						rravg2 *= 0.125;
//		  						rrlow = 0.92*rravg2;
//		  						rrhigh = 1.16*rravg2;
//		  						rrmiss = 1.66*rravg2;
//		  					}
//
//		  					prevRegular = regular;
//		  					if (rravg1 == rravg2)
//		  					{
//		  						regular = true;
//		  					}
//		  					else
//		  					{
//		  						regular = false;
//		  						if (prevRegular)
//		  						{
//		  							threshold_i1 /= 2;
//		  							threshold_f1 /= 2;
//		  						}
//		  					}
//
//		  					break;
//		  				}
//		  			}
//		  		}
//
//		  		if (qrs)
//		  		{
//		  			outputSignal[current] = false;
//		  			outputSignal[i] = true;
//		  			if (sample > DELAY + BUFFSIZE)
//		  				output(outputSignal[0]);
//		  			continue;
//		  		}
//		  	}
//
//		  	// Definitely no signal peak was detected.
//		  	if (!qrs)
//		  	{
//		  		// If some kind of peak had been detected, then it's certainly a noise peak. Thresholds must be updated accordingly.
//		  		if ((integral[current] >= threshold_i1) || (highpass[current] >= threshold_f1))
//		  		{
//		  			peak_i = integral[current];
//		  			npk_i = 0.125*peak_i + 0.875*npk_i;
//		  			threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
//		  			threshold_i2 = 0.5*threshold_i1;
//		  			peak_f = highpass[current];
//		  			npk_f = 0.125*peak_f + 0.875*npk_f;
//		  			threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
//		  			threshold_f2 = 0.5*threshold_f1;
//		  		}
//		  	}
//		  }
//		  // The current implementation outputs '0' for every sample where no peak was detected,
//		  // and '1' for every sample where a peak was detected. It should be changed to fit
//		  // the desired application.
//		  // The 'if' accounts for the delay introduced by the filters: we only start outputting after the delay.
//		  // However, it updates a few samples back from the buffer. The reason is that if we update the detection
//		  // for the current sample, we might miss a peak that could've been found later by backsearching using
//		  // lighter thresholds. The final waveform output does match the original signal, though.
//		  outputSignal[current] = qrs;
//		  if (sample > DELAY + BUFFSIZE)
//		  	output(outputSignal[0]);
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
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
  htim6.Init.Prescaler = 16-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65536-1;
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
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|DEBUG_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PUSHBUTTON_Pin */
  GPIO_InitStruct.Pin = PUSHBUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PUSHBUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Pin DEBUG_Pin */
  GPIO_InitStruct.Pin = LED_Pin|DEBUG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void init_loop_counter(void) {

	// Start the timer being used for the 1 ms counter
	HAL_TIM_Base_Start(MHZ_TIMER);

	// Set the counter
	loop_counter = __HAL_TIM_GET_COUNTER(MHZ_TIMER);
}

bool loop_triggered(void) {
	return __HAL_TIM_GET_COUNTER(MHZ_TIMER) - loop_counter >= (1E6/FS) ? true : false;
}

void reset_loop_counter(void) {
	loop_counter = __HAL_TIM_GET_COUNTER(MHZ_TIMER);
}

dataType input(void) {
	dataType data;

	HAL_ADC_Start(ECG_ADC);
	HAL_ADC_PollForConversion(ECG_ADC, HAL_MAX_DELAY);
	data = (dataType) HAL_ADC_GetValue(ECG_ADC);
	HAL_ADC_Stop(ECG_ADC);

	return data;
}

void output(dataType data) {

	if (data) {
		HAL_GPIO_TogglePin(DEBUG_GPIO_Port, DEBUG_Pin);
	}

	return;
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
