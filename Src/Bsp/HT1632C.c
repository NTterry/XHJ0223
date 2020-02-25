/**********************************************
Copyright (C),2017-2018,ENST Tech.Co.,Ltd.
File name	:HT1632C.c
Author		:Terry
Description	:HT1632C驱动及显示程序
Others		:None
Date		:2017.07.05  - 2017.08.01
***********************************************/
#include "config.h"
#include "string.h"
#include "GUI.h"

#define SYS_DIS					0x00	 	/*关闭内部晶振*/
#define SYS_EN					0x01		/*打开内部晶振*/
#define LED_OFF					0x02		/*关闭LED显示*/
#define LED_ON					0x03		/*打开LED显示*/
#define BLINK_OFF				0x08		/*关闭闪烁*/
#define BLINK_ON				0x09		/*打开闪烁*/
#define SLAVE_MODE				0x10		/*从模式*/
#define RC_MASTER_MODE			0x18		/*内部RC时钟*/
#define COM_OPTION				0x20		/*32:8com*/
#define PWM_DUTY				0xA6		/*PWM亮度控制  5/16级亮度*/

#define HTBUFF_LEN				24
#define FRQS					20			/*减小该值可以增加频率*/

#define LEDNULL					0x00
#define REDDOT					0x01
#define GREENDOT				0x02
#define YELLOWDOT				0x03
#define BLINKS					0x80

#define COM7					0x10
#define COM6					0x20
#define COM5					0x40
#define COM4					0x80
#define COM3					0x01
#define COM2					0x02
#define COM1					0x04
#define COM0					0x08


const uint8_t decoder[8] = 	{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
const uint8_t LEDCODE[21] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,    		/*0 ~ 9        0 1 2 3 4 5 6 7 8 9*/
                             0x77,0x7c,0x39,0x5e,0x79,0x71,0x00,0x80,0x40,0x73,0x79};             /*10 ~ 18       A b C d E F NON DP - P E*/

														 
volatile static uint8_t htbuff[HTBUFF_LEN];			/*HT1632 内存缓存*/
/*对HT1632写数据时，指示灯微亮的干扰，异常，所以防止频繁刷新所有的显示*/
volatile static uint8_t bkbuff[HTBUFF_LEN];			/* 上一帧的显示内存，只刷新变动过的部分，防止显示微亮的干扰*/			 

static uint8_t PullForce[24];		/*拉力显示灯柱    0x01 红  0x02 绿  0x03 黄  0x7X 闪烁*/
static uint8_t HighSet[24];			/*高度设定*/
static uint8_t Loosen[9];			/*松弛度*/
static uint8_t Display[3];			/*待显示的数值*/
static uint8_t Dled[6];				/*独立的LED指示灯*/

void show_brush(void);


/*							 松弛度     高度             拉力         对应显示内存表
			COM7	COM6	COM5	COM4	COM3	COM2	COM1	COM0	
R0			RXR0	HR13	HR1		PR13	PR1		D31		D21		D11
R1			RXR1	HR14	HR2		PR14	PR2		D32		D22		D12
R2			RXR2	HR15	HR3		PR15	PR3		D33		D23		D13
R3			RXR3	HR16	HR4		PR16	PR4		D34		D24		D14
R4			RXR4	HR17	HR5		PR17	PR5		D35		D25		D15
R5			RXR5	HR18	HR6		PR18	PR6		D36		D26		D16
R6			RXR6	HR19	HR7		PR19	PR7		D37		D27		D17
R7			RXR7	HR20	HR8		PR20	PR8		D38		D28		D18
R8			RXR8	HR21	HR9		PR21	PR9		
R9			RXR9	HR22	HR10	PR22	PR10
R10					HR23	HR11	PR23	PR11
R11					HR24	HR12	PR24	PR12
R12			RXG0											GD1		
R13			RXG1											GD2		
R14			RXG2											GD3		
R15			RXG3											GD4		
R16			RXG4											GD5		
R17			RXG5											
R18			RXG6													
R19			RXG7													
R20			RXG8
R21			RXG9
R22
R23						 
*/
/********************************************************
Function	: Led2Buff
Description	: 显示双色LED灯的中间程序，将业务数据写入显示缓存
Input		: p	    待显示信息
			: len	显示信息的长度
			：blink 是否闪烁  非0闪烁
			：com   显示的位置
Return		: None
Others		: 中间函数，不对外
*********************************************************/
static void Led2Buff(uint8_t *p,uint8_t len, uint8_t blink,uint8_t com)
{
	uint8_t i;
	
	if(len > 12)
		len = 12;
	
	for(i = 0;i < len;i++)
	{
		if((blink) && (p[i] & BLINKS))
			continue;							/*闪烁 灭*/
		else
		{
			if(p[i] & REDDOT)
				htbuff[i] |= com;
			if(p[i] & GREENDOT)
				htbuff[i + 12] |= com;
		}	
	}
}
    /*独立指示灯的数据写入显示缓存  只是单色灯，可以与上面合并的*/
static void DLed2Buff(uint8_t *p,uint8_t len, uint8_t blink,uint8_t com)
{
	uint8_t i;
	
	if(len > 12)
		len = 12;
	
	for(i = 0;i < len;i++)
	{
		if((blink) && (p[i] & BLINKS))
			continue;							/*闪烁 灭*/
		else
		{
			if(p[i] & GREENDOT)
				htbuff[i + 12] |= com;
		}	
	}
}
/*将待显示的内容写入到缓存里，每50ms调用一次*/
/********************************************************
Function	: W2Buff
Description	: 将待显示的内容写入到缓存里，每50ms调用一次
Input		: None
Return		: None
Others		: None
*********************************************************/
void W2Buff(void)
{
	uint8_t i,blink;
	static uint8_t blinkcnt = 0;
	
	blinkcnt++;
	if(blinkcnt > 4)
	{
		blinkcnt = 0;
	}
	
	if(blinkcnt == 1)
		blink = 0;
	if(blinkcnt == 3)
		blink = 1;
	/*先清空缓存*/
	for(i = 0;i < HTBUFF_LEN; i++)
		htbuff[i] = 0x00;
	
	
	/*松弛度状态*/
	Led2Buff(Loosen, 9, blink, COM7);
	/*拉力显示*/
	Led2Buff(PullForce, 12, blink, COM3);
	Led2Buff(&PullForce[12], 12, blink, COM4);
	/*高度显示*/
	Led2Buff(HighSet, 12, blink, COM5);
	Led2Buff(&HighSet[12], 12, blink, COM6);
	/*LED独立指示灯*/
	DLed2Buff(Dled,6,blink,COM1);
	/*数码管显示*/
	for(i = 0; i < 8; i++)
	{
		if(Display[2] & (1 << i))
		{
			htbuff[i] |= COM0;
		}
	}
	for(i = 0; i < 8; i++)
	{
		if(Display[1] & (1 << i))
		{
			htbuff[i] |= COM1;
		}
	}
	for(i = 0; i < 8; i++)
	{
		if(Display[0] & (1 << i))
		{
			htbuff[i] |= COM2;
		}
	}
}
														 
														 
void delayus(uint16_t ticks)
{
	uint16_t i;
	
	while(ticks--)
	{
		for(i = 0 ; i < 5;i++)
			;
	}
}
/********************************************************
Function	: HT1632_Write
Description	: 串行向HT1632C写数据，高位在前
Input		: Data	:待写入数据
			: cnt	:写入位数
Return		: None
Others		: None
*********************************************************/
void HT1632_Write(uint8_t Data,uint8_t cnt)
{
	uint8_t i;

	for(i = 0; i < cnt; i++)
	{
		HWR_L;
		delayus(2);
		if(Data & 0x80)
			HDA_H;
		else
			HDA_L;
		Data = Data << 1;
		delayus(2);
		HWR_H;
		delayus(2);
	}	
}
/********************************************************
Function	: HT1632C_Write_CMD
Description	: 向HT1632Cd写命令
Input		: cmd	:命令
Return		: None
Others		: None
*********************************************************/
void HT1632C_Write_CMD(uint8_t cmd)
{
	HCS_L;
	delayus(2);
	HT1632_Write(0x80,3);
	HT1632_Write(cmd,9);
	delayus(2);
	HCS_H;
	delayus(2);
}

/********************************************************
Function	: HT1632C_Write_BYTE
Description	: 向HT1632Cd的指定地址连续写两个数据，高字节高地址
Input		: Addr	:地址
			: dat	:待写入数据
Return		: None
Others		: None
*********************************************************/
void HT1632C_Write_BYTE(uint8_t Addr,uint8_t dat)
{
	HCS_L;
	delayus(2);
	HT1632_Write(0xa0,3);
	HT1632_Write(Addr << 1,7);
	HT1632_Write(dat << 4,4);			/*先写低地址*/
	HT1632_Write(dat,4);				/*再写高地址*/
	delayus(2);
	HCS_H;
}
/********************************************************
Function	: HT1632_Init
Description	: 初始化该外设
Input		: None
Return		: None
Others		: None
*********************************************************/
void HT1632_Init(void)
{
	
	HCS_H;
	delayus(2);
	HWR_H;
	delayus(2);
	HDA_H;
	delayus(2);
	HT1632C_Write_CMD(SYS_DIS);		/*关闭内部RC时钟*/
	HT1632C_Write_CMD(COM_OPTION);	/*16 com N-MOS*/
	HT1632C_Write_CMD(RC_MASTER_MODE);
	HT1632C_Write_CMD(SYS_EN);
	HT1632C_Write_CMD(LED_ON);			/*打开LED显示*/
	HT1632C_Write_CMD(PWM_DUTY);
	HT1632C_Write_CMD(BLINK_OFF);
}


/********************************************************
Function	: show_brush
Description	: 将缓存数据写入HT1632C，连续字节写入  外部调用 , 写入奇数字节数
Input		: None
Return		: None
Others		: None
*********************************************************/
void show_brush(void)
{
	uint8_t i;
	static uint16_t cnt = 0;
	
	cnt++;
	if(cnt > 100)
	{
		HT1632_Init();		//每10秒刷新以下HT1632C
		cnt = 0;
	}

	for(i = 0;i < HTBUFF_LEN; i++)
	{

		if(bkbuff[i] != htbuff[i])
		{
			HT1632C_Write_BYTE((i<<1),htbuff[i]);
			bkbuff[i] = htbuff[i];
		}
	}	
}

/*        数码管显示对外调用函数        */

//面板指示灯
void Led_Lamp(uint8_t lamp)
{
	htbuff[16] = lamp;
}

/********************************************************
Function	: Dsp_PullLight
Description	: 显示提锤拉力大小的光柱显示
Input		: num	:拉力，表示 0 -- 24      10表示 100%的拉力，超过 200%（20）的拉力显示红色
Return		: None
Others		: None
*********************************************************/
void Dsp_PullLight(int16_t num)
{
	uint8_t i;
	
	if(num < 0)
		num = 0;
	for(i = 0; i < 24;i++)
	{
		if(i < num)
			PullForce[i] = ((num < 20) ? GREENDOT : REDDOT);			//拉力过大时，全部显示红色
		else
			PullForce[i] = LEDNULL;
	}
	
	PullForce[10] = YELLOWDOT;				// 100%的点显示黄色
}

/********************************************************
Function	: Dsp_HighLight
Description	: 显示设定高度及实际高度和离合点
Input		: num	:当前高度，表示 0 -- 24      set 设定高度  pot 离合高度  potblink 离合闪烁标志 0 不闪 1 闪烁
Return		: None
Others		: None
*********************************************************/
void Dsp_HighLight(int8_t num, int8_t set,int8_t pot,int8_t potblink,int8_t highblink)
{
	uint8_t i;
	/*绿灯显示正常高度*/
	for(i = 0; i < 24; i++)						// OK
	{
		if(i < num)
			HighSet[i] = GREENDOT;
		else
			HighSet[i] = LEDNULL;
	}
	/*红灯显示设定高度*/
	if(set > 24)
		set = 24;
	if(set < 0)
		set = 0;
	if(highblink)
		HighSet[set - 1] = REDDOT|BLINKS;
	else
		HighSet[set - 1] = REDDOT;
		
	if(pot  > 24)
		pot = 24;
	if(pot > 4)
	{
		HighSet[pot - 1] = YELLOWDOT;
		if(potblink)
			HighSet[pot - 1] |= BLINKS;					//闪烁标志
	}else												//离合小于0时，显示红色
	{
		if(pot > 0)
		{
			HighSet[pot - 1] = YELLOWDOT;
			HighSet[pot - 1] |= BLINKS;
		}
		else
			HighSet[0] = REDDOT;
	}
}

/********************************************************
Function	: Dsp_BarLight
Description	: 松弛显示表
Input		: num	:0 ~ 8 表示由松到紧  
Return		: None
Others		: None
*********************************************************/
void Dsp_BarLight(int16_t num,uint8_t flg)
{
	uint8_t i;
	
	for(i = 0; i < 9; i++)
	{
        if(num & (0x0001 << i))
        {
            if((i == 0) || (i > 3))
                Loosen[i] = YELLOWDOT;	
            else
                Loosen[i] = GREENDOT;	  
        }
        else
            Loosen[i] = LEDNULL;
	}
}


/********************************************************
Function	: Dsp_Num
Description	: 数码管显示字符
Input		: num	:待显示的数字
			  dot   :显示小数点
			  str	:显示字符
Return		: None
Others		: None
*********************************************************/
/*                数码管显示对外调用函数        */
/********************************************************
Function	: Led_dspnum 
Description	: 数码管显示一个数字
Input		: int16_t num  显示一个数值，如果为负数，则显示负号
              uint8_t dot       是否带小数点，非0即带小数点
              uint8_t str     	起始位置显示字符
Return		: None
Others		: None
*********************************************************/
void Dsp_Num(int16_t num, uint8_t dot,uint8_t str)
{
	uint8_t buff[3],i,fu,j;
	uint16_t tmp;
	
	i = 0;
	fu = 0;
	j = 0;
	
	for(i = 0; i < 3; i++)
		buff[i] = 0;
	
	if(num < 0)
	{
		tmp = -num;
		fu = 1;
	}
	else
		tmp = num;
	
	for(i = 0;i < 3;i++)
	{
		if(tmp > 0)
		{
			buff[i] = LEDCODE[tmp % 10];
			tmp = tmp/10;
			j++;
		}
		else
		{
			if((i > 0)&&(str == 0))	//正常数字前面不补0
				buff[i] = 0;		//LEDCODE[0]   0 显示 0 
			else
				buff[i] = LEDCODE[0];
		}
	}
	
	if(dot == 1)
	{
		buff[1] |= 0x80;		//加小数点
		if(buff[1] == 0x80)
		{
			buff[1] |= LEDCODE[0];
		}
	}
	else if(dot == 2)
	{
		buff[2] |= 0x80;		//加小数点
		if(buff[2] == 0x80)
		{
			buff[2] |= LEDCODE[0];
		}
	}
	
	if(fu)
	{
		if(j < 3)
			buff[j] = LEDCODE[18];		//最高位显示负号
		if(j == 1)
			buff[2] = 0;
	}
	if(str)								//如果为字符时，直接显示字符
		buff[2] = str;
	
	for(i = 0; i < 3; i++)
		Display[i] = buff[i];
}	

/*LED指示灯的显示*/
/********************************************************
Function	: Dsp_setled 
Description	: 设置Led灯
Input		: int16_t num  显示led的序号 1 2 3
              uint8_t flg       是否闪烁
              uint8_t mode     	模式 0 单打 1 双打
Return		: None
Others		: None
*********************************************************/
void Dsp_setled(uint8_t num,uint8_t flg,uint8_t mode)
{
	uint8_t i;
	
	for(i = 0;i < 6;i++)		/*现在一共6个灯 0-1-2-5  3-4*/
	{
		Dled[i] = LEDNULL;
	}
	
	if((num > 0) && (num < SHOW_CNT))
	{
		Dled[num - 1] = GREENDOT | flg;
	}
	if(num == SHOW_CNT)
		Dled[5] = GREENDOT | flg;
	
	if(num == SHOW_BACKSET)		/*后台设定模式  Terry 2019.7.9*/
	{
		Dled[0] = GREENDOT | 1;
		Dled[1] = GREENDOT | 1;
		Dled[2] = GREENDOT | 1;
		Dled[5] = GREENDOT | 1;
	}
		
	
	
	if(mode)
		Dled[4] = GREENDOT;
	else
		Dled[3] = GREENDOT;
}


/*专门用于测试的函数*/
void showtest(uint8_t sta)
{
	uint8_t i;
	
	switch(sta)
	{
		case 0:
		{
			for(i = 0; i < 24;i++)
			{
				if(i < 12)
					htbuff[i] = 0xff;
				else
					htbuff[i] = 0;
			}
		}
		break;
		case 1:
		{
			for(i = 0; i < 24;i++)
			{
				if(i < 12)
					htbuff[i] = 0;
				else
					htbuff[i] = 0xff;
			}
		}
		break;
		case 2:
		{
			for(i = 0; i < 24;i++)
			{
				htbuff[i] = 0xff;
			}
		}
		break;
		
		default:break;
	}
		
}
	
	
/******************************** end line **************************************/
