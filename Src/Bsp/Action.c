

#include "Action.h"
#include "math.h"
#include "EEPROM.h"
#include "Encoder.h"
#include "PutOffAct.h"
#include "u_log.h"
				


//struct LIHEDATA	    TTlehe;							//使用探头的离合计数模式,修改离合点后，保存所需参数
//struct LIHEDATA	    TMlehe;		    				// TM离合动作，保存所需参数
//extern int8_t Get_Fbsignal(uint8_t MASK);

//uint32_t getmilsec(uint32_t pretime)
//{
//	uint32_t milsec;	
//	milsec = osKernelSysTick();
//	if(pretime > milsec)
//		milsec = (0xffffffff - pretime) + milsec;  			/*超过一天的跨度*/
//	else
//		milsec = milsec - pretime;			
//	return 	milsec;
//}


/*获得编码器计数，确定基准方向*/
/**
  * 函数功能 获得编码器计数，确定基准方向
  * 输入参数：dir:基准方向
  *           *p:硬件编码器计数数据
  * 返回值: None
  * 说明: 无
  */
  
/*  厘米转齿数  */
//int32_t cm2num(int32_t cm)
//{
//	int32_t tmp;
//	
//	tmp = cm * g_sys_para.s_numchi / g_sys_para.s_pericm;
//	
//	return tmp;
//}

/*
卡锤时
中断上拉
拉刹车 松离合

拉离合  松刹车
*/
//void pullbreak(void)
//{
//	G_SHACHE(ACT_ON,0);
//	G_LIHE(ACT_OFF,LIHEDLY / 2);
//	osDelay(1500);
//	G_LIHE(ACT_ON,0);
//	G_SHACHE(ACT_OFF,SHACHEDLY / 2);
//}
/******************************************离合点自动计算  有探头模式 ***************************************/
/**
  * 函数功能 返回有效了离合点高度（齿数）
  * 输入参数： relax:实际松弛度
  *             *cnt:允许调整的倒计数指针

  * 返回值: int32_t 实际的离合点
  * 说明: 无
  */

//int32_t getlihenum(void)
//{
//	static int32_t lihenum;
//	
//	liheupdate();
//	lihenum = TTlehe.lihe;									//初始的离合点

//	return lihenum;
//}

/**
  * 函数功能 更新离合点的计数值
  * 输入参数： 无
  * 返回值: 无
  * 说明: 每次修改离合点数值后，都会重新计算  离合点的自动调整区间为 正负 20cm
  */
//extern struct HANGTU s_hang;  
//void liheupdate(void)
//{
//	int32_t Lihecmtmp = 0;
//	
//	TTlehe.cnt = INIT_CHUI;		//修正后默认50次的稳定时间
//	TTlehe.relaxsum = 0;		//该值正常为负值
//	TTlehe.songchi = 0;
//	
//	Lihecmtmp = g_sys_para.s_setlihecm;
//	
//	if(g_st_SigData.m_Mode == MOD_AUTOTAMP)
//	{
//		if(s_hang.dachui_cnt == g_sys_para.s_cnt)			/*第一次打锤,打松一点  2019.9.17*/
//			Lihecmtmp -= 6;
//		if(s_record.deepth > -300)						/*到达顶部时，打紧锤*/
//			Lihecmtmp += 8;			
//	}
//	
//	if(Lihecmtmp < 50)						/*最小拉离合的点为50公分  2019.12.17*/
//		Lihecmtmp = 50;
//	else if(Lihecmtmp > g_sys_para.s_sethighcm - 5)
//		Lihecmtmp = g_sys_para.s_sethighcm - 5;
//	TTlehe.lihe = cm2num(Lihecmtmp);
//}


/********************************************************************************************************/


/********************************************************
Function	: FB_CHECK
Description	: 检测控制器硬件反馈信号
Input		: None
Return		: None
Others		: 程序中连续检测
*********************************************************/
//extern void clear_all(void);
//void FB_CHECK(void)
//{
//	/*电源是否正常*/
//	if(PG_POWEROK())
//		sys_fbsta |= FB_24VOK;
//	else
//		sys_fbsta &= ~FB_24VOK;

//	/*离合有无信号*/
//	if(PG_FBLIHE())
//		sys_fbsta |= FB_LIHE;
//	else
//		sys_fbsta &= ~FB_LIHE;
//	/*刹车有无信号*/
//	if(PG_FBSC())
//		sys_fbsta |= FB_SHACHE;
//	else
//		sys_fbsta &= ~FB_SHACHE;
//	/*急停有无信号*/
//	if(PG_STOP())
//	{
//		if((sys_fbsta & FB_RUN) == 0)
//		{
//			sys_fbsta |= FB_RUN;
//			clear_all();
//		}
//	}
//	else
//	{
//		sys_fbsta &= ~FB_RUN;
//	}
//}


