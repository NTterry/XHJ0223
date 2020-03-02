

#include "SingleAct.h"
#include "PutOffAct.h"
#include "Encoder.h"
#include "Action.h"
#include "Frq_Mens.h"
#include "cmsis_os.h"
#include "u_log.h"

#define GET_SYS_TIMEMS()	osKernelSysTick()
#define DEF_TIMEOUT		static uint32_t s_pre_time
#define SET_TIME		s_pre_time = GET_SYS_TIMEMS()
#define LAST_TIME		GET_SYS_TIMEMS()
#define CHECK_TIMEOUT(x) (((GET_SYS_TIMEMS() - s_pre_time) > x) ? 1 : 0)


static enum ACT_SINGLE_UP Sta_SigTakeUp;
static enum ACT_SINGLE_DW Sta_SigLandDw;

static int Sta_Stuck = 0;

struct SIG_ACT_DATA g_st_SigData;

extern int gLiheRatio;
extern struct SYSATTR g_sys_para;
int GetEncoderSpeedCm(void);
int GetEncoderLen1Cm(void);
int GetEncoderLen2Cm(void);
int32_t Per2Power(int16_t  percent);

//Prevent pull interruption
ERR_SIG BreakOff_Check(int power,int speedcm)
{
	DEF_TIMEOUT;
	ERR_SIG ret = ERR_SIG_OK;
	
	if((speedcm > VALID_MIN_CM) && (power > Per2Power(VALID_POWM)))
	{
		SET_TIME;
		ret = ERR_SIG_OK;
	}
	else if((speedcm < VALID_MIN_CM) && (power > Per2Power(VALID_POWM)))
	{
		if(CHECK_TIMEOUT(1000))  
			ret = ERR_SIG_ENCODER;
	}
	else
	{
		if(CHECK_TIMEOUT(1000))  
			ret = ERR_SIG_PULLUP;
	}
	
	return ret;
}
/*堵转检测*/
int Stuck_Ckeck(int power,int speedcm)
{
	DEF_TIMEOUT;
	int ret = 0;
	switch(Sta_Stuck)
	{
		case 0:
			if(power > Per2Power(180) && ((speedcm) > VALID_MIN_CM))
			{
				Sta_Stuck = 1;
				SET_TIME;
			}
			else if(power > Per2Power(180))
			{
				Sta_Stuck = 2;
				SET_TIME;
			}
			break;
		case 1:
			if(power < Per2Power(150))
			{
				Sta_Stuck = 0;
			}
			if(CHECK_TIMEOUT(2000))
			{
				Sta_Stuck = 0;		// error
				ret = 1;
			}
			break;
		case 2:
			if(power < Per2Power(150) && ((speedcm) > VALID_MIN_CM))
			{
				Sta_Stuck = 0;
			}
			if(CHECK_TIMEOUT(1000))
			{
				Sta_Stuck = 0;  	// error
				ret = 2;
			}
			break;
		default:
			
			break;
	}
	return ret;
}



void Sig_ResetSta(void)
{
	Sta_SigTakeUp = SIG_IDLE;
	Sta_SigLandDw = DIG_IDLE;
}


ERR_SIG Sig_TakeUp(void)
{
	DEF_TIMEOUT;
	ERR_SIG err_sta = ERR_SIG_OK;
	static int StuckCnt = 0;
	static uint32_t StartTim = 0;
	static uint32_t ClrNullCnt = 0;
	
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();
	
//	LPrint("%d,",g_st_SigData.m_SpeedCm);
	switch(Sta_SigTakeUp)
	{
		case SIG_IDLE:
			StuckCnt = 0;
			ClrNullCnt = 0;
#ifdef USE_LIHE_PWM

	gLiheRatio = 10;
#endif
			Sta_SigTakeUp = SIG_PULL_CLUTCH;
			break;
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0); G_SHACHE(ACT_OFF,BRAKE_DLY400);
			gLiheRatio = 10;		//先给小力
			Sta_SigTakeUp = SIG_PULL_HOLD;
			SET_TIME; LPrint("Pull \r\n");
			break;
		
		case SIG_PULL_HOLD:
			if(g_st_SigData.m_SpeedCm > VALID_MIN_CM)
			{
				Sta_SigTakeUp = SIG_WAIT_VALID_UP;
				Log_e("Pos %d sp %d",g_st_SigData.m_HeightShowCm,g_st_SigData.m_SpeedCm);
				StartTim = SET_TIME;
			}
			if(CHECK_TIMEOUT(5000) || (g_st_SigData.m_HeightShowCm < -300))
			{
				if(g_st_SigData.m_Power < Per2Power(VALID_POWL))
					err_sta = ERR_SIG_CUR;
				else if(g_st_SigData.m_SpeedCm < VALID_MIN_CM)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_TIMOUT;
			}
			break;
			
		case SIG_WAIT_VALID_UP:
			if((g_st_SigData.m_SpeedCm > VALID_MIN_CM) && (g_st_SigData.m_Power > Per2Power(VALID_POWH)))
			{
				
				if(ClrNullCnt++ > 5)
				{
					#ifdef USE_LIHE_PWM
						gLiheRatio = LIHE_PWM;
						Log_e("大力");
				    #endif
					Sta_SigTakeUp = SIG_WORKUP;
					Log_e("Clr %d  %d",g_st_SigData.m_HeightShowCm,g_st_SigData.m_SpeedCm);
					Enc_Clr_TotalCnt1();
					StartTim = SET_TIME;
				}
			}
			if(g_st_SigData.m_Power < Per2Power(VALID_POWM))
			{
				ClrNullCnt = 0;
				if(CHECK_TIMEOUT(20))
				{
					Enc_Clr_TotalCnt1();
					SET_TIME;
				}
			}
			if(((LAST_TIME - StartTim) > 5000) || (g_st_SigData.m_HeightShowCm < -300))	   // need add pull
			{
				if(g_st_SigData.m_Power < Per2Power(VALID_POWM))
					err_sta = ERR_SIG_CUR;
				else if(g_st_SigData.m_SpeedCm < VALID_MIN_CM)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_PULLUP;
					
				Log_e("Takeup Err %d",err_sta);
			}
			break;
			
		case SIG_WORKUP:	// 此处高度不清零了，清零可能导致更多的问题
			if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm)
			{
				Sta_SigTakeUp = SIG_REACH_TOP;
			}
			else
			{
				#ifdef USE_LIHE_PWM
				if(g_st_SigData.m_HeightShowCm > 100)
				{
					if(gLiheRatio < 10)
					{
						gLiheRatio = 10;			//小力
						Log_e("小力");
						}
				}
				#endif
				err_sta = BreakOff_Check(g_st_SigData.m_Power,g_st_SigData.m_SpeedCm);
				if(err_sta > ERR_SIG_REACHUP)
					Log_e("%d",err_sta);
			}
			
			if(Stuck_Ckeck(g_st_SigData.m_Power,g_st_SigData.m_SpeedCm))  // if Blocked
			{
				Sta_SigTakeUp = SIG_BLOCKED;
				G_LIHE(ACT_OFF,200);
				G_SHACHE(ACT_ON,0);
				SET_TIME;
			}
			
			if(LAST_TIME > StartTim + StuckCnt * 700 + g_sys_para.s_sethighcm * 30 + 3000)   // if Timeout
			{
				err_sta = ERR_SIG_TIMOUT;
				Log_e("ERR_SIG_TIMOUT");
			}
			break;
		
		case SIG_REACH_TOP:
			err_sta = ERR_SIG_REACHUP;
#ifdef USE_LIHE_PWM
	gLiheRatio = 10;
#endif
			break;
		
		case SIG_BLOCKED:
			if(CHECK_TIMEOUT(500))
			{
				StuckCnt++;
				if(StuckCnt < 3)
				{
					G_LIHE(ACT_ON,0);
					G_SHACHE(ACT_OFF,200);
					SET_TIME;
					Sta_SigTakeUp = SIG_WORKUP;
				}
				else
				{
					err_sta = ERR_SIG_BRAKE;
					Log_e("ERR_SIG_BRAKE");
				}
			}
			break;
		
		default:break;
	}
	if(err_sta > ERR_SIG_REACHUP)
	{
		IOT_FUNC_EXIT_RC(err_sta);
	}
	else
		return err_sta;
}


ERR_SIG Sig_StudyUp(void)
{
	DEF_TIMEOUT;
	ERR_SIG err_sta = ERR_SIG_OK;
	static int dir_sure = 0,dir_cnt = 0;
	
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();
	
	switch(Sta_SigTakeUp)
	{
		case SIG_IDLE:
			g_sys_para.s_pnull = g_st_SigData.m_Power;
			Sta_SigTakeUp = SIG_PULL_CLUTCH;
			Enc_Clr_TotalCnt1();
			gLiheRatio = LIHE_PWM;
			dir_cnt = 0;dir_sure = 0;
			break;
		
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0);
			G_SHACHE(ACT_OFF,400);
			Sta_SigTakeUp = SIG_PULL_HOLD;
			Log_e("Pos %d sp %d",g_st_SigData.m_HeightShowCm,g_st_SigData.m_SpeedCm);
			SET_TIME;
		break;
		
		case SIG_PULL_HOLD:
			if(g_st_SigData.m_SpeedCm > VALID_MIN_CM)
			{
				dir_sure++;
				if(dir_sure > 40)
				{
					Sta_SigTakeUp = SIG_WAIT_VALID_UP;
				}
				if(dir_sure == 3)
				{
					Log_e("Pos %d",g_st_SigData.m_HeightShowCm);
				}
					
				SET_TIME;
			}
			if(CHECK_TIMEOUT(1000))
			{
				SET_TIME;
				if(g_st_SigData.m_SpeedCm < -VALID_MIN_CM)
				{
					if(g_sys_para.s_dir > 0)
						g_sys_para.s_dir = 0;
					else
						g_sys_para.s_dir = 1;
					Log_e("dir %d  %d",g_sys_para.s_dir,g_st_SigData.m_SpeedCm);
					Enc_Set_Dir(g_sys_para.s_dir);
				}
				
				dir_cnt++;
				if(dir_cnt > 4)
				{
					err_sta = ERR_SIG_ENCODER;
				}	
			}
			break;
		
		case SIG_WAIT_VALID_UP:
			if(g_st_SigData.m_SpeedCm > VALID_MIN_CM)
			{
				if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm / 2)
				{
					g_sys_para.s_pfull = g_st_SigData.m_Power;
					Sta_SigTakeUp = SIG_WORKUP;
				}
				SET_TIME;
			}
			if(CHECK_TIMEOUT(2000))	   // need add pull
			{
				if(g_st_SigData.m_SpeedCm < VALID_MIN_CM)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_PULLUP;
					
				Log_e("%d",g_st_SigData.m_SpeedCm);
			}
		break;
			
		case SIG_WORKUP:
			if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm)
			{
				Sta_SigTakeUp = SIG_REACH_TOP;
			}
			if(g_sys_para.s_pfull < g_sys_para.s_pnull + 1000)
			{
				err_sta = ERR_SIG_CUR;
				Log_e("f%d  n%d",g_sys_para.s_pfull,g_sys_para.s_pnull);
			}
		break;
		
		case SIG_REACH_TOP:
			err_sta = ERR_SIG_REACHUP;
		break;
		default:break;
	}
	
	if(err_sta > ERR_SIG_REACHUP)
	{
		IOT_FUNC_EXIT_RC(err_sta);
	}
	else
		return err_sta;
}


ERR_SIG Sig_LandDw(void)
{	
	DEF_TIMEOUT;
	static int32_t s_ClingCnt = 0;
	
	ERR_SIG err_dw = ERR_SIG_OK;
	
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();
//	LPrint("%d,",g_st_SigData.m_Power / 100);
	switch(Sta_SigLandDw)
	{
		case DIG_IDLE:
			SET_TIME;
			s_ClingCnt = 0;
			G_LIHE(ACT_OFF,0);
			G_SHACHE(ACT_OFF, 300);
			Sta_SigLandDw = DIG_CHKDW; 
			LPrint("Dw  %d\r\n",g_st_SigData.m_HeightShowCm);
			break;
			
		case DIG_CHKDW:
			if((g_st_SigData.m_SpeedCm < VALID_MIN_CM) && (g_st_SigData.m_HeightShowCm < (g_sys_para.s_sethighcm + 100)))
			{
				Sta_SigLandDw = DIG_WAIT_LIHE;
				SET_TIME;
			}
			if(g_st_SigData.m_HeightShowCm > (g_sys_para.s_sethighcm + 120))	// over 
			{
				err_dw = ERR_SIG_CLING;
			}else if(CHECK_TIMEOUT(2000))
			{
				Sta_SigLandDw = DIG_BLOCKED;
				G_SHACHE(ACT_ON, 0);
				SET_TIME;
				s_ClingCnt++;
				if(s_ClingCnt > 3)
				{
					err_dw = ERR_SIG_CLING;
					Log_e("ERR_SIG_CLING");
				}
			}
			break;
		case DIG_BLOCKED:
			if(g_st_SigData.m_SpeedCm >= VALID_MIN_CM)
			{
				if(CHECK_TIMEOUT(300))
				{
					Sta_SigLandDw = DIG_CHKDW;
					G_SHACHE(ACT_OFF, 0);
					SET_TIME;
					Log_e("BLOCK OK");
				}
			}
			else
			{
				G_SHACHE(ACT_OFF, 0);
				Sta_SigLandDw = DIG_CHKDW;
				Log_e("BLOCK back");
			}
			
			if(CHECK_TIMEOUT(4000) || (g_st_SigData.m_HeightShowCm > (g_sys_para.s_sethighcm + 150)))
			{
				if(g_st_SigData.m_Power > Per2Power(140))
				{
					err_dw = ERR_SIG_CLING;
					Log_e("ERR_SIG_CLING");
				}
				else
				{
					err_dw = ERR_SIG_BRAKE;
					Log_e("ERR_SIG_BRAKE");
				}
			}
			break;
			
		case DIG_WAIT_LIHE:
			if(g_st_SigData.m_HeightShowCm < g_sys_para.s_hlihe)
			{
				err_dw = ERR_SIG_REACHDW;
				Log_e("ERR_SIG_REACHDW");
			}
			if(CHECK_TIMEOUT(5000))
			{
				err_dw = ERR_SIG_TIMOUT;
				Log_e("ERR_SIG_TIMOUT");
			}
				
			break;
		
		default:break;
	}
	if(err_dw > ERR_SIG_REACHDW)
		Log_e("%d  %d",Sta_SigLandDw,err_dw);
	
	return err_dw;
}




int GetEncoderSpeedCm(void)
{
	float ftmp;
	int ret;
	static int prespeed = 0;
	
	ftmp = Enc_Get_SpeedE1();
	ftmp = ftmp * g_sys_para.s_zhou / g_sys_para.s_numchi;
	ftmp = ftmp / 10;
	ret = (int)ftmp;
	
	ret = (ret + prespeed)>> 1;
	prespeed = ret;
	return ret;
}

int GetEncoderAcceCm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_Acce();
	ftmp = ftmp * g_sys_para.s_zhou / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}

int GetEncoderLen1Cm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_CNT1();
	ftmp = ftmp * g_sys_para.s_zhou / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}

int GetEncoderLen2Cm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_CNT2();
	ftmp = ftmp * g_sys_para.s_zhou / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}

/*计算出有功功率比值对应的实际功率*/
int32_t Per2Power(int16_t  percent)
{
	uint32_t pw;
	
	pw = (g_sys_para.s_pfull - g_sys_para.s_pnull) * percent / 100;
	pw += g_sys_para.s_pnull;
	
	return pw;
}
