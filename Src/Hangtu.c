
#include "Hangtu.h"
#include "port.h"
#include "GUI.h"
#include "Encoder.h"
#include "u_log.h"
#include "SingleAct.h"

extern struct SIG_ACT_DATA g_st_SigData;

#define GET_TICK_MS()  osKernelSysTick()
#define DELAYMS(x)	   osDelay(x)

SYS_STA Checkdown(void);
SYS_STA Check_up(void);
SYS_STA hangtu(uint8_t * pst);

extern uint16_t g_errnum;

/*����������������  */
/*
�ᴸ  ����  ���   ��
*/

struct HANGTU s_hang;                                   /*��������״̬���*/
struct RECORD s_record;									/*���ϴ�������*/

extern uint16_t g_led_sta;

/*����ͣ������*/
void Halt_Stop(void)
{
	G_LIHE(ACT_OFF,0);
	G_SHACHE(ACT_ON,0);
}

/*���㵱ǰ��ʱ����*/
static uint32_t GET_MS_DIFF(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = GET_TICK_MS();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*����һ��Ŀ��*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}

/*Modbus ��ʼ��ʱ�趨������*/
void ModbusData_Init(void)
{
	usRegHoldingBuf[M_NUB] = 0;
	usRegHoldingBuf[M_TIMES] = g_sys_para.s_cnt;
	usRegHoldingBuf[M_THIGH] = g_sys_para.s_sethighcm;
	usRegHoldingBuf[M_DEEP] = 0;
	usRegHoldingBuf[M_SONGTU] = g_sys_para.s_intval;
	usRegHoldingBuf[M_STATE] = 0;
	usRegHoldingBuf[M_ERR] = 0;
	
	usRegHoldingBuf[M_PERCNT] = g_sys_para.s_numchi;
	usRegHoldingBuf[M_ZHOU] = g_sys_para.s_zhou;
	usRegHoldingBuf[M_TLIAO] = g_sys_para.s_intval;
	usRegHoldingBuf[M_CCNUB] = g_sys_para.s_cnt;
	usRegHoldingBuf[M_TICHUI] = g_sys_para.s_sethighcm;
	usRegHoldingBuf[M_LIHE] = g_sys_para.s_hlihe;
	usRegHoldingBuf[M_HPROT] = g_sys_para.s_hprot;

	
	/*��Ȩ��*/
	usRegHoldingBuf[M_ACCRE] = 100;
	
	s_record.nub = 0;
	s_record.deepth = 0;
	s_record.high = 0;
	s_record.tim = 0;
	s_record.cnt = 0;
}
extern volatile struct GUI_DATA	g_GuiData;
void ModbusData_flash(void)
{
	usRegHoldingBuf[M_PERCNT] = g_GuiData.g_num;
	usRegHoldingBuf[M_ZHOU] = g_GuiData.g_Zhoucm;
	usRegHoldingBuf[M_TLIAO] = g_GuiData.g_ts;
	usRegHoldingBuf[M_CCNUB] = g_GuiData.g_hcnt;
	usRegHoldingBuf[M_TIMES] = g_GuiData.g_hcnt;
	
	usRegHoldingBuf[M_TICHUI] = g_GuiData.g_sethighcm;
	usRegHoldingBuf[M_LIHE] = g_GuiData.g_lihe;
	usRegHoldingBuf[M_HPROT] = g_GuiData.g_HighOvercm;
}

void ModbusData_Show(void)
{
	usRegHoldingBuf[M_DEEP] = (int16_t)g_GuiData.g_nhigh;
	usRegHoldingBuf[M_THIGH] = (int16_t)g_GuiData.g_sethighcm;
	usRegHoldingBuf[M_KS] = s_hang.last_downnum;						/*��ʾ׮�����*/
	usRegHoldingBuf[M_ERR] = g_errnum;
}

/*�ⲿ�����趨 ���¼��   ѭ��*/
extern uint32_t savecnt;

void ModbusData_Chk(void)
{
	/*ÿȦ����*/
	if(usRegHoldingBuf[M_PERCNT] != g_GuiData.g_num )
	{
		if((usRegHoldingBuf[M_PERCNT] < 300) && (usRegHoldingBuf[M_PERCNT] > 10))
		{
			g_GuiData.g_num  = usRegHoldingBuf[M_PERCNT];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_PERCNT] = g_GuiData.g_num;
		}
	}
	/*��Ͳ�ܳ�*/
	if(usRegHoldingBuf[M_ZHOU] != g_GuiData.g_Zhoucm)
	{
		if((usRegHoldingBuf[M_ZHOU] < 300) && (usRegHoldingBuf[M_ZHOU] > 20))
		{
			g_GuiData.g_Zhoucm = usRegHoldingBuf[M_ZHOU];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_ZHOU] = g_GuiData.g_Zhoucm;
		}
	}
	/*���ϵ�ʱ��*/
	if(usRegHoldingBuf[M_TLIAO] != g_GuiData.g_ts)
	{
		if((usRegHoldingBuf[M_TLIAO] < 16) && (usRegHoldingBuf[M_TLIAO] > 1))
		{
			g_GuiData.g_ts = usRegHoldingBuf[M_TLIAO];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TLIAO] = g_GuiData.g_ts;
		}
	}
	/*�趨�򴸵Ĵ���*/
	if(usRegHoldingBuf[M_CCNUB] != g_GuiData.g_hcnt)
	{
		if((usRegHoldingBuf[M_CCNUB] < 11) && (usRegHoldingBuf[M_CCNUB] > 2))
		{
			g_GuiData.g_hcnt = usRegHoldingBuf[M_CCNUB];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_CCNUB] = g_GuiData.g_hcnt;
		}
	}
	/*�趨�ᴸ�߶�*/
	if(usRegHoldingBuf[M_TICHUI] != g_GuiData.g_sethighcm)
	{
		if((usRegHoldingBuf[M_TICHUI] <= MAXSET_HIGH) && (usRegHoldingBuf[M_TICHUI] > MINSET_HIGH))
		{
			g_GuiData.g_sethighcm = usRegHoldingBuf[M_TICHUI];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TICHUI] = g_GuiData.g_sethighcm;
		}
	}
	/*�趨��ϵ��λ��*/
	if(usRegHoldingBuf[M_LIHE] != g_GuiData.g_lihe)
	{
		if((usRegHoldingBuf[M_LIHE] <= (g_GuiData.g_sethighcm - 20)))
		{
			g_GuiData.g_lihe = usRegHoldingBuf[M_LIHE];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_LIHE] = g_GuiData.g_lihe;
		}
	}
	
	/*�趨�����߶ȱ���ֵ*/
	if(usRegHoldingBuf[M_HPROT] != g_GuiData.g_HighOvercm)
	{
		if(usRegHoldingBuf[M_HPROT] <= 1000)
		{
			g_GuiData.g_HighOvercm = usRegHoldingBuf[M_HPROT];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_HPROT] = g_GuiData.g_HighOvercm;
		}
	}
	
		/*�ֶδ򴸴���*/
	g_GuiData.s_high1 = usRegHoldingBuf[M_HIGH1];
	g_GuiData.s_cnt1 = usRegHoldingBuf[M_CNT1];
	g_GuiData.s_high2 = usRegHoldingBuf[M_HIGH2];
	g_GuiData.s_cnt2 = usRegHoldingBuf[M_CNT2];
}

/*Led״ָ̬ʾ��  ����ģʽ��*/
void LedSta_Show(uint8_t ledsta)
{
	LED_BIT_CLR(SIG_SALL);					/*Led������*/
	switch(ledsta)
	{
		case S_IDLE:		/*����*/
			usRegHoldingBuf[M_STATE] = 0;
			break;
		case S_PULSE:
		case S_TICHUI:		/*�ᴸ*/
			LED_BIT_SET(SIG_STICHUI);
			usRegHoldingBuf[M_STATE] = 3;
			break;
		case S_CHECK_UPSTOP:
			LED_BIT_SET(SIG_SZHUCHUI);
			usRegHoldingBuf[M_STATE] = 4;
			break;
		case S_XIALIAO:		/*����*/
			LED_BIT_SET(SIG_STU);
			usRegHoldingBuf[M_STATE] = 5;
			break;
		case S_DELAY2:
		case S_LIUF:		/*���*/
			LED_BIT_SET(SIG_SLIUF);
			usRegHoldingBuf[M_STATE] = 1;
			break;
		case S_DACHUI:		/*����*/
			LED_BIT_SET(SIG_SHANGTU);
			usRegHoldingBuf[M_STATE] = 2;
			break;
		default:break;
	}
}


/*���Ƶ�������*/


void EXT_BUTTON_CHK(void)
{
	static uint8_t b_Dd;
	static uint8_t k_Cluch;
	static uint8_t k_Brk;
	static uint16_t b_Zd;
	
	b_Dd <<= 1;
	b_Zd <<= 1;
	k_Cluch <<= 1;
	k_Brk <<= 1;
	
	if(KEY_DD == GPIO_PIN_SET)
		b_Dd |= 0x01;
	if(KEY_ZD == GPIO_PIN_SET)
		b_Zd |= 0x01;
	if(KEY_TLH == GPIO_PIN_RESET)
		k_Cluch |= 0x01;
	if(KEY_TSC == GPIO_PIN_RESET)
		k_Brk |= 0x01;
		
	if(b_Dd == 0xFF)
	{
		IOT_FUNC_ENTRY;
		if(g_sys_para.s_cmode == MOD_FREE)
			g_sys_para.s_cmode = MOD_MAN;
	}
	else
	{
		if(g_sys_para.s_cmode == MOD_MAN)
			g_sys_para.s_cmode = MOD_MANOFF;
	}
	
	if(b_Zd == 0xFFFF)
	{
		IOT_FUNC_ENTRY;
		if(g_sys_para.s_cmode == MOD_FREE)
		{
			b_Zd = 0;
			if(g_sys_para.s_mode == 0)
				g_sys_para.s_cmode = MOD_SIGACT;
		}
		else if(g_sys_para.s_cmode != MOD_FREE)
		{
			b_Zd = 0;
			g_sys_para.s_cmode = MOD_FREE;
		}
	}
	
	if((k_Cluch == 0xFF) || (k_Brk == 0xFF))
	{
		IOT_FUNC_ENTRY;
		if(g_sys_para.s_cmode == MOD_FREE)
		{
			g_sys_para.s_cmode = MOD_TST;
		}
		
		if(k_Cluch == 0xFF)
			G_LIHE(ACT_ON,0);
		else
			G_LIHE(ACT_OFF,0);
			
		if(k_Brk == 0xFF)
			G_SHACHE(ACT_OFF,0);
		else
			G_SHACHE(ACT_ON,0);
	}
	else
	{
		if(g_sys_para.s_cmode == MOD_TST)
		{
			g_sys_para.s_cmode = MOD_FREE;
			G_LIHE(ACT_OFF,0);G_SHACHE(ACT_ON,0);
		}
	}
}


static enum {
	e_STEP_READY,
	e_STEP_STUDY,
	e_STEP_PULL,
	e_STEP_DOWN,
}s_SigActStep;

uint8_t TampStep;
  
SYS_STA services(void)
{
	static	uint8_t s_down;
	
	SYS_STA ret;
	ret = ERR_NONE;
	
	if(g_errnum)				// �����Ϻ�ɲ���Ƿ�����
	{
	  G_LIHE(ACT_OFF,0);
	  G_SHACHE(ACT_ON,0);
	  DELAYMS(ERRDLY);
	  return g_errnum;
	}
	EXT_BUTTON_CHK();
	
	if(!((sys_fbsta & FB_24VOK) && (sys_fbsta & FB_RUN)))	//������״̬������
	{
		G_LIHE(ACT_OFF,0);
		G_SHACHE(ACT_ON,0);
//		Log_e("FB_24VOK | FB_RUN");
		DELAYMS(ERRDLY);

		if(!(sys_fbsta & FB_24VOK))
			return ERR_PW;
		if(!(sys_fbsta & FB_RUN))
			return ERR_HALT;
	}

	switch(g_sys_para.s_cmode)
	{
		case MOD_SIGACT:     		//�Զ���ģʽ                            
			switch(s_SigActStep)
			{
			case e_STEP_READY:
				LED_BIT_SET(SIG_TICHUI);
				LED_BIT_CLR(SIG_FANGCHUI);
				s_SigActStep = e_STEP_STUDY;
				Sig_ResetSta();
				break;
			case e_STEP_STUDY:
				ret = Sig_StudyUp();
				if(ret == ERR_SIG_REACHED)
				{
					s_SigActStep = e_STEP_DOWN;
					Sig_ResetSta();
					LED_BIT_SET(SIG_FANGCHUI);
					LED_BIT_CLR(SIG_TICHUI);
				}
				break;
			case e_STEP_PULL:
				ret = Sig_TakeUp();
				if(ret == ERR_SIG_REACHED)
				{
					s_SigActStep = e_STEP_DOWN;
					Sig_ResetSta();
					LED_BIT_SET(SIG_FANGCHUI);
					LED_BIT_CLR(SIG_TICHUI);
				}
				break;
			case e_STEP_DOWN:
				ret = Sig_LandDw();
				if(ret == ERR_SIG_REACHED)
				{
					s_SigActStep = e_STEP_PULL;
					Sig_ResetSta();
					LED_BIT_SET(SIG_TICHUI);
					LED_BIT_CLR(SIG_FANGCHUI);
				}
				break;
			 default:ret = ERR_SIG_SOFT;break;
			}
			
			if(ret > ERR_SIG_REACHED)
			{
				Halt_Stop();
				g_sys_para.s_cmode = MOD_FREE;
			}
		  break;	
		  
	  case MOD_AUTOTAMP:								// ����ģʽ
			ret = hangtu(&TampStep);        			/*��ѯ�޶���*/
			LedSta_Show(TampStep);						/*Led״̬��ʾ*/
		  break;	
	  case MOD_FREE:
		  TampStep = 0;s_down = 0;
		  s_SigActStep = e_STEP_READY;
		  
		  G_SHACHE(ACT_ON,0);
          C_DISCTR();					/* �Ĵ��� ȡ�� ���  Terry 2019.07.04*/
		  LED_BIT_CLR(SIG_TICHUI);
		  break;
	  case MOD_MAN:						//�ֶ�ģʽ
		  if(TampStep == 0)
		  {
		      LED_BIT_SET(SIG_TICHUI);
			  G_LIHE(ACT_ON,0);
			  G_SHACHE(ACT_OFF,SHACHEDLY);
			  TampStep = 1;
		  }
		  break;
	  case MOD_MANOFF:
		  G_LIHE(ACT_OFF,LIHEDLY);
		  G_SHACHE(ACT_ON,0);
		  DELAYMS(200);					/*�ȴ�ɲ���ȶ���*/
	      g_sys_para.s_cmode = MOD_FREE;
		  break;
	  case MOD_DOWN:						//�·�ģʽ  2018.9.7  С����δʹ��
		  if(s_down == 0)
		  {
			  G_LIHE(ACT_OFF,LIHEDLY);
			  G_SHACHE(ACT_OFF,0);
			  s_down = 1;
		  }
		  break;
		  
	  case MOD_TST:
			break;																																			//Terry add 2017.11.16
	  default:	
			TampStep = 0;
			g_sys_para.s_cmode = MOD_FREE;
		  break;
	}
	
		
	if(g_sys_para.s_cmode > MOD_DOWN)
		g_sys_para.s_cmode = MOD_FREE;
		
	if((g_sys_para.s_cmode == MOD_FREE) || (g_sys_para.s_cmode == MOD_TST) ||(g_sys_para.s_cmode == MOD_DOWN))
		DELAYMS(CALUTICK);													//�����ó�����������    2017.10.7
	else
		DELAYMS(HANG_TICK);
 
  return ret;
}

/*��鴸�Ƿ�ֹͣ����ʱ2��ȷ��*/
int Check_Stop(void)
{
	static int32_t stop_cnt,power;
	static uint32_t now_tim;
	int ret  = 0,LSpeedCm;
	switch(s_hang.stop_sta)
	{
		case 0:
			stop_cnt = 0;
			s_hang.stop_sta = 1;
			now_tim = GET_TICK_MS();
			power = epower(70);
			break;
		case 1:
			LSpeedCm = GetEncoderSpeedCm();
			if((LSpeedCm < 20) && ((LSpeedCm > -20)) && (g_st_SigData.m_Power < power))
				stop_cnt++;
			else
			{
				if(stop_cnt > 3)
					stop_cnt -= 3;
			}
			
			if(stop_cnt > 500/HANG_TICK)
			{
				s_hang.stop_sta = 2;			/*�ɹ���ס*/
			}
			
			if(GET_MS_DIFF(now_tim) > 4000)
			{
				if(LSpeedCm > 20)
					ret = 2;		/*�嶥����*/
				else
					ret = 3;
			}
			/*�����ĳ嶥����*/
			if(GetEncoderLen2Cm() > 160)
			{
				ret = 2;
			}
			break;
		case 2:
			ret = 1;
			break;	
	}
	
	return ret;
}

/*�õ�ʵ�ʵĴ򴸴���*/
int Get_Hcnt(int32_t pos)
{
	int cnt = 0;
	
	cnt = g_sys_para.s_cnt;
	
	/*��Ϊ 0 ʱ������*/

	if((g_GuiData.s_high1 == 0) && (g_GuiData.s_high2 < 0))
	{
		if(pos > g_GuiData.s_high2 || (g_GuiData.s_cnt2 > 0))
			cnt = g_GuiData.s_cnt2;
	}
	else if((g_GuiData.s_high1 < 0) && (g_GuiData.s_high2 == 0))
	{
		if(pos > g_GuiData.s_high1 || (g_GuiData.s_cnt1 > 0))
			cnt = g_GuiData.s_cnt1;
	}
	else if(g_GuiData.s_high1 > g_GuiData.s_high2)
	{
		if((pos > g_GuiData.s_high1) && (g_GuiData.s_cnt1 > 0))
		{
			cnt = g_GuiData.s_cnt1;
		}
		else if((pos > g_GuiData.s_high2) && (g_GuiData.s_cnt2 > 0))
		{
			cnt = g_GuiData.s_cnt2;
		}
	}
	else if(g_GuiData.s_high2 > g_GuiData.s_high1)
	{
		if((pos > g_GuiData.s_high2) && (g_GuiData.s_cnt2 > 0))
		{
			cnt = g_GuiData.s_cnt2;
		}
		else if((pos > g_GuiData.s_high1) && (g_GuiData.s_cnt1 > 0))
		{
			cnt = g_GuiData.s_cnt1;
		}
	}
	
	
	return cnt;
}


/*�Զ��������� */
extern struct PRPTECT_HANGTU Prtop;
SYS_STA hangtu(uint8_t * pst)
{
	static uint16_t stlast;		/*����Ƿ�Ϊ��һ�����  2019.11.15*/
    SYS_STA ret = 0;
    int tmp,LSpeedCm;    
    
    switch(*pst)
    {
        case S_IDLE:
            /*��¼��ǰ�߶� �����߶����� -> ��ʼ���  -> ��¼�����ʼ�߶�*/
            {
				stlast = S_IDLE;				/*��ǰת��״̬ �������*/
                s_hang.pvtimer = GET_TICK_MS();
				Enc_Clr_TotalCnt2();
                s_hang.overpow = 0;
                s_hang.speederr = 0;
				Prtop.flg = 0;					/*�Ƿ���Ҫ���¶�����ϵ�ĸ߶�*/
				G_SHACHE(ACT_LIU,0); 
				*pst = S_DELAY2;                /*�����*/
				s_hang.last_highnum = GetEncoderLen2Cm();		/*���ʱ�ĸ߶�  Terry 2019.11.12 ֻ�����ж��½��߶�*/
				s_record.deepth = 0;
                Debug("Han Start:\r\n");
            }
            break;
        case S_PULSE:
                *pst = S_TICHUI; 
                s_hang.top_sta = 0;
                G_LIHE(ACT_ON,0);
                G_SHACHE(ACT_OFF,SHACHEDLY);    /*��ʼ�ᴸ*/
                s_hang.pvtimer = GET_TICK_MS();
				s_hang.speederr = 0;
				s_hang.overpow = 0;
				s_hang.lowpow = 0;
				//���ºͷ��ͼ�¼
				s_record.nub++;
				usRegHoldingBuf[M_NUB] = s_record.nub;
				usRegHoldingBuf[M_TIMES] = s_record.cnt;
				usRegHoldingBuf[M_THIGH] = s_record.high;
				usRegHoldingBuf[M_SONGTU] = s_record.tim;
            break;
		
        case S_TICHUI:
				LSpeedCm = GetEncoderSpeedCm();
                if(Check_up() == 1) 					/*�ᴸ����������ɲ�����������*/  
                {
					if(GET_MS_DIFF(s_hang.pvtimer) > 800)	/*������� 0.8��*/
					{
						G_LIHE(ACT_OFF,LIHEDLY);
						G_SHACHE(ACT_ON,0);        		/*����ɲ��*/     
						Debug("Get Top signal\r\n");        
						*pst = S_CHECK_UPSTOP;
						s_hang.pvtimer = GET_TICK_MS();
						s_hang.stop_sta = 0;
					}
                }
				
				/*****************ʧ�ٱ���  ̽ͷʧЧ����*********************************/
                if(GetEncoderSpeedCm() < 20)		/*����ʱ���ٶ�С��20cm/s,��Ϊ�쳣*/
                    s_hang.speederr++;
                else
				{
					if(s_hang.speederr > 3)
						s_hang.speederr -= 3;
				}
                if(s_hang.speederr > 2800/HANG_TICK)
                {
                     Debug("�������������߼�������\r\n");
                     ret = ERR_LIU;
                }
                /**************************�������ر���*********************/
                tmp = epower(350);	
                if(g_st_SigData.m_Power > tmp)
                    s_hang.overpow++;
                else
                    s_hang.overpow = 0;
                
                if(s_hang.overpow > 300)   /*����1.5���޷���������ʾ��������  2019.10.7*/
                {
                    Debug("��������\r\n");
					ret |= ERR_KC;
                } 
				/************************ʧ�ر���***************************/
				tmp = epower(40);
				if(g_st_SigData.m_Power < tmp)
					s_hang.lowpow++;
				else
				{
					if(s_hang.lowpow > 3)
						s_hang.lowpow -= 3;
				}
				if(s_hang.overpow > 2000/HANG_TICK)
				{
					Debug("ʧ�ر���\r\n");
					ret |= ERR_LS;
				}
				/***********************************************************/
            break;
        case S_CHECK_UPSTOP:
			/*�˴��������Ƿ��ɿ�*/
			tmp = Check_Stop();
			if(tmp == 1)    /*���ɹ�*/
			{
				C_ENCTR();
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_XIALIAO;
                Debug("wait1 1s\r\n");
			}
			else if(tmp == 2)
			{
				ret |= ERR_TOP;		/*�嶥����*/
			}
			else if(tmp == 3)		/*ɲ���쳣*/
			{
				ret |= ERR_SC;
			}
            break;
        case S_XIALIAO:                         				/*  ����  */
			/*��ǰִ����Ų���*/
			if(GET_MS_DIFF(s_hang.pvtimer) > (g_sys_para.s_intval * 600 - 200))	
			{
				s_hang.last_highnum = GetEncoderLen2Cm();	/*���ʱ�ĸ߶�  Terry 2019.7.5*/
                G_LIHE(ACT_OFF,0);
				G_SHACHE(ACT_LIU,0); 
                *pst = S_DELAY2;
                Debug("wait2 1s\r\n");
			}
            break;
        case S_DELAY2:                          				/*��ͨ��ʱ  �տ�ʼʱ����Ҫ��ʱ*/
			if((GET_MS_DIFF(s_hang.pvtimer) > g_sys_para.s_intval * 600) || (stlast == S_IDLE))
            {
                C_DISCTR();			/*�ر��Ĵ���*/
				stlast = S_DELAY2;
				
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_LIUF;
                Debug("finish xia liao  %ds\r\n",g_sys_para.s_intval);
				s_hang.liufang_sta = 0;
            }
            break;
        case S_LIUF:
			/*�½����׼��*/
			
            tmp = Checkdown();   
			
            if(tmp == 1)			/*��⵽������*/
            {
                s_hang.dachui_cnt = g_sys_para.s_cnt;
				s_record.high = g_sys_para.s_sethighcm / 10;		/*�򴸸߶� Terry 2019.6.5*/
				s_record.tim = g_sys_para.s_intval;				/*����ʱ�� Terry 2019.6.5*/
				
                /*��¼��ǰ��ŵ����   */
                if(GetEncoderLen2Cm() > -40)  				/*�½���ȹ�С  �½�0.5��*/
                {
                    Debug("�½���ȹ�С \r\n");
                    ret |= ERR_DOWN;  
                }
                else
                {
					int LPosCm;
					
					LPosCm = GetEncoderLen2Cm();
					s_hang.dachui_cnt =	Get_Hcnt(LPosCm);
					s_record.cnt = s_hang.dachui_cnt;							/*��¼�򴸴��� Terry 2019.6.5*/	
                    s_hang.last_downnum = (int16_t)(LPosCm/10);   /*�����½���� ���� ����ֵ Terry 2019.7.5*/
                    *pst = S_DACHUI;       					/*��ʼ�� */
					
                    Debug("dao di le \r\n");
                }
            }
            if(tmp > 1)       						/*��Ŵ���*/
            {
                ret |= ERR_DOWN;
                Debug("��Ŵ��� \r\n");
            }
            break;
        case S_DACHUI:
			usRegHoldingBuf[M_ACT] = 1;				/*�ᴸ��־ Terry 2019.7.6*/
            ret = takeup();							/*����ģʽ*/
            Debug("DA chui %d c\r\n",s_hang.dachui_cnt);
            if(ret == ERR_NONE)
            {
				usRegHoldingBuf[M_ACT] = 0;			/*�ᴸ��־ Terry 2019.7.6*/
                ret |= putdown(0);					/*����ģʽ*/
            }
            
            if(ret == ERR_NONE)
            {
                s_hang.dachui_cnt--;
                if(s_hang.dachui_cnt < 1)
                {
					if(s_record.deepth > -50)			/*��ʾ��������  Terry 2019.10.18 */
					{
						*pst = S_IDLE;
						g_sys_para.s_cmode = MOD_FREE;	/*ֱ���˳�  2019.8.2*/
						G_SHACHE(ACT_ON,0);
						G_LIHE(ACT_OFF,0);
					}
					else
					{
						*pst = S_PULSE;       /*��һ���ᴸ����   ֱ���ᴸ*/
						Debug("complete \r\n");
					}					
                }
            }
            break;
    }
    /*Switch ����*/
    if(g_halt)
    {
        Debug("Stop S\r\n");
		ret |= ERR_HALT;
    }
    
    
    /*�쳣�˳�ʱ��������ɲ�����ر����ʹ�(Ī����)*/
	if(ret)
	{
		g_sys_para.s_cmode = MOD_FREE; 
		G_SHACHE(ACT_ON,0);
		G_LIHE(ACT_OFF,0);					/*2019.8.2���  Terry*/
		C_DISCTR();
		*pst = S_IDLE; 
	}
    return ret;
}


/*��鴸�Ƿ񵽵���
���� 1  ����
     >1 ����
*/
SYS_STA Checkdown(void)
{
    static uint32_t last_tim = 0;
    static uint32_t sure_cnt = 0;
	static uint32_t speed_over = 0;
	
    SYS_STA ret = 0;
	int LPosCm;
	int LSpeedCm = GetEncoderAcceCm();
	
	
    LPosCm = GetEncoderLen2Cm();
    switch(s_hang.liufang_sta)
    {
        case 0:
            last_tim = GET_TICK_MS();
            s_hang.liufang_sta = 1;
			sure_cnt = 0;
			speed_over = 0;
            break;
        case 1:
            if(GET_MS_DIFF(last_tim) > 1300)       	/*1.3���  ��ʼ�ж��ٶ�   2000  2019.07.04*/
            {
                if((LSpeedCm < -10 ) && ((s_hang.last_highnum - LPosCm) > 80))  /*�ж������ж������*/
                {
                    s_hang.liufang_sta = 2;
                }
                if(GET_MS_DIFF(last_tim) > 6000)      /*6 �볬ʱû�ж��������� 2019.8.2*/
                {
                    ret = 12;                       /*��û���������*/
                    s_hang.liufang_sta = 3;
                }		
            }
            break;
        case 2:                                     	/*������һ��*/
            /*�ж��Ƿ񵽵���*/
            if(LSpeedCm > -7)
                sure_cnt++;
            else if(LSpeedCm < -8)			/*����Ϊ ��*/
            {
                sure_cnt = 0;                       	/*�������ж�  ������*/
				last_tim = GET_TICK_MS();
            }
            
            if(sure_cnt > 1200/HANG_TICK)            	/*ȷ���Ѿ�ֹͣ  1.2��  ԭ��0.8�� */
            {
				if(s_record.deepth + 400 > LPosCm)	/*���紸�������棬����4�ף��ͻᱨ��*/
				{
					ret = 1;
					s_hang.liufang_sta = 3;             /*�����ж�*/
				}
				else
				{
					sure_cnt = 0;						/*���¿�ʼ�ۻ�*/
					if(GET_MS_DIFF(last_tim) > 4000)		/*����ٵȴ�4��*/
					{
						ret = 12;                       /*��û���������,û�����䵽�׵Ĺ��� 2019.11.16*/
						s_hang.liufang_sta = 3;
					}
				}
            } 
			/*���ٱ���*/
			if(LSpeedCm < -800)			/*�½��ٶȳ���6��/��ʱ������0.7�룬��������쳣���� 2019.12.08*/
			{
				speed_over++;
			}
			else
			{
				if(speed_over > 4)
					speed_over -= 4;
			}
			if(speed_over > 1000/HANG_TICK)
			{
				ret = 12;							/*�½��ٶȹ��챨��*/
			}	
            break;
        case 3:										/*����*/
            break;
        default:
            s_hang.liufang_sta = 3;
            break;
    }
    return ret;
}


/*������ʱ����*/
/*����1 ��⵽
  ����0 �޼�⵽
  ���� ����
*/
SYS_STA Check_up(void)
{
    SYS_STA ret = 0;
    
	 /*�ȴ�����������Ч   ���������������λ������Ч   ��Ĺ�ϵ*/
	if((GetEncoderLen2Cm() > -15) ) 	/*-30����   Terry ��ʹ�ó嶥���ź�*/
	{
		ret = 1;		 /*���Ѿ�����Ԥ��λ��*/
	}

    return ret;
}

