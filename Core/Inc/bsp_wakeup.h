
#ifndef __BSP_WAKEUP_H
#define __BSP_WAKEUP_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"

// ϵͳ�������ģʽ
static void Sys_Enter_Standby(void);
void Sys_Enter_Stop(void);

// ���Ѱ�����ʼ��
void WAKEUP_Init(void);
 
// �жϷ�����
void EXTI1_IRQHandler(void);
 
// �жϻص�����
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

