



#ifndef _SINGLEACT_H_
#define _SINGLEACT_H_
#include "stdint.h"

#define VALID_MIN_CM		10				// Mininum Speed 0.1m/s    检测阈值
#define BRAKE_DLY400		400
#define VALID_POWH			70
#define VALID_POWM			50
#define VALID_POWL			30
#define VALID_POWOVER		150


enum ACT_SINGLE_UP
{
	SIG_IDLE,
	SIG_PULL_CLUTCH,
	SIG_PULL_HOLD,
	SIG_WAIT_VALID_UP,
	SIG_WORKUP,
	SIG_REACH_TOP,
	SIG_BLOCKED,
};


enum ACT_SINGLE_DW
{
	DIG_IDLE,
	DIG_CHKDW,
	DIG_WAIT_LIHE,
	DIG_BLOCKED,
};


struct SIG_ACT_DATA
{
	int32_t m_HeightShowCm;
	int32_t m_HeighRammCm;			/*夯土时，锤的计量高度*/
	int32_t m_SpeedCm;
	int32_t m_Power;
	int32_t m_Mode;
	int32_t m_errnum;				/*错误编号*/
	int32_t m_manualflg;
	uint32_t m_errshow;				/*主执行程序的错误返回码*/
	int32_t  m_Lihenew;				/*更新后离合点的高度*/
};


typedef enum ERR_SIGACT
{
	ERR_SIG_OK,
	ERR_SIG_REACHUP,		//正常提锤到达顶部
	ERR_SIG_REACHDW,  		//正常下放打锤结束
	ERR_SIG_PULLUP,			//提锤异常
	ERR_SIG_ENCODER,		//编码器异常
	ERR_SIG_CUR,			//电流检测异常
	ERR_SIG_CLING,			//黏离合
	ERR_SIG_BRAKE,			//刹车异常
	ERR_SIG_TIMOUT,			//提锤异常
	ERR_SIG_SOFT,			//其他软件错误
	
}ERR_SIG;

int32_t Per2Power(int16_t  percent);
void Sig_ResetSta(void);
ERR_SIG Sig_TakeUp(void);
ERR_SIG Sig_StudyUp(void);
ERR_SIG Sig_LandDw(void);

void GetLiveData(void);

#endif

