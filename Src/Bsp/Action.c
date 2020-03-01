

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
			osDelay(1000);
		}
	}
	
	Enc_Clr_TotalCnt1();

	/*离合刹车动作初始化*/
	
	
    
    G_SHACHE(ACT_ON,0);         //初始为拉刹车        2018.12.24 Terry
    Enc_Set_Dir(g_sys_para.s_dir);
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


