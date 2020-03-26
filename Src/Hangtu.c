
#include "Hangtu.h"
#include "port.h"
#include "GUI.h"
#include "Encoder.h"
#include "u_log.h"
#include "SingleAct.h"
#include "Frq_Mens.h"


#define GET_TICK_MS()  osKernelSysTick()
#define DELAYMS(x)	   osDelay(x)

/*���״̬�ж�*/
#define HAS_REACH_DOWN		1
#define HAS_REACH_TIMOUT	2
#define HAS_REACH_SPOVER	3

#define STA_CLEAR(x)        x = 0

extern struct SIG_ACT_DATA g_st_SigData;			/*ʵʱ����*/
extern volatile struct GUI_DATA	g_GuiData;			/*��ʾ����ʱ����*/

static SYS_STA LiuChkDw(int LPosCm,int LSpeedCm);
static SYS_STA Check_up(int LPosCm);
static SYS_STA hangtu(uint16_t * pst);

/*����������������  */
/*
�ᴸ  ����  ���   ��
*/

static struct HANGTU s_hang;                                   /*��������״̬���*/
static struct RECORD s_record;									/*���ϴ�������*/
struct SYSATTR g_sys_para;								// ϵͳ����


/*����ͣ������*/
void Halt_Stop(uint32_t delay)
{
	G_LIHE(ACT_OFF,0);
	G_SHACHE(ACT_ON,0);
	C_DISCTR();
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


/*      ���ڴ���д������   */
void RecordIn(int idx,uint16_t data)
{
	usRegHoldingBuf[idx] = data;
}


/*Modbus ��ʼ��ʱ�趨������*/
void ModbusData_Init(void)
{
	RecordIn(M_NUB, 0);
	RecordIn(M_TIMES, g_sys_para.s_rammcnt);
	RecordIn(M_THIGH, g_sys_para.s_sethighcm);
	RecordIn(M_DEEP, 0);
	RecordIn(M_SONGTU, g_sys_para.s_feedtims);
	RecordIn(M_STATE, 0);
	RecordIn(M_ERR, 0);
	RecordIn(M_PERCNT, g_sys_para.s_numchi);
	RecordIn(M_ZHOU, g_sys_para.s_pericm);
	RecordIn(M_TLIAO, g_sys_para.s_feedtims);
	RecordIn(M_CCNUB, g_sys_para.s_rammcnt);
	RecordIn(M_TICHUI, g_sys_para.s_sethighcm);
	RecordIn(M_LIHE, g_sys_para.s_setlihecm);
	RecordIn(M_HPROT, g_sys_para.s_hprot);
	RecordIn(M_ACCRE,100);			/*��Ȩ��*/
	RecordIn(M_MAXH,g_sys_para.s_maxhigh); /*���򴸸߶�*/
	
	s_record.nub = 0;
	s_record.deepth = 0;
	s_record.high = 0;
	s_record.tim = 0;
	s_record.cnt = 0;
}
/*ϵͳ������Ϣ�ϴ�*/
void ModbusData_flash(void)
{
	RecordIn(M_PERCNT,g_GuiData.g_num);
	RecordIn(M_ZHOU,g_GuiData.g_Zhoucm);
	RecordIn(M_TLIAO,g_GuiData.g_ts);
	RecordIn(M_CCNUB,g_GuiData.g_hcnt);
	RecordIn(M_TIMES,g_GuiData.g_hcnt);
	RecordIn(M_TICHUI,g_GuiData.g_sethighcm);
	RecordIn(M_LIHE,g_GuiData.g_lihe);
	RecordIn(M_HPROT,g_GuiData.g_HighOvercm);
	RecordIn(M_MAXH,g_GuiData.g_Maxhigh); /*���򴸸߶�*/
}

/*�߶���Ϣ�������Ϣ��ʾ����������*/
void ModbusData_Show(void)
{
	RecordIn(M_DEEP,g_GuiData.g_nhigh);
	RecordIn(M_THIGH,g_GuiData.g_sethighcm);
	RecordIn(M_KS, s_hang.last_downnum);						/*��ʾ׮�����*/
	RecordIn(M_ERR,g_st_SigData.m_errnum);
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
		if((usRegHoldingBuf[M_TICHUI] <= g_sys_para.s_maxhigh) && (usRegHoldingBuf[M_TICHUI] > MINSET_HIGH))
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
	/*�趨���򴸸߶�   add 2020.3.11*/
	if(usRegHoldingBuf[M_MAXH] != g_GuiData.g_Maxhigh)
	{
		if(usRegHoldingBuf[M_MAXH] <= MAXSET_HIGH)
		{
			g_GuiData.g_Maxhigh = usRegHoldingBuf[M_MAXH];
			g_GuiData.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_MAXH] = g_GuiData.g_Maxhigh;
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
			RecordIn(M_STATE, 0);
			break;
		case S_PULSE:
		case S_TICHUI:		/*�ᴸ*/
			LED_BIT_SET(SIG_STICHUI);
			RecordIn(M_STATE, 3);
			break;
		case S_CHECK_UPSTOP:
			LED_BIT_SET(SIG_SZHUCHUI);
			RecordIn(M_STATE, 4);
			break;
		case S_XIALIAO:		/*����*/
			LED_BIT_SET(SIG_STU);
			RecordIn(M_STATE, 5);
			break;
		case S_DELAY2:
		case S_LIUF:		/*���*/
			LED_BIT_SET(SIG_SLIUF);
			RecordIn(M_STATE, 1);
			break;
		case S_DACHUI:		/*����*/
			LED_BIT_SET(SIG_SHANGTU);
			RecordIn(M_STATE, 2);
			break;
		default:break;
	}
}


/************************************************************
  * @brief   �ⲿ������⣬�㶯�ᴸ  �Զ���ť  ��ϲ���  ɲ������
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    ��÷��ڲ���ϵͳ��ϵͳʱ�ӻص�������ִ��
  ***********************************************************/
static int liu_flg = 0;      //�������ñ�Ҫ����ִֻ��1��
static int sha_flg = 0;
void EXT_BUTTON_CHK(void)
{
	static uint16_t b_Dd;
	static uint8_t k_Cluch;
	static uint8_t k_Brk;
	static uint16_t b_stop;
	static uint8_t b_liu;
	
	b_Dd <<= 1;
	k_Cluch <<= 1;
	k_Brk <<= 1;
	b_stop <<= 1;
	b_liu <<= 1;
	
	if(KEY_DD == GPIO_PIN_SET)
		b_Dd |= 0x01;
	if(KEY_TLH == GPIO_PIN_RESET)
		k_Cluch |= 0x01;
	if(KEY_TSC == GPIO_PIN_RESET)
		k_Brk |= 0x01;
	if(KEY_STOP == GPIO_PIN_SET)
		b_stop |= 0x01;
	if(KEY_LIU == GPIO_PIN_SET)
		b_liu |= 0x01;
		
	if(b_stop > 0xFFF)
	{
		g_st_SigData.m_ghalt = 0;
	}
	else
	{
		g_st_SigData.m_ghalt = 1;
	}
	if((g_st_SigData.m_Mode == MOD_SIGACT) ||(g_st_SigData.m_Mode == MOD_AUTOTAMP)||g_st_SigData.m_errshow)    //����״̬�����������
		return;
	
	if(b_Dd > 0xFFF)
	{
		IOT_FUNC_ENTRY;
		if(g_st_SigData.m_Mode == MOD_FREE)
			g_st_SigData.m_Mode = MOD_MANUAL;
	}
	else
	{
		if(g_st_SigData.m_Mode == MOD_MANUAL)
			g_st_SigData.m_Mode = MOD_MANOFF;
	}
	
	if((k_Cluch == 0xFF) || (k_Brk == 0xFF)||(b_liu == 0xff))
	{
		if(g_st_SigData.m_Mode == MOD_FREE)
		{
			g_st_SigData.m_Mode = MOD_TST;
		}
		else if(g_st_SigData.m_Mode == MOD_TST)   //ֻ�н������ģʽ�£������ֶ������������Զ�����ʱ�����
		{
			if(k_Cluch == 0xFF)
				G_LIHE(ACT_ON,0);
			else
				G_LIHE(ACT_OFF,0);
				
			if(k_Brk == 0xFF)
			{
				if(sha_flg <= 0)
				{
					G_SHACHE(ACT_OFF,0);
					sha_flg = 1;
					Log_e("ACT_OFF");
				}
			}
			else
			{
				if(sha_flg)
				{
					sha_flg = 0;
					G_SHACHE(ACT_ON,0);
					Log_e("ACT_ON");
				}
			}
			if(b_liu == 0xff)
			{
				if(liu_flg <= 0)
				{
					G_SHACHE(ACT_LIU,0);
					liu_flg = 1;
					Log_e("ACT_LIU");
				}
			}
			else
			{
				if(liu_flg > 0)
				{
					liu_flg = 0;
					G_SHACHE(ACT_ON,0);
					Log_e("ACT_ON");
				}
			}
		}
	}
	else
	{
		if(g_st_SigData.m_Mode == MOD_TST)
		{
			g_st_SigData.m_Mode = MOD_FREE;
			G_LIHE(ACT_OFF,0);G_SHACHE(ACT_ON,0);
			sha_flg = 0;liu_flg = 0;
		}
	}
}

/*������׮�Ķ���*/
static enum {
	e_STEP_READY,
	e_STEP_STUDY,
	e_STEP_PULL,
	e_STEP_DOWN,
}s_SigActStep;         		/*������׮ѭ���Ĺ���״̬*/

static uint16_t TampStep;   /*�����������̵�ѭ��״̬*/

#define SIGACT_READY    s_SigActStep = e_STEP_READY
#define TAMP_READY		TampStep = S_IDLE;

extern void SetSaveFlag(void);   //ѧϰ��ɺ�����ݱ���


/************************************************************
  * @brief   ����������̣����ᴸ�����   �Ƕ���
  * @param   mode : 0 ��һ���ᴸΪѧϰģʽ
                    1 ֱ������
  * @return  ����״̬
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    �����ᴸ�ʹ򴸵Ĺ��̣�
             �ᴸ�������� ERR_SIG_REACHUP
			 �򴸽������� ERR_SIG_REACHDW
			 ������������ ERR_SIG_OK
			 ����Ϊ����ģʽ���ⲿ������
			 ����������
  ***********************************************************/
int SingleAct(int mode)
{
	ERR_SIG ret = ERR_SIG_OK;
	
	switch(s_SigActStep)
	{
	case e_STEP_READY:
		LED_BIT_SET(SIG_TICHUI);
		LED_BIT_CLR(SIG_FANGCHUI);
		LED_BIT_CLR(SIG_SHACHE);
		if(mode)
			s_SigActStep = e_STEP_PULL;
		else
			s_SigActStep = e_STEP_STUDY;	
			
		Sig_ResetSta();   //��ʼ����һ������״̬
		break;
	case e_STEP_STUDY:
		ret = Sig_StudyUp();
		if(ret == ERR_SIG_REACHUP)
		{
			s_SigActStep = e_STEP_DOWN;
			Sig_ResetSta();
			LED_BIT_SET(SIG_FANGCHUI);
			LED_BIT_CLR(SIG_TICHUI);
			SetSaveFlag();
		}
		break;
	case e_STEP_PULL:
		ret = Sig_TakeUp();
		if(ret == ERR_SIG_REACHUP)
		{
			s_SigActStep = e_STEP_DOWN;
			Sig_ResetSta();
			LED_BIT_SET(SIG_FANGCHUI);
			LED_BIT_CLR(SIG_TICHUI);
		}
		break;
	case e_STEP_DOWN:
		ret = Sig_LandDw();
		if(ret == ERR_SIG_REACHDW)   //����״̬λ���ף�����Ҫ������ת
		{
			s_SigActStep = e_STEP_PULL;
			Sig_ResetSta();
			LED_BIT_SET(SIG_TICHUI);
			LED_BIT_CLR(SIG_FANGCHUI);
			
		}
		break;
	 default:ret = ERR_SIG_SOFT;s_SigActStep = e_STEP_READY;
	    Log_e("ERR_SIG_SOFT");
	    break;
	}
	return ret;
}
  

static SYS_STA ErrChg(int sigerr)
{
	SYS_STA sret = ERR_NONE;
	
	switch(sigerr)
	{
		case ERR_SIG_PULLUP :sret|= ERR_LS;break;
		case ERR_SIG_ENCODER:sret|= ERR_TT;break;
		case ERR_SIG_CUR    :sret|= ERR_CT;break;
		case ERR_SIG_CLING  :sret|= ERR_NC;break;
		case ERR_SIG_BRAKE  :sret|= ERR_KC;break;
		case ERR_SIG_TIMOUT :sret|= ERR_CS;break;
		default:sret|= ERR_CHAO;
			break;
	}
	
	return sret;
}


/************************************************************
  * @brief   ����ִ������  �����Զ���׮ �� �Զ�����
  * @param   mode : 0 ��һ���ᴸΪѧϰģʽ
                    1 ֱ������
  * @return  ����״̬
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    
  ***********************************************************/
SYS_STA ServicesLoop(void)
{
	SYS_STA ret = ERR_NONE;
	
	EXT_BUTTON_CHK();
	
	GetLiveData();						 /*�õ� ����λ�� �� �ٶȵĶ�̬��Ϣ�����ں���ļ���*/
	
	if(g_st_SigData.m_errshow > 0)       /*Err Check  ʹ�� m_errshow  �� m_errnumʵ�ֹ������������Դ˴�����*/
	{
		C_STOP();
		g_st_SigData.m_Mode = MOD_FREE;
		if(g_st_SigData.m_errshow == ERR_HALT)
		{
			if(g_st_SigData.m_ghalt == 0)
				g_st_SigData.m_errshow = ERR_NONE;   //��ͣ״̬������
		}
		Halt_Stop(0);			//����ʱ��ֱ��ͣ��
		return 0;
	}
	C_OK();
	if(g_st_SigData.m_ghalt)
	{
		Halt_Stop(0);
		g_st_SigData.m_errshow = ERR_HALT; 
		g_st_SigData.m_Mode = MOD_FREE;
		return 0;
	}

	switch(g_st_SigData.m_Mode)
	{
		case MOD_SIGACT:     		//�Զ���ģʽ 
		{
		    int sig_err;
			
			sig_err = SingleAct(g_st_SigData.m_eibutton);
			if(sig_err > ERR_SIG_REACHDW)
			{
				g_st_SigData.m_Mode = MOD_FREE;		// jump out
				
				ret |= ErrChg(sig_err);
				
			}
			if(g_st_SigData.m_ghalt)
			{
				ret |= ERR_HALT;
				g_st_SigData.m_Mode = MOD_FREE;
			}
		    break;	
         }			
	  case MOD_AUTOTAMP:								// ȫ�Զ�����ģʽ
			ret = hangtu(&TampStep);        			/*��ѯ�޶���*/
			LedSta_Show(TampStep);						/*Led״̬��ʾ*/
			if(g_st_SigData.m_ghalt)					/*�����쳣������ͣ��*/
			{
				ret |= ERR_HALT;
				g_st_SigData.m_Mode = MOD_FREE;
			}
		  break;	
	  case MOD_FREE:
		  TAMP_READY;
		  SIGACT_READY; 
#ifdef USE_LIHE_PWM
	extern int gLiheRatio;
	gLiheRatio = LIHE_TINY;
#endif
		  G_LIHE(ACT_OFF,0);
		  G_SHACHE(ACT_ON,0);
          C_DISCTR();									/* �Ĵ��� ȡ�� ���  Terry 2019.07.04*/
		  LED_BIT_CLR(SIG_TICHUI);
		  LED_BIT_SET(SIG_SHACHE);
		  g_st_SigData.m_manualflg = 0;
		  break;
	  case MOD_MANUAL:										//�ֶ�ģʽ
          if(g_st_SigData.m_manualflg == 0)
		  {
#ifdef USE_LIHE_PWM
			  gLiheRatio = LIHE_BIG;
#endif					
		      LED_BIT_SET(SIG_TICHUI);
		      G_LIHE(ACT_ON,0);
		      G_SHACHE(ACT_OFF,SHACHEDLY);
			  g_st_SigData.m_manualflg = 1;
		  }
			  
		  break;
	  case MOD_MANOFF:
		  G_LIHE(ACT_OFF,LIHEDLY);
		  G_SHACHE(ACT_ON,0);
		  DELAYMS(200);					/*�ȴ�ɲ���ȶ���*/
	      g_st_SigData.m_Mode = MOD_FREE;
		  break;
	  case MOD_DOWN:						//�·�ģʽ  2018.9.7  С����δʹ��
		  break;
		  
	  case MOD_TST:
			break;																																			//Terry add 2017.11.16
	  default:	
			g_st_SigData.m_Mode = MOD_FREE;
			Halt_Stop(100);
		  break;
	}

	if(g_st_SigData.m_Mode > MOD_DOWN)
		g_st_SigData.m_Mode = MOD_FREE;
		

		
	if(ret > ERR_NONE)	
	{
		g_st_SigData.m_errshow = ret; 
		Halt_Stop(100);
	}

  return ret;
}

/************************************************************
  * @brief   �ᴸ�����󣬼�鴸���Ƿ�ס����ʱ�ٶ�Ӧ�� -20 - 20
  * @param   pos: ��ǰ����λ��
  *          speed: ��ǰ���
  * @return  1 :������ס
  *          2���嶥����
  *          3: ��ʱ
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    ��÷��ڲ���ϵͳ��ϵͳʱ�ӻص�������ִ��
  ***********************************************************/
static int Check_Stop(int pos,int speed)
{
	static int32_t stop_cnt,power_L,power_H;
	static int32_t block_cnt;
	static uint32_t now_tim;
	int ret  = 0;
	
	switch(s_hang.stop_sta)
	{
		case 0:
			stop_cnt = 0;
			block_cnt = 0;
			s_hang.stop_sta = 1;
			now_tim = GET_TICK_MS();
			power_L = Per2Power(70);
			power_H = Per2Power(160);
			break;
		case 1:
			/******************** �ᴸͣ�����   ****************/
			if((speed < 20) && ((speed > -20)) && (g_st_SigData.m_Power < power_L))
			{
				stop_cnt++;
				if(stop_cnt > 500/HANG_TICK)
				{
					s_hang.stop_sta = 2;			/*�ɹ���ס*/
				}
			}
			else
			{
				if(stop_cnt > 3)
					stop_cnt -= 3;
			}
			/**********************�������***********************/
			if(g_st_SigData.m_Power > power_H)
			{
				block_cnt++;
			}
			else
			{
				if(block_cnt > 1)
					block_cnt--;
			}
			if(block_cnt > 1000/HANG_TICK)
			{
				ret = 2;
				Log_e("Blocked");
			}
			
			/**********************��ʱ���******************************/
			if((GET_MS_DIFF(now_tim) > 4000) && (speed > 10))    //û��ɲס���ж��쳣
			{
				if(speed > 20)
					ret = 2;		/*�嶥����*/
				else
					ret = 3;
			}
			/*****************�����ĳ嶥�������Ᵽ��*****************************/
			if((pos > 150) && (pos < -150))			/*�嶥���������λ�߶�Ϊ 1.5��*/
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

/**/
/************************************************************
  * @brief   �߶���Ҫ��ʱ��������Ļ������Ҫ���������ô򴸸߶�
  * @param   pos   ��ǰ���ĸ߶�
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    2019���ɽ��̫ԭ���ؿͻ�Ҫ��
  ***********************************************************/
static int Get_Hcnt(int32_t pos)
{
	int cnt = 0;
	
	cnt = g_sys_para.s_rammcnt;	  /*ϵͳ�򴸴���*/
	
	/*��Ϊ 0 ʱ������*/
	if((g_GuiData.s_high1 == 0) && (g_GuiData.s_high2 == 0))
	{
		return cnt;
	}

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


/************************************************************
  * @brief   �Զ���������
  * @param   * pst ���ϼ��
  * @return  ״̬  
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    ȫ�Զ�����������
  ***********************************************************/
static SYS_STA hangtu(uint16_t * pst)
{
    SYS_STA ret = 0;
    int tmp;
	
    switch(*pst)
    {
        case S_IDLE:
            /*��¼��ǰ�߶� �����߶����� -> ��ʼ���  -> ��¼�����ʼ�߶�*/
            {
				s_hang.last_sta = S_IDLE;						/*��ǰת��״̬ �������*/
                s_hang.pvtimer = GET_TICK_MS();
				Enc_Clr_TotalCnt2();					/*�õ����˵ĸ߶�  0*/
                s_hang.overpow = 0;
                s_hang.speederr = 0;
				G_SHACHE(ACT_LIU,0); 
				
				s_hang.last_highnum = g_st_SigData.m_HeighRammCm;		/*���ʱ�ĸ߶�  Terry 2019.11.12 ֻ�����ж��½��߶�*/
				s_record.deepth = 0;
				*pst = S_DELAY2;                	/*�����*/
				Log_e("��ʼ���");
            }
            break;
        case S_PULSE:
                
                s_hang.top_sta = 0;
                G_LIHE(ACT_ON,0);
                G_SHACHE(ACT_OFF,SHACHEDLY);    /*��ʼ�ᴸ*/
                s_hang.pvtimer = GET_TICK_MS();
				s_hang.speederr = 0;
				s_hang.overpow = 0;
				s_hang.lowpow = 0;
				//���ºͷ��ͼ�¼
				s_record.nub++;
				RecordIn(M_ACT,1);
				RecordIn(M_NUB,s_record.nub);
				RecordIn(M_TIMES,s_record.cnt);
				RecordIn(M_THIGH,s_record.high);
				RecordIn(M_SONGTU,s_record.tim);
				*pst = S_TICHUI; 
            break;
		
        case S_TICHUI:
                if(Check_up(g_st_SigData.m_HeighRammCm) == 1) 								/*�ᴸ����������ɲ�����������*/  
                {
					if(GET_MS_DIFF(s_hang.pvtimer) > 1000)			/*������� 1 ��*/
					{
						G_LIHE(ACT_OFF,LIHEDLY);
						G_SHACHE(ACT_ON,0);        					/*����ɲ��*/      
						s_hang.pvtimer = GET_TICK_MS();
						STA_CLEAR(s_hang.stop_sta);
						*pst = S_CHECK_UPSTOP;
					}
                }
				/*****************ʧ�ٱ���  ̽ͷʧЧ����*********************************/
                if(g_st_SigData.m_SpeedCm < 10)						/*����ʱ���ٶ�С��10cm/s,��Ϊ�쳣*/
                    s_hang.speederr++;
                else
				{
					if(s_hang.speederr > 3)
						s_hang.speederr -= 3;
				}
                if(s_hang.speederr > 2800/HANG_TICK)
                {
                     ret = ERR_LS;
                }
                /**************************�������ر���*********************/
                tmp = Per2Power(180);	
                if(g_st_SigData.m_Power > tmp)
                    s_hang.overpow++;
                else
                    s_hang.overpow = 0;
                
                if(s_hang.overpow > (1500/ HANG_TICK))   /*����1.5���޷���������ʾ��������  2019.10.7*/
                {
					ret |= ERR_KC;
                } 
				/************************ʧ�ر���***************************/
				tmp = Per2Power(40);
				if(g_st_SigData.m_Power < tmp)
					s_hang.lowpow++;
				else
				{
					if(s_hang.lowpow > 3)
						s_hang.lowpow -= 3;
				}
				if(s_hang.overpow > 2000/HANG_TICK)
				{
					ret |= ERR_LS;
				}
				/**************************�ᴸ��ʱ����*********************/
				/***********************************************************/
            break;
        case S_CHECK_UPSTOP:
			/*�˴��������Ƿ��ɿ�*/
			tmp = Check_Stop(g_st_SigData.m_HeighRammCm,g_st_SigData.m_SpeedCm);
			if(tmp == 1)    /*���ɹ�*/
			{
				C_ENCTR();
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_XIALIAO;
			}
			else if(tmp > 1)
			{
				if(tmp == 2) 
				    ret |= ERR_TOP;		/*�嶥����*/
				else
					ret |= ERR_SC;  /*ɲ���쳣*/	
			}
            break;
        case S_XIALIAO:                         				/*  ����  */
			/*��ǰִ����Ų���*/
			if(GET_MS_DIFF(s_hang.pvtimer) > (g_sys_para.s_feedtims * 600 - 300))	
			{
				s_hang.last_highnum = g_st_SigData.m_HeighRammCm;	/*���ʱ�ĸ߶�  Terry 2019.7.5*/
                G_LIHE(ACT_OFF,0);
				G_SHACHE(ACT_LIU,0);                             //��ǰ׼�����
                *pst = S_DELAY2;
			}
            break;
        case S_DELAY2:                          					/*��ͨ��ʱ  �տ�ʼʱ����Ҫ��ʱ*/
			if((GET_MS_DIFF(s_hang.pvtimer) > g_sys_para.s_feedtims * 600) || (s_hang.last_sta == S_IDLE))
            {
                C_DISCTR();			/*�ر��Ĵ���*/
				s_hang.last_sta = S_DELAY2;
                s_hang.pvtimer = GET_TICK_MS();
				s_hang.liufang_sta = 0;
				Log_e("���");
				*pst = S_LIUF;
            }
            break;
        case S_LIUF:
            tmp = LiuChkDw(g_st_SigData.m_HeighRammCm,g_st_SigData.m_SpeedCm);   	/*���ʱ������Ƿ񵽵���*/
            if(tmp == HAS_REACH_DOWN)							                    /*�ж���ŵ�����*/
            {
                s_hang.dachui_cnt = g_sys_para.s_rammcnt;
				s_record.high = g_sys_para.s_sethighcm / 10;		/*�򴸸߶�  ��¼��λΪ���� Terry 2019.6.5*/
				s_record.tim = g_sys_para.s_feedtims;				/*����ʱ�� Terry 2019.6.5*/
				
                /*��¼��ǰ��ŵ����   */
                if(g_st_SigData.m_HeighRammCm > -40)  				/*�½���ȹ�С  �½�0.4��*/
                {
                    ret |= ERR_DOWN;  
					Log_e("��Ź�ǳ");
                }
                else
                {	//���´򴸴����ͼ�¼����������жϴ򴸴���
					s_record.cnt = s_hang.dachui_cnt =	Get_Hcnt(g_st_SigData.m_HeighRammCm);
                    s_hang.last_downnum = (int16_t)(g_st_SigData.m_HeighRammCm / 10);   				/*�����½���� ���� ����ֵ Terry 2019.7.5*/
					
					Log_e("��ųɹ�,׼����");
                    *pst = S_DACHUI;       										/*��ʼ�� */
					Sig_ResetSta();
					SIGACT_READY;	/*��׼��*/
                }
            }else if(tmp > HAS_REACH_DOWN)       											/*��Ŵ���*/
            {
                ret |= ERR_DOWN;
				Log_e("����쳣  %x",ret);
            }
            break;
        case S_DACHUI:
		    {
				int sig_err = 0;
				
				sig_err = SingleAct(1);                 /*�򴸲���*/
				if(sig_err > ERR_SIG_REACHDW)		    /*��������*/
				{
					g_st_SigData.m_Mode = MOD_FREE;
					Log_e("�򴸴���");
					ret |= ErrChg(sig_err);             /*���ش���״̬*/
				}
				else
				{
					if((sig_err == ERR_SIG_REACHUP) && (s_hang.dachui_cnt == g_sys_para.s_rammcnt))
					{
						g_st_SigData.m_Lihenew = g_st_SigData.m_Lihenew - 12;   /*��һ�δ򴸴���һ��*/
					}
	
				    if(s_SigActStep == e_STEP_PULL)
						RecordIn(M_ACT,1);
					else
						RecordIn(M_ACT,0);
						
					if(sig_err == ERR_SIG_REACHDW)
					{
						s_hang.dachui_cnt--;
						Log_e("�򴸵��� %d",s_hang.dachui_cnt);
						s_record.deepth = g_st_SigData.m_Ddeepcm;  
						if(s_hang.dachui_cnt < 1)
						{
							if(s_record.deepth > -50)			/*��ʾ��������  Terry 2019.10.18 */
							{
								g_st_SigData.m_Mode = MOD_FREE;	/*ֱ���˳�  2019.8.2*/
								G_SHACHE(ACT_ON,0);
								G_LIHE(ACT_OFF,0);
								*pst = S_IDLE;
							}
							else
							{
								*pst = S_PULSE;       			/*��һ���ᴸ����   ֱ���ᴸ*/
								Log_e("�ᴸ");
							}					
						}
					}
				}					
				break;
		    }
	  }
       /*�쳣�˳�ʱ��������ɲ�����ر����ʹ�(Ī����)*/
		if(ret)
		{
			*pst = S_IDLE; 
		}
    return ret;
}


/************************************************************
  * @brief   �������Ƿ񵽵�
  * @param   LPosCm ��ǰ���ĺ���λ��   LSpeedCm ��ǰ�ٶȣ������ٶ�����Ϊ��ֵ��
  * @return  HAS_REACH_DOWN    ��⵽����
  *          HAS_REACH_TIMOUT  ��ų�ʱ
  *          HAS_REACH_SPOVER  ��ų���
  *          0                 ���������
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    ��÷��ڲ���ϵͳ��ϵͳʱ�ӻص�������ִ��
  ***********************************************************/
static SYS_STA LiuChkDw(int LPosCm,int LSpeedCm)
{
    static uint32_t last_tim = 0;
    static uint32_t sure_cnt = 0;
	static uint32_t speed_over = 0;
    SYS_STA ret = 0;
	
    switch(s_hang.liufang_sta)
    {
        case 0:
            last_tim = GET_TICK_MS();
			sure_cnt = 0;
			speed_over = 0;
			s_hang.liufang_sta = 1;
            break;
        case 1:
            if(GET_MS_DIFF(last_tim) > 1300)       								/*1.3���  ��ʼ�ж��ٶ�   2000  2019.07.04*/
            {
                if((LSpeedCm < -10 ) && ((s_hang.last_highnum - LPosCm) > 80))  /*�����ж���ʼ�����*/
                {
                    s_hang.liufang_sta = 2;
                }
                if(GET_MS_DIFF(last_tim) > 6000)      							/*6 �볬ʱû�ж��������� 2019.8.2*/
                {
                    ret = HAS_REACH_TIMOUT;                       							/*��û���������*/
                    s_hang.liufang_sta = 3;
                }		
            }
            break;
        case 2:                                     	/*������һ��*/
            /*�ж��Ƿ񵽵���*/
            if(LSpeedCm > -7)
                sure_cnt++;
            else if(LSpeedCm < -8)						/*����Ϊ ��*/
            {
                sure_cnt = 0;                       	/*�������ж�  ������*/
				last_tim = GET_TICK_MS();
            }
            
            if(sure_cnt > 1400/HANG_TICK)            	/*ȷ���Ѿ�ֹͣ  1.4��  ԭ��0.8�� */
            {
				if(s_record.deepth + 450 > LPosCm)		/*���紸�������棬����4�ף��ͻᱨ��*/
				{
					ret = HAS_REACH_DOWN;
					s_hang.liufang_sta = 3;             /*���� �����ж�*/
				}
				else
				{
					sure_cnt = 0;							/*���¿�ʼ�ۻ�*/
					if(GET_MS_DIFF(last_tim) > 4000)		/*����ٵȴ�4��*/
					{
						ret = HAS_REACH_TIMOUT;             /*��û���������,û�����䵽�׵Ĺ��� 2019.11.16*/
						s_hang.liufang_sta = 3;
					}
				}
            } 
			/*���ٱ���*/
			if(LSpeedCm < -700)								/*�½��ٶȳ���6��/��ʱ������1�룬��������쳣���� 2019.12.08*/
			{
				speed_over++;
				if(speed_over > 1000/HANG_TICK)
				{
					ret = HAS_REACH_SPOVER;									/*�½��ٶȹ��챨��*/
				}
			}
			else
			{
				if(speed_over > 6)
					speed_over -= 4;
			}
			
			if(LPosCm < -3000)
			{
				ret = HAS_REACH_TIMOUT;     //�������������Ϊ30��
			}
			
            break;
        case 3:												/*����*/
            break;
        default:
            s_hang.liufang_sta = 2;
            break;
    }
    return ret;
}


/*������ʱ����*/
/*����1 ��⵽
  ����0 �޼�⵽
  ���� ����
*/
SYS_STA Check_up(int PosCm)
{
    SYS_STA ret = 0;
    
	 /*�ȴ�����������Ч   ���������������λ������Ч   ��Ĺ�ϵ*/
	if(PosCm > -15) 	/*-30����   Terry ��ʹ�ó嶥���ź�*/
	{
		ret = 1;		 /*���Ѿ�����Ԥ��λ��*/
	}
    return ret;
}


/************************************************************
  * @brief   ��ʼ�����Բ�������
  * @param   none
  * @return  none
  * @author  Terry
  * @date    2020.2.16
  * @version v1.0
  * @note    default task,��ʼ��ϵͳ���Բ���   2017.11.8
  ***********************************************************/
#include "EEPROM.h"
void sysattr_init(uint16_t flg)
{
	HAL_StatusTypeDef status;
	uint32_t ID;
	
	osDelay(100);
	/*Ĭ��ֵ*/
	g_sys_para.s_dir = 0;
	g_sys_para.s_feedtims = 5;                  	//����ʱ��  Terry 2019.6.6
	g_sys_para.s_numchi = 70;					//ÿȦ����
	
	if(flg == 0)
		g_sys_para.s_chickid = CHK_ID;
		
	g_sys_para.s_sethighcm = 300;
	g_sys_para.s_pericm = 106;
	g_sys_para.s_pnull = 50;
	g_sys_para.s_pfull = 500;
	g_st_SigData.m_Mode = MOD_FREE;
	g_sys_para.s_setlihecm = g_sys_para.s_sethighcm * 2 / 3;	/*ʹ��̽ͷʱ����ϵ� cm*/
	g_sys_para.s_mode = 0;							/*0:�Զ��� 1:ȫ�Զ�ǿ��*/
    g_sys_para.s_rammcnt = 6;         				/*�������� Terry 2019.5.21*/
	g_sys_para.s_hprot = 150;						/*Ĭ�ϸ߶ȱ������� Terry 2019.7.6*/
	g_sys_para.s_pset = 0;							/*У����Ч Terry 2019.7.9*/
	g_sys_para.s_maxhigh = 500;
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
			osDelay(100);
		}
	}
	Enc_Clr_TotalCnt1();
	/*���ɲ��������ʼ��*/
	G_LIHE(ACT_OFF,0);
    G_SHACHE(ACT_ON,0);         						//��ʼΪ��ɲ��        2018.12.24 Terry
    Enc_Set_Dir(g_sys_para.s_dir);						//��ʼ������������
	g_st_SigData.m_Mode = MOD_FREE;						//�ϵ�󱣳ֿ���ģʽ
	
	g_sys_para.s_setlihecm = g_sys_para.s_sethighcm * 5 / 6;     //��ʼ����ϸ߶�Ϊ�ᴸ�߶ȵ�5/6
}


/****************************************end Hangtu,c**********************************************/
