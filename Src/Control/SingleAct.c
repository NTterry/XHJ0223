

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


static enum ACT_SINGLE_UP Sta_SigTakeUp;     //提锤检测状态
static enum ACT_SINGLE_DW Sta_SigLandDw;     //下锤检测状态
static int Sta_Stuck = 0;					 //堵转检测状态

struct SIG_ACT_DATA g_st_SigData;			 //打锤时的实时高度，速度信息

extern int gLiheRatio;						 //离合信号比例 0 - 10
extern struct SYSATTR g_sys_para;

int GetEncoderSpeedCm(void);
int GetEncoderLen1Cm(void);
int GetEncoderLen2Cm(void);
int32_t Per2Power(int16_t  percent);



void GetLiveData(void)
{
	g_st_SigData.m_HeightShowCm = GetEncoderLen1Cm();
	g_st_SigData.m_Power = Get_FRQE2();
	g_st_SigData.m_SpeedCm = GetEncoderSpeedCm();
	g_st_SigData.m_HeighRammCm = GetEncoderLen2Cm();
}

/*
	work up check  if proper taking up
	return if it back to normal
	
*/
static ERR_SIG BreakOff_Check(int power,int speedcm)
{
	DEF_TIMEOUT;
	ERR_SIG ret = ERR_SIG_OK;
	
	if((speedcm > VALID_MIN_CM) && (power > Per2Power(VALID_POWM)) && (power < Per2Power(VALID_POWOVER)))
	{
		SET_TIME;
		ret = ERR_SIG_OK;
	}
	else if((speedcm < VALID_MIN_CM) && (power > Per2Power(VALID_POWOVER)))
	{
		if(CHECK_TIMEOUT(1000))  
			ret = ERR_SIG_PULLUP;
	}
	else if((speedcm < VALID_MIN_CM) && (power > Per2Power(VALID_POWM)))
	{
		if(CHECK_TIMEOUT(3000))  
			ret = ERR_SIG_ENCODER;
	}else
	{
		if(CHECK_TIMEOUT(3000))  
			ret = ERR_SIG_SOFT;
	}
	
	return ret;
}

/*堵转检测*/
static int Stuck_Ckeck(int power,int speedcm)
{
	DEF_TIMEOUT;
	int ret = 0;
	switch(Sta_Stuck)
	{
		case 0:
			if(power > Per2Power(160) && ((speedcm) > VALID_MIN_CM))
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


/************************************************************
  * @brief   初始化为Reset 状态
  * @param   none
  * @return  状态
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    none
  ***********************************************************/
void Sig_ResetSta(void)
{
	Sta_SigTakeUp = SIG_IDLE;
	Sta_SigLandDw = DIG_IDLE;
}


/************************************************************
  * @brief   正常模式的提锤
  * @param   none
  * @return  状态
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    故障判断，超时时间由提锤高度确定
  ***********************************************************/
ERR_SIG Sig_TakeUp(void)
{
	DEF_TIMEOUT;
	ERR_SIG err_sta = ERR_SIG_OK;
	static int StuckCnt = 0;
	static uint32_t StartTim = 0;
	static uint32_t ClrNullCnt = 0;
	
	switch(Sta_SigTakeUp)
	{
		case SIG_IDLE:
			StuckCnt = 0;
			ClrNullCnt = 0;
			Sta_SigTakeUp = SIG_PULL_CLUTCH;
			break;
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0); G_SHACHE(ACT_OFF,BRAKE_DLY400);
			
#ifdef USE_LIHE_PWM
			gLiheRatio = LIHE_TINY;							/*先给小力  */
#endif
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
						gLiheRatio = LIHE_BIG;
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
			if((g_st_SigData.m_HeighRammCm >  g_sys_para.s_hprot) && g_sys_para.s_mode)     /*夯土模式下，设定保护高度*/
			{
				g_st_SigData.m_Lihenew = g_sys_para.s_setlihecm * g_st_SigData.m_HeightShowCm / g_sys_para.s_sethighcm;// 比例更新离合点的高度
				/*更新离合点的高度*/
				Sta_SigTakeUp = SIG_REACH_TOP;
			}
			else if(g_st_SigData.m_HeightShowCm > g_sys_para.s_sethighcm)
			{
				Sta_SigTakeUp = SIG_REACH_TOP;
				g_st_SigData.m_Lihenew = g_sys_para.s_setlihecm;   //正常更新离合点的高度
			}
			else
			{
#ifdef USE_LIHE_PWM
				if(g_st_SigData.m_HeightShowCm > 100)
				{
					gLiheRatio = LIHE_TINY;			//小力
					Log_e("小力");
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
			gLiheRatio = LIHE_TINY;
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

/************************************************************
  * @brief   学习模式的提锤
  * @param   none
  * @return  状态
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    先进行编码器清零
			 提锤超时时间由提锤高度确定
  ***********************************************************/
ERR_SIG Sig_StudyUp(void)
{
	DEF_TIMEOUT;
	ERR_SIG err_sta = ERR_SIG_OK;
	static int dir_sure = 0,dir_cnt = 0;
	
	switch(Sta_SigTakeUp)
	{
		case SIG_IDLE:
			g_sys_para.s_pnull = g_st_SigData.m_Power;   //当前的功率为空载功率
			Enc_Clr_TotalCnt1();
			
#ifdef USE_LIHE_PWM
			gLiheRatio = LIHE_BIG;
#endif
			dir_cnt = 0;dir_sure = 0;
			Sta_SigTakeUp = SIG_PULL_CLUTCH;
			break;
		
		case SIG_PULL_CLUTCH:
			G_LIHE(ACT_ON,0);
			G_SHACHE(ACT_OFF,400);
			Log_e("Pos %d sp %d",g_st_SigData.m_HeightShowCm,g_st_SigData.m_SpeedCm);
			SET_TIME;
			Sta_SigTakeUp = SIG_PULL_HOLD;
		break;
		
		case SIG_PULL_HOLD:
			if(g_st_SigData.m_SpeedCm > VALID_MIN_CM)    //检测到速度有效
			{
				dir_sure++;
				if(dir_sure > 40)
				{
					Sta_SigTakeUp = SIG_WAIT_VALID_UP;
				}
				if(dir_sure == 3)
				{
					Log_e("upPos %d",g_st_SigData.m_HeightShowCm);
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
					g_sys_para.s_pfull = g_st_SigData.m_Power; // 超过设定一半时，保存有效拉力值
					g_st_SigData.m_Lihenew = g_sys_para.s_setlihecm;
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
			if(CHECK_TIMEOUT(5000))
			{
				err_sta = ERR_SIG_TIMOUT;
				Log_e("Time out");
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

/************************************************************
  * @brief   d打锤动作，先松离合，再松刹车(if exist)
  * @param   none
  * @return  状态
            ERR_SIG_CLING:黏离合
			ERR_SIG_BRAKE:刹车故障
			ERR_SIG_REACHDW:正常到底
			ERR_SIG_TIMOUT：下降超时
			
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    状态机判断，不堵塞执行
  ***********************************************************/
ERR_SIG Sig_LandDw(void)
{	
	DEF_TIMEOUT;
	static int32_t s_ClingCnt = 0;
	
	ERR_SIG err_dw = ERR_SIG_OK;
	
	switch(Sta_SigLandDw)
	{
		case DIG_IDLE:
			SET_TIME;
			s_ClingCnt = 0;
			G_LIHE(ACT_OFF,0);
			G_SHACHE(ACT_OFF, 300);
			Sta_SigLandDw = DIG_CHKDW; 
			LPrint("Dw Top  %d\r\n",g_st_SigData.m_HeightShowCm);
			break;
			
		case DIG_CHKDW:
			if((g_st_SigData.m_SpeedCm < VALID_MIN_CM) && (g_st_SigData.m_HeightShowCm < (g_sys_para.s_sethighcm + 100)))
			{
				Sta_SigLandDw = DIG_WAIT_LIHE;
				SET_TIME;
			}
			if(g_st_SigData.m_HeightShowCm > (g_sys_para.s_sethighcm + 120))	// 黏离合的保护超程为1.2米
			{
				err_dw = ERR_SIG_CLING;
				Log_e("ERR_SIG_CLING");
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
			if(g_st_SigData.m_HeightShowCm < g_st_SigData.m_Lihenew)    //与更新后离合点的高度比较
			{
				err_dw = ERR_SIG_REACHDW;
				Log_e("ERR_SIG_REACHDW");
			}
			if(CHECK_TIMEOUT(6000))       /*该时间需要继续判断*/
			{
				err_dw = ERR_SIG_TIMOUT;
				Log_e("ERR_SIG_TIMOUT");
			}
				
			break;
		
		default:
			Log_e("Not Exist");
			break;
	}
	
	return err_dw;
}



/************************************************************
  * @brief   得到编码器速度，带方向，向上为正
  * @param   none
  * @return  cm/s
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    内部滤波处理
  ***********************************************************/
int GetEncoderSpeedCm(void)
{
	float ftmp;
	int ret;
	static int prespeed = 0;
	
	ftmp = Enc_Get_SpeedE1();
	ftmp = ftmp * g_sys_para.s_pericm / g_sys_para.s_numchi;
	ftmp = ftmp / 10;
	ret = (int)ftmp;
	
	ret = (ret + prespeed)>> 1;
	prespeed = ret;
	return ret;
}

/************************************************************
  * @brief   得到编码器速度，带方向，向上为正
  * @param   none
  * @return  cm/s
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    内部滤波处理
  ***********************************************************/
int GetEncoderAcceCm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_Acce();
	ftmp = ftmp * g_sys_para.s_pericm / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}
/************************************************************
  * @brief   得到速度1，用于打锤的计量，带方向，向上为正
  * @param   none
  * @return  cm/s
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    内部滤波处理
  ***********************************************************/
int GetEncoderLen1Cm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_CNT1();
	ftmp = ftmp * g_sys_para.s_pericm / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}
/************************************************************
  * @brief   得到编码器方向，用于夯土流程，带方向，向上为正
  * @param   none
  * @return  cm/s
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    none
  ***********************************************************/
int GetEncoderLen2Cm(void)
{
	float ftmp;
	int ret;
	
	ftmp = Enc_Get_CNT2();
	ftmp = ftmp * g_sys_para.s_pericm / g_sys_para.s_numchi;
	
	ret = (int)ftmp;
	return ret;
}

/************************************************************
  * @brief   计算实际有效值的比例
  * @param   0-100  
  * @return  cm/s
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    none
  ***********************************************************/
int32_t Per2Power(int16_t  percent)
{
	uint32_t pw;
	
	pw = (g_sys_para.s_pfull - g_sys_para.s_pnull) * percent / 100;
	pw += g_sys_para.s_pnull;
	
	return pw;
}
