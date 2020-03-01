

#include "Action.h"
#include "math.h"
#include "EEPROM.h"
#include "Encoder.h"
#include "PutOffAct.h"
#include "u_log.h"
struct STADATA sys_stadata;						// ϵͳ����

struct SYSATTR g_sys_para;

struct LIHEDATA	    TTlehe;							//ʹ��̽ͷ����ϼ���ģʽ,�޸���ϵ�󣬱����������
struct LIHEDATA	    TMlehe;		    				// TM��϶����������������

struct PRPTECT_HANGTU Prtop;						//��ϵ��������ݼ�¼  Terry 2019.7.24

volatile uint32_t g_halt = 0;

volatile uint8_t sys_fbsta;						// �ⲿ�����ź�

//extern uint32_t savecnt;						//��������
int8_t Get_Fbsignal(uint8_t MASK);
SYS_STA Get_Action_Sta(void);
extern int8_t Get_Fbsignal(uint8_t MASK);
extern struct RECORD s_record;

uint32_t getmilsec(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = osKernelSysTick();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*����һ��Ŀ��*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}


/*��ñ�����������ȷ����׼����*/
/**
  * �������� ��ñ�����������ȷ����׼����
  * ���������dir:��׼����
  *           *p:Ӳ����������������
  * ����ֵ: None
  * ˵��: ��
  */
  
/*  ����ת����  */
int32_t cm2num(int32_t cm)
{
	int32_t tmp;
	
	tmp = cm * g_sys_para.s_numchi / g_sys_para.s_zhou;
	
	return tmp;
}

/*
����ʱ
�ж�����
��ɲ�� �����

�����  ��ɲ��
*/
void pullbreak(void)
{
	G_SHACHE(ACT_ON,0);
	G_LIHE(ACT_OFF,LIHEDLY / 2);
	osDelay(1500);
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,SHACHEDLY / 2);
}
/******************************************��ϵ��Զ�����  ��̽ͷģʽ ***************************************/
/**
  * �������� ������Ч����ϵ�߶ȣ�������
  * ��������� relax:ʵ���ɳڶ�
  *             *cnt:��������ĵ�����ָ��

  * ����ֵ: int32_t ʵ�ʵ���ϵ�
  * ˵��: ��
  */

int32_t getlihenum(void)
{
	static int32_t lihenum;
	
	liheupdate();
	lihenum = TTlehe.lihe;									//��ʼ����ϵ�

	return lihenum;
}

/**
  * �������� ������ϵ�ļ���ֵ
  * ��������� ��
  * ����ֵ: ��
  * ˵��: ÿ���޸���ϵ���ֵ�󣬶������¼���  ��ϵ���Զ���������Ϊ ���� 20cm
  */
extern struct HANGTU s_hang;  
void liheupdate(void)
{
	int32_t Lihecmtmp = 0;
	
	TTlehe.cnt = INIT_CHUI;		//������Ĭ��50�ε��ȶ�ʱ��
	TTlehe.relaxsum = 0;		//��ֵ����Ϊ��ֵ
	TTlehe.songchi = 0;
	
	Lihecmtmp = g_sys_para.s_hlihe;
	
	if(g_sys_para.s_cmode == MOD_AUTOTAMP)
	{
		if(s_hang.dachui_cnt == g_sys_para.s_cnt)			/*��һ�δ�,����һ��  2019.9.17*/
			Lihecmtmp -= 6;
		if(s_record.deepth > -300)						/*���ﶥ��ʱ�������*/
			Lihecmtmp += 8;			
	}
	
	if(Lihecmtmp < 50)						/*��С����ϵĵ�Ϊ50����  2019.12.17*/
		Lihecmtmp = 50;
	else if(Lihecmtmp > g_sys_para.s_sethighcm - 5)
		Lihecmtmp = g_sys_para.s_sethighcm - 5;
	TTlehe.lihe = cm2num(Lihecmtmp);
}


/********************************************************************************************************/

/*��ʼ�����Բ�������
�����ã�default task,��ʼ��ϵͳ���Բ���   2017.11.8
*/
void sysattr_init(uint16_t flg)
{
	HAL_StatusTypeDef status;
	uint32_t ID;
	
	osDelay(500);
	FB_CHECK();
	/*Ĭ��ֵ*/
	g_sys_para.s_dir = 0;
	g_sys_para.s_intval = 5;                  //����ʱ��  Terry 2019.6.6
	g_sys_para.s_numchi = 70;					//ÿȦ����
	
	if(flg == 0)
		g_sys_para.s_chickid = CHK_ID;
		
	g_sys_para.s_sethighcm = 300;
	g_sys_para.s_zhou = 106;
	g_sys_para.s_pnull = 50;
	g_sys_para.s_pfull = 500;
	g_sys_para.s_cmode = MOD_FREE;
	g_sys_para.s_hlihe = g_sys_para.s_sethighcm * 2 / 3;	/*ʹ��̽ͷʱ����ϵ� cm*/
	g_sys_para.s_mode = 0;							/*0:�Զ��� 1:ȫ�Զ�ǿ��*/
    g_sys_para.s_cnt = 6;         /*�������� Terry 2019.5.21*/
	g_sys_para.s_hprot = 150;		/*Ĭ�ϸ߶ȱ������� Terry 2019.7.6*/
	g_sys_para.s_pset = 0;		/*У����Ч Terry 2019.7.9*/
	
	/*����ڴ��Ƿ�����   2017.11.8  */
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
	
	if(status)		//У�����ʱ������д���ڴ�
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

	/*���ɲ��������ʼ��*/
	
	
    
    G_SHACHE(ACT_ON,0);         //��ʼΪ��ɲ��        2018.12.24 Terry
    Enc_Set_Dir(g_sys_para.s_dir);
	FB_CHECK();
    
	sys_stadata.TTCHK = 1;
	g_sys_para.s_cmode = MOD_FREE;		//�ϵ�󱣳ֿ���ģʽ
}

/********************************************************
Function	: FB_CHECK
Description	: ��������Ӳ�������ź�
Input		: None
Return		: None
Others		: �������������
*********************************************************/
extern void clear_all(void);
void FB_CHECK(void)
{
	/*��Դ�Ƿ�����*/
	if(PG_POWEROK())
		sys_fbsta |= FB_24VOK;
	else
		sys_fbsta &= ~FB_24VOK;

	/*��������ź�*/
	if(PG_FBLIHE())
		sys_fbsta |= FB_LIHE;
	else
		sys_fbsta &= ~FB_LIHE;
	/*ɲ�������ź�*/
	if(PG_FBSC())
		sys_fbsta |= FB_SHACHE;
	else
		sys_fbsta &= ~FB_SHACHE;
	/*��ͣ�����ź�*/
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


