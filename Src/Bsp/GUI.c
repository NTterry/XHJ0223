


#include "GUI.h"
#include "Action.h"
#include "EEPROM.h"
#include "Hangtu.h"
#include "u_log.h"

#define VERSION     19     					/*�汾�� C97  2019.9.17*/

extern struct SYSATTR sys_attr;				/* ϵͳ����  */
extern void W2Buff(void);
extern void HT1632_Init(void);
void CmdProc(void);

volatile uint8_t key_scan[MAX_KEY];  
volatile uint8_t key_scan_bak[MAX_KEY];
volatile uint8_t key_status[MAX_KEY];  // ��ǰ����״̬

volatile uint16_t keySup_longPress_delay_number;
volatile uint16_t keySdw_longPress_delay_number;
volatile uint16_t keyChg_longPress_delay_number;
volatile uint16_t keyStart_longPress_delay_number;
volatile uint16_t keyZd_longPress_delay_number;

/*Terry add 2019.6.6*/
volatile uint16_t keyhighup_longPress_delay_number;
volatile uint16_t keyhighdw_longPress_delay_number;
volatile uint16_t keyliheup_longPress_delay_number;
volatile uint16_t keylihedw_longPress_delay_number;
volatile uint16_t keySet_longPress_delay_number;

uint32_t g_errshow;
uint16_t g_errnum;		/*������*/

uint8_t g_stopflg = 0;
uint32_t g_testflg = 0;				//����ģʽ

int8_t liheblink,highblink;  	/*�����˸��־  �߶���˸��־*/
uint32_t savecnt = 0;
volatile struct GUI_DATA	g_showdata;
void GUI_showdata(void);
void savedatas(void);
int16_t get_errnum(uint32_t err);
void clear_err(uint32_t *perr);

void KeyStatus(void)
{   
	
	/*���ð���*/
    if(KEY_SET)
    {
        key_scan[KEY_SET_BUTTON] <<= 1;
        key_scan[KEY_SET_BUTTON] += 1;
        if((key_scan[KEY_SET_BUTTON] == 0xff)  && (key_scan_bak[KEY_SET_BUTTON] != 0xff))
        {
            key_status[KEY_SET_BUTTON] |= KEY_UNPRESSED;
			keySet_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_SET_BUTTON] <<= 1;
        if((key_scan[KEY_SET_BUTTON] == 0x00) && (key_scan_bak[KEY_SET_BUTTON] != 0x00))
        {
            key_status[KEY_SET_BUTTON] |= KEY_PRESSED;											//�����Ѱ���   4�볬ʱ��
			keySet_longPress_delay_number = 400;
        }
    }
    key_scan_bak[KEY_SET_BUTTON] = key_scan[KEY_SET_BUTTON];
	/*�߶ȼ�*/
    if(KEY_HUP)
    {
        key_scan[KEY_HUP_BUTTON] <<= 1;
        key_scan[KEY_HUP_BUTTON] += 1;
        if((key_scan[KEY_HUP_BUTTON] == 0xff)  && (key_scan_bak[KEY_HUP_BUTTON] != 0xff))
        {
            key_status[KEY_HUP_BUTTON] |= KEY_UNPRESSED;
			keyhighup_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_HUP_BUTTON] <<= 1;
        if((key_scan[KEY_HUP_BUTTON] == 0x00) && (key_scan_bak[KEY_HUP_BUTTON] != 0x00))
        {
            key_status[KEY_HUP_BUTTON] |= KEY_PRESSED;
			keyhighup_longPress_delay_number = 100;
        }
    }
	key_scan_bak[KEY_HUP_BUTTON] = key_scan[KEY_HUP_BUTTON];
	
	if(KEY_HDW)	/*��Ч*/
    {
        key_scan[KEY_HDW_BUTTON] <<= 1;
        key_scan[KEY_HDW_BUTTON] += 1;
        if((key_scan[KEY_HDW_BUTTON] == 0xff)  && (key_scan_bak[KEY_HDW_BUTTON] != 0xff))
        {
            key_status[KEY_HDW_BUTTON] |= KEY_UNPRESSED;
			keyhighdw_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_HDW_BUTTON] <<= 1;
        if((key_scan[KEY_HDW_BUTTON] == 0x00) && (key_scan_bak[KEY_HDW_BUTTON] != 0x00))
        {
            key_status[KEY_HDW_BUTTON] |= KEY_PRESSED;
			keyhighdw_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_HDW_BUTTON] = key_scan[KEY_HDW_BUTTON];
	
    if(KEY_LUP)
    {
        key_scan[KEY_LUP_BUTTON] <<= 1;
        key_scan[KEY_LUP_BUTTON] += 1;
        if((key_scan[KEY_LUP_BUTTON] == 0xff)  && (key_scan_bak[KEY_LUP_BUTTON] != 0xff))
        {
            key_status[KEY_LUP_BUTTON] |= KEY_UNPRESSED;
			keyliheup_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_LUP_BUTTON] <<= 1;
        if((key_scan[KEY_LUP_BUTTON] == 0x00) && (key_scan_bak[KEY_LUP_BUTTON] != 0x00))
        {
            key_status[KEY_LUP_BUTTON] |= KEY_PRESSED;
			keyliheup_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_LUP_BUTTON] = key_scan[KEY_LUP_BUTTON];
	
    if(KEY_LDW)
    {
        key_scan[KEY_LDW_BUTTON] <<= 1;
        key_scan[KEY_LDW_BUTTON] += 1;
        if((key_scan[KEY_LDW_BUTTON] == 0xff)  && (key_scan_bak[KEY_LDW_BUTTON] != 0xff))
        {
            key_status[KEY_LDW_BUTTON] |= KEY_UNPRESSED;
			keylihedw_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_LDW_BUTTON] <<= 1;
        if((key_scan[KEY_LDW_BUTTON] == 0x00) && (key_scan_bak[KEY_LDW_BUTTON] != 0x00))
        {
            key_status[KEY_LDW_BUTTON] |= KEY_PRESSED;
			keylihedw_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_LDW_BUTTON] = key_scan[KEY_LDW_BUTTON];
	

	if(KEY_SUP)
    {
        key_scan[KEY_SUP_BUTTON] <<= 1;
        key_scan[KEY_SUP_BUTTON] += 1;
        if((key_scan[KEY_SUP_BUTTON] == 0xff)  && (key_scan_bak[KEY_SUP_BUTTON] != 0xff))
        {
            key_status[KEY_SUP_BUTTON] |= KEY_UNPRESSED;
			keySup_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_SUP_BUTTON] <<= 1;
        if((key_scan[KEY_SUP_BUTTON] == 0x00) && (key_scan_bak[KEY_SUP_BUTTON] != 0x00))
        {
            key_status[KEY_SUP_BUTTON] |= KEY_PRESSED;
			keySup_longPress_delay_number = 100;											//������ס1�룬��������
        }
    }
    key_scan_bak[KEY_SUP_BUTTON] = key_scan[KEY_SUP_BUTTON];
	
	if(KEY_SDW)
    {
        key_scan[KEY_SDW_BUTTON] <<= 1;
        key_scan[KEY_SDW_BUTTON] += 1;
        if((key_scan[KEY_SDW_BUTTON] == 0xff)  && (key_scan_bak[KEY_SDW_BUTTON] != 0xff))
        {
            key_status[KEY_SDW_BUTTON] |= KEY_UNPRESSED;
			keySdw_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_SDW_BUTTON] <<= 1;
        if((key_scan[KEY_SDW_BUTTON] == 0x00) && (key_scan_bak[KEY_SDW_BUTTON] != 0x00))
        {
            key_status[KEY_SDW_BUTTON] |= KEY_PRESSED;
			keySdw_longPress_delay_number = 100;											//������ס1�룬���ټ�С
        }
    }
    key_scan_bak[KEY_SDW_BUTTON] = key_scan[KEY_SDW_BUTTON];
	
	
	
	
	if(KEY_CHG)
    {
        key_scan[KEY_CHG_BUTTON] <<= 1;
        key_scan[KEY_CHG_BUTTON] += 1;
        if((key_scan[KEY_CHG_BUTTON] == 0xff)  && (key_scan_bak[KEY_CHG_BUTTON] != 0xff))
        {
            key_status[KEY_CHG_BUTTON] |= KEY_UNPRESSED;
			keyChg_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_CHG_BUTTON] <<= 1;
        if((key_scan[KEY_CHG_BUTTON] == 0x00) && (key_scan_bak[KEY_CHG_BUTTON] != 0x00))
        {
            key_status[KEY_CHG_BUTTON] |= KEY_PRESSED;
			keyChg_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_CHG_BUTTON] = key_scan[KEY_CHG_BUTTON];
	
	/*�������ɲ��  ������Ч*/
	if(KEY_TLH)			//�ɿ�
    {
        key_scan[KEY_TLH_BUTTON] <<= 1;
        key_scan[KEY_TLH_BUTTON] += 1;
        if(key_scan[KEY_TLH_BUTTON] == 0xff)
        {
            key_status[KEY_TLH_BUTTON] = 0;
        }
    }
    else				//��ס
    {
        key_scan[KEY_TLH_BUTTON] <<= 1;
        if(key_scan[KEY_TLH_BUTTON] == 0x00)
        {
			if(key_status[KEY_TLH_BUTTON] < 50)
				key_status[KEY_TLH_BUTTON]++;
        }

    }
	
	
	if(KEY_TSC)		//�����ð�����ֱ�Ӷ���
    {
        key_scan[KEY_TSC_BUTTON] <<= 1;
        key_scan[KEY_TSC_BUTTON] += 1;
        if(key_scan[KEY_TSC_BUTTON] == 0xff)
        {
            key_status[KEY_TSC_BUTTON] = 0;
        }
    }
    else
    {
        key_scan[KEY_TSC_BUTTON] <<= 1;
        if(key_scan[KEY_TSC_BUTTON] == 0x00)
        {
           if(key_status[KEY_TSC_BUTTON] < 40)
		   {
			   key_status[KEY_TSC_BUTTON]++;
		   }
        }
    }
	
    
    if(!KEY_LIU)		//�����ð�����ֱ�Ӷ���
    {
        key_scan[KEY_LIU_OBUTTON] <<= 1;
        key_scan[KEY_LIU_OBUTTON] += 1;
        if(key_scan[KEY_LIU_OBUTTON] == 0xff)
        {
            key_status[KEY_LIU_OBUTTON] = 0;
        }
    }
    else
    {
        key_scan[KEY_LIU_OBUTTON] <<= 1;
        if(key_scan[KEY_LIU_OBUTTON] == 0x00)
        {
           if(key_status[KEY_LIU_OBUTTON] < 20)
		   {
			   key_status[KEY_LIU_OBUTTON]++;
		   }
        }
    }
/***************************һ����ͣ��ť******************************************/
	if(KEY_START)
    {
        key_scan[KEY_START_BUTTON] <<= 1;
        key_scan[KEY_START_BUTTON] += 1;
        if((key_scan[KEY_START_BUTTON] == 0xff)  && (key_scan_bak[KEY_START_BUTTON] != 0xff))
        {
            key_status[KEY_START_BUTTON] |= KEY_UNPRESSED;
			keyStart_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_START_BUTTON] <<= 1;
        if((key_scan[KEY_START_BUTTON] == 0x00) && (key_scan_bak[KEY_START_BUTTON] != 0x00))
        {
            key_status[KEY_START_BUTTON] |= KEY_PRESSED;
			keyStart_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_START_BUTTON] = key_scan[KEY_START_BUTTON];
	
	
	/*�ⲿ�㶯���Զ���ť �� ֹͣ��ť*/
	//�ⲿ���� ����סΪ1    �ⲿ          һ����ͣ��ť  2019.8.28
	if(KEY_ZD == GPIO_PIN_RESET)          /*�ⲿһ����ͣ��ť*/
    {
        key_scan[KEY_ZD_OBUTTON] <<= 1;
        key_scan[KEY_ZD_OBUTTON] += 1;
        if((key_scan[KEY_ZD_OBUTTON] == 0xff)  && (key_scan_bak[KEY_ZD_OBUTTON] != 0xff))
        {
            key_status[KEY_ZD_OBUTTON] |= KEY_UNPRESSED;
			keyZd_longPress_delay_number = 0;
        }
    }
    else
    {
        key_scan[KEY_ZD_OBUTTON] <<= 1;
        if((key_scan[KEY_ZD_OBUTTON] == 0x00) && (key_scan_bak[KEY_ZD_OBUTTON] != 0x00))
        {
            key_status[KEY_ZD_OBUTTON] |= KEY_PRESSED;
			keyZd_longPress_delay_number = 100;
        }
    }
    key_scan_bak[KEY_ZD_OBUTTON] = key_scan[KEY_ZD_OBUTTON];
	
	
	/*�ⲿ �㶯�ᴸ��ť*/
	if(!KEY_DD)
    {
        key_scan[KEY_DD_OBUTTON] <<= 1;
        key_scan[KEY_DD_OBUTTON] += 1;
        if(key_scan[KEY_DD_OBUTTON] == 0xff)
        {
            key_status[KEY_DD_OBUTTON] = 0;
        }
    }
    else
    {
        key_scan[KEY_DD_OBUTTON] <<= 1;
        if(key_scan[KEY_DD_OBUTTON] == 0x00)
        {
            if(key_status[KEY_DD_OBUTTON] < 6)
			{
				key_status[KEY_DD_OBUTTON]++;
			}
        }
    }
	/*�ⲿֹͣ��ť*/
	if(!KEY_STOP)
    {
        key_scan[KEY_STOP_OBUTTON] <<= 1;
        key_scan[KEY_STOP_OBUTTON] += 1;
        if(key_scan[KEY_STOP_OBUTTON] == 0xff)
        {
            key_status[KEY_STOP_OBUTTON] = 0;
            if(g_stopflg == 0)
            {
                g_stopflg = 1;
                G_SHACHE(ACT_OFF,0);
                G_SHACHE(ACT_ON,0);
            }
			g_halt = 1;								//����ֹͣ
            Debug("sp 1\r\n");
        }
    }
    else
    {
        key_scan[KEY_STOP_OBUTTON] <<= 1;
        if(key_scan[KEY_STOP_OBUTTON] == 0x00)
        {
            if(key_status[KEY_STOP_OBUTTON] < 200)
			{
				key_status[KEY_STOP_OBUTTON]++;
			}
			
			if(key_status[KEY_STOP_OBUTTON] > 20)
				g_stopflg = 0;
        }
    }

	
	/*************************�������źŴ���  **********************************/	
	/*���ø߶� ���� ��*/
	if(keyhighup_longPress_delay_number)
	{
		if(!--keyhighup_longPress_delay_number)
        {
            key_status[KEY_HUP_BUTTON] |= KEY_LONG_PRESSED;  /*��ʾ������Ч*/
        }
	}
	/*���ø߶ȳ�����*/
	if(keyhighdw_longPress_delay_number)
	{
		if(!--keyhighdw_longPress_delay_number)
        {
            key_status[KEY_HDW_BUTTON] |= KEY_LONG_PRESSED;  /*��ʾ������Ч*/
        }
	}
	
	if(keyliheup_longPress_delay_number)
	{
		if(!--keyliheup_longPress_delay_number)
        {
            key_status[KEY_LUP_BUTTON] |= KEY_LONG_PRESSED;  /*��ʾ������Ч*/
        }
	}
	
	if(keylihedw_longPress_delay_number)
	{
		if(!--keylihedw_longPress_delay_number)
        {
            key_status[KEY_LDW_BUTTON] |= KEY_LONG_PRESSED;  /*��ʾ������Ч*/
        }
	}
	
	
    if(keySup_longPress_delay_number)
    {
        if(!--keySup_longPress_delay_number)
        {
            key_status[KEY_SUP_BUTTON] |= KEY_LONG_PRESSED;
        }
    }
	
	if(keySdw_longPress_delay_number)
    {
        if(!--keySdw_longPress_delay_number)
        {
            key_status[KEY_SDW_BUTTON] |= KEY_LONG_PRESSED;
        }
    }
	
	if(keyChg_longPress_delay_number)
    {
        if(!--keyChg_longPress_delay_number)
        {
            key_status[KEY_CHG_BUTTON] |= KEY_LONG_PRESSED;
        }
    }
	
	if(keyStart_longPress_delay_number)
    {
        if(!--keyStart_longPress_delay_number)
        {
            key_status[KEY_START_BUTTON] |= KEY_LONG_PRESSED;
        }
    }
	
	if(keyZd_longPress_delay_number)
    {
        if(!--keyZd_longPress_delay_number)
        {
            key_status[KEY_ZD_OBUTTON] |= KEY_LONG_PRESSED;
        }
    }
	/*Terty 2019.7.9*/
	if(keySet_longPress_delay_number)
	{
		if(!--keySet_longPress_delay_number)
        {
			if(!KEY_CHG)
				key_status[KEY_SET_BUTTON] |= KEY_LONG_PRESSED;
        }
	}
}




/****************************************************************************************
** ��������: KeyDiff
** ��������: �Ը��ְ�����Ͻ����ж�
** ��    ��: void
** �� �� ֵ: ������       
** ����  ��: 
** ��  ����: 2017��3��15��
**---------------------------------------------------------------------------------------
** �� �� ��: 
** �ա�  ��: 
**--------------------------------------------------------------------------------------
****************************************************************************************/
uint8_t KeyDiff(void)
{
    uint8_t Key = KEYSTATUS_NONE; 
    static uint8_t tstflg = 0;

	/*�̰���*/
    if(key_status[KEY_HUP_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_HUP_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_HUP;
    }
    if(key_status[KEY_HDW_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_HDW_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_HDW;
    }
    if(key_status[KEY_LUP_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_LUP_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_LUP;
    }
    if(key_status[KEY_LDW_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_LDW_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_LDW;
    }
	
	if(key_status[KEY_SUP_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_SUP_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_SUP;
    }
    if(key_status[KEY_SDW_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_SDW_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_SDW;
    }
	
	if(key_status[KEY_SET_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_SET_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_SET;
    }
	
	if(key_status[KEY_CHG_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_CHG_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_CHG;
    }
	/*******************������������ ����1���ʼִ��**************************/
	
	if(key_status[KEY_TLH_BUTTON]> 30)                                          /*���*/
    {
		if(sys_attr.s_zidong != MOD_TT2)
		{
			if(sys_attr.s_zidong == MOD_FREE)
				sys_attr.s_zidong = MOD_TST;
			Key = KEYSTATUS_TLH;
			G_LIHE(ACT_ON,0);
			Debug("lihe");
		}
    }
	else if(key_status[KEY_TSC_BUTTON] > 30)                                    /*ɲ��*/
    {
		if(sys_attr.s_zidong != MOD_TT2)
		{
			if(sys_attr.s_zidong == MOD_FREE)
				sys_attr.s_zidong = MOD_TST;
			Key = KEYSTATUS_TSC;
			G_SHACHE(ACT_OFF,0);                        /*2019.4.11*/
            tstflg = 1;
		}
    }
    else if(key_status[KEY_LIU_OBUTTON] > 15)
    {
        if(sys_attr.s_zidong != MOD_TT2)
		{
            if(sys_attr.s_zidong == MOD_FREE)
				sys_attr.s_zidong = MOD_TST;
            
			G_SHACHE(ACT_LIU,0);                     /*ʹ����Ź���*/
            tstflg = 1;
		}
    }
	else                                                                         /*�ָ�*/
	{
        if(sys_attr.s_zidong == MOD_TST)                                        /*�������   2018.12.24*/
        {
            if(tstflg)
            {
                G_SHACHE(ACT_ON,0);
                tstflg = 0;
            }
            else
            {
                G_LIHE(ACT_OFF,0);
            }
            sys_attr.s_zidong = MOD_FREE;
        }
	}
    
	
	//	/*�Զ�*/
	if(key_status[KEY_ZD_OBUTTON] & KEY_PRESSED)
    {
        key_status[KEY_ZD_OBUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_ZD;
    }
	
	/*�㶯*/
	if(key_status[KEY_DD_OBUTTON] > 5)		//�㶯�ᴸ   2018.9.7
    {
		if((sys_attr.s_zidong == MOD_FREE) || (sys_attr.s_zidong == MOD_DOWN) || (sys_attr.s_zidong == MOD_MANOFF))
			sys_attr.s_zidong = MOD_MAN;
    }
	else   								//ɲס   2018.9.7
	{	
		if((sys_attr.s_zidong == MOD_MAN) || (sys_attr.s_zidong == MOD_DOWN))    //������ֹ
			sys_attr.s_zidong = MOD_MANOFF;	
	}
	


	/*********************************************/
	if(key_status[KEY_START_BUTTON] & KEY_PRESSED)
    {
        key_status[KEY_START_BUTTON] &= ~KEY_PRESSED;
        Key = KEYSTATUS_START;
    }
	
	/*������*/
	/*���Ӹ߶Ⱥ���ϵĳ�������*/
	
	if(key_status[KEY_HUP_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_HUP_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_HUPLONG;
    }
	
	if(key_status[KEY_HDW_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_HDW_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_HDWLONG;
    }
	
	if(key_status[KEY_LUP_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_LUP_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_LHUPLONG;
    }
	
	if(key_status[KEY_LDW_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_LDW_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_LHDWLONG;
    }
	/*2019.6.6 Terry*/
	
	
    if(key_status[KEY_SUP_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_SUP_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_SUPLONG;
    }
	
	if(key_status[KEY_SDW_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_SDW_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_SDWLONG;
    }
	if(key_status[KEY_CHG_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_CHG_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_CHGLONG;
    }
	if(key_status[KEY_START_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_START_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_STARTLONG;
    }
	if(key_status[KEY_ZD_OBUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_ZD_OBUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_ZDLONG;
    }
	
	if(key_status[KEY_SET_BUTTON] & KEY_LONG_PRESSED)
    {
        key_status[KEY_SET_BUTTON] &= ~KEY_LONG_PRESSED;
        Key = KEYSTATUS_SETLONG;
    }
    return Key;
}

/*��尴������*/
void keyprocess(void)
{
	uint8_t Key = KEYSTATUS_NONE; 
	static uint32_t updatetime = 0;
    Key = KeyDiff();
	
	switch(Key)
	{
		case KEYSTATUS_HUP:
			if(g_showdata.g_sethighcm < MAXSET_HIGH)   /*���12��  Terry  2019.5.21*/
			{
				g_showdata.g_sethighcm += PER_HIGH;
				g_showdata.g_HasChanged = 1;
				highblink = HIGHSHOW;
				Debug("Hup %d\r\n",g_showdata.g_sethighcm);
			}
			if(g_showdata.g_sethighcm > MAXSET_HIGH)
				g_showdata.g_sethighcm = MAXSET_HIGH;
			break;
		case KEYSTATUS_HUPLONG:
			if(g_showdata.g_sethighcm < MAXSET_HIGH)   /*���12��  Terry  2019.5.21*/
			{
				g_showdata.g_sethighcm += PER_HIGH * 10;
				g_showdata.g_HasChanged = 1;
				highblink = HIGHSHOW * 2;
				Debug("Hup %d\r\n",g_showdata.g_sethighcm);
			}
			if(g_showdata.g_sethighcm > MAXSET_HIGH)
				g_showdata.g_sethighcm = MAXSET_HIGH;
			break;
		case KEYSTATUS_HDW:
			if(g_showdata.g_sethighcm > MINSET_HIGH)   /* ��С1��  Terry  2019.5,21*/
			{
				
				g_showdata.g_sethighcm -= PER_HIGH;		//�߶ȼ�С10cm
				g_showdata.g_HasChanged = 1;
				
				if(g_showdata.g_sethighcm < (g_showdata.g_lihe + 5))			//��ϱ���С���趨�߶�10cm
					g_showdata.g_lihe = g_showdata.g_sethighcm - 5;
				
				highblink = HIGHSHOW;
				Debug("Hdw %d\r\n",g_showdata.g_sethighcm);
			}
			if(g_showdata.g_sethighcm < MINSET_HIGH)
				g_showdata.g_sethighcm = MINSET_HIGH;
			break;
		case KEYSTATUS_HDWLONG:
			if(g_showdata.g_sethighcm > MINSET_HIGH)   /* ��С1��  Terry  2019.5,21*/
			{
				
				g_showdata.g_sethighcm -= PER_HIGH * 10;		//�߶ȼ�С10cm
				g_showdata.g_HasChanged = 1;
				
				if(g_showdata.g_sethighcm < (g_showdata.g_lihe + 5))			//��ϱ���С���趨�߶�10cm
					g_showdata.g_lihe = g_showdata.g_sethighcm - 5;
				
				highblink = HIGHSHOW * 2;
				Debug("Hdw %d\r\n",g_showdata.g_sethighcm);
			}
			if(g_showdata.g_sethighcm < MINSET_HIGH)
				g_showdata.g_sethighcm = MINSET_HIGH;
			break;
		/*�������˵�� ���ø�ֵ���50���ϸ��ո�ֵ���ƣ�����¼��ʱ���ɳڶȣ�50���Ժ�ʼ΢���Զ�����PI����*/
		case KEYSTATUS_LUP:
			if(g_showdata.g_lihe < (g_showdata.g_sethighcm - 6))
			{
				if(g_showdata.g_lihe < -10)
					g_showdata.g_lihe +=4;   //Ϊ�����ǣ�������һЩʱ��
				else
					g_showdata.g_lihe +=2; //ÿ��ֻ�ܼ�2����   2017.11.10
				
				g_showdata.g_HasChanged = 1;
				liheblink = LIHESHOW;			//��˸һ��
				highblink = 0;
				Debug("Lup %d\r\n",g_showdata.g_lihe);
			}
			break;
		case KEYSTATUS_LHUPLONG:
			if(g_showdata.g_lihe < (g_showdata.g_sethighcm - 6))
			{
				if(g_showdata.g_lihe < -10)
					g_showdata.g_lihe +=20;   //Ϊ�����ǣ�������һЩʱ��
				else
					g_showdata.g_lihe +=20; //ÿ��ֻ�ܼ�2����   2017.11.10
				
				g_showdata.g_HasChanged = 1;
				liheblink = LIHESHOW * 2;			//��˸һ��
				highblink = 0;
				Debug("Lup %d\r\n",g_showdata.g_lihe);
			}
			break;
		case KEYSTATUS_LDW:
			if(g_showdata.g_lihe > -200)		
			{
				if(g_showdata.g_lihe < -10)
					g_showdata.g_lihe -=6;
				else
					g_showdata.g_lihe -=3;				
				
				g_showdata.g_HasChanged = 1;						/* ���ݱ����־ */
				liheblink = LIHESHOW;
				highblink = 0;			
				Debug("Ldw %d\r\n",g_showdata.g_lihe);
			}
			break;
		case KEYSTATUS_LHDWLONG:
			if(g_showdata.g_lihe > -200)		
			{
				if(g_showdata.g_lihe < -10)
					g_showdata.g_lihe -=60;
				else
					g_showdata.g_lihe -=30;				
				
				g_showdata.g_HasChanged = 1;						/* ���ݱ����־ */
				liheblink = LIHESHOW * 2;
				highblink = 0;			
				Debug("Ldw %d\r\n",g_showdata.g_lihe);
			}
			break;
		
		/*��������*/
		case KEYSTATUS_SUP:						//��ֵ����
		{
			if(g_errshow)
				clear_err(&g_errshow);
			else
			{
				switch(g_showdata.g_index)
				{
					case SHOW_NONE:break;
					case SHOW_TTC:
                            if(g_showdata.g_num < 300)								// 2017.12.18
                            {
                                g_showdata.g_num++;	
                                g_showdata.g_HasChanged = 1;
                                Debug("unum %d\r\n",g_showdata.g_num);
                            }
                            break;
					case SHOW_ZHOU:
								g_showdata.g_Zhoucm++;	
								g_showdata.g_HasChanged = 1;
                                Debug("uzhou %d\r\n",g_showdata.g_Zhoucm);break;
					case SHOW_TS:
                            if(g_showdata.g_ts < 20)
                            {
                                g_showdata.g_ts++;	
                                g_showdata.g_HasChanged = 1;
                                Debug("uts %d\r\n",g_showdata.g_ts);
                            }break;
                    case SHOW_CNT:                             /*Terry add 2019.*/
                            if(g_showdata.g_hcnt < 10)
                            {
                                g_showdata.g_hcnt++;
                                g_showdata.g_HasChanged = 1;
                            }
                            break;
					case SHOW_BACKSET:
							if(g_showdata.g_pset < 99 )
							{
								g_showdata.g_pset++;
							}
					break;
				}
			}
		}
			updatetime = osKernelSysTick();						//����ʱ�� 2017.12.18
			Debug("sup \r\n");
			break;
		case KEYSTATUS_SDW:						//��ֵ����
		{
			if(g_errshow)
				clear_err(&g_errshow);
			else
			{
				switch(g_showdata.g_index)
				{
					case SHOW_NONE:break;
					case SHOW_TTC:
						if(g_showdata.g_num > 15)
						{
							g_showdata.g_num--;
							g_showdata.g_HasChanged = 1;
							Debug("dnum %d\r\n",g_showdata.g_num);
						}
						break;;
					case SHOW_ZHOU:
						if(g_showdata.g_Zhoucm > 50)
						{
							g_showdata.g_Zhoucm--;
							g_showdata.g_HasChanged = 1;
							Debug("dzhou %d\r\n",g_showdata.g_Zhoucm);
						}
							break;
					case SHOW_TS:
						if(g_showdata.g_ts > 1)
						{
							g_showdata.g_ts--;
							g_showdata.g_HasChanged = 1;
							Debug("dts %d\r\n",g_showdata.g_ts);
						}
						break;
                    case SHOW_CNT:                      /*Terry add*/
                        if(g_showdata.g_hcnt > 3)
                        {
                            g_showdata.g_hcnt--;
                            g_showdata.g_HasChanged = 1;
                        }
                        break;
					case SHOW_BACKSET:
						if(g_showdata.g_pset > 0 )
						{
							g_showdata.g_pset--;				/*Terry 2019.7.9  */
						}
					break;
				}
			}
			
			updatetime = osKernelSysTick();						//����ʱ�� 2017.12.18
			Debug("sdw \r\n");
			break;
		}
		case KEYSTATUS_SET:			//�л���ʾ
			updatetime = osKernelSysTick();						//����ʱ�� 2017.12.18
			if(g_errshow)
				clear_err(&g_errshow);
			else if(g_showdata.g_index == SHOW_BACKSET)
			{
				/*��̨��������*/
				CmdProc();
				g_showdata.g_index = SHOW_NONE;
			}
			else
			{
				g_showdata.g_index++;
				if(g_showdata.g_index > SHOW_CNT)
					g_showdata.g_index = SHOW_NONE;
				Debug("index %d\r\n",g_showdata.g_index);
			}
			
			Debug("set \r\n");
			break;
			
		case KEYSTATUS_SETLONG:
			g_showdata.g_index = SHOW_BACKSET;			/*��̨����ģʽ*/
			g_showdata.g_show = 0;
			break;
		case KEYSTATUS_CHG:
			
			Debug("chg \r\n");
			break;
		case KEYSTATUS_CHGLONG:
			if(sys_attr.s_zidong == MOD_FREE)
			{
				if(g_showdata.g_mode)
				{
					g_showdata.g_mode = 0;
					Debug("����");
					g_showdata.g_HasChanged = 1;
				}
				else
				{
					g_showdata.g_mode = 1;
					Debug("����ģʽ");
					g_showdata.g_HasChanged = 1;
				}
			}
			break;
		
		case KEYSTATUS_SUPLONG:
			switch(g_showdata.g_index)
			{
				case SHOW_NONE:break;
				case SHOW_TTC:
				
					g_showdata.g_num += 10;
					g_showdata.g_HasChanged = 1;
					break;;
				case SHOW_ZHOU:
					g_showdata.g_Zhoucm += 10;
					g_showdata.g_HasChanged = 1;
					break;
				case SHOW_TS:
                    g_showdata.g_ts += 10;
                    break;
                case SHOW_CNT:
                    break;
			}
			break;
		case KEYSTATUS_SDWLONG:
			switch(g_showdata.g_index)
			{
				case SHOW_NONE:break;
				case SHOW_TTC:
					if(g_showdata.g_num >= 20)
					{
						g_showdata.g_num -= 10;
						g_showdata.g_HasChanged = 1;
					}
					if(g_showdata.g_num < 10)
						g_showdata.g_num = 10;
					break;;
				case SHOW_ZHOU:
					if(g_showdata.g_Zhoucm >= 60)
					{
						g_showdata.g_Zhoucm -= 10;
						g_showdata.g_HasChanged = 1;
					}
						break;
				case SHOW_TS:
                    if(g_showdata.g_ts > 15)
                        g_showdata.g_ts = 15;
                    break;
                case SHOW_CNT:
                    break;
			}
			break;
		
		
		case KEYSTATUS_START:							//��һ�¼�ֹͣ
		case KEYSTATUS_ZD:
			if(sys_attr.s_zidong != MOD_FREE)
			{
				sys_attr.s_zidong = MOD_FREE;
				G_SHACHE(ACT_OFF,0);   
				G_SHACHE(ACT_ON,0);                 //����ִ��ɲ������ֹ�����ź�û�и���  2019.1.6
				g_halt = 1;
                Debug("sp 2\r\n");
			}
            else
            {
                 G_SHACHE(ACT_OFF,0);   
                 G_SHACHE(ACT_ON,0);
            }
			
				Debug("start \r\n");
			break;
		case KEYSTATUS_STARTLONG:
		case KEYSTATUS_ZDLONG:			/*�Զ��� �ڲ�һ����ͣЧ��һ��*/
			if(sys_attr.s_zidong == MOD_FREE)
			{
				if(g_halt)  g_halt = 0;
				
				if(sys_attr.s_mode == 0)
                    sys_attr.s_zidong = MOD_TT2;	//��̽ͷһ������
                else
                    sys_attr.s_zidong = MOD_ZTT2;	//�Զ���������

				Debug("zidong = %d\r\n",sys_attr.s_zidong);
			}											
			break;
			
		default:
			break;

	}
	/*�Զ��˳�ģʽ*/
	extern uint32_t getmilsec(uint32_t pretime);
	if(g_showdata.g_index != 0)
	{
		if(getmilsec(updatetime) > 20000)				//  10���޲��� �˳�
			g_showdata.g_index = 0;
	}
	else
	{
		updatetime = osKernelSysTick();		
	}
}





extern int8_t Get_Fbsignal(uint8_t MASK);
extern uint16_t g_speed;
extern struct Locationcm user_encoder;   

void Task_GUI_Function(void) 
{
	keyprocess();
	switch(g_showdata.g_index)
	{
		case SHOW_NONE:
			if(highblink)
			{
				g_showdata.g_show = g_showdata.g_sethighcm/10;
			}
			else if(liheblink)				// ��ʾ��ϵ�߶�
			{							
				//��⵽̽ͷʱ��ʾ��ϵ�߶�,��λ0.1��
				g_showdata.g_show = g_showdata.g_lihe/10;
			}
			else													//��ʾ�����߶�
			{
				if(sys_attr.s_zidong == MOD_TT2)
				{
					g_showdata.g_show =GetEncoderLen1Cm() / 10;  //�߶���ʾΪ����  num2cm(sys_stadata.m_high.TotalCount)
					g_showdata.g_nhigh = g_showdata.g_show;							/*����*/
				}
				else
				{
					g_showdata.g_show = GetEncoderLen2Cm() / 10;				 //��λ 0.1��
					g_showdata.g_nhigh = g_showdata.g_show;							/*����*/
					g_showdata.g_show = g_showdata.g_show / 10;		/*����ʱ ������ֻ����ʾ�� Terry 2019.7.5*/
				}
			}
			break;
		case SHOW_TTC:g_showdata.g_show = g_showdata.g_num;break;
		case SHOW_ZHOU:g_showdata.g_show = g_showdata.g_Zhoucm;break;
		case SHOW_TS:g_showdata.g_show = g_showdata.g_ts;break;
        case SHOW_CNT:g_showdata.g_show = g_showdata.g_hcnt;break;     /*�򴸵Ĵ���*/
		case SHOW_BACKSET:g_showdata.g_show = g_showdata.g_pset;break;
		default:break;
	}
	
	
	if(g_showdata.g_HasChanged == 0)
	{
		ModbusData_Chk();
	}
	else if(g_showdata.g_HasChanged == 1)
	{
		ModbusData_flash();
	}
	ModbusData_Show();
	/*��ʼд�������*/
	if(g_showdata.g_HasChanged)
	{
		g_showdata.g_HasChanged = 0;
		/*��������*/
//		portENTER_CRITICAL();
		if(sys_attr.s_hlihe != g_showdata.g_lihe)
		{
			sys_attr.s_hlihe = g_showdata.g_lihe;

			Debug("update lihe %d\r\n",sys_attr.s_hlihe);

			liheupdate();		//������ϲ���ֵ
			savecnt = 20;
		}
		
		if(g_showdata.g_sethighcm != sys_attr.s_sethighcm)		//���¸߶�
		{
			sys_attr.s_sethighcm = g_showdata.g_sethighcm;
			liheupdate();		//������ϲ���ֵ
			savecnt = 20;
		}
		/*ÿȦ����*/
		if(g_showdata.g_num != sys_attr.s_numchi)
		{
			sys_attr.s_numchi = g_showdata.g_num;
			liheupdate();		//������ϲ���ֵ
			savecnt = 20;
		}
		/*��˫��ģʽ*/
		if(g_showdata.g_mode != sys_attr.s_mode)
		{
			sys_attr.s_mode = g_showdata.g_mode;
			savecnt = 20;
		}
		/*˫����ʱ��*/
		if(g_showdata.g_ts != sys_attr.s_intval)
		{
			sys_attr.s_intval = g_showdata.g_ts;
			savecnt = 20;
		}
        /*��������    Terry 2019.5.21*/
        if(g_showdata.g_hcnt != sys_attr.s_cnt)
		{
			sys_attr.s_cnt = g_showdata.g_hcnt;
			savecnt = 20;
		}
        
		/*�ܳ�  ���� */
		if(g_showdata.g_Zhoucm != sys_attr.s_zhou)
		{
			sys_attr.s_zhou = g_showdata.g_Zhoucm;
			savecnt = 20;
		}
		/*Terry add 2019.7.6 ���õ��������޵ĸ߶�*/
		if(g_showdata.g_HighOvercm != sys_attr.s_hprot)
		{
			sys_attr.s_hprot = g_showdata.g_HighOvercm;
			savecnt = 20;
		}
		/*��������*/
		if(g_showdata.g_index == SHOW_BACKSET)
			savecnt = 10;
			
//		portEXIT_CRITICAL();
	}
	if(g_testflg == 0)
	{
		GUI_showdata();
		W2Buff();
	}
	else
	{
		g_testflg--;
		if(g_testflg > 60)
			showtest(0);
		else if(g_testflg > 40)
			showtest(1);
		else if(g_testflg > 20)
			showtest(2);
		else
		{
			if(Get_Fbsignal(FB_LIHE))   
				Debug("����� ");
			else
				Debug("����� ");
			
			if(sys_fbsta & FB_SHACHE)
				Debug("��ɲ��\r\n");
			else
				Debug("��ɲ��\r\n");
			g_testflg = 0;
			Debug("Test end  %x\r\n",sys_fbsta);
		}
	}
	
	show_brush();
	savedatas();
	/*�޴����Ҽ�ͣδ����ʱ����ͣOK���Ͽ�*/
	if(g_errshow || g_stopflg)
		C_STOP();
	else
		C_OK();
}

extern volatile uint8_t sys_fbsta;	
extern void ModbusData_flash(void);
void savedatas(void)
{
	HAL_StatusTypeDef status;
	
	status = HAL_OK;
	if(savecnt > 0)
		savecnt--;
	if(savecnt == 1)
	{
		if(sys_attr.s_dir > 1)
			sys_attr.s_dir = 1;
		if((sys_fbsta & FB_24VOK))
		{
			status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&sys_attr,sizeof(sys_attr));
			if((status == HAL_OK) && (g_testflg))
				Debug("Save ok\r\n");
		}
		savecnt = 72000;		// 2H�����Զ���������
		osDelay(2);
	}
}

/**
  * �������� GUI������ֵ�ĳ�ʼ��
  * ��������� ��
  * ����ֵ: ��
  * ˵��: ��ʼ������ʾ�Ĳ�����������EEPROM�и���
  */
void GUI_Init(void)
{
	/*������ʾ*/
	g_showdata.g_index = 0;
	g_showdata.g_HasChanged = 1;
	g_showdata.g_sethighcm = sys_attr.s_sethighcm;
	g_showdata.g_lihe = sys_attr.s_hlihe;
	g_showdata.g_num = sys_attr.s_numchi;
	g_showdata.g_ts = sys_attr.s_intval;
	g_showdata.g_Zhoucm = sys_attr.s_zhou;
	g_showdata.g_mode = sys_attr.s_mode;
    g_showdata.g_hcnt = sys_attr.s_cnt;
	g_showdata.g_HighOvercm = sys_attr.s_hprot;			/*Terry 2019.7.6*/
	
	liheupdate();										//������ϲ���ֵ
	HT1632_Init();
	g_testflg = 0;										//���Ա�־
	liheblink = 0;
	highblink = 0;
}

extern const uint8_t LEDCODE[21];
void GUI_showdata(void)
{
	int LPosCm;
	int16_t tmp,tmp1,high;
	
	tmp = (sys_stadata.m_power.Speed - sys_attr.s_pnull) * 10 / (sys_attr.s_pfull - sys_attr.s_pnull);
	if(tmp < 0)
		tmp = 0;

	Dsp_PullLight(tmp);
	
	tmp = g_showdata.g_sethighcm / 50;     						/*0 - 24��Ӧ 0 - 12��  Terry  2019.5.21*/
		
	tmp1 = g_showdata.g_lihe/50;								/*ֱ����ʾ��ϵ�  2019.6.5*/

	LPosCm = GetEncoderLen1Cm();
    if(LPosCm > 0)
        high = LPosCm / 50;      /*0-24��Ӧ  0 - 12��  Terry 2019.5.21*/
    else
        high = 0;

	
	Dsp_HighLight(high,tmp,tmp1,liheblink,highblink);
    
	if(liheblink > 0)
		liheblink--;
		
	if(highblink > 0)
		highblink--;

	
	if(g_errshow == ERR_NONE)
	{
		/*������ʾ��*/
        if(liheblink|highblink)
		{
			Dsp_Num(g_showdata.g_show,1,0);
		}
        else if(g_showdata.g_index == SHOW_NONE)
		{
            if(sys_attr.s_zidong == MOD_TT2)
				Dsp_Num(g_showdata.g_show,1,0);
			else
				Dsp_Num(g_showdata.g_show,0,0);			/*����ģ#ʽ��������ʾ ��*/
		}
		else
		{
			Dsp_Num(g_showdata.g_show,0,0);
		}
			
		BUZZER(0);
		g_errnum = 0;
	}
	else
	{
		tmp = get_errnum(g_errshow);		//��ʾ���ϴ���
		g_errnum = tmp;
		if(tmp < 10)
		{
			Dsp_Num(tmp,0,LEDCODE[19]);  // P��E
			BUZZER(1);
		}
		else
		{
			if(tmp != 16)
			{
				Dsp_Num(tmp,0,LEDCODE[20]);
				BUZZER(1);							//��ͣ
			}
			else
			{
				Dsp_Num(VERSION,0,LEDCODE[12]);			//�汾��    2019.1.2
				
			}
		}
	}
	
	
	/*ѡ��ָʾ�ƺ�ģʽָʾ��*/
	Dsp_setled(g_showdata.g_index,0,g_showdata.g_mode);
}


/*
GUI��ʾ�����͸߶ȣ���ϵ㣩
��̽ͷģʽʱ���ø߶ȷ��Ƴ�����ʱ�䣬��ϵ㷴������ϵ�ʱ��
*/


/*���ش���ֵ*/
int16_t get_errnum(uint32_t err)
{
	int16_t ret;
	
	if(err & ERR_KC) ret = 1;
	else if(err & ERR_NC) ret = 2;
	else if(err & ERR_LS) ret = 3;
	else if(err & ERR_CS) ret = 4;
	else if(err & ERR_TOP) ret = 5;
	else if(err & ERR_LIU) ret = 6;
	else if(err & ERR_DOWN) ret = 7;
	else if(err & ERR_CHAO) ret = 8;
	
	else if(err & ERR_PW) ret = 11;
	else if(err & ERR_TT) ret = 12;
	else if(err & ERR_LH) ret = 13;
	else if(err & ERR_SC) ret = 14;
	else if(err & ERR_CT) ret = 15;
	else if(err & ERR_HALT) ret = 16;
	else if(err & ERR_ACE)  ret = 18; /*Terry add 2019.7.6*/
	
	return ret;
}

/*���ִ�����ʾʱ��ÿ��һ�ΰ��������һ�δ���*/
void clear_err(uint32_t *perr)
{
	if(*perr & ERR_KC) *perr &=  ~ERR_KC;
	else if(*perr & ERR_NC) *perr &=  ~ERR_NC;
	else if(*perr & ERR_LS) *perr &=  ~ERR_LS;
	else if(*perr & ERR_CS) *perr &=  ~ERR_CS;
	/*��Ŵ�����*/
	else if(*perr & ERR_TOP) *perr &=  ~ERR_TOP;
	else if(*perr & ERR_LIU) *perr &=  ~ERR_LIU;
	else if(*perr & ERR_DOWN) *perr &=  ~ERR_DOWN;
	else if(*perr & ERR_CHAO) *perr &=  ~ERR_CHAO;
	
	else if(*perr & ERR_PW) *perr &=  ~ERR_PW;
	else if(*perr & ERR_TT) *perr &=  ~ERR_TT;
	else if(*perr & ERR_LH) *perr &=  ~ERR_LH;
	else if(*perr & ERR_SC) *perr &=  ~ERR_SC;
	else if(*perr & ERR_CT) *perr &=  ~ERR_CT;
	else if(*perr & ERR_HALT) *perr &=  ~ERR_HALT;
	else if(*perr & ERR_ACE) *perr &=  ~ERR_ACE;

}



void clear_all(void)
{
	g_errshow = 0;
}

void CmdProc(void)
{
	switch(g_showdata.g_show)
	{
		case 1:						/*����-��������*/
		case 2:
		case 3:
		case 4:
		case 6:	
			g_showdata.g_HighOvercm = g_showdata.g_show * 100;
			g_showdata.g_HasChanged = 1;
			break;
		case 7:						/*��λ*/
			ModbusData_Init();
			GUI_Init();
			break;
		case 8:						/*��У��*/
			sys_attr.s_pset = 1;
			g_showdata.g_HasChanged = 1;
			savecnt = 10;
			break;
		case 9:						/*�ر�У��*/
			sys_attr.s_pset = 0;
			g_showdata.g_HasChanged = 1;
			savecnt = 10;
			break;
	}
}
