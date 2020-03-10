


#ifndef __HANGTU_H_
#define __HANGTU_H_

#include "Action.h"
#include "PutOffAct.h"

SYS_STA ServicesLoop(void);


#define M_NUB		0
#define M_TIMES		1     /*�򴸴���*/
#define M_THIGH		2	  /*�ᴸ�߶�*/
#define M_DEEP		4  	  /*��ǰ���*/
#define M_SONGTU	3     /*����ʱ��*/
#define M_STATE		5	  /*״̬*/
#define M_ERR		6	  /*������*/
#define M_KS		7	  /*�׵����(׮��) Terry add 2019.7.5*/
#define M_ACT		8     /*���� 1 Ϊ�ᴸ 0 Ϊ�Ŵ�*/
#define M_ACCRE		9     /*��Ȩ��*/
#define M_HPROT		10    /*�߶ȱ���ֵ*/


#define M_PERCNT	13	  /*ÿȦ��������*/
#define M_ZHOU	    14	  /*��Ͳ�ܳ�*/
#define M_TLIAO		15	  /*����ʱ��*/
#define M_CCNUB		16	  /*ÿ���ڴ򴸴���*/
#define M_TICHUI	17	  /*�趨�ᴸ�߶�*/
#define M_LIHE 		18    /*��ϵ��λ��*/
/*�ֶδ򴸴���*/
#define M_HIGH1		19	  /*�߶�1*/
#define M_CNT1		20	  /*����1*/
#define M_HIGH2		21	  
#define M_CNT2		22

extern  uint16_t g_led_sta;
#define LED_BIT_SET(x)		g_led_sta |= x
#define LED_BIT_CLR(x)		g_led_sta &= ~x





extern void ModbusData_Init(void);
extern void ModbusData_Chk(void);
extern void ModbusData_flash(void);
extern void ModbusData_Show(void);
#endif

