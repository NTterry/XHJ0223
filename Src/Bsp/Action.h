
/**********************************************
Copyright (C),2015-2016,TT Tech.Co.,Ltd.
File name	:main.c
Author		:Terry
Description	:打桩机板级测试驱动主程序V1.0
Others		:None
Date		:2015.06.01  - 2017.02.11
***********************************************/

#ifndef  _ACTION_H
#define  _ACTION_H
#include "config.h"
#include "tim.h"

#define CALUTICK				5					//40毫秒一次
#define SHACHEDLY				(800)
#define LIHEDLY         		(4 * 110)    /*0.4秒*/
#define ERRDLY					500


#define HANG_TICK       		5      /*夯土工作时的间隔时间*/
#define T_GAV					(6)
#define LIHESHOW				15
#define HIGHSHOW				15

#define LALIMAXPER		60					//2019.11.15      原来 70%
#define LALIMIDPER		50
#define LALIMINPER		55					// 实际下锤时间改为 55% ,刚才目测下锤时有黏离合的现象  // 2018.6.1

#define UPTMOUT			20000				//上拉超时时间
#define VUP				25					// 0.4米的提锤速度下  每米需要2.5秒
#define ERR_NONE		(uint32_t)0x000000
#define ERR_KC			(uint32_t)0x01		// 黏锤 / 卡锤
#define ERR_NC			(uint32_t)(1<<1)	// 离合未脱开
#define ERR_LS			(uint32_t)(1<<2)	//离合响应过慢
#define ERR_CS			(uint32_t)(1<<3)	//提锤时间超时
#define ERR_TOP			(uint32_t)(1<<4)	//冲顶保护
#define ERR_LIU			(uint32_t)(1<<5)	//溜放故障
#define ERR_DOWN		(uint32_t)(1<<6)	//下降深度过小
#define ERR_CHAO		(uint32_t)(1<<7)	//上拉超限



#define ERR_PW			(uint32_t)(1<<8)	//电源故障
#define ERR_TT			(uint32_t)(1<<9)	//探头2故障
#define ERR_LH			(uint32_t)(1<<10)	//离合线路故障
#define ERR_SC			(uint32_t)(1<<11)	//刹车线路故障
#define ERR_CT			(uint32_t)(1<<12)	//主机未启动或互感器故障
#define ERR_HALT		(uint32_t)(1<<30)	//手动停止
#define ERR_ACE			(uint32_t)(1<<13)   //授权错误




/*工作状态*/

enum emWORKMODE
{
    MOD_FREE = 0,
    MOD_SIGACT,
    MOD_AUTOTAMP,       /*自动夯土流程*/
    MOD_MANUAL,
    MOD_MANOFF,
    MOD_TST,
    MOD_DOWN,       /*手动溜放模式  Terry 2019.5.21*/
};

enum emHANGTU
{
    S_IDLE	 = 0,
    S_PULSE,        /*1次中断 */
    S_TICHUI,       /*提锤 到上顶点*/
    S_CHECK_UPSTOP,	/*确认主机停止*/
    S_XIALIAO,      /*下料*/
    S_DELAY2,
    S_LIUF,         /*溜放*/
    S_DACHUI,       /*打锤*/
};

struct HANGTU
{
    uint32_t pvtimer;       /*上次打锤时间*/
    int32_t last_highnum;   /*上拉时的高度*/
    int32_t overpow;        /*超重累积计数*/
	int32_t lowpow;			/*无电流保护*/
    int32_t speederr;       /*失速累积计数*/
    int32_t dachui_cnt;     /*打锤的次数*/
    int32_t liufang_sta;    /*溜放状态检测*/
    int32_t top_sta;        /*到顶状态检测*/
	int32_t stop_sta;		/*上拉到顶停机状态检测*/
    int32_t surecnt;		
	int16_t last_downnum;   /*上次溜放的齿数*/
	int16_t last_sta;
};    







#define SIG_HAN         (0x01 <<1)   /*冲顶信号*/

#define KG_CHUI			6		/*假设锤的重力加速度为 6m/s2*/
#define SP_CHUI			0.35	/*假设锤的上升速度为0.35m/s*/

#define INIT_CHUI		50		/*手动干预后，50锤不改变设定值*/

/*探头提锤时，提锤点的判定*/
#define PULL_IDLE		0		/*非有效提锤*/
#define PULL_OBS		1		/*待观察*/
#define PULL_EFFECT		2		/*有效提锤*/




/**********************记录的数据结构********************************************/

/*系统属性，设置数据*/
__packed struct SYSATTR
{
	uint32_t s_chickid;					/*校验码  0xA5A6A7A8*/
	int32_t s_sethighcm;				//厘米
	int32_t s_setlihecm;				//厘米			设定的离合高度
	int32_t s_pericm;					//卷筒周长  厘米
	int32_t s_numchi;					//每周齿数  个数
	int32_t s_pnull;					/*空载时的平均功率*/
	int32_t s_pfull;					/*满载时的平均功率*/
	int32_t s_rammcnt;                      /*夯土次数  3-10次 Terry 2019.5.21  */
	uint32_t s_dir;						/*编码器方向标志信号*/
	int32_t s_hprot;					/*高度保护设置 默认 300cm*/
	int16_t s_pset;						/*永久授权模式*/
	uint8_t s_feedtims;					/*双打间隔时间，单位 0.1秒   改成 送料时间 Terry 2019.5.21  单位 秒  2-15秒*/
	int8_t  s_mode;						/*0 单打  1 表示双打*/
};

/*有探头与无探头共用的离合调整数据*/
__packed struct LIHEDATA			/*离合与松弛度的中间计算值*/
{
	int32_t relaxsum;				/*松弛度的累加值*/
	int32_t powersum;				/*提锤有效功率平均值*/
	int32_t cnt;					/*松弛度累加次数*/
	int32_t lihemax;				/*离合调整量上限*/
	int32_t lihemim;				/*离合调整量下限*/
	int32_t lihe;					/*离合中间量*/
	int32_t relaxon;				/*标准的松弛度参考，手动模式下前N锤的松弛度平均值*/
	int32_t powerave;				/*上拉时的平均功率*/
	uint32_t uptime;
	int16_t	songchi;				/*松弛度，为0 时表示正在适应，为1时表示在自动调整中*/
};


__packed struct PRPTECT_HANGTU
{
	int16_t flg;			/*到顶的标志*/
	int16_t tmp;
	int32_t p_high;			/*实际的提锤高度 ，单位齿数*/
	int32_t p_lihe;			/*离合的位置 齿数*/
};

__packed struct RECORD
{
	uint16_t nub;			/*序号*/
	uint16_t cnt;			/*次数*/
	uint16_t high;			/*当前深度  0.1米*/
	uint16_t tim;			/*送土时间*/
	int16_t deepth;			/*当前深度  0.1米*/
};

/*速度测量结构体*/
typedef __packed struct SPEED
{
    uint16_t sta;
    uint16_t lastcount;
    uint32_t lasttim;
    int32_t pertim;
    int32_t pertick;
    int32_t speed;         //  100 * tick /s
    int32_t acce;          //加速度 100 * tick /(s*s)
    int16_t readyflg;
}SSPEED;


#define HALT_BREAK	{			\
						if(g_halt)	\
						{status |= ERR_HALT; break;}  \
                        if(g_st_SigData.m_Mode == MOD_FREE)\
                            break;\
					}

#define ERR_BREAK   if(status)  break

typedef  uint32_t SYS_STA;

extern struct SYSATTR g_sys_para;
extern struct STADATA sys_stadata;
extern volatile uint8_t sys_fbsta;						// 外部反馈信号

/*获得编码器计数，确定基准方向*/
//void GetEncode(uint32_t dir, struct EncoderCnt *p);
/*获得当前功率*/
void GetPower(struct EtrCnt *p);

/********无探头提锤动作***********/
SYS_STA ntakeup(void);
/*一键启动*/
SYS_STA nstarttaking(void);
/*打锤*/
SYS_STA nputdown(int32_t delay);

void sysattr_init(uint16_t flg);


/*离合刹车动作单独执行*/
void g_action(void);
void G_LIHE(uint32_t sta, uint32_t delay);
void G_SHACHE(uint32_t sta, uint32_t delay);
void G_SHACHE_SG(uint32_t sta, uint32_t delay);     //不带辅助信号
void liheupdate(void);//更新离合参数值

uint16_t Shache_Proc(void);
//void EncoderClr(struct EncoderCnt *p);

int GetEncoderSpeedCm(void);
int GetEncoderAcceCm(void);
int GetEncoderLen1Cm(void);
int GetEncoderLen2Cm(void);

#endif

