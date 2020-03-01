/******************************************************************************
  Copyright (C),2018-2020, JJKJ Tech.Co., Ltd
  FileName: Encoder.c
  Author: Terryc           Version:V1.0
  Description: 使用STM32 Timer的编码器功能，另外配置一个信号外部中断脚，结合1ms定时器
               完成编码器实时读取长度（扩展32位），瞬时速度值和加速度值
  Function List:
            void HwEcInit(void);
			void HwEcExitCallBack(void);
			void HwEcTimerTick1ms(void);
			static uint32_t Enc_DiffTimes(uint32_t t1, uint32_t t2);
			void EncoderExti(void);
			inline void EncoderCacuMs(void);
			int32_t Enc_Get_SpeedE1(void);
			int32_t Enc_Get_CNT1(void);
			int32_t Enc_Get_CNT2(void);
			void Enc_Set_Dir(int dir);
			int32_t Enc_Get_Acce(void);
  History:
  Terryc  2020.03.01     <V1.0>    build this module
******************************************************************************/

#include "Encoder.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "u_log.h"
#include "stdio.h"


//测速状态机
typedef enum {
    SP_INIT = 0,
    SP_CALU,
    SP_REC,
    SP_DLY = 61,         // 最长测量时间 61ms  只影响速度值和加速度值的更新
}SPEED_STATE;

/*编码长度，速度和加速度存储*/
typedef  struct
{
    SPEED_STATE Sta;
    uint16_t    LastResCnt;         //寄存器值
    int32_t     TotalCnt1;          //总计数值
    int32_t     TotalCnt2;          //备份计数器值
    uint32_t    LastTimMs;          //上次记录的时间
    int32_t     PerTimMs;           //当前记录时间
    int32_t     PerTick;
    int32_t     Speed;              // 10倍  n/s
    int32_t 	PreSpeed;
    int32_t     Acce;               // n/s^2
}ENCODER;


static ENCODER  Encoder1Data;

static inline void EncoderCacuMs(void);
static void EncoderExti(void);
static int s_start_flag = 0;
static int s_dir_flag = 0;

/**********************外部调用接口*****************************/
#define HwDisExti()       HAL_NVIC_DisableIRQ(EXTI9_5_IRQn)
#define HwEnExti()        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn)
#define HwGetCnt()        __HAL_TIM_GET_COUNTER(&htim4)				/* Read Encoder cnt by Timer cnt*/
#define HwGetCurTickMs()  HAL_GetTick();					/*Get Ticks per 1ms             */


/*Encoder Init*/
void HwEcInit(void)
{
   //定时器初始化，开启编码模式，开启硬件滤波，编码器启动
   //配置编码器其中一个引脚中断模式
	MX_TIM4_Init();
	s_start_flag = 1;
	HwEnExti();
}

/*called by Encoder pin Exti Interrupt*/
void HwEcExitCallBack(void)
{
	if(s_start_flag)
		EncoderExti();
}
/*Called by 1ms period Timers*/
void HwEcTimerTick1ms(void)
{
	if(s_start_flag)
		EncoderCacuMs();
}

/**************************************************************/

/************************************************************
  * @brief   求出两次时间的差值
  * @param   t1:上一次时间  t2:后来的时间（新）
  * @return  时间的差值
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    NULL
  ***********************************************************/
static uint32_t Enc_DiffTimes(uint32_t t1, uint32_t t2)
{
    uint32_t res;

    if(t2 < t1)
    {
        res = 0xFFFFFFFF - t1 + t2;
    }
    else
    {
        /* code */
        res = t2 - t1;
    }
    return res;
}
/************************************************************
  * @brief   两个编码器脉冲计数值解析
  * @param   none
  * @return  计数（带正负号） int32
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    间隔时间T内，计数值不能超过MAXCOUNT
  *          假设T = 0.1秒，每秒计数最大值为MAXCOUNT * 10
  ***********************************************************/
static int32_t Enc_DiffCnt(uint16_t c1,uint16_t c2)
{
	int32_t Angle = c2 - c1;
	
	if(Angle > MAXCOUNT)
	{
		Angle -= ENCODER_TIM_PERIOD;
	}
	if(Angle < -MAXCOUNT)
	{
		Angle += ENCODER_TIM_PERIOD;
	}
	
	if(s_dir_flag)
		return -Angle;
	else
		return Angle;
}




/************************************************************
  * @brief   编码器CNT1外部中断
  * @param   GPIO_Pin:引脚号
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    中断函数
  ***********************************************************/
void EncoderExti(void)
{	
	uint16_t TmpCnt;
	uint32_t TmpTim;
	int32_t  TmpSp;
	
	  switch (Encoder1Data.Sta)
      {
          case SP_INIT:
              Encoder1Data.LastTimMs = HwGetCurTickMs();    			    //获得当前系统时间
              Encoder1Data.LastResCnt = HwGetCnt();  						//得到当前计数器值
              HwDisExti();
              Encoder1Data.Sta = SP_DLY;							        // 关闭中断 等待100ms
              break;

          case SP_CALU:
              TmpCnt = HwGetCnt();
              TmpTim = HwGetCurTickMs();
              Encoder1Data.PerTimMs = Enc_DiffTimes(Encoder1Data.LastTimMs,TmpTim);		//间隔时间
              Encoder1Data.PerTick = Enc_DiffCnt(Encoder1Data.LastResCnt,TmpCnt);      // 间隔脉冲数
              /*此处累计编码计数*/
              Encoder1Data.TotalCnt2 += Encoder1Data.PerTick;
              Encoder1Data.TotalCnt1 += Encoder1Data.PerTick;
              TmpSp = Encoder1Data.PerTick * (10000 /CNT_DIV) / Encoder1Data.PerTimMs;        //multiply by 10
			  
	      /*Soft filter*/
	      Encoder1Data.Speed = (TmpSp + Encoder1Data.PreSpeed) >> 1;
			  
              /*与上一次的速度比较，计算加速度的值*/
              Encoder1Data.Acce = 100 * (Encoder1Data.Speed - Encoder1Data.PreSpeed)/Encoder1Data.PerTimMs;   // ticks / (s * s)
	      Encoder1Data.PreSpeed = Encoder1Data.Speed;
              Encoder1Data.Speed = TmpSp;
              /*保存当前值*/
              Encoder1Data.LastTimMs = TmpTim;
              Encoder1Data.LastResCnt = TmpCnt;
              HwDisExti();                      //关闭外部中断
              Encoder1Data.Sta = SP_DLY;
              break;
          default:
              break;
      }
}


/************************************************************
  * @brief   编码器计算,每毫秒调用一次
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    最好放在操作系统的系统时钟回调函数中执行
  ***********************************************************/
inline void EncoderCacuMs(void)
{
    uint32_t cur_tim;
	
    if(Encoder1Data.Sta == SP_REC)						
    {
        HwEnExti();					
        Encoder1Data.Sta = SP_CALU;		
    }
    if(Encoder1Data.Sta > SP_REC)
    {
        Encoder1Data.Sta--;
    }
    
    cur_tim = HwGetCurTickMs();
    if(Enc_DiffTimes(Encoder1Data.LastTimMs,cur_tim) >= SPTIMEOUT )    		// 如果超时，将触发错误信号
    {
        uint16_t counter;
	    
	counter = HwGetCnt();
        Encoder1Data.PerTick = Enc_DiffCnt(Encoder1Data.LastResCnt,counter);
        Encoder1Data.TotalCnt2 += Encoder1Data.PerTick;
        Encoder1Data.TotalCnt1 += Encoder1Data.PerTick;
        Encoder1Data.LastResCnt = counter;
        Encoder1Data.LastTimMs = cur_tim;
        //Calculate the Speed
        Encoder1Data.Speed = Encoder1Data.PerTick * (10000 /CNT_DIV)  / SPTIMEOUT;  // multiply by 10
	Encoder1Data.Speed = (Encoder1Data.Speed + Encoder1Data.PreSpeed) >> 1;
	Encoder1Data.PreSpeed = Encoder1Data.Speed;
        Encoder1Data.Acce = 0;
        Encoder1Data.Sta = SP_DLY;
	HwEnExti();
    }
}
/**User use  multiply 10**/
int32_t Enc_Get_SpeedE1(void)
{
	return Encoder1Data.Speed;
}
int32_t Enc_Get_Acce(void)
{
	return Encoder1Data.Acce;
}
/*Get Real Total Count1*/
int32_t Enc_Get_CNT1(void)
{
	int CNT1;
	uint16_t counter;
	
	counter = HwGetCnt();
	CNT1 = Encoder1Data.TotalCnt1 + Enc_DiffCnt(Encoder1Data.LastResCnt,counter);
	
	return CNT1 / CNT_DIV;
}
/*Get Real Total Count2*/
int32_t Enc_Get_CNT2(void)
{
	int CNT2;
	uint16_t counter;
	
	counter = HwGetCnt();
	CNT2 = Encoder1Data.TotalCnt2 + Enc_DiffCnt(Encoder1Data.LastResCnt,counter);
	return CNT2 / CNT_DIV;
}

/*Clear Total Count1 Zero*/
void Enc_Clr_TotalCnt1(void)
{
	Encoder1Data.TotalCnt1 = 0;    // not clear the register
}

/*Clear Total Count2 Zero*/
void Enc_Clr_TotalCnt2(void)
{
	Encoder1Data.TotalCnt2 = 0;
}
/*Set the Direction*/
void Enc_Set_Dir(int dir)
{	
	s_dir_flag = dir;
}
