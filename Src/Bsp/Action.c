

#include "Action.h"
#include "math.h"
#include "EEPROM.h"
#include "Encoder.h"
#include "PutOffAct.h"
#include "u_log.h"
				


//struct LIHEDATA	    TTlehe;							//ʹ��̽ͷ����ϼ���ģʽ,�޸���ϵ�󣬱����������
//struct LIHEDATA	    TMlehe;		    				// TM��϶����������������
//extern int8_t Get_Fbsignal(uint8_t MASK);

//uint32_t getmilsec(uint32_t pretime)
//{
//	uint32_t milsec;	
//	milsec = osKernelSysTick();
//	if(pretime > milsec)
//		milsec = (0xffffffff - pretime) + milsec;  			/*����һ��Ŀ��*/
//	else
//		milsec = milsec - pretime;			
//	return 	milsec;
//}


/*��ñ�����������ȷ����׼����*/
/**
  * �������� ��ñ�����������ȷ����׼����
  * ���������dir:��׼����
  *           *p:Ӳ����������������
  * ����ֵ: None
  * ˵��: ��
  */
  
/*  ����ת����  */
//int32_t cm2num(int32_t cm)
//{
//	int32_t tmp;
//	
//	tmp = cm * g_sys_para.s_numchi / g_sys_para.s_pericm;
//	
//	return tmp;
//}

/*
����ʱ
�ж�����
��ɲ�� �����

�����  ��ɲ��
*/
//void pullbreak(void)
//{
//	G_SHACHE(ACT_ON,0);
//	G_LIHE(ACT_OFF,LIHEDLY / 2);
//	osDelay(1500);
//	G_LIHE(ACT_ON,0);
//	G_SHACHE(ACT_OFF,SHACHEDLY / 2);
//}
/******************************************��ϵ��Զ�����  ��̽ͷģʽ ***************************************/
/**
  * �������� ������Ч����ϵ�߶ȣ�������
  * ��������� relax:ʵ���ɳڶ�
  *             *cnt:��������ĵ�����ָ��

  * ����ֵ: int32_t ʵ�ʵ���ϵ�
  * ˵��: ��
  */

//int32_t getlihenum(void)
//{
//	static int32_t lihenum;
//	
//	liheupdate();
//	lihenum = TTlehe.lihe;									//��ʼ����ϵ�

//	return lihenum;
//}

/**
  * �������� ������ϵ�ļ���ֵ
  * ��������� ��
  * ����ֵ: ��
  * ˵��: ÿ���޸���ϵ���ֵ�󣬶������¼���  ��ϵ���Զ���������Ϊ ���� 20cm
  */
//extern struct HANGTU s_hang;  
//void liheupdate(void)
//{
//	int32_t Lihecmtmp = 0;
//	
//	TTlehe.cnt = INIT_CHUI;		//������Ĭ��50�ε��ȶ�ʱ��
//	TTlehe.relaxsum = 0;		//��ֵ����Ϊ��ֵ
//	TTlehe.songchi = 0;
//	
//	Lihecmtmp = g_sys_para.s_setlihecm;
//	
//	if(g_st_SigData.m_Mode == MOD_AUTOTAMP)
//	{
//		if(s_hang.dachui_cnt == g_sys_para.s_cnt)			/*��һ�δ�,����һ��  2019.9.17*/
//			Lihecmtmp -= 6;
//		if(s_record.deepth > -300)						/*���ﶥ��ʱ�������*/
//			Lihecmtmp += 8;			
//	}
//	
//	if(Lihecmtmp < 50)						/*��С����ϵĵ�Ϊ50����  2019.12.17*/
//		Lihecmtmp = 50;
//	else if(Lihecmtmp > g_sys_para.s_sethighcm - 5)
//		Lihecmtmp = g_sys_para.s_sethighcm - 5;
//	TTlehe.lihe = cm2num(Lihecmtmp);
//}


/********************************************************************************************************/


/********************************************************
Function	: FB_CHECK
Description	: ��������Ӳ�������ź�
Input		: None
Return		: None
Others		: �������������
*********************************************************/
//extern void clear_all(void);
//void FB_CHECK(void)
//{
//	/*��Դ�Ƿ�����*/
//	if(PG_POWEROK())
//		sys_fbsta |= FB_24VOK;
//	else
//		sys_fbsta &= ~FB_24VOK;

//	/*��������ź�*/
//	if(PG_FBLIHE())
//		sys_fbsta |= FB_LIHE;
//	else
//		sys_fbsta &= ~FB_LIHE;
//	/*ɲ�������ź�*/
//	if(PG_FBSC())
//		sys_fbsta |= FB_SHACHE;
//	else
//		sys_fbsta &= ~FB_SHACHE;
//	/*��ͣ�����ź�*/
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


