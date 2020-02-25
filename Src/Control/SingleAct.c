

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
#define CHECK_TIMEOUT(x) (((GET_SYS_TIMEMS() - s_pre_time) > x) ? 1 : 0)


static enum ACT_SINGLE_UP Sta_SigTakeUp;
static enum ACT_SINGLE_DW Sta_SigLandDw;

struct SIG_ACT_DATA g_st_SigData;


extern struct SYSATTR g_sys_para;
int GetEncoderSpeedCm(void);
int GetEncoderLen1Cm(void);
int GetEncoderLen2Cm(void);
int32_t Per2Power(int16_t  percent);


int Stuck_Ckeck(int power,int speed)
{
	
	return 0;
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
	
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();

	switch(Sta_SigTakeUp)
	{
		case SIG_IDLE:
			Sta_SigTakeUp = SIG_PULL_CLUTCH;
			break;
		
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0);
			G_SHACHE(ACT_OFF,400);
			Sta_SigTakeUp = SIG_PULL_HOLD;
			SET_TIME;
			break;
		
		case SIG_PULL_HOLD:
			if(g_st_SigData.m_SpeedCm > 10)
			{
				Sta_SigTakeUp = SIG_WAIT_VALID_UP;
				SET_TIME;
			}
			if(CHECK_TIMEOUT(3000) || (g_st_SigData.m_HeightShowCm < 300))
			{
				if(g_st_SigData.m_Power < Per2Power(30))
					err_sta = ERR_SIG_CUR;
				else if(g_st_SigData.m_SpeedCm < 10)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_PULLUP;
			}
			break;
			
		case SIG_WAIT_VALID_UP:
			if((g_st_SigData.m_SpeedCm > 10) && (g_st_SigData.m_Power > Per2Power(50)))
			{
				Sta_SigTakeUp = SIG_WORKUP;
				Enc_Clr_TotalCnt1();
				SET_TIME;
			}
			if(CHECK_TIMEOUT(2000) || (g_st_SigData.m_HeightShowCm < 300))	   // need add pull
			{
				if(g_st_SigData.m_Power < Per2Power(30))
					err_sta = ERR_SIG_CUR;
				else if(g_st_SigData.m_SpeedCm < 10)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_PULLUP;
			}
			break;
			
		case SIG_WORKUP:
			if(Stuck_Ckeck(g_st_SigData.m_Power,g_st_SigData.m_SpeedCm))
			{
				Sta_SigTakeUp = SIG_BLOCKED;
				SET_TIME;
			}
			if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm)
			{
				Sta_SigTakeUp = SIG_REACH_TOP;
			}
			break;
		
		case SIG_REACH_TOP:
			err_sta = ERR_SIG_REACHED;
			break;
		
		case SIG_BLOCKED:
			
			break;
		
		default:break;
	}
	
	IOT_FUNC_EXIT_RC(err_sta);
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
			dir_cnt = 0;dir_sure = 0;
		break;
		
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0);
			G_SHACHE(ACT_OFF,400);
			Sta_SigTakeUp = SIG_PULL_HOLD;
			SET_TIME;
		break;
		
		case SIG_PULL_HOLD:
			if(g_st_SigData.m_SpeedCm > 10)
			{
				dir_sure++;
				if(dir_sure > 40)
				{
					Sta_SigTakeUp = SIG_WAIT_VALID_UP;
				}
				SET_TIME;
			}
			if(CHECK_TIMEOUT(1000))
			{
				SET_TIME;
				if(g_st_SigData.m_SpeedCm < -10)
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
			if(g_st_SigData.m_SpeedCm > 10)
			{
				if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm / 2)
				{
					g_sys_para.s_pfull = g_st_SigData.m_Power;
					Sta_SigTakeUp = SIG_WORKUP;
					SET_TIME;
				}
			}
			if(CHECK_TIMEOUT(2000))	   // need add pull
			{
				if(g_st_SigData.m_SpeedCm < 10)
					err_sta = ERR_SIG_ENCODER;
				else
					err_sta = ERR_SIG_PULLUP;
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
			}
		break;
		
		case SIG_REACH_TOP:
			err_sta = ERR_SIG_REACHED;
		break;
		default:break;
	}
	
	IOT_FUNC_EXIT_RC(err_sta);
}


ERR_SIG Sig_LandDw(void)
{	
	DEF_TIMEOUT;
	static int32_t s_ClingCnt = 0;
	
	ERR_SIG err_dw = ERR_SIG_OK;
	
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();
	
	switch(Sta_SigLandDw)
	{
		case DIG_IDLE:
			SET_TIME;
			s_ClingCnt = 0;
			G_LIHE(ACT_OFF,0);
			G_SHACHE(ACT_OFF, 300);
			Sta_SigLandDw = DIG_CHKDW; break;
			
		case DIG_CHKDW:
			if((g_st_SigData.m_SpeedCm < -5) && (g_st_SigData.m_HeightShowCm < (g_sys_para.s_sethighcm + 100)))
			{
				if(g_st_SigData.m_Power < Per2Power(70))
				{
					Sta_SigLandDw = DIG_WAIT_LIHE;
				}
			}
			if(CHECK_TIMEOUT(1200))
			{
				Sta_SigLandDw = DIG_BLOCKED;
				G_SHACHE(ACT_ON, 300);
				SET_TIME;
				s_ClingCnt++;
				if(s_ClingCnt > 3)
					err_dw = ERR_SIG_CLING;
			}
			break;
		case DIG_BLOCKED:
			if(g_st_SigData.m_SpeedCm < 10)
			{
				if(CHECK_TIMEOUT(300))
				{
					Sta_SigLandDw = DIG_CHKDW;
					G_SHACHE(ACT_OFF, 300);
					SET_TIME;
				}
			}
			if(CHECK_TIMEOUT(3000) || (g_st_SigData.m_HeightShowCm < (g_sys_para.s_sethighcm + 150)))
			{
				if(g_st_SigData.m_Power > Per2Power(140))
				{
					err_dw = ERR_SIG_CLING;
				}
				else
					err_dw = ERR_SIG_BRAKE;
			}
			break;
			
		case DIG_WAIT_LIHE:
			if(g_st_SigData.m_HeightShowCm < g_sys_para.s_hlihe)
			{
				err_dw = ERR_SIG_REACHED;
			}
			break;
		
		default:break;
	}
	Log_e("%d  %d",Sta_SigLandDw,err_dw);
	
	return err_dw;
//	IOT_FUNC_EXIT_RC(err_dw);
}




int GetEncoderSpeedCm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_Speed();
	ftmp = ftmp * g_sys_para.s_zhou / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
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
