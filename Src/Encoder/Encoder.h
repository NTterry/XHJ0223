

#ifndef _ENCODER_H_
#define _ENCODER_H_
#include "stdint.h"

#define MAXCOUNT            30000                   //最大计数值
#define ENCODER_TIM_PERIOD  (MAXCOUNT * 2)          //自动重装值
#define SPTIMEOUT           300                     // 速度检测超时时间  300ms
#define CNT_DIV     		4     //TIM_ENCODERMODE_TI12 模式下实际结果需要4分频

extern void HwEcInit(void);
extern void HwEcExitCallBack(void);			// configuration a encoder's pin with Exti Interrupt for this fun
extern void HwEcTimerTick1ms(void);			//  call by 1ms Tick fun


/*接口函数*/
void Enc_Clr_TotalCnt1(void); 
void Enc_Clr_TotalCnt2(void); 
int32_t Enc_Get_SpeedE1(void);
int32_t Enc_Get_Acce(void);
int32_t Enc_Get_CNT1(void);
int32_t Enc_Get_CNT2(void);
void Enc_Set_Dir(int dir);

/*对于局部函数，不允许在其他文件中直接操作，必须使用函数接口的方式进行操作*/
/*2020.2.29 现在可以几乎实时的速度读取编码器长度了*/
#endif
