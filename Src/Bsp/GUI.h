
#ifndef _GUI_H
#define _GUI_H
#include "config.h"

#define MAX_KEY		15


#define KEY_HUP_BUTTON         	0x00
#define KEY_HDW_BUTTON         	0x01
#define KEY_LUP_BUTTON         	0x02
#define KEY_LDW_BUTTON         	0x03
#define KEY_SUP_BUTTON			0x04
#define KEY_SDW_BUTTON			0x05
#define KEY_SET_BUTTON			0x06
#define KEY_CHG_BUTTON			0x07
#define KEY_TLH_BUTTON			0x08
#define KEY_TSC_BUTTON			0x09
#define KEY_START_BUTTON		0x0a

#define KEY_DD_OBUTTON			0x0b
#define KEY_ZD_OBUTTON			0x0c
#define KEY_STOP_OBUTTON		0x0d
#define KEY_LIU_OBUTTON         0x0e


#define KEY_PRESSED             0x01
#define KEY_UNPRESSED           0x02
#define KEY_LONG_PRESSED        0x04

/*参数最大限制值*/
#define MAXSET_HIGH			1000  /*厘米*/
#define PER_HIGH			10	  /*每次调节10公分*/
#define MINSET_HIGH			100


enum emKEYSTATUS
{
    KEYSTATUS_NONE  = 0,
    KEYSTATUS_HUP,
    KEYSTATUS_HDW,
    KEYSTATUS_LUP,
    KEYSTATUS_LDW,
	KEYSTATUS_SUP,
	KEYSTATUS_SDW,
	KEYSTATUS_SET,
	KEYSTATUS_CHG,
	KEYSTATUS_TLH,			//测试离合的，可能不需要
	KEYSTATUS_TSC ,			//测试刹车的，同上
	KEYSTATUS_START,		//一键启动
	KEYSTATUS_ZD, 
	KEYSTATUS_SUPLONG ,
    KEYSTATUS_SDWLONG ,
	KEYSTATUS_STARTLONG ,
	KEYSTATUS_ZDLONG ,
	KEYSTATUS_CHGLONG ,
	
	KEYSTATUS_HUPLONG,		/*长按消息 2019.6.6 Terry*/
	KEYSTATUS_HDWLONG,
	KEYSTATUS_LHUPLONG,
	KEYSTATUS_LHDWLONG,
	KEYSTATUS_SETLONG,		/**/
};


enum emGUISHOW
{
    SHOW_NONE = 0,
    SHOW_TTC,
    SHOW_ZHOU,
    SHOW_TS,        /*送料时间*/
    SHOW_CNT,       /*夯土次数*/
	SHOW_BACKSET,		/*后台设置模式  Terry 2019.7.9*/
};

//显示参数
struct GUI_DATA
{
	int32_t g_sethighcm;		/*设定的上升高度  单位厘米 */
	int32_t g_lihe;				/*设定的离合点的高度  单位厘米  */
	int32_t g_nhigh;			/*锤上升的齿数*/
	int32_t g_num;				/*每圈齿数*/
	int32_t g_Zhoucm;				/*周长*/
	int32_t g_ts;				/*双打间隔时间 单位最小0.1秒  改成送料时间  2019.5.21*/
    int32_t g_hcnt;             /*夯土次数  3 - 10次  Terry 2019.5.21*/
	int32_t g_index;			/*数码管显示序号  */
	int32_t g_show;				/*数码管待显示的数值*/
	int32_t g_HighOvercm;		/*上拉超限保护值  Terry 2019.7.6*/
	int32_t g_Maxhigh;
	int8_t g_HasChanged;				/*数据是否更新标志*/
	int8_t g_mode;				/*单打和双打的标志   改成自动夯土 和 单打模式 */
	int16_t g_pset;				/*后台设置模式  Terry 2019.7.9*/
	
	int16_t s_high1;					/*高度1*/
	int16_t s_cnt1;						/*次数1*/
	int16_t s_high2;					/*高度2*/
	int16_t s_cnt2;						/*次数2*/
};



void KeyStatus(void);			/*按键检测，每10ms 一次*/
void Task_GUI_Function(void); 
void show_brush(void);
extern void GUI_Init(void);

void Dsp_PullLight(int16_t num);   // 拉力灯柱显示
void Dsp_HighLight(int8_t num, int8_t set,int8_t pot,int8_t potblink,int8_t highblink); //高度灯柱显示
void Dsp_BarLight(int16_t num,uint8_t flg);	/*显示松弛度条*/
void Dsp_Num(int16_t num, uint8_t dot,uint8_t str);		//显示数值（数码管）
void Dsp_setled(uint8_t num,uint8_t flg,uint8_t mode);  //LED指示灯
void showtest(uint8_t sta);		//测试用
#endif



