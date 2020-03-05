
#include "Hangtu.h"
#include "port.h"
#include "GUI.h"
#include "Encoder.h"
#include "u_log.h"
#include "SingleAct.h"
#include "Frq_Mens.h"


#define GET_TICK_MS()  osKernelSysTick()
#define DELAYMS(x)	   osDelay(x)

#define HAS_REACH_DOWN		1

#define STA_CLEAR(x)        x = 0

extern struct SIG_ACT_DATA g_st_SigData;
extern volatile struct GUI_DATA	g_GuiData;

static SYS_STA LiuChkDw(int LPosCm,int LSpeedCm);
static SYS_STA Check_up(int LPosCm);
static SYS_STA hangtu(uint16_t * pst);


volatile uint32_t g_halt = 0;


/*夯土整个工艺流程  */
/*
提锤  下料  溜放   打锤
*/

struct HANGTU s_hang;                                   /*夯土流程状态检测*/
struct RECORD s_record;									/*待上传的数据*/
struct SYSATTR g_sys_para;								// 系统参数
extern uint16_t g_led_sta;

/*紧急停机处理*/
void Halt_Stop(void)
{
	G_LIHE(ACT_OFF,0);
	G_SHACHE(ACT_ON,0);
}

/*计算当前的时间间隔*/
static uint32_t GET_MS_DIFF(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = GET_TICK_MS();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*超过一天的跨度*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}


/*      向内存中写入数据   */
void RecordIn(int idx,uint16_t data)
{
	usRegHoldingBuf[idx] = data;
}


/*Modbus 初始化时设定的数据*/
void ModbusData_Init(void)
{
	RecordIn(M_NUB, 0);
	RecordIn(M_TIMES, g_sys_para.s_rammcnt);
	RecordIn(M_THIGH, g_sys_para.s_sethighcm);
	RecordIn(M_DEEP, 0);
	RecordIn(M_SONGTU, g_sys_para.s_feedtims);
	RecordIn(M_STATE, 0);
	RecordIn(M_ERR, 0);
	RecordIn(M_PERCNT, g_sys_para.s_numchi);
	RecordIn(M_ZHOU, g_sys_para.s_pericm);
	RecordIn(M_TLIAO, g_sys_para.s_feedtims);
	RecordIn(M_CCNUB, g_sys_para.s_rammcnt);
	RecordIn(M_TICHUI, g_sys_para.s_sethighcm);
	RecordIn(M_LIHE, g_sys_para.s_setlihecm);
	RecordIn(M_HPROT, g_sys_para.s_hprot);
	RecordIn(M_ACCRE,100);			/*授权码*/
	
	s_record.nub = 0;
	s_record.deepth = 0;
	s_record.high = 0;
	s_record.tim = 0;
	s_record.cnt = 0;
}
/*系统参数信息上传*/
void ModbusData_flash(void)
{
	RecordIn(M_PERCNT,g_GuiData.g_num);
	RecordIn(M_ZHOU,g_GuiData.g_Zhoucm);
	RecordIn(M_TLIAO,g_GuiData.g_ts);
	RecordIn(M_CCNUB,g_GuiData.g_hcnt);
	RecordIn(M_TIMES,g_GuiData.g_hcnt);
	RecordIn(M_TICHUI,g_GuiData.g_sethighcm);
	RecordIn(M_LIHE,g_GuiData.g_lihe);
	RecordIn(M_HPROT,g_GuiData.g_HighOvercm);
}

/*高度信息，深度信息显示到触摸屏上*/
void ModbusData_Show(void)
{
	RecordIn(M_DEEP,g_GuiData.g_nhigh);
	RecordIn(M_THIGH,g_GuiData.g_sethighcm);
	RecordIn(M_KS, s_hang.last_downnum);						/*显示桩底深度*/
	RecordIn(M_ERR,g_st_SigData.m_errnum);
}

/*外部参数设定 更新检查   循环*/
extern uint32_t savecnt;

void ModbusData_Chk(void)
{
	/*每圈齿数*/
	if(usRegHoldingBuf[M_PERCNT] != g_GuiData.g_num )
	{
		if((usRegHoldingBuf[M_PERCNT] < 300) && (usRegHoldingBuf[M_PERCNT] > 10))
		{
			g_GuiData.g_num  = usRegHoldingBuf[M_PERCNT];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_PERCNT] = g_GuiData.g_num;
		}
	}
	/*卷筒周长*/
	if(usRegHoldingBuf[M_ZHOU] != g_GuiData.g_Zhoucm)
	{
		if((usRegHoldingBuf[M_ZHOU] < 300) && (usRegHoldingBuf[M_ZHOU] > 20))
		{
			g_GuiData.g_Zhoucm = usRegHoldingBuf[M_ZHOU];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_ZHOU] = g_GuiData.g_Zhoucm;
		}
	}
	/*送料的时间*/
	if(usRegHoldingBuf[M_TLIAO] != g_GuiData.g_ts)
	{
		if((usRegHoldingBuf[M_TLIAO] < 16) && (usRegHoldingBuf[M_TLIAO] > 1))
		{
			g_GuiData.g_ts = usRegHoldingBuf[M_TLIAO];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TLIAO] = g_GuiData.g_ts;
		}
	}
	/*设定打锤的次数*/
	if(usRegHoldingBuf[M_CCNUB] != g_GuiData.g_hcnt)
	{
		if((usRegHoldingBuf[M_CCNUB] < 11) && (usRegHoldingBuf[M_CCNUB] > 2))
		{
			g_GuiData.g_hcnt = usRegHoldingBuf[M_CCNUB];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_CCNUB] = g_GuiData.g_hcnt;
		}
	}
	/*设定提锤高度*/
	if(usRegHoldingBuf[M_TICHUI] != g_GuiData.g_sethighcm)
	{
		if((usRegHoldingBuf[M_TICHUI] <= MAXSET_HIGH) && (usRegHoldingBuf[M_TICHUI] > MINSET_HIGH))
		{
			g_GuiData.g_sethighcm = usRegHoldingBuf[M_TICHUI];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TICHUI] = g_GuiData.g_sethighcm;
		}
	}
	/*设定离合点的位置*/
	if(usRegHoldingBuf[M_LIHE] != g_GuiData.g_lihe)
	{
		if((usRegHoldingBuf[M_LIHE] <= (g_GuiData.g_sethighcm - 20)))
		{
			g_GuiData.g_lihe = usRegHoldingBuf[M_LIHE];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_LIHE] = g_GuiData.g_lihe;
		}
	}
	
	/*设定上拉高度保护值*/
	if(usRegHoldingBuf[M_HPROT] != g_GuiData.g_HighOvercm)
	{
		if(usRegHoldingBuf[M_HPROT] <= 1000)
		{
			g_GuiData.g_HighOvercm = usRegHoldingBuf[M_HPROT];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_HPROT] = g_GuiData.g_HighOvercm;
		}
	}
	
		/*分段打锤次数*/
	g_GuiData.s_high1 = usRegHoldingBuf[M_HIGH1];
	g_GuiData.s_cnt1 = usRegHoldingBuf[M_CNT1];
	g_GuiData.s_high2 = usRegHoldingBuf[M_HIGH2];
	g_GuiData.s_cnt2 = usRegHoldingBuf[M_CNT2];
}

/*Led状态指示灯  夯土模式下*/
void LedSta_Show(uint8_t ledsta)
{
	LED_BIT_CLR(SIG_SALL);					/*Led灯清零*/
	switch(ledsta)
	{
		case S_IDLE:		/*空闲*/
			RecordIn(M_STATE, 0);
			break;
		case S_PULSE:
		case S_TICHUI:		/*提锤*/
			LED_BIT_SET(SIG_STICHUI);
			RecordIn(M_STATE, 3);
			break;
		case S_CHECK_UPSTOP:
			LED_BIT_SET(SIG_SZHUCHUI);
			RecordIn(M_STATE, 4);
			break;
		case S_XIALIAO:		/*下料*/
			LED_BIT_SET(SIG_STU);
			RecordIn(M_STATE, 5);
			break;
		case S_DELAY2:
		case S_LIUF:		/*溜放*/
			LED_BIT_SET(SIG_SLIUF);
			RecordIn(M_STATE, 1);
			break;
		case S_DACHUI:		/*夯土*/
			LED_BIT_SET(SIG_SHANGTU);
			RecordIn(M_STATE, 2);
			break;
		default:break;
	}
}


/************************************************************
  * @brief   外部按键检测，点动提锤  自动按钮  离合测试  刹车测试
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    最好放在操作系统的系统时钟回调函数中执行
  ***********************************************************/
int liu_flg = 0;
int sha_flg = 0;
void EXT_BUTTON_CHK(void)
{
	static uint16_t b_Dd;
	static uint8_t k_Cluch;
	static uint8_t k_Brk;
	static uint16_t b_Zd;
	static uint16_t b_stop;
	static uint8_t b_liu;
	
	b_Dd <<= 1;
	b_Zd <<= 1;
	k_Cluch <<= 1;
	k_Brk <<= 1;
	b_stop <<= 1;
	b_liu <<= 1;
	
	if(KEY_DD == GPIO_PIN_SET)
		b_Dd |= 0x01;
	if(KEY_ZD == GPIO_PIN_SET)
		b_Zd |= 0x01;
	if(KEY_TLH == GPIO_PIN_RESET)
		k_Cluch |= 0x01;
	if(KEY_TSC == GPIO_PIN_RESET)
		k_Brk |= 0x01;
	if(KEY_STOP == GPIO_PIN_SET)
		b_stop |= 0x01;
	if(KEY_LIU == GPIO_PIN_SET)
		b_liu |= 0x01;
		
	if(b_stop > 0xFFF)
	{
		g_halt = 0;
	}
	else
	{
		g_halt = 1;
	}
		
	if(b_Dd > 0xFFF)
	{
		IOT_FUNC_ENTRY;
		if(g_st_SigData.m_Mode == MOD_FREE)
			g_st_SigData.m_Mode = MOD_MANUAL;
	}
	else
	{
		if(g_st_SigData.m_Mode == MOD_MANUAL)
			g_st_SigData.m_Mode = MOD_MANOFF;
	}
	
	if(b_Zd > 0x1FFF)   //增强抗干扰能力
	{
		IOT_FUNC_ENTRY;
		if(g_st_SigData.m_Mode == MOD_FREE)
		{
			b_Zd = 0;
			if(g_sys_para.s_mode == 0)
				g_st_SigData.m_Mode = MOD_SIGACT;     /*进入自动打锤模式*/
		}
		else if(g_st_SigData.m_Mode != MOD_FREE)      /*这个按钮可能失灵了，会自动停机*/
		{
			b_Zd = 0;
			g_st_SigData.m_Mode = MOD_FREE;
		}
	}
	
	if((k_Cluch == 0xFF) || (k_Brk == 0xFF)||(b_liu == 0xff))
	{
		if(g_st_SigData.m_Mode == MOD_FREE)
		{
			g_st_SigData.m_Mode = MOD_TST;
		}
		
		if(k_Cluch == 0xFF)
			G_LIHE(ACT_ON,0);
		else
			G_LIHE(ACT_OFF,0);
			
		if(k_Brk == 0xFF)
		{
			if(sha_flg <= 0)
			{
				G_SHACHE(ACT_OFF,0);
				sha_flg = 1;
				Log_e("ACT_OFF");
			}
		}
		else
		{
			if(sha_flg)
			{
				sha_flg = 0;
				G_SHACHE(ACT_ON,0);
				Log_e("ACT_ON");
			}
		}
		
		if(b_liu == 0xff)
		{
			if(liu_flg <= 0)
			{
				G_SHACHE(ACT_LIU,0);
				liu_flg = 1;
				Log_e("ACT_LIU");
			}
		}
		else
		{
			if(liu_flg > 0)
			{
				liu_flg = 0;
				G_SHACHE(ACT_ON,0);
				Log_e("ACT_ON");
			}
		}
	}
	else
	{
		if(g_st_SigData.m_Mode == MOD_TST)
		{
			g_st_SigData.m_Mode = MOD_FREE;
			G_LIHE(ACT_OFF,0);G_SHACHE(ACT_ON,0);
			sha_flg = 0;liu_flg = 0;
		}
	}
}


static enum {
	e_STEP_READY,
	e_STEP_STUDY,
	e_STEP_PULL,
	e_STEP_DOWN,
}s_SigActStep;


uint16_t TampStep;

#define SIGACT_READY    s_SigActStep = e_STEP_READY
#define TAMP_READY		TampStep = S_IDLE;

extern void SetSaveFlag(void);


/************************************************************
  * @brief   整个单打过程，先提锤，后打锤   非堵塞
  * @param   mode : 0 第一下提锤为学习模式
                    1 直接启动
  * @return  错误状态
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    包含提锤和打锤的过程，
             提锤到顶返回 ERR_SIG_REACHUP
			 打锤结束返回 ERR_SIG_REACHDW
			 正常工作返回 ERR_SIG_OK
			 其余为错误模式
			 函数非重入
  ***********************************************************/
int SingleAct(int mode)
{
	ERR_SIG ret = ERR_SIG_OK;
	
	switch(s_SigActStep)
	{
	case e_STEP_READY:
		LED_BIT_SET(SIG_TICHUI);
		LED_BIT_CLR(SIG_FANGCHUI);
		if(mode)
			s_SigActStep = e_STEP_PULL;
		else
			s_SigActStep = e_STEP_STUDY;	
			
		Sig_ResetSta();
		break;
	case e_STEP_STUDY:
		ret = Sig_StudyUp();
		if(ret == ERR_SIG_REACHUP)
		{
			s_SigActStep = e_STEP_DOWN;
			Sig_ResetSta();
			LED_BIT_SET(SIG_FANGCHUI);
			LED_BIT_CLR(SIG_TICHUI);
			SetSaveFlag();
		}
		break;
	case e_STEP_PULL:
		ret = Sig_TakeUp();
		if(ret == ERR_SIG_REACHUP)
		{
			s_SigActStep = e_STEP_DOWN;
			Sig_ResetSta();
			LED_BIT_SET(SIG_FANGCHUI);
			LED_BIT_CLR(SIG_TICHUI);
		}
		break;
	case e_STEP_DOWN:
		ret = Sig_LandDw();
		if(ret == ERR_SIG_REACHDW)
		{
			s_SigActStep = e_STEP_PULL;
			Sig_ResetSta();
			LED_BIT_SET(SIG_TICHUI);
			LED_BIT_CLR(SIG_FANGCHUI);
		}
		break;
	 default:ret = ERR_SIG_SOFT;s_SigActStep = e_STEP_READY;
	    Log_e("ERR_SIG_SOFT");
	    break;
	}
	
	return ret;
}
  



/************************************************************
  * @brief   动作执行流程  包括自动打桩 和 自动夯土
  * @param   mode : 0 第一下提锤为学习模式
                    1 直接启动
  * @return  错误状态
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    
  ***********************************************************/
SYS_STA ServicesLoop(void)
{
	SYS_STA ret = ERR_NONE;
	
	EXT_BUTTON_CHK();
	
	GetLiveData();						/*得到 拉力位置 和 速度的动态信息，用于后面的计算*/
	
	if(g_st_SigData.m_errnum > 0)       /*Err Check  使用 m_errshow  和 m_errnum实现故障自锁，所以此处跳出*/
	{
		Halt_Stop();
		g_st_SigData.m_Mode = MOD_FREE;
		return 0;
	}

	switch(g_st_SigData.m_Mode)
	{
		case MOD_SIGACT:     		//自动打锤模式 
		{
		    ERR_SIG sig_err;
			
			sig_err = SingleAct(0);
			if((sig_err > ERR_SIG_REACHDW) || g_halt)
			{
				Halt_Stop();
				Log_e("Halt stop");
				g_st_SigData.m_Mode = MOD_FREE;		// jump out
				switch(sig_err)
				{
					case ERR_SIG_PULLUP :ret|= ERR_LS;break;
					case ERR_SIG_ENCODER:ret|= ERR_TT;break;
					case ERR_SIG_CUR    :ret|= ERR_CT;break;
					case ERR_SIG_CLING  :ret|= ERR_NC;break;
					case ERR_SIG_BRAKE  :ret|= ERR_KC;break;
					case ERR_SIG_TIMOUT :ret|= ERR_CS;break;
					default:break;
				}
			}
		    break;	
         }			
	  case MOD_AUTOTAMP:								// 全自动夯土模式
			ret = hangtu(&TampStep);        			/*轮询无堵塞*/
			LedSta_Show(TampStep);						/*Led状态显示*/
		  break;	
	  case MOD_FREE:
		  TAMP_READY;
		  SIGACT_READY; 
#ifdef USE_LIHE_PWM
	extern int gLiheRatio;
	gLiheRatio = LIHE_TINY;
#endif
		  G_SHACHE(ACT_ON,0);
          C_DISCTR();									/* 履带机 取消 如果  Terry 2019.07.04*/
		  LED_BIT_CLR(SIG_TICHUI);
		  g_st_SigData.m_manualflg = 0;
		  break;
	  case MOD_MANUAL:										//手动模式
          if(g_st_SigData.m_manualflg == 0)
		  {
#ifdef USE_LIHE_PWM
			  gLiheRatio = LIHE_BIG;
#endif					
		      LED_BIT_SET(SIG_TICHUI);
		      G_LIHE(ACT_ON,0);
		      G_SHACHE(ACT_OFF,SHACHEDLY);
			  g_st_SigData.m_manualflg = 1;
		  }
			  
		  break;
	  case MOD_MANOFF:
		  G_LIHE(ACT_OFF,LIHEDLY);
		  G_SHACHE(ACT_ON,0);
		  DELAYMS(200);					/*等待刹车先动作*/
	      g_st_SigData.m_Mode = MOD_FREE;
		  break;
	  case MOD_DOWN:						//下放模式  2018.9.7  小夯机未使用
		  break;
		  
	  case MOD_TST:
			break;																																			//Terry add 2017.11.16
	  default:	
			g_st_SigData.m_Mode = MOD_FREE;
			Halt_Stop();
		  break;
	}

	if(g_st_SigData.m_Mode > MOD_DOWN)
		g_st_SigData.m_Mode = MOD_FREE;
		
	if(ret > ERR_NONE)	
		g_st_SigData.m_errshow = ret; 

  return ret;
}

/*检查锤是否停止，延时2秒确认*/
/************************************************************
  * @brief   提锤到顶后，检查锤子是否定住，此时速度应该 -20 - 20
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    最好放在操作系统的系统时钟回调函数中执行
  ***********************************************************/
static int Check_Stop(int pos,int speed)
{
	static int32_t stop_cnt,power_L,power_H;
	static int32_t block_cnt;
	static uint32_t now_tim;
	int ret  = 0;
	
	switch(s_hang.stop_sta)
	{
		case 0:
			stop_cnt = 0;
			block_cnt = 0;
			s_hang.stop_sta = 1;
			now_tim = GET_TICK_MS();
			power_L = Per2Power(70);
			power_H = Per2Power(160);
			break;
		case 1:
			/******************** 溜放停机检测   ****************/
			if((speed < 20) && ((speed > -20)) && (g_st_SigData.m_Power < power_L))
				stop_cnt++;
			else
			{
				if(stop_cnt > 3)
					stop_cnt -= 3;
			}
			if(stop_cnt > 600/HANG_TICK)
			{
				s_hang.stop_sta = 2;			/*成功定住*/
			}
			
			/**********************卡锤检测***********************/
			if(g_st_SigData.m_Power > power_H)
			{
				block_cnt++;
			}
			else
			{
				if(block_cnt > 1)
					block_cnt--;
			}
			if(block_cnt > 1000/HANG_TICK)
			{
				ret = 2;
				Log_e("Blocked");
			}
			
			/**********************超时检测******************************/
			if((GET_MS_DIFF(now_tim) > 4000) && (speed > 10))    //没有刹住，判断异常
			{
				if(speed > 20)
					ret = 2;		/*冲顶保护*/
				else
					ret = 3;
			}
			/*****************计数的冲顶和下溜检测保护*****************************/
			if((pos > 150) && (pos < -150))			/*冲顶保护检测限位高度为 1.5米*/
			{
				ret = 2;
			}
			
			break;
		case 2:
			ret = 1;
			break;	
	}
	
	return ret;
}

/**/
/************************************************************
  * @brief   高度有要求时，根据屏幕给定的要求，重新设置打锤高度
  * @param   pos   当前锤的高度
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    2019年底山西太原工地客户要求
  ***********************************************************/
static int Get_Hcnt(int32_t pos)
{
	int cnt = 0;
	
	cnt = g_sys_para.s_rammcnt;	  /*系统打锤次数*/
	
	/*都为 0 时，跳过*/
	if((g_GuiData.s_high1 == 0) && (g_GuiData.s_high2 == 0))
	{
		return cnt;
	}

	if((g_GuiData.s_high1 == 0) && (g_GuiData.s_high2 < 0))
	{
		if(pos > g_GuiData.s_high2 || (g_GuiData.s_cnt2 > 0))
			cnt = g_GuiData.s_cnt2;
	}
	else if((g_GuiData.s_high1 < 0) && (g_GuiData.s_high2 == 0))
	{
		if(pos > g_GuiData.s_high1 || (g_GuiData.s_cnt1 > 0))
			cnt = g_GuiData.s_cnt1;
	}
	else if(g_GuiData.s_high1 > g_GuiData.s_high2)
	{
		if((pos > g_GuiData.s_high1) && (g_GuiData.s_cnt1 > 0))
		{
			cnt = g_GuiData.s_cnt1;
		}
		else if((pos > g_GuiData.s_high2) && (g_GuiData.s_cnt2 > 0))
		{
			cnt = g_GuiData.s_cnt2;
		}
	}
	else if(g_GuiData.s_high2 > g_GuiData.s_high1)
	{
		if((pos > g_GuiData.s_high2) && (g_GuiData.s_cnt2 > 0))
		{
			cnt = g_GuiData.s_cnt2;
		}
		else if((pos > g_GuiData.s_high1) && (g_GuiData.s_cnt1 > 0))
		{
			cnt = g_GuiData.s_cnt1;
		}
	}
	
	return cnt;
}


/************************************************************
  * @brief   自动夯土流程
  * @param   * pst 故障检测
  * @return  状态  
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    全自动夯土的流程
  ***********************************************************/
static SYS_STA hangtu(uint16_t * pst)
{
    SYS_STA ret = 0;
    int tmp;
	
    switch(*pst)
    {
        case S_IDLE:
            /*记录当前高度 夯土高度清零 -> 开始溜放  -> 记录溜放起始高度*/
            {
				s_hang.last_sta = S_IDLE;						/*当前转移状态 后来添加*/
                s_hang.pvtimer = GET_TICK_MS();
				Enc_Clr_TotalCnt2();					/*得到顶端的高度  0*/
                s_hang.overpow = 0;
                s_hang.speederr = 0;
				
				G_SHACHE(ACT_LIU,0); 
				
				s_hang.last_highnum = g_st_SigData.m_HeighRammCm;		/*溜放时的高度  Terry 2019.11.12 只用于判断下降高度*/
				s_record.deepth = 0;
				*pst = S_DELAY2;                	/*先溜放*/
				Log_e("开始溜放");
            }
            break;
        case S_PULSE:
                *pst = S_TICHUI; 
                s_hang.top_sta = 0;
                G_LIHE(ACT_ON,0);
                G_SHACHE(ACT_OFF,SHACHEDLY);    /*开始提锤*/
                s_hang.pvtimer = GET_TICK_MS();
				s_hang.speederr = 0;
				s_hang.overpow = 0;
				s_hang.lowpow = 0;
				//更新和发送记录
				s_record.nub++;
				RecordIn(M_NUB,s_record.nub);
				RecordIn(M_TIMES,s_record.cnt);
				RecordIn(M_THIGH,s_record.high);
				RecordIn(M_SONGTU,s_record.tim);
            break;
		
        case S_TICHUI:
                if(Check_up(g_st_SigData.m_HeighRammCm) == 1) 								/*提锤到顶，先拉刹车，后松离合*/  
                {
					if(GET_MS_DIFF(s_hang.pvtimer) > 800)			/*间隔至少 0.8秒*/
					{
						G_LIHE(ACT_OFF,LIHEDLY);
						G_SHACHE(ACT_ON,0);        					/*先拉刹车*/      
						*pst = S_CHECK_UPSTOP;
						s_hang.pvtimer = GET_TICK_MS();
						STA_CLEAR(s_hang.stop_sta);
					}
                }
				/*****************失速保护  探头失效保护*********************************/
                if(g_st_SigData.m_SpeedCm < 20)						/*上升时的速度小于20cm/s,视为异常*/
                    s_hang.speederr++;
                else
				{
					if(s_hang.speederr > 3)
						s_hang.speederr -= 3;
				}
                if(s_hang.speederr > 2800/HANG_TICK)
                {
                     ret = ERR_LIU;
                }
                /**************************拉力过载保护*********************/
                tmp = Per2Power(200);	
                if(g_st_SigData.m_Power > tmp)
                    s_hang.overpow++;
                else
                    s_hang.overpow = 0;
                
                if(s_hang.overpow > 300)   /*持续1.5秒无法拉锤，显示卡锤故障  2019.10.7*/
                {
					ret |= ERR_KC;
                } 
				/************************失重保护***************************/
				tmp = Per2Power(40);
				if(g_st_SigData.m_Power < tmp)
					s_hang.lowpow++;
				else
				{
					if(s_hang.lowpow > 3)
						s_hang.lowpow -= 3;
				}
				if(s_hang.overpow > 2000/HANG_TICK)
				{
					ret |= ERR_LS;
				}
				/***********************************************************/
            break;
        case S_CHECK_UPSTOP:
			/*此处检查离合是否松开*/
			tmp = Check_Stop(g_st_SigData.m_HeighRammCm,g_st_SigData.m_SpeedCm);
			if(tmp == 1)    /*检查成功*/
			{
				C_ENCTR();
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_XIALIAO;
			}
			else if(tmp == 2)
			{
				ret |= ERR_TOP;		/*冲顶保护*/
			}
			else if(tmp == 3)		/*刹车异常*/
			{
				ret |= ERR_SC;
			}
            break;
        case S_XIALIAO:                         				/*  下料  */
			/*提前执行溜放操作*/
			if(GET_MS_DIFF(s_hang.pvtimer) > (g_sys_para.s_feedtims * 600 - 200))	
			{
				s_hang.last_highnum = GetEncoderLen2Cm();	/*溜放时的高度  Terry 2019.7.5*/
                G_LIHE(ACT_OFF,0);
				G_SHACHE(ACT_LIU,0); 
                *pst = S_DELAY2;
			}
            break;
        case S_DELAY2:                          					/*普通延时  刚开始时不需要延时*/
			if((GET_MS_DIFF(s_hang.pvtimer) > g_sys_para.s_feedtims * 600) || (s_hang.last_sta == S_IDLE))
            {
                C_DISCTR();			/*关闭履带机*/
				s_hang.last_sta = S_DELAY2;
                s_hang.pvtimer = GET_TICK_MS();
				s_hang.liufang_sta = 0;
				Log_e("溜放");
				*pst = S_LIUF;
            }
            break;
        case S_LIUF:
			
            tmp = LiuChkDw(g_st_SigData.m_HeighRammCm,g_st_SigData.m_SpeedCm);   									/*溜放时，检测是否到底了*/
            if(tmp == HAS_REACH_DOWN)
            {
				int LPosCm;
				
				LPosCm = GetEncoderLen2Cm();
                s_hang.dachui_cnt = g_sys_para.s_rammcnt;
				s_record.high = g_sys_para.s_sethighcm / 10;		/*打锤高度  记录单位为分米 Terry 2019.6.5*/
				s_record.tim = g_sys_para.s_feedtims;				/*送料时间 Terry 2019.6.5*/
				
                /*记录当前溜放的深度   */
                if(LPosCm > -40)  				/*下降深度过小  下降0.5米*/
                {
                    ret |= ERR_DOWN;  
					Log_e("溜放过浅");
                }
                else
                {	//更新打锤次数和记录，根据深度判断打锤次数
					s_record.cnt = s_hang.dachui_cnt =	Get_Hcnt(LPosCm);
                    s_hang.last_downnum = (int16_t)(LPosCm/10);   				/*本次下降深度 长度 绝对值 Terry 2019.7.5*/
					
					Log_e("溜放成功,准备打锤");
                    *pst = S_DACHUI;       										/*开始打锤 */
				
					SIGACT_READY;	/*打锤准备*/
                }
            }else if(tmp > 1)       											/*溜放错误*/
            {
                ret |= ERR_DOWN;
				Log_e("溜放异常  %x",ret);
            }
            break;
        case S_DACHUI:
			RecordIn(M_ACT,1);  
			
			ret = SingleAct(1);
			if(ret > ERR_SIG_REACHDW)		/*发生错误*/
			{
				Halt_Stop();
				g_st_SigData.m_Mode = MOD_FREE;
				Log_e("打锤错误");
			}
			else
			{
				if(ret == ERR_SIG_REACHDW)
				{
					
					s_hang.dachui_cnt--;
					Log_e("打锤到底 %d",s_hang.dachui_cnt);
					if(s_hang.dachui_cnt < 1)
					{
						if(s_record.deepth > -50)			/*表示夯土结束  Terry 2019.10.18 */
						{
							g_st_SigData.m_Mode = MOD_FREE;	/*直接退出  2019.8.2*/
							G_SHACHE(ACT_ON,0);
							G_LIHE(ACT_OFF,200);
							*pst = S_IDLE;
						}
						else
						{
							*pst = S_PULSE;       			/*下一轮提锤操作   直接提锤*/
							Log_e("提锤");
						}					
					}
				}
			}
            if(ret == ERR_NONE)
            {
				RecordIn(M_ACT,0);		           /*提锤标志 Terry 2019.7.6*/			
                ret = Sig_LandDw();					/*堵塞模式*/
            }
            break;
    }
    /*Switch 结束*/
    if(g_halt)
    {
		ret |= ERR_HALT;
    }
    /*异常退出时，立即拉刹车，关闭输送带(莫须有)*/
	if(ret)
	{
		g_st_SigData.m_Mode = MOD_FREE; 
		G_SHACHE(ACT_ON,0);
		G_LIHE(ACT_OFF,300);					/*2019.8.2添加  Terry*/
		C_DISCTR();
		*pst = S_IDLE; 
	}
    return ret;
}


/************************************************************
  * @brief   检测溜放是否到底
  * @param   LPosCm 当前锤的夯土位置   LSpeedCm 当前速度（向下速度正常为负值）
  * @return  状态  正常 返回1   异常返回12
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    最好放在操作系统的系统时钟回调函数中执行
  ***********************************************************/
SYS_STA LiuChkDw(int LPosCm,int LSpeedCm)
{
    static uint32_t last_tim = 0;
    static uint32_t sure_cnt = 0;
	static uint32_t speed_over = 0;
    SYS_STA ret = 0;
	
    switch(s_hang.liufang_sta)
    {
        case 0:
            last_tim = GET_TICK_MS();
			sure_cnt = 0;
			speed_over = 0;
			s_hang.liufang_sta = 1;
            break;
        case 1:
            if(GET_MS_DIFF(last_tim) > 1300)       								/*1.3秒后  开始判断速度   2000  2019.07.04*/
            {
                if((LSpeedCm < -10 ) && ((s_hang.last_highnum - LPosCm) > 80))  /*可以判定开始溜放了*/
                {
                    s_hang.liufang_sta = 2;
                }
                if(GET_MS_DIFF(last_tim) > 6000)      							/*6 秒超时没有动作就跳出 2019.8.2*/
                {
                    ret = 12;                       							/*锤没有下落故障*/
                    s_hang.liufang_sta = 3;
                }		
            }
            break;
        case 2:                                     	/*工作过一次*/
            /*判断是否到底了*/
            if(LSpeedCm > -7)
                sure_cnt++;
            else if(LSpeedCm < -8)						/*向下为 负*/
            {
                sure_cnt = 0;                       	/*锤超速判断  ？？？*/
				last_tim = GET_TICK_MS();
            }
            
            if(sure_cnt > 1400/HANG_TICK)            	/*确认已经停止  1.4秒  原来0.8秒 */
            {
				if(s_record.deepth + 400 > LPosCm)		/*假如锤卡在上面，超过4米，就会报警*/
				{
					ret = 1;
					s_hang.liufang_sta = 3;             /*正常 跳出判断*/
				}
				else
				{
					sure_cnt = 0;							/*重新开始累积*/
					if(GET_MS_DIFF(last_tim) > 4000)		/*最多再等待4秒*/
					{
						ret = 12;                       	/*锤没有下落故障,没有下落到底的故障 2019.11.16*/
						s_hang.liufang_sta = 3;
					}
				}
            } 
			/*超速报警*/
			if(LSpeedCm < -800)								/*下降速度超过6米/秒时，持续1秒，将抱溜放异常错误 2019.12.08*/
			{
				speed_over++;
			}
			else
			{
				if(speed_over > 6)
					speed_over -= 4;
			}
			
			if(speed_over > 1000/HANG_TICK)
			{
				ret = 12;									/*下降速度过快报警*/
			}	
            break;
        case 3:												/*结束*/
            break;
        default:
            s_hang.liufang_sta = 3;
            break;
    }
    return ret;
}


/*上拉超时保护*/
/*返回1 检测到
  返回0 无检测到
  其它 错误
*/
SYS_STA Check_up(int PosCm)
{
    SYS_STA ret = 0;
    
	 /*等待锤上升到无效   计数到达或者上限位开关有效   或的关系*/
	if(PosCm > -15) 	/*-30公分   Terry 不使用冲顶的信号*/
	{
		ret = 1;		 /*锤已经到达预定位置*/
	}
    return ret;
}


/************************************************************
  * @brief   初始的属性参数设置
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    default task,初始化系统属性参数   2017.11.8
  ***********************************************************/
#include "EEPROM.h"
void sysattr_init(uint16_t flg)
{
	HAL_StatusTypeDef status;
	uint32_t ID;
	
	osDelay(100);
	/*默认值*/
	g_sys_para.s_dir = 0;
	g_sys_para.s_feedtims = 5;                  	//送料时间  Terry 2019.6.6
	g_sys_para.s_numchi = 70;					//每圈齿数
	
	if(flg == 0)
		g_sys_para.s_chickid = CHK_ID;
		
	g_sys_para.s_sethighcm = 300;
	g_sys_para.s_pericm = 106;
	g_sys_para.s_pnull = 50;
	g_sys_para.s_pfull = 500;
	g_st_SigData.m_Mode = MOD_FREE;
	g_sys_para.s_setlihecm = g_sys_para.s_sethighcm * 2 / 3;	/*使用探头时的离合点 cm*/
	g_sys_para.s_mode = 0;							/*0:自动打锤 1:全自动强夯*/
    g_sys_para.s_rammcnt = 6;         				/*夯土次数 Terry 2019.5.21*/
	g_sys_para.s_hprot = 150;						/*默认高度保护设置 Terry 2019.7.6*/
	g_sys_para.s_pset = 0;							/*校验无效 Terry 2019.7.9*/
	
	/*检查内存是否完整   2017.11.8  */
	status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&ID, 4);
	osDelay(5);
	if(status == HAL_OK)
	{
		if(ID == CHK_ID)
		{
			status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(2);
		}
		else
			status = HAL_ERROR;
	}
	
	if(status)		//校验错误时，重新写入内存
	{
		status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
		osDelay(2);
		
		while(status != HAL_OK)
		{
			status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(100);
		}
	}
	Enc_Clr_TotalCnt1();
	/*离合刹车动作初始化*/
	G_LIHE(ACT_OFF,0);
    G_SHACHE(ACT_ON,0);         						//初始为拉刹车        2018.12.24 Terry
    Enc_Set_Dir(g_sys_para.s_dir);
	g_st_SigData.m_Mode = MOD_FREE;						//上电后保持空闲模式
}


