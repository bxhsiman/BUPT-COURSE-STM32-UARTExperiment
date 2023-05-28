/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN_RIGHT_Pin GPIO_PIN_13
#define BTN_RIGHT_GPIO_Port GPIOC
#define BEEP_Pin GPIO_PIN_0
#define BEEP_GPIO_Port GPIOA
#define BTN_CENTER_Pin GPIO_PIN_1
#define BTN_CENTER_GPIO_Port GPIOA
#define BTN_UP_Pin GPIO_PIN_2
#define BTN_UP_GPIO_Port GPIOA
#define BTN_LEFT_Pin GPIO_PIN_3
#define BTN_LEFT_GPIO_Port GPIOA
#define SEG_EN5_Pin GPIO_PIN_4
#define SEG_EN5_GPIO_Port GPIOA
#define SEG_EN4_Pin GPIO_PIN_5
#define SEG_EN4_GPIO_Port GPIOA
#define SEG_EN3_Pin GPIO_PIN_6
#define SEG_EN3_GPIO_Port GPIOA
#define SEG_D_Pin GPIO_PIN_7
#define SEG_D_GPIO_Port GPIOA
#define SEG_E_Pin GPIO_PIN_0
#define SEG_E_GPIO_Port GPIOB
#define SEG_F_Pin GPIO_PIN_1
#define SEG_F_GPIO_Port GPIOB
#define SEG_A_Pin GPIO_PIN_2
#define SEG_A_GPIO_Port GPIOB
#define SEG_C_Pin GPIO_PIN_10
#define SEG_C_GPIO_Port GPIOB
#define SEG_P_Pin GPIO_PIN_11
#define SEG_P_GPIO_Port GPIOB
#define SEG_G_Pin GPIO_PIN_12
#define SEG_G_GPIO_Port GPIOB
#define SEG_B_Pin GPIO_PIN_13
#define SEG_B_GPIO_Port GPIOB
#define SEG_EN2_Pin GPIO_PIN_14
#define SEG_EN2_GPIO_Port GPIOB
#define SEG_EN1_Pin GPIO_PIN_15
#define SEG_EN1_GPIO_Port GPIOB
#define SEG_EN0_Pin GPIO_PIN_8
#define SEG_EN0_GPIO_Port GPIOA
#define BTN_DOWN_Pin GPIO_PIN_15
#define BTN_DOWN_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_3
#define LED0_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_4
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_6
#define LED3_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_7
#define LED4_GPIO_Port GPIOB
#define LED5_Pin GPIO_PIN_8
#define LED5_GPIO_Port GPIOB
#define LED6_Pin GPIO_PIN_9
#define LED6_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
