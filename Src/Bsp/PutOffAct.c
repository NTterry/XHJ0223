
#include "PutOffAct.h"
#include "config.h"
#include "u_log.h"
#include "gpio.h"

//松离合

	

/*软件模拟使用*/
//#define C_SONGSHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)
//#define C_LASHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET)

/*实际使用时，使用以下配置*/
/* 刹车制动器 */
#define C_SZHIDONG()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET) //输出24V  为刹车制动状态
#define C_SNZ()			HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)	//取消制动


#ifdef USE_LIHE_PWM
	int gLiheRatio = 0;		//比值 1 - 10
	int LiheRLY	 = ACT_OFF;
#endif

/*离合动作和刹车动作共用的标准数据*/
struct ACT_STA
{
	uint32_t	sta;
	int32_t		delay;				//单位 毫秒
    uint32_t    pact;
};


/*    useless          */
extern volatile uint8_t sys_fbsta;						// 外部反馈信号
static volatile struct ACT_STA g_alihe,g_shache;		//离合 刹车 动作控制  需要初始化设置
static enum SC sact;                					//刹车执行的状态机


/*计算当前的时间间隔*/
static uint32_t HAL_MS_DIFF(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = HAL_GetTick();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*超过一天的跨度*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}





/* 刹车动作  ACT_ON  拉刹车    ACT_OFF  松刹车*/
void G_SHACHE(uint32_t sta, uint32_t delay)
{
	g_shache.delay = delay / T_ACT_MS;
    g_shache.sta = sta;
	
	if(delay == 0)						/*2019.12.17*/
	{
		if(g_shache.sta == ACT_ON)		/*立即拉刹车  在任务中直接执行*/
		{
#ifdef USE_LIUF
			C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
#else
			 C_SNZ();
#endif
		}
#ifndef USE_LIUF	
		if(g_shache.sta == ACT_OFF)
		{
			C_SZHIDONG();
		}
#endif

	}
}


/* */
/********************************************************
Function	: G_LIHE
Description	: 离合延时动作   如果延时为0时，立即执行
Input		: sta   动作指令
            ：delay 延时时间
Return		: None
Others		: 1ms 中断中调用
*********************************************************/
void G_LIHE(uint32_t sta, uint32_t delay)
{
	g_alihe.delay = delay / T_ACT_MS;
	g_alihe.sta = sta;
	if((delay == 0) && (sta == ACT_ON))
	{
#ifndef USE_LIHE_PWM
		C_LALIHE();
#else
		LiheRLY = sta;
#endif
	}
	if((delay == 0) && (sta == ACT_OFF))
	{
#ifndef USE_LIHE_PWM
		C_SONGLIHE();
#else
		LiheRLY = sta;
#endif
	}	
}

/********************************************************
Function	: Lihe_Generate_PWM  
Description	: PWM输出离合控制信号,大力 gLiheRatio = 10  和 小力 gLiheRatio = 5
Input		: None
Return		: None
Others		: 1ms 中断中调用
*********************************************************/
void Lihe_Generate_PWM(void)
{
#ifdef USE_LIHE_PWM
	static int scnt;
	if(LiheRLY == ACT_ON)
	{
		if(scnt <  gLiheRatio)
			C_LALIHE();
		else
			C_SONGLIHE();
			
		scnt++;
		scnt = scnt % 10;
	}
	else
		C_SONGLIHE();
#endif
}

/********************************************************
Function	: Shache_Proc  
Description	: 根据控制指令，执行刹车控制信号输出，松刹车 拉刹车 溜放等
Input		: None
Return		: LED指示灯信息
Others		: None
*********************************************************/
static uint16_t Shache_Proc(void)
{
    static uint32_t sctime;
	int16_t ledsta = 0;
    switch(sact)
    {
        case S_NOACT:		
            C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
			ledsta = SIG_SHACHE;					/*刹车指示信号*/
			if(g_shache.pact == ACT_OFF)
            {
				sact = S_SONG1;
                sctime = HAL_GetTick();
            }
			if(g_shache.pact == ACT_LIU)
			{
				sact = S_LIUFANG;
			}
            break;
        case S_LIUFANG:
            C_SNZ();                        		/*取消制动*/
            C_AS1_EN();
            C_AS2_DS();
			if(g_shache.pact == ACT_ON)
				sact = S_NOACT;
			if(g_shache.pact == ACT_OFF)
			{
				sact = S_SONG1;
				sctime = HAL_GetTick();
			}
			ledsta = SIG_SLIUF;
			
            break;
        case S_SONG1:
            C_SNZ();
            C_AS1_EN();
            C_AS2_EN();
            if(HAL_MS_DIFF(sctime) > 6000)				/*刹车抱刹制动时间*/
				sact = S_SONG2;
			
			if(g_shache.pact == ACT_ON)
				sact = S_NOACT;
			if(g_shache.pact == ACT_LIU)   /*先进入刹车模式，再进入溜放*/
			{
				sact = S_NOACT;
			}
			ledsta = 0;
            break;
        case S_SONG2:
            C_SZHIDONG();   /*切换为制动器锁死*/
            C_AS1_DS();
            C_AS2_DS();
		
			if(g_shache.pact == ACT_ON)		// 拉刹车
				sact = S_NOACT;
			if(g_shache.pact == ACT_LIU)						/*先刹车。后溜放*/
			{
				sact = S_NOACT;
			}
			ledsta = 0;
			break;
        default:
            sact = S_NOACT;
            break;
    }
	
	return ledsta;
}


/**
  * 函数功能 外部执行离合 和刹车 延时执行操作
  * 输入参数： 无

  * 返回值: 无
  * 说明: 调用任务 StartDefaultTask  每 10 毫秒执行一次
  * 到达设定时间后，执行
  */
static void g_action(void)
{	 
    if(g_alihe.delay > 0)
    {
        g_alihe.delay--;
    }
    else
    {
        g_alihe.delay = 0;
        if(g_alihe.sta == ACT_OFF)
        {
           				//每10ms确认一次
#ifndef USE_LIHE_PWM
			C_SONGLIHE();
#else
			LiheRLY = ACT_OFF;
#endif
        }
        else
        {
#ifndef USE_LIHE_PWM
			C_LALIHE();
#else
			LiheRLY = ACT_ON;
#endif
        }
    }
#ifdef USE_LIUF
    /*刹车延时信号*/
	if(g_shache.delay == 0)
    {
        g_shache.pact = g_shache.sta;
    } 
    if(g_shache.delay >= 0)
    {
        g_shache.delay--;
    }
#else
    if(g_shache.delay > 0)
    {
        g_shache.delay--;
    }
    else
    {
        g_shache.delay = 0;
        if(g_shache.sta == ACT_OFF)
        {
            C_SZHIDONG();				//每10ms确认一次
        }
        else
        {
            C_SNZ();		// 刹住
        }
    }
#endif
}

/********************************************************
Function	: G_ActPoll_T  
Description	: 定时器中断调用，离合和刹车信号的顺序执行  执行周期 CALUTICK毫秒
Input		: None
Return		: None
Others		: None
*********************************************************/
void G_ActPoll_T(void)
{
	g_action();
#ifdef USE_LIUF
	Shache_Proc();
#endif
}

/********************************************************
Function	: Pf_Shache_Init
Description	: 刹车信号上电初始化
Input		: None
Return		: None
Others		: None
*********************************************************/
void Pf_Shache_Init(void)
{
	g_shache.delay = 10;
	g_shache.sta = ACT_OFF;
	
	C_SONGLIHE();
	C_SNZ();
	C_AS1_DS();
	C_AS2_DS();
}

