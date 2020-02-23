
#include "Hangtu.h"
#include "port.h"
#include "GUI.h"
#include "Encoder.h"
#include "u_log.h"


#define GET_TICK_MS()  osKernelSysTick()
#define DELAYMS(x)	   osDelay(x)

SYS_STA Checkdown(void);
SYS_STA Check_up(void);
SYS_STA hangtu(uint8_t * pst);

extern uint16_t g_errnum;

/*夯土整个工艺流程  */
/*
提锤  下料  溜放   打锤
*/

struct HANGTU s_hang;                                   /*夯土流程状态检测*/
struct RECORD s_record;									/*待上传的数据*/

extern uint16_t led_sta;


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

/*Modbus 初始化时设定的数据*/
void ModbusData_Init(void)
{
	usRegHoldingBuf[M_NUB] = 0;
	usRegHoldingBuf[M_TIMES] = sys_attr.s_cnt;
	usRegHoldingBuf[M_THIGH] = sys_attr.s_sethighcm;
	usRegHoldingBuf[M_DEEP] = 0;
	usRegHoldingBuf[M_SONGTU] = sys_attr.s_intval;
	usRegHoldingBuf[M_STATE] = 0;
	usRegHoldingBuf[M_ERR] = 0;
	
	usRegHoldingBuf[M_PERCNT] = sys_attr.s_numchi;
	usRegHoldingBuf[M_ZHOU] = sys_attr.s_zhou;
	usRegHoldingBuf[M_TLIAO] = sys_attr.s_intval;
	usRegHoldingBuf[M_CCNUB] = sys_attr.s_cnt;
	usRegHoldingBuf[M_TICHUI] = sys_attr.s_sethighcm;
	usRegHoldingBuf[M_LIHE] = sys_attr.s_hlihe;
	usRegHoldingBuf[M_HPROT] = sys_attr.s_hprot;

	
	/*授权码*/
	usRegHoldingBuf[M_ACCRE] = 100;
	
	s_record.nub = 0;
	s_record.deepth = 0;
	s_record.high = 0;
	s_record.tim = 0;
	s_record.cnt = 0;
}
extern volatile struct GUI_DATA	g_showdata;
void ModbusData_flash(void)
{
	usRegHoldingBuf[M_PERCNT] = g_showdata.g_num;
	usRegHoldingBuf[M_ZHOU] = g_showdata.g_Zhoucm;
	usRegHoldingBuf[M_TLIAO] = g_showdata.g_ts;
	usRegHoldingBuf[M_CCNUB] = g_showdata.g_hcnt;
	usRegHoldingBuf[M_TIMES] = g_showdata.g_hcnt;
	
	usRegHoldingBuf[M_TICHUI] = g_showdata.g_sethighcm;
	usRegHoldingBuf[M_LIHE] = g_showdata.g_lihe;
	usRegHoldingBuf[M_HPROT] = g_showdata.g_HighOvercm;
}

void ModbusData_Show(void)
{
	usRegHoldingBuf[M_DEEP] = (int16_t)g_showdata.g_nhigh;
	usRegHoldingBuf[M_THIGH] = (int16_t)g_showdata.g_sethighcm;
	usRegHoldingBuf[M_KS] = s_hang.last_downnum;						/*显示桩底深度*/
	usRegHoldingBuf[M_ERR] = g_errnum;
}

/*外部参数设定 更新检查   循环*/
extern uint32_t savecnt;

void ModbusData_Chk(void)
{
	/*每圈齿数*/
	if(usRegHoldingBuf[M_PERCNT] != g_showdata.g_num )
	{
		if((usRegHoldingBuf[M_PERCNT] < 300) && (usRegHoldingBuf[M_PERCNT] > 10))
		{
			g_showdata.g_num  = usRegHoldingBuf[M_PERCNT];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_PERCNT] = g_showdata.g_num;
		}
	}
	/*卷筒周长*/
	if(usRegHoldingBuf[M_ZHOU] != g_showdata.g_Zhoucm)
	{
		if((usRegHoldingBuf[M_ZHOU] < 300) && (usRegHoldingBuf[M_ZHOU] > 20))
		{
			g_showdata.g_Zhoucm = usRegHoldingBuf[M_ZHOU];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_ZHOU] = g_showdata.g_Zhoucm;
		}
	}
	/*送料的时间*/
	if(usRegHoldingBuf[M_TLIAO] != g_showdata.g_ts)
	{
		if((usRegHoldingBuf[M_TLIAO] < 16) && (usRegHoldingBuf[M_TLIAO] > 1))
		{
			g_showdata.g_ts = usRegHoldingBuf[M_TLIAO];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TLIAO] = g_showdata.g_ts;
		}
	}
	/*设定打锤的次数*/
	if(usRegHoldingBuf[M_CCNUB] != g_showdata.g_hcnt)
	{
		if((usRegHoldingBuf[M_CCNUB] < 11) && (usRegHoldingBuf[M_CCNUB] > 2))
		{
			g_showdata.g_hcnt = usRegHoldingBuf[M_CCNUB];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_CCNUB] = g_showdata.g_hcnt;
		}
	}
	/*设定提锤高度*/
	if(usRegHoldingBuf[M_TICHUI] != g_showdata.g_sethighcm)
	{
		if((usRegHoldingBuf[M_TICHUI] <= MAXSET_HIGH) && (usRegHoldingBuf[M_TICHUI] > MINSET_HIGH))
		{
			g_showdata.g_sethighcm = usRegHoldingBuf[M_TICHUI];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TICHUI] = g_showdata.g_sethighcm;
		}
	}
	/*设定离合点的位置*/
	if(usRegHoldingBuf[M_LIHE] != g_showdata.g_lihe)
	{
		if((usRegHoldingBuf[M_LIHE] <= (g_showdata.g_sethighcm - 20)))
		{
			g_showdata.g_lihe = usRegHoldingBuf[M_LIHE];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_LIHE] = g_showdata.g_lihe;
		}
	}
	
	/*设定上拉高度保护值*/
	if(usRegHoldingBuf[M_HPROT] != g_showdata.g_HighOvercm)
	{
		if(usRegHoldingBuf[M_HPROT] <= 1000)
		{
			g_showdata.g_HighOvercm = usRegHoldingBuf[M_HPROT];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_HPROT] = g_showdata.g_HighOvercm;
		}
	}
	
		/*分段打锤次数*/
	g_showdata.s_high1 = usRegHoldingBuf[M_HIGH1];
	g_showdata.s_cnt1 = usRegHoldingBuf[M_CNT1];
	g_showdata.s_high2 = usRegHoldingBuf[M_HIGH2];
	g_showdata.s_cnt2 = usRegHoldingBuf[M_CNT2];
}

/*Led状态指示灯  夯土模式下*/
void LedSta_Show(uint8_t ledsta)
{
	led_sta &= ~SIG_SALL;					/*Led灯清零*/
	switch(ledsta)
	{
		case S_IDLE:		/*空闲*/
			usRegHoldingBuf[M_STATE] = 0;
			break;
		case S_PULSE:
		case S_TICHUI:		/*提锤*/
			led_sta |= SIG_STICHUI;
			usRegHoldingBuf[M_STATE] = 3;
			break;
		case S_CHECK_UPSTOP:
			led_sta |= SIG_SZHUCHUI;
			usRegHoldingBuf[M_STATE] = 4;
			break;
		case S_XIALIAO:		/*下料*/
			led_sta |= SIG_STU;
			usRegHoldingBuf[M_STATE] = 5;
			break;
		case S_DELAY2:
		case S_LIUF:		/*溜放*/
			led_sta |= SIG_SLIUF;
			usRegHoldingBuf[M_STATE] = 1;
			break;
		case S_DACHUI:		/*夯土*/
			led_sta |= SIG_SHANGTU;
			usRegHoldingBuf[M_STATE] = 2;
			break;
		default:break;
	}
}


/*控制的主任务*/
extern uint32_t g_erract;    
SYS_STA services(void)
{
	static	uint8_t step,down;
	
	SYS_STA ret;
	ret = ERR_NONE;
	
	if(g_erract | g_errnum)				// 检测离合和刹车是否正常
	{
	  G_LIHE(ACT_OFF,0);
	  G_SHACHE(ACT_ON,0);
	  Log_e("err -- g_erract | g_errnum");
	  
	  DELAYMS(ERRDLY);
	  return g_erract;
	}
	
	if(!((sys_fbsta & FB_24VOK) && (sys_fbsta & FB_RUN)))	//非正常状态，跳出
	{
		G_LIHE(ACT_OFF,0);
		G_SHACHE(ACT_ON,0);
		
		Log_e("FB_24VOK | FB_RUN");
		DELAYMS(ERRDLY);
		
		

		if(!(sys_fbsta & FB_24VOK))
			return ERR_PW;
		if(!(sys_fbsta & FB_RUN))
			return ERR_HALT;
	}

  switch(sys_attr.s_zidong)
  {
	  case MOD_TT2:                                         /*有探头的一键启动*/
	  {
		  if(step == 0)
		  {
			  liheupdate();
			  led_sta |= SIG_TICHUI;
			  led_sta &= ~SIG_FANGCHUI;
			  ret = starttaking();          				/*拉力的学习*/
			  step = 1;
		  }
		  else
          {
			  led_sta |= SIG_TICHUI;
			  led_sta &= ~SIG_FANGCHUI;
			  ret = takeup();			  
              Debug("up err %x\r\n",ret);
          }
		  
		  if(ret == ERR_NONE)
          {
			  led_sta &= ~SIG_TICHUI;
			  led_sta |= SIG_FANGCHUI;
              ret = putdown(0);	  
              Debug("down err %d\r\n",ret);
          }
		  
		  if(ret != ERR_NONE)
		  {
			 sys_attr.s_zidong = MOD_FREE; 
			 step = 0;
		  }
		  if(g_halt != 0)
		  {
			  g_halt = 0;
		  }
		  break;
	  }		
	  
	  case MOD_ZTT2:			//正常启动	  夯土模式
			ret = hangtu(&step);        			/*轮询加堵塞*/
			/* LED强夯流程显示 */
			LedSta_Show(step);						/*Led状态显示*/
		  break;	
	  case MOD_FREE:
		  step = 0;
		  down = 0;
		  G_SHACHE(ACT_ON,0);
          C_DISCTR();					/* 履带机 取消   Terry 2019.07.04*/
		  led_sta &= ~SIG_TICHUI;
		  break;
	  case MOD_MAN:			//手动模式
		  if(step == 0)
		  {
		      led_sta |= SIG_TICHUI;
			  G_LIHE(ACT_ON,0);
			  G_SHACHE(ACT_OFF,SHACHEDLY);
			  step = 1;
		  }
		  break;
	  case MOD_MANOFF:
		  G_LIHE(ACT_OFF,LIHEDLY);
		  G_SHACHE(ACT_ON,0);
		  DELAYMS(200);					/*等待刹车先动作*/
	      sys_attr.s_zidong = MOD_FREE;
		  break;
	  case MOD_DOWN:						//下放模式  2018.9.7  小夯机未使用
		  if(down == 0)
		  {
			  G_LIHE(ACT_OFF,LIHEDLY);
			  G_SHACHE(ACT_OFF,0);
			  down = 1;
		  }
		  break;
		  
	  case MOD_TST:
			break;																																			//Terry add 2017.11.16
	  default:	
			step = 0;
			sys_attr.s_zidong = MOD_FREE;
		  break;
	}
	
		
	if(sys_attr.s_zidong > MOD_DOWN)
		sys_attr.s_zidong = MOD_FREE;
		
	if((sys_attr.s_zidong == MOD_FREE) || (sys_attr.s_zidong == MOD_TST) ||(sys_attr.s_zidong == MOD_DOWN))
		DELAYMS(CALUTICK);													//主动让出给其它任务    2017.10.7
	else
		DELAYMS(HANG_TICK);
 
  return ret;
}

/*检查锤是否停止，延时2秒确认*/
int Check_Stop(void)
{
	static int32_t stop_cnt,power;
	static uint32_t now_tim;
	int ret  = 0,LSpeedCm;
	switch(s_hang.stop_sta)
	{
		case 0:
			stop_cnt = 0;
			s_hang.stop_sta = 1;
			now_tim = GET_TICK_MS();
			power = epower(70);
			break;
		case 1:
			LSpeedCm = GetEncoderSpeedCm();
			if((LSpeedCm < 20) && ((LSpeedCm > -20)) && (sys_stadata.m_power.Speed < power))
				stop_cnt++;
			else
			{
				if(stop_cnt > 3)
					stop_cnt -= 3;
			}
			
			if(stop_cnt > 500/HANG_TICK)
			{
				s_hang.stop_sta = 2;			/*成功定住*/
			}
			
			if(GET_MS_DIFF(now_tim) > 4000)
			{
				if(LSpeedCm > 20)
					ret = 2;		/*冲顶保护*/
				else
					ret = 3;
			}
			/*计数的冲顶保护*/
			if(GetEncoderLen2Cm() > 160)
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

/*得到实际的打锤次数*/
int Get_Hcnt(int32_t pos)
{
	int cnt = 0;
	
	cnt = sys_attr.s_cnt;
	
	/*都为 0 时，跳过*/

	if((g_showdata.s_high1 == 0) && (g_showdata.s_high2 < 0))
	{
		if(pos > g_showdata.s_high2 || (g_showdata.s_cnt2 > 0))
			cnt = g_showdata.s_cnt2;
	}
	else if((g_showdata.s_high1 < 0) && (g_showdata.s_high2 == 0))
	{
		if(pos > g_showdata.s_high1 || (g_showdata.s_cnt1 > 0))
			cnt = g_showdata.s_cnt1;
	}
	else if(g_showdata.s_high1 > g_showdata.s_high2)
	{
		if((pos > g_showdata.s_high1) && (g_showdata.s_cnt1 > 0))
		{
			cnt = g_showdata.s_cnt1;
		}
		else if((pos > g_showdata.s_high2) && (g_showdata.s_cnt2 > 0))
		{
			cnt = g_showdata.s_cnt2;
		}
	}
	else if(g_showdata.s_high2 > g_showdata.s_high1)
	{
		if((pos > g_showdata.s_high2) && (g_showdata.s_cnt2 > 0))
		{
			cnt = g_showdata.s_cnt2;
		}
		else if((pos > g_showdata.s_high1) && (g_showdata.s_cnt1 > 0))
		{
			cnt = g_showdata.s_cnt1;
		}
	}
	
	
	return cnt;
}


/*自动夯土流程 */
extern struct PRPTECT_HANGTU Prtop;
SYS_STA hangtu(uint8_t * pst)
{
	static uint16_t stlast;		/*标记是否为第一次溜放  2019.11.15*/
    SYS_STA ret = 0;
    int tmp,LSpeedCm;    
    
    switch(*pst)
    {
        case S_IDLE:
            /*记录当前高度 夯土高度清零 -> 开始溜放  -> 记录溜放起始高度*/
            {
				stlast = S_IDLE;				/*当前转移状态 后来添加*/
                s_hang.pvtimer = GET_TICK_MS();
				Enc_Clr_TotalCnt2();
                s_hang.overpow = 0;
                s_hang.speederr = 0;
				Prtop.flg = 0;					/*是否需要重新定义离合点的高度*/
				G_SHACHE(ACT_LIU,0); 
				*pst = S_DELAY2;                /*先溜放*/
				s_hang.last_highnum = GetEncoderLen2Cm();		/*溜放时的高度  Terry 2019.11.12 只用于判断下降高度*/
				s_record.deepth = 0;
                Debug("Han Start:\r\n");
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
				usRegHoldingBuf[M_NUB] = s_record.nub;
				usRegHoldingBuf[M_TIMES] = s_record.cnt;
				usRegHoldingBuf[M_THIGH] = s_record.high;
				usRegHoldingBuf[M_SONGTU] = s_record.tim;
            break;
		
        case S_TICHUI:
				LSpeedCm = GetEncoderSpeedCm();
                if(Check_up() == 1) 					/*提锤到顶，先拉刹车，后松离合*/  
                {
					if(GET_MS_DIFF(s_hang.pvtimer) > 800)	/*间隔至少 0.8秒*/
					{
						G_LIHE(ACT_OFF,LIHEDLY);
						G_SHACHE(ACT_ON,0);        		/*先拉刹车*/     
						Debug("Get Top signal\r\n");        
						*pst = S_CHECK_UPSTOP;
						s_hang.pvtimer = GET_TICK_MS();
						s_hang.stop_sta = 0;
					}
                }
				
				/*****************失速保护  探头失效保护*********************************/
                if(GetEncoderSpeedCm() < 20)		/*上升时的速度小于20cm/s,视为异常*/
                    s_hang.speederr++;
                else
				{
					if(s_hang.speederr > 3)
						s_hang.speederr -= 3;
				}
                if(s_hang.speederr > 2800/HANG_TICK)
                {
                     Debug("上升溜绳，或者计数错误\r\n");
                     ret = ERR_LIU;
                }
                /**************************拉力过载保护*********************/
                tmp = epower(350);	
                if(sys_stadata.m_power.Speed > tmp)
                    s_hang.overpow++;
                else
                    s_hang.overpow = 0;
                
                if(s_hang.overpow > 300)   /*持续1.5秒无法拉锤，显示卡锤故障  2019.10.7*/
                {
                    Debug("上升卡锤\r\n");
					ret |= ERR_KC;
                } 
				/************************失重保护***************************/
				tmp = epower(40);
				if(sys_stadata.m_power.Speed < tmp)
					s_hang.lowpow++;
				else
				{
					if(s_hang.lowpow > 3)
						s_hang.lowpow -= 3;
				}
				if(s_hang.overpow > 2000/HANG_TICK)
				{
					Debug("失重保护\r\n");
					ret |= ERR_LS;
				}
				/***********************************************************/
            break;
        case S_CHECK_UPSTOP:
			/*此处检查离合是否松开*/
			tmp = Check_Stop();
			if(tmp == 1)    /*检查成功*/
			{
				C_ENCTR();
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_XIALIAO;
                Debug("wait1 1s\r\n");
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
			if(GET_MS_DIFF(s_hang.pvtimer) > (sys_attr.s_intval * 600 - 200))	
			{
				s_hang.last_highnum = GetEncoderLen2Cm();	/*溜放时的高度  Terry 2019.7.5*/
                G_LIHE(ACT_OFF,0);
				G_SHACHE(ACT_LIU,0); 
                *pst = S_DELAY2;
                Debug("wait2 1s\r\n");
			}
            break;
        case S_DELAY2:                          				/*普通延时  刚开始时不需要延时*/
			if((GET_MS_DIFF(s_hang.pvtimer) > sys_attr.s_intval * 600) || (stlast == S_IDLE))
            {
                C_DISCTR();			/*关闭履带机*/
				stlast = S_DELAY2;
				
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_LIUF;
                Debug("finish xia liao  %ds\r\n",sys_attr.s_intval);
				s_hang.liufang_sta = 0;
            }
            break;
        case S_LIUF:
			/*下降到底检测*/
			
            tmp = Checkdown();   
			
            if(tmp == 1)			/*检测到到底了*/
            {
                s_hang.dachui_cnt = sys_attr.s_cnt;
				s_record.high = sys_attr.s_sethighcm / 10;		/*打锤高度 Terry 2019.6.5*/
				s_record.tim = sys_attr.s_intval;				/*送料时间 Terry 2019.6.5*/
				
                /*记录当前溜放的深度   */
                if(GetEncoderLen2Cm() > -40)  				/*下降深度过小  下降0.5米*/
                {
                    Debug("下降深度过小 \r\n");
                    ret |= ERR_DOWN;  
                }
                else
                {
					int LPosCm;
					
					LPosCm = GetEncoderLen2Cm();
					s_hang.dachui_cnt =	Get_Hcnt(LPosCm);
					s_record.cnt = s_hang.dachui_cnt;							/*记录打锤次数 Terry 2019.6.5*/	
                    s_hang.last_downnum = (int16_t)(LPosCm/10);   /*本次下降深度 长度 绝对值 Terry 2019.7.5*/
                    *pst = S_DACHUI;       					/*开始打锤 */
					
                    Debug("dao di le \r\n");
                }
            }
            if(tmp > 1)       						/*溜放错误*/
            {
                ret |= ERR_DOWN;
                Debug("溜放错误 \r\n");
            }
            break;
        case S_DACHUI:
			usRegHoldingBuf[M_ACT] = 1;				/*提锤标志 Terry 2019.7.6*/
            ret = takeup();							/*堵塞模式*/
            Debug("DA chui %d c\r\n",s_hang.dachui_cnt);
            if(ret == ERR_NONE)
            {
				usRegHoldingBuf[M_ACT] = 0;			/*提锤标志 Terry 2019.7.6*/
                ret |= putdown(0);					/*堵塞模式*/
            }
            
            if(ret == ERR_NONE)
            {
                s_hang.dachui_cnt--;
                if(s_hang.dachui_cnt < 1)
                {
					if(s_record.deepth > -50)			/*表示夯土结束  Terry 2019.10.18 */
					{
						*pst = S_IDLE;
						sys_attr.s_zidong = MOD_FREE;	/*直接退出  2019.8.2*/
						G_SHACHE(ACT_ON,0);
						G_LIHE(ACT_OFF,0);
					}
					else
					{
						*pst = S_PULSE;       /*下一轮提锤操作   直接提锤*/
						Debug("complete \r\n");
					}					
                }
            }
            break;
    }
    /*Switch 结束*/
    if(g_halt)
    {
        Debug("Stop S\r\n");
		ret |= ERR_HALT;
    }
    
    
    /*异常退出时，立即拉刹车，关闭输送带(莫须有)*/
	if(ret)
	{
		sys_attr.s_zidong = MOD_FREE; 
		G_SHACHE(ACT_ON,0);
		G_LIHE(ACT_OFF,0);					/*2019.8.2添加  Terry*/
		C_DISCTR();
		*pst = S_IDLE; 
	}
    return ret;
}


/*检查锤是否到底了
返回 1  正常
     >1 错误
*/
SYS_STA Checkdown(void)
{
    static uint32_t last_tim = 0;
    static uint32_t sure_cnt = 0;
	static uint32_t speed_over = 0;
	
    SYS_STA ret = 0;
	int LPosCm;
	int LSpeedCm = GetEncoderAcceCm();
	
	
    LPosCm = GetEncoderLen2Cm();
    switch(s_hang.liufang_sta)
    {
        case 0:
            last_tim = GET_TICK_MS();
            s_hang.liufang_sta = 1;
			sure_cnt = 0;
			speed_over = 0;
            break;
        case 1:
            if(GET_MS_DIFF(last_tim) > 1300)       	/*1.3秒后  开始判断速度   2000  2019.07.04*/
            {
                if((LSpeedCm < -10 ) && ((s_hang.last_highnum - LPosCm) > 80))  /*判定可以判定溜放了*/
                {
                    s_hang.liufang_sta = 2;
                }
                if(GET_MS_DIFF(last_tim) > 6000)      /*6 秒超时没有动作就跳出 2019.8.2*/
                {
                    ret = 12;                       /*锤没有下落故障*/
                    s_hang.liufang_sta = 3;
                }		
            }
            break;
        case 2:                                     	/*工作过一次*/
            /*判断是否到底了*/
            if(LSpeedCm > -7)
                sure_cnt++;
            else if(LSpeedCm < -8)			/*向下为 负*/
            {
                sure_cnt = 0;                       	/*锤超速判断  ？？？*/
				last_tim = GET_TICK_MS();
            }
            
            if(sure_cnt > 1200/HANG_TICK)            	/*确认已经停止  1.2秒  原来0.8秒 */
            {
				if(s_record.deepth + 400 > LPosCm)	/*假如锤卡在上面，超过4米，就会报警*/
				{
					ret = 1;
					s_hang.liufang_sta = 3;             /*跳出判断*/
				}
				else
				{
					sure_cnt = 0;						/*重新开始累积*/
					if(GET_MS_DIFF(last_tim) > 4000)		/*最多再等待4秒*/
					{
						ret = 12;                       /*锤没有下落故障,没有下落到底的故障 2019.11.16*/
						s_hang.liufang_sta = 3;
					}
				}
            } 
			/*超速报警*/
			if(LSpeedCm < -800)			/*下降速度超过6米/秒时，持续0.7秒，将抱溜放异常错误 2019.12.08*/
			{
				speed_over++;
			}
			else
			{
				if(speed_over > 4)
					speed_over -= 4;
			}
			if(speed_over > 1000/HANG_TICK)
			{
				ret = 12;							/*下降速度过快报警*/
			}	
            break;
        case 3:										/*结束*/
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
SYS_STA Check_up(void)
{
    SYS_STA ret = 0;
    
	 /*等待锤上升到无效   计数到达或者上限位开关有效   或的关系*/
	if((GetEncoderLen2Cm() > -15) ) 	/*-30公分   Terry 不使用冲顶的信号*/
	{
		ret = 1;		 /*锤已经到达预定位置*/
	}

    return ret;
}

