/**
  ******************************************************************************
  * File Name          : freertos.c
  * Date               : 06/07/2017 18:55:41
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "usart.h"
#include "tim.h"
#include "Action.h"
#include "wwdg.h"
#include "GUI.h"
#include "ModbusRec.h"
#include "Hangtu.h"
#include "u_log.h"
/* USER CODE BEGIN Includes */     

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId NomalTaskHandle;
osThreadId Tick50msHandle;
osThreadId ActionTaskHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
SYS_STA services(void);
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */
/* Hook prototypes */


/****Modbus 显示屏的通信任务****************/
extern QUEUE u1_rx;
void ModbusComm(void const * argument)
{
  /* USER CODE BEGIN 5 */
	
	osDelay(500);
#ifdef USE_MOBUS_SLAVE
    eMBInit(MB_RTU, 1, 3, 9600, MB_PAR_NONE,1);  
    eMBEnable();
#else
	MX_USART1_UART_Init(57600);
#endif
	osDelay(500);
  /* Infinite loop */
    for(;;)
    {
        osDelay(5);
		
#ifdef USE_MOBUS_SLAVE
        MBSlavePoll();
#endif

    }
  /* USER CODE END 5 */ 
}
/* Init FreeRTOS */


void MX_FREERTOS_Init() {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Create the thread(s) */
  /* 按键检测 喂狗 其他任务 */
  osThreadDef(NomalTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  NomalTaskHandle = osThreadCreate(osThread(NomalTask), NULL);
  
  /* 主工作任务*/
  osThreadDef(ActionTask, StartTask03, osPriorityHigh, 0, 768);
  ActionTaskHandle = osThreadCreate(osThread(ActionTask), NULL);
  /*Modbus 通信任务*/
  osThreadDef(CLITask, ModbusComm, osPriorityBelowNormal, 0, 512);
  ActionTaskHandle = osThreadCreate(osThread(CLITask), NULL);
}

/*执行周期 CALUTICK 毫秒*/
void Timer3_CallBack(void)
{
	G_ActPoll_10ms();
}

/* StartTask03 function 
	主业务程序
*/
extern uint32_t g_errshow;
void StartTask03(void const * argument)
{
	SYS_STA status;
	
	osDelay(10);
	
	for(;;)
	{
		/*有探头一键启动模式*/
		status = services();
			
		if(status > ERR_NONE)	//传递错误
		{
			Log_e("%x",status);
			g_errshow = status;
			
		}
		
	}
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Application */


/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
