#include "Encoder.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "u_log.h"
#include "stdio.h"

#define  CNT_DIV     4     //TIM_ENCODERMODE_TI12 模式下实际结果需要4分频

ENCODER  Encoder1Data;

static inline void EncoderCacuMs(void);
static void EncoderExti(void);
static int s_start_flag = 0;

/**********************外部调用接口*****************************/
#define HwDisExti()       HAL_NVIC_DisableIRQ(EXTI9_5_IRQn)
#define HwEnExti()        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn)
#define HwGetCnt()        __HAL_TIM_GET_COUNTER(&htim4)
#define HwGetCurTickMs()  HAL_GetTick();

void HwEcInit(void)
{
   //定时器初始化，开启编码模式，开启硬件滤波，编码器启动
   //配置编码器其中一个引脚中断模式
	MX_TIM4_Init();
	s_start_flag = 1;
}

void HwEcExitCallBack(void)
{
	if(s_start_flag)
		EncoderExti();
}

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
              /*与上一次的速度比较，计算加速度的值*/
              Encoder1Data.Acce = 100 * (TmpSp - Encoder1Data.Speed)/Encoder1Data.PerTimMs;   // ticks / (s * s)
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
    uint32_t tmp;

	if(Encoder1Data.Sta == SP_REC)						
	{
		HwEnExti();					/*¿ªÆô±àÂëÆ÷Òý½ÅÍâ²¿ÖÐ¶Ï*/
		Encoder1Data.Sta = SP_CALU;								/*¿ªÊ¼¼ì²â*/
	}
    if(Encoder1Data.Sta > SP_REC)
	{
        Encoder1Data.Sta--;
	}
    
    tmp = HwGetCurTickMs();
    if(Enc_DiffTimes(Encoder1Data.LastTimMs,tmp) >= SPTIMEOUT )    		// 如果超时，将触发错误信号
    {
        uint16_t counter;
		counter = HwGetCnt();
        Encoder1Data.PerTick = Enc_DiffCnt(Encoder1Data.LastResCnt,counter);
        Encoder1Data.TotalCnt2 += Encoder1Data.PerTick;
        Encoder1Data.TotalCnt1 += Encoder1Data.PerTick;
        Encoder1Data.LastResCnt = counter;
        Encoder1Data.LastTimMs = tmp;
        //计算速度
        Encoder1Data.Speed = Encoder1Data.PerTick * (10000 /CNT_DIV)  / SPTIMEOUT;  // multiply by 10
        Encoder1Data.Acce = 0;
        Encoder1Data.Sta = SP_DLY;
		HwEnExti();
//		Log_e("cnt");
    }
}
/**User use**/
int32_t Enc_Get_Speed(void)
{
	return Encoder1Data.Speed;
}
int32_t Enc_Get_Acce(void)
{
	return Encoder1Data.Acce;
}
int32_t Enc_Get_CNT1(void)
{
	return Encoder1Data.TotalCnt1 / CNT_DIV;
}
int32_t Enc_Get_CNT2(void)
{
	return Encoder1Data.TotalCnt2 / CNT_DIV;
}

void Enc_Clr_TotalCnt1(void)
{
	Encoder1Data.TotalCnt1 = 0;
}
void Enc_Clr_TotalCnt2(void)
{
	Encoder1Data.TotalCnt2 = 0;
}
