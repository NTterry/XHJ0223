/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 06/07/2017 18:55:42
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "wwdg.h"
#include "gpio.h"
#include "stdio.h"
#include "GUI.h"
#include "Action.h"
#include "Hangtu.h"
#include "u_log.h"
#include "Encoder.h"
#include "Frq_Mens.h"
#include "SingleAct.h"


extern struct SIG_ACT_DATA g_st_SigData;
/* USER CODE BEGIN Includes */
uint16_t g_led_sta;
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Speed_Send(void);
void MX_FREERTOS_Init(void);
extern void HT1632_Init(void);

IWDG_HandleTypeDef hiwdg;
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/**
  * 函数功能: 独立看门狗初始化配置
  * 输入参数: Tout = prv/40 * rlv (s) prv可以是[4,8,16,32,64,128,256]
  *            prv:预分频器值，取值如下：
  *            参数 IWDG_PRESCALER_4: IWDG prescaler set to 4
  *            参数 IWDG_PRESCALER_8: IWDG prescaler set to 8
  *            参数 IWDG_PRESCALER_16: IWDG prescaler set to 16
  *            参数 IWDG_PRESCALER_32: IWDG prescaler set to 32
  *            参数 IWDG_PRESCALER_64: IWDG prescaler set to 64
  *            参数 IWDG_PRESCALER_128: IWDG prescaler set to 128
  *            参数 IWDG_PRESCALER_256: IWDG prescaler set to 256
  *
  *            rlv:预分频器值，取值范围为：0-0XFFF
  * 返 回 值: 无
  * 说    明：函数调用举例：
  *           IWDG_Config(IWDG_Prescaler_64 ,625);  // IWDG 1s 超时溢出
  */
void MX_IWDG_Init(uint8_t prv ,uint16_t rlv)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = prv;
  hiwdg.Init.Reload = rlv;
  HAL_IWDG_Init(&hiwdg);

}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  
  MX_TIM2_Init();                   //电能芯片计数
  MX_TIM3_Init();                   //定时器测量电能计数
  MX_USART3_UART_Init();			//刹车板通信
  
  MX_I2C2_Init();
  HwEcInit();
  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */
  

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();  //禁用JTAG口，只使用SW口
  __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
extern void KeySta_Poll(void);
extern void keyInit(void);
void StartDefaultTask(void const * argument)
{
    uint32_t i = 0;
  /* USER CODE BEGIN 5 */
	
	osDelay(200);
	sysattr_init(0);
	osDelay(100);
	ModbusData_Init();
	GUI_Init();
	keyInit();
	
#if (WCH_DOG == 1)
	/* USER CODE BEGIN 2 */
	MX_IWDG_Init(IWDG_PRESCALER_64,625);			// 1秒超时溢出复位 2017.9.11
	/* 启动独立看门狗 */
	HAL_IWDG_Start(&hiwdg);
#endif
	
	/* USER CODE END 2 */
	IC_Mens_Init();
  /* Infinite loop */
	for(;;)										// 10ms per ticks
	{
		i++;
		
		KeySta_Poll();
		if((i % 13) == 1)				/*GUI刷新*/
			Task_GUI_Function();	
		
		if((i % 17) == 1)	
			Speed_Send();   /*定时向刹车板发送速度信号*/
				
		osDelay(10);
        
		/*指示灯 的显示 */
	  	(g_st_SigData.m_Power > 5)? (LED_BIT_SET(SIG_CUR)): (LED_BIT_CLR(SIG_CUR));
		Dsp_BarLight(g_led_sta,0);  	/*LED 指示显示*/
		
#if (WCH_DOG == 1)
		if(i % 49 == 1)
			HAL_IWDG_Refresh(&hiwdg);		
#endif	
  }
  /* USER CODE END 5 */ 
}




/*速度 和 加速度数据的发送 */
extern struct Locationcm user_encoder;  
void Speed_Send(void)
{
    uint8_t sbuff[7] = {0};
    
    uint16_t sp;
    int16_t ace;
	int32_t tmp,acce;
	
	tmp = GetEncoderSpeedCm();			/*发送速度的绝对值  2019.12.08*/
	
	if(tmp < 0)
		tmp = -tmp;						/*不应该小于0   取反 2019.8.2*/
	
	if(tmp > 10000)						/*限制*/
		tmp = 10000;
		
	sp = (uint16_t)tmp;              	/*单位 厘米*/
	
	/*加速度改成向下为正*/

	acce = GetEncoderAcceCm();    	/*与原速度方向相反,下降的加速度为正*/


	if(acce > 1000)
		acce = 1000;
	else if(acce < -1000)
		acce = -1000;
    
    ace = (int16_t)acce;
    
    sbuff[0] = 0x55;
    sbuff[1] = (sp >> 8) &0xff;			/*单位 cm /s*/
    sbuff[2] = sp & 0xff;
    sbuff[3] = (ace >> 8) &0xff;		/*单位 cm/s^2*/
    sbuff[4] = ace & 0xff;
    
    sbuff[5] = sbuff[0] + sbuff[1] + sbuff[2] + sbuff[3] + sbuff[4];
    
    HAL_UART_Transmit(&huart3, (uint8_t *)&sbuff, 6, 20);
	tmp = Enc_Get_CNT1();
	acce = Enc_Get_Acce();
//	Log_e("C%d,acc %d",tmp,acce);
}



#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
