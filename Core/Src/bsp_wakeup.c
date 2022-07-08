#include "bsp_wakeup.h"
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "lcd.h"
#include "touch.h"
#include "as608.h"

extern SysPara AS608Para;//指纹模块AS608参数
extern uint8_t states;
uint8_t flag_stop = 0;

// 系统进入待机模式
static void Sys_Enter_Standby(void)
{
	__HAL_RCC_PWR_CLK_ENABLE(); // 使能 PWR 时钟
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 清除 Wake_UP 标志
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); // 设置 WKUP 用于唤醒
	printf("system will enter standby\r\n");
	HAL_PWR_EnterSTANDBYMode(); // 进入待机模式
}

void Sys_Enter_Stop(void)
{
	HAL_SuspendTick();
	//__HAL_RCC_AHB1_FORCE_RESET();       //复位所有IO口 
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	
	WAKEUP_Init();
	HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
	printf("system will enter stop\r\n");
	flag_stop = 1;
	states = 0;
	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);
	HAL_ResumeTick();
}

// 唤醒按键初始化
void WAKEUP_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure; // 定义初始化GPIO结构体变量
	__HAL_RCC_GPIOA_CLK_ENABLE(); // 开始GPIOA时钟
	
	GPIO_Initure.Pin = GPIO_PIN_1; // PA1引脚
	GPIO_Initure.Pull = GPIO_PULLDOWN; // 
	GPIO_Initure.Mode = GPIO_MODE_IT_RISING; // shang沿触发
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
	HAL_GPIO_Init(GPIOA, &GPIO_Initure); // GPIOE初始化
	
	HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 2);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}
 
// 中断服务函数
void EXTI1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
 
// 中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_1)
	{
		SystemClock_Config();
		MX_GPIO_Init();
		MX_FSMC_Init();
		MX_USART1_UART_Init();
		MX_USART2_UART_Init();
		MX_TIM6_Init();
		LCD_Init();					//LCD初始化
		tp_dev.init();				//触摸屏初始化
		LCD_keyboard_display();		/*加载指纹识别实验界面*/
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		
		USART2_RX_STA=0;		//清0
		HAL_UART_Receive_IT(&huart2, &res, 1);
		while(PS_HandShake(&AS608Addr))//与AS608模块握手
		{
			delay_ms(400);
			LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
			LCD_ShowString(20,40,200,16 ,16, (uint8_t *)"Module not detected!!!");
			printf("Module not detected!!!\n");
			delay_ms(800);
			LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
			LCD_ShowString(20,40,200,32 ,16, (uint8_t *)"Try connecting the module...");	
			printf("Try connecting the module...\n");	  
		}
		if(flag_stop)
		{
			LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
			LCD_ShowString(40,50,200,24 ,24, (uint8_t *)"Welcome Home");
			LCD_ShowString(40,80,200,24 ,24, (uint8_t *)"Your Majesty");
			flag_stop = 0;
		}
	}	
}
