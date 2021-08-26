/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ReceiveTransmit.h"

#include <stdio.h>
#include <string.h>
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"

#define tV_25   0.76            // Напряжение (в вольтах) на датчике при температуре 25 °C.
#define tSlope  0.0025          // �?зменение напряжения (в вольтах) при изменении температуры на градус.
#define Vref    3.3             // Образцовое напряжение АЦП (в вольтах).
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern uint16_t usSRegInBuf[];
extern uint16_t usSRegHoldBuf[];

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
float Result = 0, voltageADC = 0, meanADC = 0, AverageADC = 0;
float voltageADC2_IN8 = 0, meanADC2_IN8 = 0, AverageADC2_IN8 = 0;
float temp;
uint16_t MassifADC[5];
uint16_t MassifADC2_IN8[5];
uint32_t ADC_temperatura[10] = {0,};
uint32_t ADC_IN8[10] = {0,};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
   HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_9);
  for (int i = 0; i < 5; i++) {
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK) {   // Ожидание завершения преобразования.
      Result = HAL_ADC_GetValue(&hadc2);                        // Считывание с АЦП.
      MassifADC2_IN8[i] = Result;
      HAL_ADC_Stop(&hadc2);                                     // Остановка АЦП.
    }
  }
  AverageADC2_IN8 = 0;
  for (int i = 0; i < 5; i++) {
    AverageADC2_IN8 += MassifADC2_IN8[i];
  }
  meanADC2_IN8 = AverageADC2_IN8 / 5;
  voltageADC2_IN8 = (float) meanADC2_IN8 / 4096 * Vref;

  ST7735_Charger_v1(105, 0, Result / 42, 6, ST7735_WHITE);

  sprintf((char*) ADC_IN8, "%ld.%03d", (uint32_t)voltageADC2_IN8, (uint16_t)((voltageADC2_IN8 - (uint32_t)voltageADC2_IN8)*1000.) );
  ST7735_WriteString(0, 0, "ADC voltage", Font_7x10, ST7735_RED, ST7735_BLACK); 
  ST7735_WriteString(40, 10, "V", Font_7x10, ST7735_WHITE, ST7735_BLACK); 
  ST7735_WriteString(0, 10, (char*) ADC_IN8, Font_7x10, ST7735_WHITE, ST7735_BLACK); 

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
  for (int i = 0; i < 5; i++) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {   // Ожидание завершения преобразования.
      Result = HAL_ADC_GetValue(&hadc1);                      // Считывание с АЦП.
      MassifADC[i] = Result;
      HAL_ADC_Stop(&hadc1);                                   // Остановка АЦП.
    }
  }
  AverageADC = 0;
  for (int j = 0; j < 5; j++) {
    AverageADC += MassifADC[j];
  }
  meanADC = AverageADC / 5;
  voltageADC = (float) meanADC / 4096 * Vref;               // Напряжение в вольтах на датчике.
  temp = (voltageADC - tV_25) / tSlope + 25;                // Температура в градусах.
  Result = (float) temp;
  dataTransmit(0, Result);                                  // Передача данных Modbus registr 0.

  sprintf((char*) ADC_temperatura, "%ld.%03d", (uint32_t)Result, (uint16_t)((Result - (uint32_t)Result)*1000.) );
  ST7735_WriteString(0, 25, "CPU temperatura", Font_7x10, ST7735_CYAN, ST7735_BLACK);
  ST7735_WriteString(45, 35, "*C", Font_7x10, ST7735_WHITE, ST7735_BLACK);
  ST7735_WriteString(0, 35, (char*) ADC_temperatura, Font_7x10, ST7735_WHITE, ST7735_BLACK);

  usSRegInBuf[2] += 1;

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
