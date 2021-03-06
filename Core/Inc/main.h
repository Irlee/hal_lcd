/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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
#define KEY0_Pin GPIO_PIN_4
#define KEY0_GPIO_Port GPIOE
#define T_CS_Pin GPIO_PIN_13
#define T_CS_GPIO_Port GPIOC
#define LED0_Pin GPIO_PIN_9
#define LED0_GPIO_Port GPIOF
#define LED1_Pin GPIO_PIN_10
#define LED1_GPIO_Port GPIOF
#define PRS_Sta_Pin GPIO_PIN_1
#define PRS_Sta_GPIO_Port GPIOA
#define USART2_TX_Pin GPIO_PIN_2
#define USART2_TX_GPIO_Port GPIOA
#define USART2_RX_Pin GPIO_PIN_3
#define USART2_RX_GPIO_Port GPIOA
#define T_SCK_Pin GPIO_PIN_0
#define T_SCK_GPIO_Port GPIOB
#define T_PEN_Pin GPIO_PIN_1
#define T_PEN_GPIO_Port GPIOB
#define T_MISO_Pin GPIO_PIN_2
#define T_MISO_GPIO_Port GPIOB
#define T_MOSI_Pin GPIO_PIN_11
#define T_MOSI_GPIO_Port GPIOF
#define LCD_BL_Pin GPIO_PIN_15
#define LCD_BL_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_8
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
typedef __IO uint32_t  vu32;

//0,??????ucos
//1,????ucos
#define SYSTEM_SUPPORT_OS		0		//??????????????????????UCOS
																	    
	 
//????????,????51??????GPIO????????
//????????????,????<<CM3????????>>??????(87??~92??).M4??M3????,??????????????????.
//IO????????????
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO??????????
#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414 
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814 
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14 
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014 
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414    
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814   
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14    
#define GPIOI_ODR_Addr    (GPIOI_BASE+20) //0x40022014     

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010 
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410 
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810 
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10 
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010 
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410 
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810 
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10 
#define GPIOI_IDR_Addr    (GPIOI_BASE+16) //0x40022010 
 
//IO??????,??????????IO??!
//????n????????16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //???? 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //???? 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //???? 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //???? 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //???? 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //???? 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //???? 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //???? 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //???? 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //????

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //???? 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //????

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //???? 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //????

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //???? 
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  //????

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //???? 
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  //????

//??????????????
void WFI_SET(void);		//????WFI????
void INTX_DISABLE(void);//????????????
void INTX_ENABLE(void);	//????????????
void MSR_MSP(uint32_t addr);	//???????????? 
void SystemClock_Config(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
