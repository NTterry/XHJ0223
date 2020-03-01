/**********************************************
Copyright (C),2017-2018,ENKS Tech.Co.,Ltd.
File name	:config.h
Author		:Terry
Description	:��׮��������Ӳ������
Others		:None
Date		:2017.07.7  - 2017.9.11

���: ��
***********************************************/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "stdio.h"


#define  	WCH_DOG		0			/*1 ʹ�ܴ����Ź�*/


#define USE_LIHE_PWM

#ifdef USE_LIHE_PWM
	#define LIHE_PWM		7
#endif


//#define USE_LIUF




//#define 	USE_MOBUS_SLAVE		  // Enable Modbus GUI

/*�������ʾ������*/
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



#define LEDWORK_ON		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_RESET)		//����ָʾ����
#define LEDWORK_OFF		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,GPIO_PIN_SET)			//����ָʾ��Ϩ��


//�ⲿ ����
#define B_DD			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)					//�㶯��ť
#define B_ZD			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1)					//�Զ���ť
#define B_STOP			HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2)					//ֹͣ��ť

//��尴��
#define KEY_HUP       	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)					//�߶����ü�  K1
#define KEY_HDW       	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)					//�߶����ü�  K2
#define KEY_LUP        	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)					//K3
#define KEY_LDW        	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)					//K4

#define KEY_SET         HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4)					//C4          K9

#define KEY_SUP        	HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5)					//C5          K11
#define KEY_SDW        	HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)					//B0	  K2
#define KEY_CHG			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) 				//��˫���л� B1  K4

#define KEY_TSC			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)					//ɲ������ B2  K5

#define KEY_TLH			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)				//��ϲ���  B10   K8
#define KEY_START		HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)				//һ������  B11	  K10

//�ⲿ����
#define KEY_DD			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)				//�㶯  �ֶ���
//#define KEY_ZD			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)			//�Զ�
#define KEY_ZD			HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)					//һ����ͣ��ť  ���¶���

#define KEY_STOP		HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)				//ֹͣ

//#define KEY_LIU			HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)					//��ſ����ź�    �ֶ���
#define KEY_LIU			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)	
/*������ �����˿�  PA6 PA7		*/
/*���ʼ����˿�     PA0			*/


#define PIN_KSTOP		GPIO_PIN_4	
/*��ͣ����*/
#define C_STOP()		HAL_GPIO_WritePin(GPIOB, PIN_KSTOP,GPIO_PIN_RESET)
#define C_OK()			HAL_GPIO_WritePin(GPIOB, PIN_KSTOP,GPIO_PIN_SET)        //�������Ķ�

/*�Ĵ��������ź�*/
#define C_ENCTR()		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,GPIO_PIN_SET)
#define C_DISCTR()		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,GPIO_PIN_RESET)

/*���ָʾ�ź�*/
#define LED_SOLA_EN()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,GPIO_PIN_SET)
#define LED_SOLA_DIS()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,GPIO_PIN_RESET)

/*������*/
#define BUZZER(x)	           do{if(x) \
	                                  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,GPIO_PIN_SET);\
								else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,GPIO_PIN_RESET);\
								}while(0)
								

/*���ͷ����źŶ˿�*/
extern volatile uint32_t g_halt;
#define PIN_24VL			GPIO_PIN_13				//PC13
//#define PIN_TTCHK			GPIO_PIN_9				//PC9
#define PIN_FBLIHE			GPIO_PIN_1				//PA3
#define PIN_FBSC			GPIO_PIN_2				//PA2
//#define PIN_TTL				GPIO_PIN_3				//PA3

#define PG_POWEROK()		HAL_GPIO_ReadPin(GPIOC, PIN_24VL)		// 1 OK  0 �쳣
//#define PG_TTOK()			HAL_GPIO_ReadPin(GPIOC,PIN_TTCHK)		// 1 ���� 0 δ����      ��Ϊ�Ĵ������ź�
#define PG_FBLIHE()			HAL_GPIO_ReadPin(GPIOA,PIN_FBLIHE)		// 1 ��ʾ���ź� 0��ʾ���ź�  ������ȥ�۲�
#define PG_FBSC()			HAL_GPIO_ReadPin(GPIOA,PIN_FBSC)		// 1��ʾ���ź�  0��ʾ���ź�
//#define PG_TTCHK()			HAL_GPIO_ReadPin(GPIOA,PIN_TTL)			// 1 ���� 0 ��⵽��·
#define PG_STOP()			HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)		//  
#define PG_UP()             HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2)      /*�嶥���г̿��ص���   δʹ�� */

								
void Timer3_CallBack(void);
#endif

