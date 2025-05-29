/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart3;

#define BUFFER_LEN     10
#define LED_COUNT      5
#define BUTTON_COUNT   5
#define SEQUENCE_LEN   3

uint8_t RX_BUFFER[BUFFER_LEN] = { 0 };
uint8_t sequence[SEQUENCE_LEN];
uint8_t user_input[SEQUENCE_LEN];
uint8_t input_index = 0;
uint8_t game_running = 0;

GPIO_TypeDef *LED_PORTS[LED_COUNT] = {
GPIOE, GPIOE, GPIOE, GPIOE,
GPIOE };

uint16_t LED_PINS[LED_COUNT] = {
GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
GPIO_PIN_4 };

GPIO_TypeDef *BUTTON_PORTS[BUTTON_COUNT] = {
GPIOD, GPIOD, GPIOD, GPIOD, GPIOB };

uint16_t BUTTON_PINS[BUTTON_COUNT] = {
GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_8 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void simple_delay(void) {
	for (volatile int i = 0; i < 5000000; i++)
		;
}

void generate_sequence(void) {
	for (int i = 0; i < SEQUENCE_LEN; i++) {
		sequence[i] = rand() % LED_COUNT;
	}
}

void play_sequence(void) {
	for (int i = 0; i < SEQUENCE_LEN; i++) {
		HAL_GPIO_WritePin(LED_PORTS[sequence[i]], LED_PINS[sequence[i]],
				GPIO_PIN_SET);
		simple_delay();  // 500ms yerine
		HAL_GPIO_WritePin(LED_PORTS[sequence[i]], LED_PINS[sequence[i]],
				GPIO_PIN_RESET);
		simple_delay();  // 500ms yerine

	}
}

void show_result(uint8_t success) {
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < LED_COUNT; i++) {
			HAL_GPIO_WritePin(LED_PORTS[i], LED_PINS[i],
					success ? GPIO_PIN_SET : GPIO_PIN_RESET);

		}
		HAL_Delay(200);
		for (int i = 0; i < LED_COUNT; i++) {
			HAL_GPIO_WritePin(LED_PORTS[i], LED_PINS[i],
					success ? GPIO_PIN_RESET : GPIO_PIN_SET);

		}
		HAL_Delay(200);
	}
	// Tüm LED'leri kapat
	for (int i = 0; i < LED_COUNT; i++) {
		HAL_GPIO_WritePin(GPIOE, LED_PINS[i], GPIO_PIN_RESET);
	}
}

void play_buzzer(void) {
	for (int i = 0; i < 3; i++) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);  // Buzzer ON
		HAL_Delay(150);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // Buzzer OFF
		HAL_Delay(150);
	}
}

void check_user_input(void) {
	uint8_t success = memcmp(sequence, user_input, SEQUENCE_LEN) == 0;

	if (!success) {
		play_buzzer();  // yanlışsa buzzer çalsın
	}
	show_result(success);
	game_running = 0;

	char done_msg[] = "bitti\r\n";
	HAL_UART_Transmit(&huart3, (uint8_t*) done_msg, strlen(done_msg),
			HAL_MAX_DELAY);

}

void check_buttons(void) {
	if (!game_running)
		return;

	for (int i = 0; i < BUTTON_COUNT; i++) {
		if (HAL_GPIO_ReadPin(BUTTON_PORTS[i], BUTTON_PINS[i]) == GPIO_PIN_SET) {
			user_input[input_index++] = i;

			while (HAL_GPIO_ReadPin(BUTTON_PORTS[i], BUTTON_PINS[i])
					== GPIO_PIN_SET)
				; // basılı kalma önleme
			HAL_Delay(100); // debounce

			if (input_index == SEQUENCE_LEN) {
				check_user_input();
			}
		}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		if (strstr((char*) RX_BUFFER, "start") != NULL) {
			srand(HAL_GetTick());  // her seferinde farklı dizi
			generate_sequence();
			play_sequence();
			input_index = 0;
			game_running = 1;
		}
		memset(RX_BUFFER, 0, BUFFER_LEN);
		HAL_UART_Receive_IT(&huart3, RX_BUFFER, BUFFER_LEN);
	}
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

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
	MX_USART3_UART_Init();
	/* USER CODE BEGIN 2 */

	HAL_UART_Receive_IT(&huart3, RX_BUFFER, BUFFER_LEN);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		check_buttons();

	}
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
