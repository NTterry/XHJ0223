/**********************************************************************************
  Copyright (C),2018-2020, JSJJ Tech.Co., Ltd
  FileName: Frq_Mens.c
  Author:   Terryc     Version:V1.0    Data:2020.3.1
  Description: 测量电能芯片的脉冲频率
  Others: None
  Function List:
      void IC_Mens_Init(void);
      uint32_t Frg_Tim_Diff(FRQ_TIM pre,FRQ_TIM last); 
      void ICaptureIRQ(uint32_t ic_val);
      void ICOverLoadIRQ(void);
      int Get_FRQE2(void);
  History: 
      Terryc  V1.0   2020.3.1    build this module
***********************************************************************************/
#include "Frq_Mens.h"
#include "u_log.h"
#include "string.h"

// Timer Init
extern void MX_TIM2_Init(void);

typedef struct
{
	uint32_t tim_H;		
	uint32_t tim_L;         
}FRQ_TIM;

typedef struct
{
	FRQ_TIM pre_tim;				//上一次的捕获时间
	FRQ_TIM last_tim;				//本次捕获时间
	uint32_t longtime;				//间隔时间
	uint32_t time_high;
}FRQ_MENS;


/*电能芯片捕获相关寄存器*/
static uint32_t T_lastval,T_preval;
static FRQ_MENS frg_mens;


void IC_Mens_Init(void)
{
	memset(&frg_mens,0,sizeof(FRQ_MENS));
	MX_TIM2_Init();		// Init timer and start the IC
}

/*diff = last - pre*/
uint32_t Frg_Tim_Diff(FRQ_TIM pre,FRQ_TIM last)
{
	uint32_t ret = 0;
	
	if(last.tim_H > pre.tim_H)
	{
		ret = (last.tim_H - pre.tim_H) * TIM_PREIOD;
	}
	
	if(last.tim_L >= pre.tim_L)
	{
		ret = ret + last.tim_L - pre.tim_L;
	}
	else
	{
		if(ret > 0)
		{
			ret = ret - pre.tim_L + last.tim_L; 
		}
	}
	
	return ret;
}

/************************************************************
  * @brief   Called by 捕获中断
  * @param   ic_val :捕获的定时器时间
  * @return  None
  * @author  Terryc
  * @date    2020.2.16
  * @version v1.0
  * @note    该函数使用了全局变量，不可重入
  ***********************************************************/
void ICaptureIRQ(uint32_t ic_val)
{
	frg_mens.last_tim.tim_L = (uint16_t)ic_val;
	frg_mens.last_tim.tim_H = frg_mens.time_high;
	
	/*Get the interval time   10us Per tick*/
	frg_mens.longtime = Frg_Tim_Diff(frg_mens.pre_tim,frg_mens.last_tim);
	frg_mens.pre_tim = frg_mens.last_tim;
	
	/* Soft Filter    时间越短，滤波系数越高*/
	if(frg_mens.longtime < 300)					//  333Hz
		T_lastval = (frg_mens.longtime + T_preval * 15) >> 4;
	else if(frg_mens.longtime < 1000)			// 100Hz
		T_lastval = (frg_mens.longtime + T_preval * 7) >> 3;
	else if(frg_mens.longtime < 2000)			// 50Hz
		T_lastval = (frg_mens.longtime + T_preval * 3) >> 2;
	else										// < 50Hz
		T_lastval = (frg_mens.longtime + T_preval) >> 1;
		
	T_preval = T_lastval;
}

/************************************************************
  * @brief   Called by 捕获定时器的溢出中断
  * @param   None
  * @return  None
  * @author  Terryc
  * @date    2020.3.1
  * @version v1.0
  * @note    被中断函数调用
  ***********************************************************/
void ICOverLoadIRQ(void)
{
	uint32_t stim;
	FRQ_TIM tmp = {0,0};
	
	frg_mens.time_high++;            // Update Higher time
	tmp.tim_H = frg_mens.time_high;
	
	stim = Frg_Tim_Diff(frg_mens.pre_tim,tmp);
	/*over time check ,when lost pulse*/
	if( stim > MAX_TIMUS) 
	{	
		frg_mens.last_tim.tim_H = frg_mens.time_high;
		frg_mens.last_tim.tim_L = 0;
		
		frg_mens.longtime = Frg_Tim_Diff(frg_mens.pre_tim,frg_mens.last_tim);
		
		if(frg_mens.longtime > MAX_TIMUS)
			frg_mens.longtime = MAX_TIMUS * 2;
		
		T_lastval = (frg_mens.longtime + T_preval * 3) >> 2;
		T_preval = T_lastval;
		
		frg_mens.pre_tim = frg_mens.last_tim;
		
		Log_e("power lost over");
	}
}

/*返回当前频率10倍的值*/
int Get_FRQE2(void)
{
	int fqu;
	
	if(T_lastval > MAX_TIMUS)
		return 0;
		
	fqu = (1000000 * 10 /  TIM_TICK)   / T_lastval;
	
	return fqu;
}



