

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



/********************************************************
Function	: PICtr()
Description	: ���ɳڶȵ�������PI���ƣ�����
Input		: sub	����Ϊ��������Ϊ����
Return		: ��ϵ������������λ ������������ϵ����������
Others		: �ֶ�������ϵ�ʱ����ͣһ��ʱ�䲻�����Զ�����
5�������ڵ��ɳڶ�������
*********************************************************/
int32_t PICtr(int32_t subs)
{
	static float Totals = 0;
	
	float ret;
	int32_t tmp;
	
	if(subs > 5)
		Totals += (subs - 5);					//�������л���
	else if(subs < 0)
		Totals += subs;
	else										/*�� 0 �� 5 ��������*/
	{
		if(Totals < 0)							/*��ǿ�ȶ���*/
			Totals += subs;
		else if(Totals > 20)
			Totals -= (5 - subs);	
	}
	
	
	if(subs > 5)								//�Ƚ��ɵ�ʱ��
	{
		ret = 0.03 * (subs - 5);					
	}
	else if(subs < 0)
	{
		ret = 0.03 * subs;
	}
	else
		ret = 0;
	
	ret = 0.05 * Totals + ret;					//��Ҫ�ǿ��ۼ����ȥ����
	
	tmp = (int32_t)ret;
	
	if(tmp != 0)
	{
		Totals = Totals / 2;
	}
	Debug("PI %d  ",tmp);
	return tmp;
}


int32_t PITCtr(int32_t subs)
{
	static int32_t last_err = 0;
	
	float ftmp;
	int32_t ret;
	
	ftmp = 0.04*((subs - last_err) + 0.1 * subs);
	if(ftmp > 20)
		ftmp = 20;
	else if(ftmp < -20)
		ftmp = -20;
	
	ftmp = ftmp / 3;
	
	if(ftmp > 12)
		ftmp = 12;
	else if(ftmp < -12)
		ftmp = -12;
	
	last_err = subs;
	
	ret = -ftmp;
	
	return ret;
}

/*��õ�ǰ����*/
void GetPower(struct EtrCnt *p)
{
	static uint32_t lastspeed = 0;
	
	p->Speed = sys_Etrcnt.Speed + lastspeed;				//˫���ٶ�
	lastspeed = sys_Etrcnt.Speed;
}


/*������й����ʱ�ֵ��Ӧ��ʵ�ʹ���*/
int32_t epower(int16_t  ratio)
{
	uint32_t pw;
	
	pw = (g_sys_para.s_pfull - g_sys_para.s_pnull) * ratio / 100;
	pw += g_sys_para.s_pnull;
	
	return pw;
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

/*2018.9.1 �����ᴸ���״̬���жϣ���ֹ�ᴸ����������ʶ��*/
void Pull_Clear(int16_t *psta)
{
	static int8_t ocnt = 0;
	int32_t tmp;
	uint32_t ctime;
	
	
	ctime = osKernelSysTick();
	switch(*psta)
	{
		case PULL_IDLE:
			tmp = epower(70);
			ocnt = 0;
			
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 150)	    //������Ч�ᴸ���� 150ms  ��Ϊ��Ч�ᴸ
				{
					*psta = PULL_EFFECT;
					Enc_Clr_TotalCnt1();
                    Debug("speed %d\r\n",sys_stadata.m_power.Speed);
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_OBS:
			tmp = epower(80);
			Enc_Clr_TotalCnt1();
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 80)	    //������Ч�ᴸ���� 80ms  ��Ϊ��Ч�ᴸ
				{
					ocnt++;
					*psta = PULL_EFFECT;
					
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_EFFECT:
			tmp = epower(55);
			if(ocnt < 1)
			{
				while(sys_stadata.m_power.Speed < tmp)
				{
					if(getmilsec(ctime) > 60)	    //������Ч�ᴸС�� 120ms  ��Ϊ�ճ�
					{
						*psta = PULL_OBS;
						break;
					}
					osDelay(1);
				}
				
			}
			break;
		default:*psta = PULL_IDLE;
	}
}

/*�����ᴸ����*/
extern struct Locationcm user_encoder;  

SYS_STA takeup(void)
{
	SYS_STA status;
	uint32_t ctime,intime;
	int32_t loadpw,sethnum,overpw;
	int8_t kccnt = 0;
	int32_t protectT = 0;
	int32_t speedcnt = 0;
	int32_t powcnt = 0;
	int32_t starthighcm = 0;
	
	status = ERR_NONE;
	
	do
	{
		G_LIHE(ACT_ON,0);G_LIHE(ACT_ON,0);			       /*���ƺ�ִ�зֿ�   ���������*/
		G_SHACHE(ACT_OFF,SHACHEDLY);
		G_LIHE(ACT_ON,0);
		starthighcm = GetEncoderLen1Cm();	/*����ϵĸ߶ȵ� 2019.12.19 Terry add*/
		ctime = osKernelSysTick();
		/*�����������Ƿ���*/
		while(speedcnt < 3)									//��ʱ�������½�����ʱ4��ȽϺ���    Terry 2019.5.25
		{
			if(GetEncoderSpeedCm() > -10)
				speedcnt++;
			else
			{
				if(speedcnt > 0) 
					speedcnt--;
			}
			
			if(getmilsec(ctime) > 5000)						//����Ϻ�3�����ϻ��Ƿ�ת���򱨹���
			{				
				if(sys_stadata.m_power.Speed < (g_sys_para.s_pnull / 2))
					status |= ERR_CT;						//����δ�����򻥸�����
				else
					status |= ERR_TT;						//©����һ���������ô�������Ϊ��̽ͷ����
					
				ERR_BREAK;
			}
			/*Ԥ�������Ĵ��� ������źź󣬼�������6�� ������ 2019.12.17  Terry add*/
			if((starthighcm - GetEncoderLen1Cm()) > 800)		/*����ɳڹ���ֱ�ӱ���  ����ϵ���͵㳬��6�� 2019.12.19*/
			{
				status |= ERR_LS;
				ERR_BREAK;
			}
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		
		ERR_BREAK;										/*�쳣ֱ������ѭ��  do while*/		
		
		ctime = osKernelSysTick();
		Enc_Clr_TotalCnt1();
		loadpw = epower(70);
		while( powcnt < 4)								//�ȴ� 80%��Ч�ᴸ��  300ms
		{
			if(sys_stadata.m_power.Speed > loadpw)
				powcnt++;								/*  ����70%������,��Ϊ��Ч�ᴸ*/
			else
			{
				if(powcnt > 1) 
					powcnt -= 1;
				else
					Enc_Clr_TotalCnt1();
			}
			
			//8�볬ʱ
			if(getmilsec(ctime) > 6000)						/*��������ʱ�� 6��  �����ú��ɣ��Զ�����*/
			{
				status |= ERR_CT;							// ��Ϲ��ɻ���Ӧ����
				break;
			}
			
			if(GetEncoderLen1Cm() < -300)		/*����ɳڹ���ֱ�ӱ���*/
			{
				status |= ERR_CT;
				ERR_BREAK;
			}
			
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		ERR_BREAK;													/*�쳣ֱ������ѭ��  do while*/		
		sethnum = cm2num(g_sys_para.s_sethighcm);						//�趨�߶ȣ�����ɳ���
        protectT = g_sys_para.s_sethighcm * 30 + 5000;       			//�޸�Bug   2018.11.9
		sys_stadata.clihe = getlihenum();  //�õ�����ʱ����ϵ�ĸ߶�
		
		
		/*��¼��ǰ�����Ϊ��͵�   Terry 2019.6.5*/
		s_record.deepth = GetEncoderLen2Cm();				/*��λ ���� */
		
Debug("upnum %d\r\n",tmp);
Debug("autolihe %d",sys_stadata.clihe);

		speedcnt = 0;
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)				//�趨�߶�  �Ƚϵĳ���
		{
			/******************������ʱ�ж�**************************/
			if(getmilsec(ctime) > protectT)			    		//�ᴸʱ�䳬��8�룬������
			{
				status |= ERR_LS;
                Debug("err 2");
				break;		/*ֱ������*/
			}
			ERR_BREAK;
			HALT_BREAK;

			
			/*�Զ�����ʱ�ı���  2019.7.24*/
			if(g_sys_para.s_cmode == MOD_AUTOTAMP)
			{
				if(GetEncoderLen1Cm() > g_sys_para.s_hprot)	/*����0.5�ף��������� ��������������¶�����ϵ�ĸ߶� 2019.10.18 Terry*/
				{
															/*��ס��ǰ���ᴸ�߶� */
					Prtop.flg = 1;
					Prtop.p_high = Enc_Get_CNT1();
					Prtop.p_lihe = sys_stadata.clihe *  Enc_Get_CNT1() * 11 / sethnum;  /*���ձ���������Ͽ���*/
					Prtop.p_lihe = Prtop.p_lihe / 10;
					if(Prtop.p_lihe > sys_stadata.clihe)
						Prtop.p_lihe = sys_stadata.clihe;
					
					break;							/*��������ѭ�� �ܹؼ�*/
				}
			}
			
			/**************�����ж�***********************/
			intime = osKernelSysTick();
			overpw = epower(250);
			while(sys_stadata.m_power.Speed > overpw)
			{
				//1�볬ʱ
				if(getmilsec(intime) > 3000)				// ����3�볬ʱ
				{
					status |= ERR_KC;						// ������

					Debug("up kc\r\n");
					kccnt++;
					break;
				}
				HALT_BREAK;
				ERR_BREAK;				/*������ѭ��*/
				osDelay(CALUTICK);
			}
			// pull break;  ��������
			if((status & ERR_KC) && (kccnt < 3))
			{
				pullbreak();
				status &= ~ERR_KC;
				protectT += 3000;			/*��ʱ�ж����� 3�� */
			}
			else
			{
				/******************��;�ﴸ�ж�**************************/
				if(Enc_Get_SpeedE1() < 0)
					speedcnt++;
				else
				{
					if(speedcnt > 3)  
						speedcnt-=3;
				}
				
				if(speedcnt > 800)		/*������3�룬�᲻�����ͱ���*/
				{
					status |= ERR_LS;
					Debug("�ﴸ��\r\n");
				}
					
				ERR_BREAK;
			}
			osDelay(5);
			ERR_BREAK;
		}
	}while(0);
	
	Debug("high %d \r\n",sys_stadata.m_high.TotalCount);
	
	return status;
}


/*һ������*/
SYS_STA starttaking(void)
{
	SYS_STA status;
	int32_t avepow;
	uint16_t i;
	int32_t sethnum,cnt;
	int32_t noloadpw;
	uint32_t ctime,intime;
	
	status = ERR_NONE;
	cnt = 0;
	/*��ƽ�����ع���*/
	avepow = 0;
	
	IOT_FUNC_ENTRY;
	for(i = 0;i < 30;i++)
	{
		avepow += sys_stadata.m_power.Speed;
		osDelay(CALUTICK);
	}
	g_sys_para.s_pnull = avepow / 30;
    avepow = 0;
	Debug("null %d\r\n",g_sys_para.s_pnull);

	//����� 0.4�����ɲ��
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,30);
	Enc_Clr_TotalCnt1();
	osDelay(1000);
	//�ȴ���Ч����
	do
	{
		ctime = osKernelSysTick();
		noloadpw = g_sys_para.s_pnull * 3 / 2 + 1;			//�ȴ�1.5���Ŀ��ع���  ������
		while(sys_stadata.m_power.Speed < noloadpw)			
		{
			if(getmilsec(ctime) > 3000)		/*����4���ⲻ���������ͱ���  2019.11.7*/
			{
				status |= ERR_CS;
				break;
			}
			osDelay(CALUTICK);
			HALT_BREAK;
		}
		ERR_BREAK;
		
		for(i = 0;i < 20;i++)
		{
			HALT_BREAK;
			osDelay(CALUTICK);
		}

		cnt = 0;
		avepow = 0;
//		savecnt = 0;


//		CheckDir(&sys_stadata.m_high);								//�ı��������ֵ		
		sethnum = cm2num(g_sys_para.s_sethighcm);							//���߶Ȼ���ɳ���
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)					//�趨�߶�
		{
			avepow += sys_stadata.m_power.Speed;
			cnt++;
			/******************��;�ﴸ�ж�**************************/
			intime = osKernelSysTick();
			while(Enc_Get_SpeedE1() < 1) 		//��;�����ﴸ
			{
                Debug("hsed %d ",sys_stadata.m_high.Speed);
				if(getmilsec(intime) > 4000)			// 1�볬ʱ
				{
					status |= ERR_LS;						// ��Ϲ���,�����
					break;
				}
				HALT_BREAK;
				osDelay(5);
			}
			if(status) break;

			/***************������ʱ�ж�***********/
			if(getmilsec(ctime) > (g_sys_para.s_sethighcm * 30 + 5000))   		/*������ʱ�ж�*/         
			{
				status |= ERR_CS;
				break;
			}
			
			if(status) break;
			osDelay(2);
		}
		
	}while(0);
	
	if(status == ERR_NONE)
	{
		if(cnt)
		  g_sys_para.s_pfull = avepow / cnt;
		else
		  g_sys_para.s_pfull = sys_stadata.m_power.Speed;
		
		sys_stadata.clihe = cm2num(g_sys_para.s_hlihe);
//		savecnt = 20;											/*�޴��󼴿�ʼ��������*/
	}
	
	IOT_FUNC_EXIT_RC(status);
}



/********************************************************
Function	: putdown
Description	: �򴸳���(�����˫��)
Input		: delay	  ���ʱ�䣬����Ϊ 0 ��˫�� Ϊ 300+
Return		: SYS_STA
Others		: ��������

1. ����
2. �½���ʱ
*********************************************************/
SYS_STA putdown(int32_t delay)
{
	SYS_STA status;
	uint32_t ctime;
	int32_t tmp;
	
	IOT_FUNC_ENTRY;
	G_LIHE(ACT_OFF,0);
	status = ERR_NONE;
	
	do
	{
		ctime = osKernelSysTick();
		/*��ⴸ�Ƿ��ѿ�*/
		while((sys_stadata.m_power.Speed > epower(LALIMAXPER)) ||(Enc_Get_SpeedE1() > 20))	/*2019.11.15  ͬʱ����ٶ���*/
		{
			if(getmilsec(ctime) > 1500)				//����ᴸʱ�䳬��0.8��  ִֻ��1��
			{
				Debug("����\r\t");
				G_SHACHE(ACT_ON,0);
				if(getmilsec(ctime) > 3000)			//���� 3�볬ʱ
					status = ERR_NC;				//����ɲ���  
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		if(status == ERR_NONE)						//�޴������ɲ��
			G_SHACHE(ACT_OFF,0);
		
		ERR_BREAK;
		HALT_BREAK;
		
		/*�ȴ���ϵ�  Terry 2019.7.24*/
		if(Prtop.flg)
		{
			tmp = Prtop.p_lihe;							/*���¶�����ϵ�ĸ߶�*/
			Prtop.flg = 0;
		}
		else
		{
			tmp = sys_stadata.clihe;                   /*���ݹ�������ϵ�  ������ʱ��*/
		}

		ctime = osKernelSysTick();

		Debug("lihenum %d \r\n",tmp);

		while(Enc_Get_CNT1() > tmp)			//��ϵ�ĸ߶�
		{
			if(getmilsec(ctime) > (5200))					// �½��½�ʱ�䳬��5�룬�ж�������
			{
				status |= ERR_KC;							//�½�ʱ������
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(2);
		}
		
	}while(0);
	
	IOT_FUNC_EXIT_RC(status);
}



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
			Debug("EEread ok\r\n");
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
			Debug("EEread err\r\n");
			status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(1000);
		}
	}
	
	Enc_Clr_TotalCnt1();

	/*���ɲ��������ʼ��*/
	
	
    
    G_SHACHE(ACT_ON,0);         //��ʼΪ��ɲ��        2018.12.24 Terry
    
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


