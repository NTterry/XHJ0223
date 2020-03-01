



#ifndef _SINGLEACT_H_
#define _SINGLEACT_H_
#include "stdint.h"

#define VALID_MIN_CM		10
#define BRAKE_DLY400		400
#define VALID_POWH			70
#define VALID_POWM			50
#define VALID_POWL			30

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
	int32_t m_SpeedCm;
	int32_t m_Power;
};


typedef enum ERR_SIGACT
{
	ERR_SIG_OK,
	ERR_SIG_REACHUP,	
	ERR_SIG_REACHDW,  // finish
	ERR_SIG_PULLUP,
	ERR_SIG_ENCODER,
	ERR_SIG_CUR,
	ERR_SIG_CLING,
	ERR_SIG_BRAKE,
	ERR_SIG_TIMOUT,
	ERR_SIG_SOFT,
	
}ERR_SIG;

int32_t Per2Power(int16_t  percent);
void Sig_ResetSta(void);
ERR_SIG Sig_TakeUp(void);
ERR_SIG Sig_StudyUp(void);
ERR_SIG Sig_LandDw(void);

#endif

