

#include "Action.h"
#include "math.h"
#include "EEPROM.h"
#include "Encoder.h"
#include "PutOffAct.h"
#include "u_log.h"
struct STADATA sys_stadata;						// 系统参数

struct SYSATTR g_sys_para;

struct LIHEDATA	    TTlehe;							//使用探头的离合计数模式,修改离合点后，保存所需参数
struct LIHEDATA	    TMlehe;		    				// TM离合动作，保存所需参数

struct PRPTECT_HANGTU Prtop;						//离合到顶的数据记录  Terry 2019.7.24

volatile uint32_t g_halt = 0;

volatile uint8_t sys_fbsta;						// 外部反馈信号

//extern uint32_t savecnt;						//保存数据
int8_t Get_Fbsignal(uint8_t MASK);
SYS_STA Get_Action_Sta(void);
extern int8_t Get_Fbsignal(uint8_t MASK);
extern struct RECORD s_record;

uint32_t getmilsec(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = osKernelSysTick();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*超过一天的跨度*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}


/*获得编码器计数，确定基准方向*/
/**
  * 函数功能 获得编码器计数，确定基准方向
  * 输入参数：dir:基准方向
  *           *p:硬件编码器计数数据
  * 返回值: None
  * 说明: 无
  */
  
/*  厘米转齿数  */
int32_t cm2num(int32_t cm)
{
	int32_t tmp;
	
	tmp = cm * g_sys_para.s_numchi / g_sys_para.s_zhou;
	
	return tmp;
}



/********************************************************
Function	: PICtr()
Description	: 对松弛度的误差进行PI控制，齿数
Input		: sub	过松为正，过紧为负数
Return		: 离合点的修正量，单位 齿数，返回离合点调整的增量
Others		: 手动设置离合点时，暂停一段时间不进行自动调整
5个齿以内的松弛度误差，允许
*********************************************************/
int32_t PICtr(int32_t subs)
{
	static float Totals = 0;
	
	float ret;
	int32_t tmp;
	
	if(subs > 5)
		Totals += (subs - 5);					//对误差进行积分
	else if(subs < 0)
		Totals += subs;
	else										/*在 0 到 5 的区间内*/
	{
		if(Totals < 0)							/*增强稳定性*/
			Totals += subs;
		else if(Totals > 20)
			Totals -= (5 - subs);	
	}
	
	
	if(subs > 5)								//比较松的时候
	{
		ret = 0.03 * (subs - 5);					
	}
	else if(subs < 0)
	{
		ret = 0.03 * subs;
	}
	else
		ret = 0;
	
	ret = 0.05 * Totals + ret;					//主要是靠累计误差去调整
	
	tmp = (int32_t)ret;
	
	if(tmp != 0)
	{
		Totals = Totals / 2;
	}
	Debug("PI %d  ",tmp);
	return tmp;
}


int32_t PITCtr(int32_t subs)
{
	static int32_t last_err = 0;
	
	float ftmp;
	int32_t ret;
	
	ftmp = 0.04*((subs - last_err) + 0.1 * subs);
	if(ftmp > 20)
		ftmp = 20;
	else if(ftmp < -20)
		ftmp = -20;
	
	ftmp = ftmp / 3;
	
	if(ftmp > 12)
		ftmp = 12;
	else if(ftmp < -12)
		ftmp = -12;
	
	last_err = subs;
	
	ret = -ftmp;
	
	return ret;
}

/*获得当前功率*/
void GetPower(struct EtrCnt *p)
{
	static uint32_t lastspeed = 0;
	
	p->Speed = sys_Etrcnt.Speed + lastspeed;				//双倍速度
	lastspeed = sys_Etrcnt.Speed;
}


/*计算出有功功率比值对应的实际功率*/
int32_t epower(int16_t  ratio)
{
	uint32_t pw;
	
	pw = (g_sys_para.s_pfull - g_sys_para.s_pnull) * ratio / 100;
	pw += g_sys_para.s_pnull;
	
	return pw;
}







/*
卡锤时
中断上拉
拉刹车 松离合

拉离合  松刹车
*/
void pullbreak(void)
{
	G_SHACHE(ACT_ON,0);
	G_LIHE(ACT_OFF,LIHEDLY / 2);
	osDelay(1500);
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,SHACHEDLY / 2);
}
/******************************************离合点自动计算  有探头模式 ***************************************/
/**
  * 函数功能 返回有效了离合点高度（齿数）
  * 输入参数： relax:实际松弛度
  *             *cnt:允许调整的倒计数指针

  * 返回值: int32_t 实际的离合点
  * 说明: 无
  */

int32_t getlihenum(void)
{
	static int32_t lihenum;
	
	liheupdate();
	lihenum = TTlehe.lihe;									//初始的离合点

	return lihenum;
}

/**
  * 函数功能 更新离合点的计数值
  * 输入参数： 无
  * 返回值: 无
  * 说明: 每次修改离合点数值后，都会重新计算  离合点的自动调整区间为 正负 20cm
  */
extern struct HANGTU s_hang;  
void liheupdate(void)
{
	int32_t Lihecmtmp = 0;
	
	TTlehe.cnt = INIT_CHUI;		//修正后默认50次的稳定时间
	TTlehe.relaxsum = 0;		//该值正常为负值
	TTlehe.songchi = 0;
	
	Lihecmtmp = g_sys_para.s_hlihe;
	
	if(g_sys_para.s_cmode == MOD_AUTOTAMP)
	{
		if(s_hang.dachui_cnt == g_sys_para.s_cnt)			/*第一次打锤,打松一点  2019.9.17*/
			Lihecmtmp -= 6;
		if(s_record.deepth > -300)						/*到达顶部时，打紧锤*/
			Lihecmtmp += 8;			
	}
	
	if(Lihecmtmp < 50)						/*最小拉离合的点为50公分  2019.12.17*/
		Lihecmtmp = 50;
	else if(Lihecmtmp > g_sys_para.s_sethighcm - 5)
		Lihecmtmp = g_sys_para.s_sethighcm - 5;
	TTlehe.lihe = cm2num(Lihecmtmp);
}


/********************************************************************************************************/

/*2018.9.1 增加提锤点的状态机判断，防止提锤点的误操作误识别*/
void Pull_Clear(int16_t *psta)
{
	static int8_t ocnt = 0;
	int32_t tmp;
	uint32_t ctime;
	
	
	ctime = osKernelSysTick();
	switch(*psta)
	{
		case PULL_IDLE:
			tmp = epower(70);
			ocnt = 0;
			
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 150)	    //连续有效提锤超过 150ms  认为有效提锤
				{
					*psta = PULL_EFFECT;
					Enc_Clr_TotalCnt1();
                    Debug("speed %d\r\n",sys_stadata.m_power.Speed);
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_OBS:
			tmp = epower(80);
			Enc_Clr_TotalCnt1();
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 80)	    //连续有效提锤超过 80ms  认为有效提锤
				{
					ocnt++;
					*psta = PULL_EFFECT;
					
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_EFFECT:
			tmp = epower(55);
			if(ocnt < 1)
			{
				while(sys_stadata.m_power.Speed < tmp)
				{
					if(getmilsec(ctime) > 60)	    //连续有效提锤小于 120ms  认为空程
					{
						*psta = PULL_OBS;
						break;
					}
					osDelay(1);
				}
				
			}
			break;
		default:*psta = PULL_IDLE;
	}
}

/*正常提锤动作*/
extern struct Locationcm user_encoder;  

SYS_STA takeup(void)
{
	SYS_STA status;
	uint32_t ctime,intime;
	int32_t loadpw,sethnum,overpw;
	int8_t kccnt = 0;
	int32_t protectT = 0;
	int32_t speedcnt = 0;
	int32_t powcnt = 0;
	int32_t starthighcm = 0;
	
	status = ERR_NONE;
	
	do
	{
		G_LIHE(ACT_ON,0);G_LIHE(ACT_ON,0);			       /*控制和执行分开   立即拉离合*/
		G_SHACHE(ACT_OFF,SHACHEDLY);
		G_LIHE(ACT_ON,0);
		starthighcm = GetEncoderLen1Cm();	/*拉离合的高度点 2019.12.19 Terry add*/
		ctime = osKernelSysTick();
		/*连续检查离合是否动作*/
		while(speedcnt < 3)									//此时锤还在下降，超时4秒比较合适    Terry 2019.5.25
		{
			if(GetEncoderSpeedCm() > -10)
				speedcnt++;
			else
			{
				if(speedcnt > 0) 
					speedcnt--;
			}
			
			if(getmilsec(ctime) > 5000)						//拉离合后3秒后，离合还是反转，则报故障
			{				
				if(sys_stadata.m_power.Speed < (g_sys_para.s_pnull / 2))
					status |= ERR_CT;						//主机未启动或互感器损坏
				else
					status |= ERR_TT;						//漏掉了一个条件，该错误下认为是探头故障
					
				ERR_BREAK;
			}
			/*预防溜绳的处理 拉离合信号后，继续下行6米 报错误 2019.12.17  Terry add*/
			if((starthighcm - GetEncoderLen1Cm()) > 800)		/*如果松弛过大，直接报警  拉离合到最低点超过6米 2019.12.19*/
			{
				status |= ERR_LS;
				ERR_BREAK;
			}
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		
		ERR_BREAK;										/*异常直接跳出循环  do while*/		
		
		ctime = osKernelSysTick();
		Enc_Clr_TotalCnt1();
		loadpw = epower(70);
		while( powcnt < 4)								//等待 80%有效提锤点  300ms
		{
			if(sys_stadata.m_power.Speed > loadpw)
				powcnt++;								/*  超过70%的拉力,认为有效提锤*/
			else
			{
				if(powcnt > 1) 
					powcnt -= 1;
				else
					Enc_Clr_TotalCnt1();
			}
			
			//8秒超时
			if(getmilsec(ctime) > 6000)						/*拉锤保护时间 6秒  如果打得很松，自动保护*/
			{
				status |= ERR_CT;							// 离合过松或响应过慢
				break;
			}
			
			if(GetEncoderLen1Cm() < -300)		/*如果松弛过大，直接报警*/
			{
				status |= ERR_CT;
				ERR_BREAK;
			}
			
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		ERR_BREAK;													/*异常直接跳出循环  do while*/		
		sethnum = cm2num(g_sys_para.s_sethighcm);						//设定高度，换算成齿数
        protectT = g_sys_para.s_sethighcm * 30 + 5000;       			//修改Bug   2018.11.9
		sys_stadata.clihe = getlihenum();  //得到下落时，离合点的高度
		
		
		/*记录当前的深度为最低点   Terry 2019.6.5*/
		s_record.deepth = GetEncoderLen2Cm();				/*单位 厘米 */
		
Debug("upnum %d\r\n",tmp);
Debug("autolihe %d",sys_stadata.clihe);

		speedcnt = 0;
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)				//设定高度  比较的齿数
		{
			/******************上拉超时判断**************************/
			if(getmilsec(ctime) > protectT)			    		//提锤时间超过8秒，报故障
			{
				status |= ERR_LS;
                Debug("err 2");
				break;		/*直接跳出*/
			}
			ERR_BREAK;
			HALT_BREAK;

			
			/*自动夯土时的保护  2019.7.24*/
			if(g_sys_para.s_cmode == MOD_AUTOTAMP)
			{
				if(GetEncoderLen1Cm() > g_sys_para.s_hprot)	/*超过0.5米，触发保护 辜绦福匦露ㄒ謇牒系愕母叨� 2019.10.18 Terry*/
				{
															/*记住当前的提锤高度 */
					Prtop.flg = 1;
					Prtop.p_high = Enc_Get_CNT1();
					Prtop.p_lihe = sys_stadata.clihe *  Enc_Get_CNT1() * 11 / sethnum;  /*按照比例进行离合控制*/
					Prtop.p_lihe = Prtop.p_lihe / 10;
					if(Prtop.p_lihe > sys_stadata.clihe)
						Prtop.p_lihe = sys_stadata.clihe;
					
					break;							/*跳出计数循环 很关键*/
				}
			}
			
			/**************卡锤判断***********************/
			intime = osKernelSysTick();
			overpw = epower(250);
			while(sys_stadata.m_power.Speed > overpw)
			{
				//1秒超时
				if(getmilsec(intime) > 3000)				// 连续3秒超时
				{
					status |= ERR_KC;						// 卡锤了

					Debug("up kc\r\n");
					kccnt++;
					break;
				}
				HALT_BREAK;
				ERR_BREAK;				/*跳出本循环*/
				osDelay(CALUTICK);
			}
			// pull break;  卡锤处理
			if((status & ERR_KC) && (kccnt < 3))
			{
				pullbreak();
				status &= ~ERR_KC;
				protectT += 3000;			/*超时判断增加 3秒 */
			}
			else
			{
				/******************中途溜锤判断**************************/
				if(Enc_Get_SpeedE1() < 0)
					speedcnt++;
				else
				{
					if(speedcnt > 3)  
						speedcnt-=3;
				}
				
				if(speedcnt > 800)		/*连续提3秒，提不起来就报警*/
				{
					status |= ERR_LS;
					Debug("溜锤了\r\n");
				}
					
				ERR_BREAK;
			}
			osDelay(5);
			ERR_BREAK;
		}
	}while(0);
	
	Debug("high %d \r\n",sys_stadata.m_high.TotalCount);
	
	return status;
}


/*一键启动*/
SYS_STA starttaking(void)
{
	SYS_STA status;
	int32_t avepow;
	uint16_t i;
	int32_t sethnum,cnt;
	int32_t noloadpw;
	uint32_t ctime,intime;
	
	status = ERR_NONE;
	cnt = 0;
	/*求平均空载功率*/
	avepow = 0;
	
	IOT_FUNC_ENTRY;
	for(i = 0;i < 30;i++)
	{
		avepow += sys_stadata.m_power.Speed;
		osDelay(CALUTICK);
	}
	g_sys_para.s_pnull = avepow / 30;
    avepow = 0;
	Debug("null %d\r\n",g_sys_para.s_pnull);

	//拉离合 0.4秒后松刹车
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,30);
	Enc_Clr_TotalCnt1();
	osDelay(1000);
	//等待有效拉力
	do
	{
		ctime = osKernelSysTick();
		noloadpw = g_sys_para.s_pnull * 3 / 2 + 1;			//等待1.5倍的空载功率  ？？？
		while(sys_stadata.m_power.Speed < noloadpw)			
		{
			if(getmilsec(ctime) > 3000)		/*连续4秒检测不到拉力，就报警  2019.11.7*/
			{
				status |= ERR_CS;
				break;
			}
			osDelay(CALUTICK);
			HALT_BREAK;
		}
		ERR_BREAK;
		
		for(i = 0;i < 20;i++)
		{
			HALT_BREAK;
			osDelay(CALUTICK);
		}

		cnt = 0;
		avepow = 0;
//		savecnt = 0;


//		CheckDir(&sys_stadata.m_high);								//改变计数方向值		
		sethnum = cm2num(g_sys_para.s_sethighcm);							//将高度换算成齿数
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)					//设定高度
		{
			avepow += sys_stadata.m_power.Speed;
			cnt++;
			/******************中途溜锤判断**************************/
			intime = osKernelSysTick();
			while(Enc_Get_SpeedE1() < 1) 		//中途发现溜锤
			{
                Debug("hsed %d ",sys_stadata.m_high.Speed);
				if(getmilsec(intime) > 4000)			// 1秒超时
				{
					status |= ERR_LS;						// 离合过松,需调紧
					break;
				}
				HALT_BREAK;
				osDelay(5);
			}
			if(status) break;

			/***************上拉超时判断***********/
			if(getmilsec(ctime) > (g_sys_para.s_sethighcm * 30 + 5000))   		/*上拉超时判断*/         
			{
				status |= ERR_CS;
				break;
			}
			
			if(status) break;
			osDelay(2);
		}
		
	}while(0);
	
	if(status == ERR_NONE)
	{
		if(cnt)
		  g_sys_para.s_pfull = avepow / cnt;
		else
		  g_sys_para.s_pfull = sys_stadata.m_power.Speed;
		
		sys_stadata.clihe = cm2num(g_sys_para.s_hlihe);
//		savecnt = 20;											/*无错误即开始保存数据*/
	}
	
	IOT_FUNC_EXIT_RC(status);
}



/********************************************************
Function	: putdown
Description	: 打锤程序(单打和双打)
Input		: delay	  间隔时间，单打为 0 ，双打 为 300+
Return		: SYS_STA
Others		: 故障类型

1. 黏离合
2. 下降超时
*********************************************************/
SYS_STA putdown(int32_t delay)
{
	SYS_STA status;
	uint32_t ctime;
	int32_t tmp;
	
	IOT_FUNC_ENTRY;
	G_LIHE(ACT_OFF,0);
	status = ERR_NONE;
	
	do
	{
		ctime = osKernelSysTick();
		/*检测锤是否脱开*/
		while((sys_stadata.m_power.Speed > epower(LALIMAXPER)) ||(Enc_Get_SpeedE1() > 20))	/*2019.11.15  同时检测速度量*/
		{
			if(getmilsec(ctime) > 1500)				//如果提锤时间超过0.8秒  只执行1次
			{
				Debug("黏离合\r\t");
				G_SHACHE(ACT_ON,0);
				if(getmilsec(ctime) > 3000)			//黏离合 3秒超时
					status = ERR_NC;				//离合松不开  
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		if(status == ERR_NONE)						//无错误后松刹车
			G_SHACHE(ACT_OFF,0);
		
		ERR_BREAK;
		HALT_BREAK;
		
		/*等待离合点  Terry 2019.7.24*/
		if(Prtop.flg)
		{
			tmp = Prtop.p_lihe;							/*重新定义离合点的高度*/
			Prtop.flg = 0;
		}
		else
		{
			tmp = sys_stadata.clihe;                   /*传递过来的离合点  正常的时候*/
		}

		ctime = osKernelSysTick();

		Debug("lihenum %d \r\n",tmp);

		while(Enc_Get_CNT1() > tmp)			//离合点的高度
		{
			if(getmilsec(ctime) > (5200))					// 下降下降时间超过5秒，判定卡锤了
			{
				status |= ERR_KC;							//下降时卡锤了
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(2);
		}
		
	}while(0);
	
	IOT_FUNC_EXIT_RC(status);
}



/*初始的属性参数设置
被调用：default task,初始化系统属性参数   2017.11.8
*/
void sysattr_init(uint16_t flg)
{
	HAL_StatusTypeDef status;
	uint32_t ID;
	
	osDelay(500);
	FB_CHECK();
	/*默认值*/
	g_sys_para.s_dir = 0;
	g_sys_para.s_intval = 5;                  //送料时间  Terry 2019.6.6
	g_sys_para.s_numchi = 70;					//每圈齿数
	
	if(flg == 0)
		g_sys_para.s_chickid = CHK_ID;
		
	g_sys_para.s_sethighcm = 300;
	g_sys_para.s_zhou = 106;
	g_sys_para.s_pnull = 50;
	g_sys_para.s_pfull = 500;
	g_sys_para.s_cmode = MOD_FREE;
	g_sys_para.s_hlihe = g_sys_para.s_sethighcm * 2 / 3;	/*使用探头时的离合点 cm*/
	g_sys_para.s_mode = 0;							/*0:自动打锤 1:全自动强夯*/
    g_sys_para.s_cnt = 6;         /*夯土次数 Terry 2019.5.21*/
	g_sys_para.s_hprot = 150;		/*默认高度保护设置 Terry 2019.7.6*/
	g_sys_para.s_pset = 0;		/*校验无效 Terry 2019.7.9*/
	
	/*检查内存是否完整   2017.11.8  */
	status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&ID, 4);
	osDelay(5);
	if(status == HAL_OK)
	{
		if(ID == CHK_ID)
		{
			status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(2);
			Debug("EEread ok\r\n");
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
			Debug("EEread err\r\n");
			status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(1000);
		}
	}
	
	Enc_Clr_TotalCnt1();

	/*离合刹车动作初始化*/
	
	
    
    G_SHACHE(ACT_ON,0);         //初始为拉刹车        2018.12.24 Terry
    
	FB_CHECK();
    
	sys_stadata.TTCHK = 1;
	g_sys_para.s_cmode = MOD_FREE;		//上电后保持空闲模式
}

/********************************************************
Function	: FB_CHECK
Description	: 检测控制器硬件反馈信号
Input		: None
Return		: None
Others		: 程序中连续检测
*********************************************************/
extern void clear_all(void);
void FB_CHECK(void)
{
	/*电源是否正常*/
	if(PG_POWEROK())
		sys_fbsta |= FB_24VOK;
	else
		sys_fbsta &= ~FB_24VOK;

	/*离合有无信号*/
	if(PG_FBLIHE())
		sys_fbsta |= FB_LIHE;
	else
		sys_fbsta &= ~FB_LIHE;
	/*刹车有无信号*/
	if(PG_FBSC())
		sys_fbsta |= FB_SHACHE;
	else
		sys_fbsta &= ~FB_SHACHE;
	/*急停有无信号*/
	if(PG_STOP())
	{
		if((sys_fbsta & FB_RUN) == 0)
		{
			sys_fbsta |= FB_RUN;
			clear_all();
		}
	}
	else
	{
		sys_fbsta &= ~FB_RUN;
	}
}


