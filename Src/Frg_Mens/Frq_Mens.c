


#include "Frq_Mens.h"
#include "u_log.h"
#include "string.h"

// Timer Init
extern void MX_TIM2_Init(void);


// Type Def
typedef struct
{
	uint32_t tim_H;			// mis  over
	uint32_t tim_L;
}FRQ_TIM;

typedef struct
{
	FRQ_TIM pre_tim;
	FRQ_TIM last_tim;
	uint32_t longtime;
	uint32_t time_high;
}FRQ_MENS;

static uint32_t T_lastval,T_preval;
static FRQ_MENS frg_mens;


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


void IC_Mens_Init(void)
{
	memset(&frg_mens,0,sizeof(FRQ_MENS));
	MX_TIM2_Init();		// Init timer and start the IC
}

void ICaptureIRQ(uint32_t ic_val)
{
	frg_mens.last_tim.tim_L = (uint16_t)ic_val;
	frg_mens.last_tim.tim_H = frg_mens.time_high;
	
	frg_mens.longtime = Frg_Tim_Diff(frg_mens.pre_tim,frg_mens.last_tim);
	
	frg_mens.pre_tim = frg_mens.last_tim;
	
	T_lastval = (frg_mens.longtime + T_preval * 3) >> 2;
	T_preval = T_lastval;
}

void ICOverLoadIRQ(void)
{
	uint32_t stim;
	FRQ_TIM tmp = {0,0};
	
	frg_mens.time_high++;
	
	tmp.tim_H = frg_mens.time_high;
	
	stim = Frg_Tim_Diff(frg_mens.pre_tim,tmp);
	
	if( stim > MAX_TIMUS) /*超时了*/
	{	
		frg_mens.last_tim.tim_H = frg_mens.time_high;
		frg_mens.last_tim.tim_L = 0;
		
		frg_mens.longtime = Frg_Tim_Diff(frg_mens.pre_tim,frg_mens.last_tim);
		
		if(frg_mens.longtime > MAX_TIMUS)
			frg_mens.longtime = MAX_TIMUS * 2;
		
		T_lastval = (frg_mens.longtime + T_preval * 3) >> 2;
		T_preval = T_lastval;
		
		frg_mens.pre_tim = frg_mens.last_tim;
		
		Log_e("over");
	}
	else
	{
//		Log_e("IC %d  %d",T_preval,stim);
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



