


#ifndef __HANGTU_H_
#define __HANGTU_H_

#include "Action.h"
#include "PutOffAct.h"

SYS_STA ServicesLoop(void);


#define M_NUB		0
#define M_TIMES		1     /*打锤次数*/
#define M_THIGH		2	  /*提锤高度*/
#define M_DEEP		4  	  /*当前深度*/
#define M_SONGTU	3     /*送土时间*/
#define M_STATE		5	  /*状态*/
#define M_ERR		6	  /*错误码*/
#define M_KS		7	  /*孔的深度(桩底) Terry add 2019.7.5*/
#define M_ACT		8     /*动作 1 为提锤 0 为放锤*/
#define M_ACCRE		9     /*授权码*/
#define M_HPROT		10    /*高度保护值*/


#define M_PERCNT	13	  /*每圈计数齿数*/
#define M_ZHOU	    14	  /*卷筒周长*/
#define M_TLIAO		15	  /*送料时间*/
#define M_CCNUB		16	  /*每周期打锤次数*/
#define M_TICHUI	17	  /*设定提锤高度*/
#define M_LIHE 		18    /*离合点的位置*/
/*分段打锤次数*/
#define M_HIGH1		19	  /*高度1*/
#define M_CNT1		20	  /*次数1*/
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

