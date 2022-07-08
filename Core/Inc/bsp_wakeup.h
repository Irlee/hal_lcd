
#ifndef __BSP_WAKEUP_H
#define __BSP_WAKEUP_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"

// 系统进入待机模式
static void Sys_Enter_Standby(void);
void Sys_Enter_Stop(void);

// 唤醒按键初始化
void WAKEUP_Init(void);
 
// 中断服务函数
void EXTI1_IRQHandler(void);
 
// 中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

