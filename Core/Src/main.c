/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "touch.h"
#include <stdio.h>
#include <string.h>
#include "as608.h"
#include "bsp_wakeup.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
///----------------------------------------------------------------------------------------///
//重定义fputc函数 
#if 1
#pragma import(__use_no_semihosting)             

//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif
///-----------------------------------------------------------------------------------------///

SysPara AS608Para;//指纹模块AS608参数
uint16_t ValidN;//模块内有效指纹个数
//uint8_t** kbd_tbl;

uint8_t res;
uint8_t states = 0;
uint8_t flag_ID = 0;		//0->密码没有输入成功过， 1->成功输入了一次密码
uint16_t times=0;				//清除flag_ID
uint32_t password=0;
uint32_t tempassword=0;
uint32_t* newpassword;

uint16_t Add_FR(uint16_t ID);	//录指纹
uint16_t press_FR(void);		//刷指纹
void Del_FR(void);	//删除指纹
void ShowErrMessage(uint8_t ensure);//显示确认码错误信息
uint32_t GET_NUM(void);//获取数值
void LCD_Screen_Toggle(void);//LCD页面切换
uint8_t passcheck(void);//检查密码

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t ensure;
	newpassword = (uint32_t*)0x10002000;
	//*newpassword = 123456;
	
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
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  printf("Try connecting the module...\n");	
  LEDR(0);						//初始化LED
  LEDG(0);
  
  LCD_Init();					//LCD初始化
  tp_dev.init();				//触摸屏初始化

  //LCD_ShowString(30,60,210,24,24,(uint8_t*)"Explorer STM32F4");	
  LCD_keyboard_display();		/*加载指纹识别实验界面*/

  HAL_UART_Transmit(&huart1, (uint8_t *)"Hello LiYi\n", 11, HAL_MAX_DELAY);
  
  USART2_RX_STA=0;		//清零
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
  
  LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	LCD_ShowString(20,40,200,32 ,16, (uint8_t *)"Communication is successful!!!");	  
	printf("Communication is successful!!!\n");	
	delay_ms(800);  
	printf("波特率:%d   地址:%x",57600, AS608Addr);
	ensure = PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		ShowErrMessage(ensure);			//显示确认码错误信息	
	ensure = PS_ReadSysPara(&AS608Para);  //读参数 
	delay_ms(800);
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	if(ensure==0x00)
	{
		printf("库容量:%d     对比等级: %d\n",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		LCD_ShowString(20,40,200,16 ,16, (uint8_t *)"Library capacity:");
		LCD_ShowNum(160,40,AS608Para.PS_max-ValidN,3,16);
		LCD_ShowString(20,60,200,16 ,16, (uint8_t *)"Comparison level:");
		LCD_ShowNum(160,60,AS608Para.PS_level,3,16);
		LCD_ShowString(20,100,200,48,16, (uint8_t *)"Please click anywhere on the screen to enter the  user interface");
	}
	else
		ShowErrMessage(ensure);	
	
	delay_ms(800);
	times = 0;
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(PS_Sta)
	  {
		  if(press_FR() != 0xffff)//刷指纹
		  {
			  LCD_ShowString(20,60,200,16 ,16, (uint8_t *)"password is correct");
			  LCD_ShowString(20,80,200,16 ,16, (uint8_t *)"lock is open");
			  flag_ID=1;
		//	  delay_ms(1500);
		  }
	  }
	  
	  if(times>=2000)
	  {
		  LEDR(0);
		  Sys_Enter_Stop();
		  times = 0;
		  flag_ID = 0;
	  }
	  else if(times>=1700)
	  {
		  if(times%50==0)
		  HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
	  }
	  
	  times++;

	  LCD_Screen_Toggle();
	  HAL_Delay(10);

	  
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void LCD_Screen_Toggle(void)
{
	uint8_t key = 0xff;
	uint16_t ID;
	static uint8_t change_times = 0;
	
	key = keyboard_scan();
	if(key != 0xff)
	{
		times = times/2;
		
		if(flag_ID==1)
		{
			states =1;
		}
		else
			states = 0;
		if(states == 1)
		{
			if(key == '1')
				states = 2;
			else if(key == '2')
				states = 3;
			else if(key == '3')
				states = 4;
			else if(key == '4')
				states = 5;
			else if(key == 'b')
			{
				if(flag_ID==0)
					states = 0;
				else
					states =1;
			}
		}
		switch(states)
		{
			case 0:
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(75,10,200,24 ,24, (uint8_t *)"Welcome");
				LCD_ShowString(10,40,220,50 ,16, (uint8_t *)"Please press the fingerprint firmly on the window");
				LCD_ShowString(10,80,220,50 ,16, (uint8_t *)"or enter password to unlock");
				delay_ms(2000);
				if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
				{
					if(press_FR() != 0xffff)//刷指纹
					{
						flag_ID=1;
						states = 1;
					}
					else
					{
						flag_ID=0;
						break;
					}
				}
				else
				{
					password=GET_NUM();
					if(password != 0xffffffff)
					{	
						if(password == *newpassword)
						{
							LCD_ShowString(20,60,200,16 ,16, (uint8_t *)"password is correct");
							LCD_ShowString(20,80,200,16 ,16, (uint8_t *)"lock is open");
							flag_ID=1;
							states = 1;
						}
						else
						{
							LCD_ShowString(20,60,220,16 ,16, (uint8_t *)"password is error");
							LCD_ShowString(20,80,220,16 ,16, (uint8_t *)"please try again");
							LCD_ShowString(20,120,220,16 ,16, (uint8_t *)"press any key to continue");
							flag_ID=0;
							break;
						}
					}
					else
					{
						LCD_ShowString(20,120,220,16 ,16, (uint8_t *)"press any key to continue");
						break;
					}
				}
				
			case 1:					//Menu
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(40,20,200,16 ,16, (uint8_t *)"1.Add fingerprint");
				LCD_ShowString(40,40,200,16 ,16, (uint8_t *)"2.Del fingerprints");
				LCD_ShowString(40,60,200,16 ,16, (uint8_t *)"3.Search fingerprints");
				LCD_ShowString(40,80,200,16 ,16, (uint8_t *)"4.change the password");
				LCD_ShowString(10,125,220,50 ,16, (uint8_t *)"Please press the number key to get the corresponding function");
				break;
			case 2:					//Add fingerprint
				if(passcheck()==0)
				{
					LCD_ShowString(10,125,220,50 ,16, (uint8_t *)"Please press the 'Back' key to return last step");
					break;
				}
			a:	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
			    LCD_ShowString(20,50,200,32 ,16, (uint8_t *)"Please enter your fingerprint ID");
				LCD_ShowString(30,120,220, 32,16, (uint8_t *)"0 <= ID < 300");
				ID = GET_NUM();
				if(ID >= 300)
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					LCD_ShowString(20,120,200,16 ,16, (uint8_t *)"invalid ID");
					delay_ms(1000);
					goto a;
				}
				else
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					LCD_ShowString(20,60,200,16 ,16, (uint8_t *)"ID successfully");
					LCD_ShowString(20,90,200,16 ,16, (uint8_t *)"ID = ");
					LCD_ShowNum(100,120,ID,16,16);
					//LCD_ShowString(10,120,200,16 ,16, (uint8_t *)"Please press the fingerprint firmly on the window");
					if(Add_FR(ID)!=0xffff)
					{
						LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
						LCD_ShowString(20,40,200,48,24, (uint8_t *)"Please press the back button");
					}
				}
				break;
			case 3:					//Delect fingerprint
				if(passcheck())
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					Del_FR();
				}
				break;
			
			case 4:					//Search fingerprint
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				press_FR();
				LCD_ShowString(20,40,200,48,24, (uint8_t *)"Please press the back button");
				break;
			case 5:					//更改密码
				
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(10,60,220,32 ,16, (uint8_t *)"please input your old password");
				delay_ms(1000);
				if(passcheck())
				{
					LCD_ShowString(10,60,220,32 ,16, (uint8_t *)"please input your new password");
					password = GET_NUM();
					if(*newpassword != password)
					{
						tempassword = password;
						LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
						LCD_ShowString(10,60,220,32 ,16, (uint8_t *)"please confirm your new password");
						password = GET_NUM();
						if(tempassword == password)
						{
							LCD_ShowString(10,100,220,32 ,16, (uint8_t *)"password changed successfully");
							delay_ms(1000);
							*newpassword = tempassword;
						}
						else
						{
							LCD_ShowString(10,100,220,32 ,16, (uint8_t *)"The passwords entered twice do not match");
							delay_ms(1000);
						}
					}
					else
					{
						LCD_ShowString(10,100,220,32 ,16, (uint8_t *)"New password is same as the old password");
						delay_ms(1000);	
					}
				}
				
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(20,40,200,48,24, (uint8_t *)"Please press the back button");
				break;
		}
	}
}

//检查密码 0：密码错误    1：正确
uint8_t passcheck(void)
{
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	LCD_ShowString(10,60,220,32 ,16, (uint8_t *)"please input your password");
	password = GET_NUM();
	if(*newpassword != password)
	{
		LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
		LCD_ShowString(10,60,220,32 ,16, (uint8_t *)"password is error");
		delay_ms(1000);
		return 0;
	}
	else
		return 1;
}


//获取键盘数值  长时间没有键按下自动退出，返回0xffffffff
uint32_t GET_NUM(void)
{	
	int x=0;
	uint8_t value;
	uint8_t pstr[9]={"\0"};
	uint8_t newpstr[9]="123456";
	//uint8_t blank[9]={"        "};
	uint8_t p=0;
	uint32_t num = 0xffffffff;
	
	times = 0;
	
	LCD_Fill(1*wide,320-5*high, 1*wide+12*8-1, 320-5*high+23, CYAN);

	while(1)
	{
	  value = keyboard_scan();
	  if(value != 0xff)
	  {
		  x = x/2;
		  if(value >= '0' && value <= '9')
		  {
			  pstr[p++] = value;
			  if(p>=9)
			  {
				  p = p%9;
				  pstr[0]='\0';
			  }
			  else
				  pstr[p]='\0';
		  }
		  else if(value == '<' && p != 0)
		  {
			  p--;
			  pstr[p] = '\0';			  
		  }
		  else if(value == 'e' && pstr[0]!='\0')
		  {
			  strcpy(newpstr, pstr);
			  printf("newpass :%s\n", newpstr);
			  num = (uint32_t)atoi(newpstr);
			  LCD_ShowString(1*wide+12*8-1,320-5*high, 210, 24, 24,(uint8_t*)"Entered");
			  delay_ms(1000);
			  break;
		  }

		  LCD_Fill(1*wide,320-5*high, 1*wide+12*8-1, 320-5*high+23, CYAN);
		  LCD_ShowString(1*wide,320-5*high, 210, 24, 24, pstr);
		  printf("%c\n", value);
		  printf("string :%s\n", pstr);
	  }
	  x=x+1;
	  if(x>=800000)
		  break;
	}
	LCD_Fill(0,0, lcddev.width, 320-4*high-2, WHITE);
	  return num;
}

//显示确认码错误信息
void ShowErrMessage(uint8_t ensure)
{
	LCD_Fill(0,120,lcddev.width,160,WHITE);
	LCD_ShowString(60,12,200,100 ,24, (uint8_t *)"Error:");
	LCD_ShowNum(135, 12, ensure, 2, 24);
	POINT_COLOR = RED;
	LCD_ShowString(10,60,220,70,16, (uint8_t*)EnsureMessage(ensure));
	POINT_COLOR = BLACK;
	delay_ms(2000);
	LCD_Fill(0,120,lcddev.width,160,WHITE);
	//LCD_ShowString(20,40,200,48,24, (uint8_t *)"Please press the back button");
	printf("%x:%s\n",ensure, EnsureMessage(ensure));
	//Show_Str_Mid(0,120,(uint8_t*)EnsureMessage(ensure),16,240);
}
//录指纹  成功：返回ID   失败：返回0xffff
uint16_t Add_FR(uint16_t ID)
{
	uint8_t i = 0,ensure = 0,processnum=0;
	
	while(1)
	{
		switch(processnum)
		{
			case 0:
				i++;
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(20,40,200,16 ,16, (uint8_t *)"Please press finger");
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					LEDG(1);
					ensure=PS_GenChar(CharBuffer1);//生成特征
					LEDG(0);
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
						LCD_ShowString(20,40,200,16 ,16, (uint8_t *)"Fingerprint is OK");
						//Show_Str_Mid(0,120,"指纹正常",16,240);
						i=0;
						processnum=1;//跳到第二步						
					}
					else 
						ShowErrMessage(ensure);				
				}
				else 
					ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(20,40,200,32,16, (uint8_t *)"Please press finger again");
				//Show_Str_Mid(0,100,"请按再按一次指纹",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					LEDG(1);
					ensure=PS_GenChar(CharBuffer2);//生成特征
					LEDG(0);
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
						LCD_ShowString(20,40,200,16 ,16, (uint8_t *)"Fingerprint is OK");
						i=0;
						processnum=2;//跳到第三步
					}
					else
						ShowErrMessage(ensure);	
				}
				else 
					ShowErrMessage(ensure);		
				break;

			case 2:
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				LCD_ShowString(20,40,200,32,16, (uint8_t *)"Compare two fingerprints");
				//Show_Str_Mid(0,100,"对比两次指纹",16,240);
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					LCD_ShowString(20,40,200,32,16, (uint8_t *)"Fingerprint comparison is successful");
					//Show_Str_Mid(0,120,"对比成功,两次指纹一样",16,240);
					processnum=3;//跳到第四步
				}
				else 
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					LCD_ShowString(20,40,200,50,16, (uint8_t *)"The comparison failed, please reenter your fingerprints");
					//Show_Str_Mid(0,100,"对比失败，请重新录入指纹",16,240);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//跳回第一步		
				}
				delay_ms(1200);
				break;

			case 3:
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				//Show_Str_Mid(0,100,"生成指纹模板",16,240);
				LCD_ShowString(20,40,200,32,16, (uint8_t *)"Generate fingerprint template");
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					//Show_Str_Mid(0,120,"生成指纹模板成功",16,240);
					LCD_ShowString(20,40,200,32,16, (uint8_t *)"Generate fingerprint template successfully");
					processnum=4;//跳到第五步
				}
				else 
				{
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					processnum=0;
					ShowErrMessage(ensure);
				}
				delay_ms(1200);
				break;
				
			case 4:	
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{			
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);		
					//Show_Str_Mid(0,120,"录入指纹成功",16,240);
					LCD_ShowString(20,40,200,32,16, (uint8_t *)"Fingerprint entered successfully");
					PS_ValidTempleteNum(&ValidN);//读库指纹个数

					LCD_ShowString(20,70,200,16 ,16, (uint8_t *)"Comparison level:");
					LCD_ShowNum(160,70,AS608Para.PS_level,3,16);
					delay_ms(1500);
					LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
					return ID;
				}
				else 
				{
					processnum=0;
					ShowErrMessage(ensure);
				}					
				break;				
		}
		delay_ms(400);
		if(i==5)//超过5次没有按手指则退出
		{
			LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
			break;	
		}				
	}
	return 0xffff;
}

//刷指纹		找到指纹：返回ID		没有找到：返回0xffff
uint16_t press_FR(void)
{
	SearchResult search;
	uint8_t ensure;
	char *str;
	uint16_t ID=0xffff;
	
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	LCD_ShowString(20,40,200,32,16, (uint8_t *)"Please press your finger!");
	delay_ms(500);
	LEDG(1);
	delay_ms(500);
	LEDG(0);
	
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	
	delay_ms(100);
	ensure=PS_GetImage();
	
	if(ensure==0x00)//获取图像成功 
	{	
		//BEEP=1;//打开蜂鸣器	
		LEDG(1);
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			//BEEP=0;//关闭蜂鸣器	
			LEDG(0);
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&search);
			if(ensure==0x00)//搜索成功
			{
				LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
				//Show_Str_Mid(0,100,"刷指纹成功",16,240);	
				LCD_ShowString(20,40,200,32,16, (uint8_t *)"Fingerprint identification succeeded");
				
				//str=mymalloc(SRAMIN,2000);
				//sprintf(str,"确有此人,ID:%d  匹配得分:%d",search.pageID,search.mathscore);
				//Show_Str_Mid(0,140,(uint8_t*)str,16,240);
				printf("确有此人,ID:%d  匹配得分:%d",search.pageID,search.mathscore);
				LCD_ShowString(20,70,200,16,16, (uint8_t *)"Find the fingerprint");
				LCD_ShowString(20,90,24,16,16, (uint8_t *)"ID=");
				ID = search.pageID;
				LCD_ShowNum(45,90,ID,3,16);
				LCD_ShowString(20,120,48,16,16, (uint8_t *)"Score=");
				LCD_ShowNum(70,120,search.mathscore,3,16);
			}
			else 
				ShowErrMessage(ensure);					
		}
		else
			ShowErrMessage(ensure);
	    //BEEP=0;//关闭蜂鸣器
		LEDG(0);
	   delay_ms(3000);
	   LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	}
	else
	{
		ShowErrMessage(ensure);		
		delay_ms(2000);
		LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	}
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	
	return ID;
}

//删除指纹
void Del_FR(void)
{
	uint8_t flag_count=0;
	uint8_t  ensure;
	uint32_t num;
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	//Show_Str_Mid(0,100,"删除指纹",16,240);
	//Show_Str_Mid(0,120,"请输入指纹ID按Enter发送",16,240);
	//Show_Str_Mid(0,140,"0=< ID <=299",16,240);
	LCD_ShowString(20,20,200,16 ,16, (uint8_t *)"Delete fingerprint");
	LCD_ShowString(10,50,220,32 ,16, (uint8_t *)"Please enter your fingerprint ID and press Enter to send");
	LCD_ShowString(60,90,112,16 ,16, (uint8_t *)"0 =< ID <= 299");
	
	POINT_COLOR = RED;
	LCD_ShowString(10,110,220,32,16, (uint8_t *)"If the input ID = 11110000, All Fingerprints Will Be Deleted!!!");
	POINT_COLOR = BLACK;
	
	delay_ms(1000);
	
	//AS608_load_keyboard(0,170,(uint8_t**)kbd_delFR);
	
	num=GET_NUM();//获取返回的数值
	if(num==0xFFFFFFFF)
		goto MENU ; //返回主页面
	else if(num==11110000)
		ensure=PS_Empty();//清空指纹库
	else if(num>=300)
	{
		LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE); 
		LCD_ShowString(20,40,200,32,16, (uint8_t *)"ID number too large");
		goto MENU ; //返回主页面
	}
	else
		ensure=PS_DeletChar(num,1);//删除单个指纹
	if(ensure==0)
	{
		LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
		LCD_ShowString(20,40,200,32,16, (uint8_t *)"Delete fingerprint successfully");
	//	Show_Str_Mid(0,140,"删除指纹成功",16,240);		
	}
    else
		ShowErrMessage(ensure);	
	
	delay_ms(1200);
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
	LCD_ShowString(10,80,220,32,16, (uint8_t *)"The number of existing fingerprints in the library is ");
	LCD_ShowNum(100,120,AS608Para.PS_max-ValidN,3,16);
	delay_ms(1200);
MENU:	
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	delay_ms(50);
	//AS608_load_keyboard(0,170,(uint8_t**)kbd_menu);
	LCD_Fill(0,0,lcddev.width,lcddev.height-4*high-2,WHITE);
	LCD_ShowString(20,40,200,48,24, (uint8_t *)"Please press the back button");
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
