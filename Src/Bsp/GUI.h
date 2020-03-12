
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

/*�����������ֵ*/
#define MAXSET_HIGH			1000  /*����*/
#define PER_HIGH			10	  /*ÿ�ε���10����*/
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
	KEYSTATUS_TLH,			//������ϵģ����ܲ���Ҫ
	KEYSTATUS_TSC ,			//����ɲ���ģ�ͬ��
	KEYSTATUS_START,		//һ������
	KEYSTATUS_ZD, 
	KEYSTATUS_SUPLONG ,
    KEYSTATUS_SDWLONG ,
	KEYSTATUS_STARTLONG ,
	KEYSTATUS_ZDLONG ,
	KEYSTATUS_CHGLONG ,
	
	KEYSTATUS_HUPLONG,		/*������Ϣ 2019.6.6 Terry*/
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
    SHOW_TS,        /*����ʱ��*/
    SHOW_CNT,       /*��������*/
	SHOW_BACKSET,		/*��̨����ģʽ  Terry 2019.7.9*/
};

//��ʾ����
struct GUI_DATA
{
	int32_t g_sethighcm;		/*�趨�������߶�  ��λ���� */
	int32_t g_lihe;				/*�趨����ϵ�ĸ߶�  ��λ����  */
	int32_t g_nhigh;			/*�������ĳ���*/
	int32_t g_num;				/*ÿȦ����*/
	int32_t g_Zhoucm;				/*�ܳ�*/
	int32_t g_ts;				/*˫����ʱ�� ��λ��С0.1��  �ĳ�����ʱ��  2019.5.21*/
    int32_t g_hcnt;             /*��������  3 - 10��  Terry 2019.5.21*/
	int32_t g_index;			/*�������ʾ���  */
	int32_t g_show;				/*����ܴ���ʾ����ֵ*/
	int32_t g_HighOvercm;		/*�������ޱ���ֵ  Terry 2019.7.6*/
	int32_t g_Maxhigh;
	int8_t g_HasChanged;				/*�����Ƿ���±�־*/
	int8_t g_mode;				/*�����˫��ı�־   �ĳ��Զ����� �� ����ģʽ */
	int16_t g_pset;				/*��̨����ģʽ  Terry 2019.7.9*/
	
	int16_t s_high1;					/*�߶�1*/
	int16_t s_cnt1;						/*����1*/
	int16_t s_high2;					/*�߶�2*/
	int16_t s_cnt2;						/*����2*/
};



void KeyStatus(void);			/*������⣬ÿ10ms һ��*/
void Task_GUI_Function(void); 
void show_brush(void);
extern void GUI_Init(void);

void Dsp_PullLight(int16_t num);   // ����������ʾ
void Dsp_HighLight(int8_t num, int8_t set,int8_t pot,int8_t potblink,int8_t highblink); //�߶ȵ�����ʾ
void Dsp_BarLight(int16_t num,uint8_t flg);	/*��ʾ�ɳڶ���*/
void Dsp_Num(int16_t num, uint8_t dot,uint8_t str);		//��ʾ��ֵ������ܣ�
void Dsp_setled(uint8_t num,uint8_t flg,uint8_t mode);  //LEDָʾ��
void showtest(uint8_t sta);		//������
#endif



