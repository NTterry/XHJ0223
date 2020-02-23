
#include "PutOffAct.h"
#include "config.h"


// 执行器
#define PIN_FLH			GPIO_PIN_11							//离合控制脚
#define PIN_FSC			GPIO_PIN_12							//刹车控制脚
						//急停控制脚
#define PIN_AS1         GPIO_PIN_6                          //刹车辅助信号引脚      2018.12.24
#define PIN_AS2         GPIO_PIN_7                          //刹车辅助信号引脚      2018.12.24

#define PORT_CTR		GPIOA

#define PORT_CAS        GPIOC

#define C_LALIHE()		HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_RESET)										//拉离合
#define C_SONGLIHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_SET)										//松离合

/*刹车辅助信号*/
#define C_AS1_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_RESET)	
#define C_AS1_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_SET)	
#define C_AS2_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_RESET)	
#define C_AS2_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_SET)	

/*软件模拟使用*/
//#define C_SONGSHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)
//#define C_LASHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET)

/*实际使用时，使用以下配置*/
/* 刹车制动器 */
#define C_SZHIDONG()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET) //输出24V  为刹车制动状态
#define C_SNZ()			HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)	//取消制动


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





/*返回反馈信号*/
int8_t Get_Fbsignal(uint8_t MASK)
{
	if(sys_fbsta & MASK)
		return 1;
	else
		return 0;
}

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
			C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
		}
	}
}


/* 离合动作   如果延时为0时，立即执行*/
void G_LIHE(uint32_t sta, uint32_t delay)
{
//	portENTER_CRITICAL();
	g_alihe.delay = delay / T_ACT_MS;
	g_alihe.sta = sta;
	if((delay == 0) && (sta == ACT_ON))
	{
		C_LALIHE();
	}
	if((delay == 0) && (sta == ACT_OFF))
	{
		C_SONGLIHE();
	}
		
//	portEXIT_CRITICAL();
}

uint16_t Shache_Proc(void)
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
            if(HAL_MS_DIFF(sctime) > 5000)				/*刹车抱刹制动时间*/
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
  * 函数功能 外部执行离合 和刹车 操作
  * 输入参数： 无

  * 返回值: 无
  * 说明: 调用任务 StartDefaultTask  每 CALUTICK 毫秒执行一次
  */
//static uint16_t s_lihecnt = 0;
void g_action(void)
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
            C_SONGLIHE();				//每10ms确认一次
        }
        else
        {
            C_LALIHE();
        }
    }
    /*刹车延时信号*/
	if(g_shache.delay == 0)
    {
        g_shache.pact = g_shache.sta;
    } 
	
    if(g_shache.delay >= 0)
    {
        g_shache.delay--;
    }

/*Fb_Err_check()*/
//	if(g_alihe.delay == 0)
//	{
//		if((g_alihe.sta == ACT_OFF) && (Get_Fbsignal(FB_LIHE) == 1))	// 松离合
//		{
//			s_lihecnt++;
//		}
//		else if((g_alihe.sta == ACT_ON) && (Get_Fbsignal(FB_LIHE) == 0))	// 拉离合
//		{
//			s_lihecnt++;
//		}
//		else
//		{
//			s_lihecnt = 0;
//		}
//	}
//	if(s_lihecnt > 120)			/*Terry 2019.7.9*/
//	{
//		g_erract |= ERR_LH;
//	}
}

/*Called by Timer as 10ms*/
void G_ActPoll_10ms(void)
{
	g_action();
	Shache_Proc();
}

void Pf_Lihe_Init(void)
{
	g_alihe.delay = 10;
	g_alihe.sta = ACT_OFF;
}

void Pf_Shache_Init(void)
{
	g_shache.delay = 10;
	g_shache.sta = ACT_OFF;
	
	C_SONGLIHE();
	C_SNZ();
	C_AS1_DS();
	C_AS2_DS();
}

