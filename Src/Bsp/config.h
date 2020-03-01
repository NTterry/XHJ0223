/**********************************************
Copyright (C),2017-2018,ENKS Tech.Co.,Ltd.
File name	:config.h
Author		:Terry
Description	:打桩机控制器硬件配置
Others		:None
Date		:2017.07.7  - 2017.9.11

变更: 无
***********************************************/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "stdio.h"


#define  	WCH_DOG		0			/*1 使能窗看门狗*/


#define USE_LIHE_PWM

#ifdef USE_LIHE_PWM
	#define LIHE_PWM		7
#endif


//#define USE_LIUF




//#define 	USE_MOBUS_SLAVE		  // Enable Modbus GUI

/*数码管显示驱动脚*/
#define PIN_HCS		GPIO_PIN_3	
#define PIN_HRD		GPIO_PIN_2	
#define PIN_HWR		GPIO_PIN_1	
#define PIN_HDA		GPIO_PIN_0	

#define PORT_HT1632		GPIOC

#define HCS_L		HAL_GPIO_WritePin(PORT_HT1632, PIN_HCS,GPIO_PIN_RESET)
#define HCS_H		HAL_GPIO_WritePin(PORT_HT1632, PIN_HCS,GPIO_PIN_SET)

#define HRD_L		HAL_GPIO_WritePin(PORT_HT1632, PIN_HRD,GPIO_PIN_RESET)
#define HRD_H		HAL_GPIO_WritePin(PORT_HT1632, PIN_HRD,GPIO_PIN_SET)

#define HWR_L		HAL_GPIO_WritePin(PORT_HT1632, PIN_HWR,GPIO_PIN_RESET)
#define HWR_H		HAL_GPIO_WritePin(PORT_HT1632, PIN_HWR,GPIO_PIN_SET)

#define HDA_L		HAL_GPIO_WritePin(PORT_HT1632, PIN_HDA,GPIO_PIN_RESET)
#define HDA_H		HAL_GPIO_WritePin(PORT_HT1632, PIN_HDA,GPIO_PIN_SET)



#define LEDWORK_ON		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_RESET)		//工作指示灯亮
#define LEDWORK_OFF		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_SET)			//工作指示灯熄灭


//外部 按键
#define B_DD			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)					//点动按钮
#define B_ZD			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1)					//自动按钮
#define B_STOP			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2)					//停止按钮

//面板按键
#define KEY_HUP       	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)					//高度设置加  K1
#define KEY_HDW       	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)					//高度设置减  K2
#define KEY_LUP        	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)					//K3
#define KEY_LDW        	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)					//K4

#define KEY_SET         HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4)					//C4          K9

#define KEY_SUP        	HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5)					//C5          K11
#define KEY_SDW        	HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)					//B0	  K2
#define KEY_CHG			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) 				//单双打切换 B1  K4

#define KEY_TSC			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)					//刹车测试 B2  K5

#define KEY_TLH			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)				//离合测试  B10   K8
#define KEY_START		HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)				//一键启动  B11	  K10

//外部按键
#define KEY_DD			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)				//点动  手动提
//#define KEY_ZD			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)			//自动
#define KEY_ZD			HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)					//一键启停按钮  重新定义

#define KEY_STOP		HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)				//停止

//#define KEY_LIU			HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)					//溜放控制信号    手动溜
#define KEY_LIU			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)	
/*编码器 计数端口  PA6 PA7		*/
/*功率计数端口     PA0			*/


#define PIN_KSTOP		GPIO_PIN_4	
/*急停控制*/
#define C_STOP()		HAL_GPIO_WritePin(GPIOB, PIN_KSTOP,GPIO_PIN_RESET)
#define C_OK()			HAL_GPIO_WritePin(GPIOB, PIN_KSTOP,GPIO_PIN_SET)        //夯土机改动

/*履带机控制信号*/
#define C_ENCTR()		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,GPIO_PIN_SET)
#define C_DISCTR()		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,GPIO_PIN_RESET)

/*光电指示信号*/
#define LED_SOLA_EN()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,GPIO_PIN_SET)
#define LED_SOLA_DIS()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,GPIO_PIN_RESET)

/*蜂鸣器*/
#define BUZZER(x)	           do{if(x) \
	                                  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,GPIO_PIN_SET);\
								else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,GPIO_PIN_RESET);\
								}while(0)
								

/*检测和反馈信号端口*/
extern volatile uint32_t g_halt;
#define PIN_24VL			GPIO_PIN_13				//PC13
//#define PIN_TTCHK			GPIO_PIN_9				//PC9
#define PIN_FBLIHE			GPIO_PIN_1				//PA3
#define PIN_FBSC			GPIO_PIN_2				//PA2
//#define PIN_TTL				GPIO_PIN_3				//PA3

#define PG_POWEROK()		HAL_GPIO_ReadPin(GPIOC, PIN_24VL)		// 1 OK  0 异常
//#define PG_TTOK()			HAL_GPIO_ReadPin(GPIOC,PIN_TTCHK)		// 1 接入 0 未接入      改为履带控制信号
#define PG_FBLIHE()			HAL_GPIO_ReadPin(GPIOA,PIN_FBLIHE)		// 1 表示有信号 0表示无信号  可以人去观察
#define PG_FBSC()			HAL_GPIO_ReadPin(GPIOA,PIN_FBSC)		// 1表示有信号  0表示无信号
//#define PG_TTCHK()			HAL_GPIO_ReadPin(GPIOA,PIN_TTL)			// 1 正常 0 检测到短路
#define PG_STOP()			HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)		//  
#define PG_UP()             HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)      /*冲顶的行程开关到达   未使用 */

								
void Timer3_CallBack(void);
#endif

