/**********************************************************************************
  Copyright (C),2018-2020, JSJJ Tech.Co., Ltd
  FileName: Encoder.h
  Author:   Terryc     Version:V1.0    Data:2020.3.1
  Description: 增量编码器测量长度，速度和加速度的算法和应用
  Others: None
  Function List:
      void Enc_Clr_TotalCnt1(void);
      void Enc_Clr_TotalCnt2(void); 
      int32_t Enc_Get_Acce(void);
      int32_t Enc_Get_CNT1(void);
      int32_t Enc_Get_CNT2(void);
      void Enc_Set_Dir(int dir);
  History: 
      Terryc  V1.0   2020.3.1    build this module
***********************************************************************************/
#ifndef _ENCODER_H_
#define _ENCODER_H_
#include "stdint.h"

#define MAXCOUNT            30000                   //最大计数值
#define ENCODER_TIM_PERIOD  (MAXCOUNT * 2)          //自动重装值
#define SPTIMEOUT           300                     // 速度检测超时时间  300ms
#define CNT_DIV     		    4                       //TIM_ENCODERMODE_TI12 模式下实际结果需要4分频

extern void HwEcInit(void);
extern void HwEcExitCallBack(void);			// configuration a encoder's pin with Exti Interrupt for this fun
extern void HwEcTimerTick1ms(void);			//  call by 1ms Tick fun


/*接口函数*/
void Enc_Clr_TotalCnt1(void); 
void Enc_Clr_TotalCnt2(void); 
int32_t Enc_Get_Acce(void);
int32_t Enc_Get_CNT1(void);
int32_t Enc_Get_CNT2(void);
void Enc_Set_Dir(int dir);

/*对于局部函数，不允许在其他文件中直接操作，必须使用函数接口的方式进行操作*/
/*2020.2.29 现在可以几乎实时的速度读取编码器长度了*/
#endif
