
#include "Hangtu.h"
#include "port.h"
#include "GUI.h"
#include "Encoder.h"
#include "u_log.h"


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

extern uint16_t led_sta;


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
	usRegHoldingBuf[M_TIMES] = sys_attr.s_cnt;
	usRegHoldingBuf[M_THIGH] = sys_attr.s_sethighcm;
	usRegHoldingBuf[M_DEEP] = 0;
	usRegHoldingBuf[M_SONGTU] = sys_attr.s_intval;
	usRegHoldingBuf[M_STATE] = 0;
	usRegHoldingBuf[M_ERR] = 0;
	
	usRegHoldingBuf[M_PERCNT] = sys_attr.s_numchi;
	usRegHoldingBuf[M_ZHOU] = sys_attr.s_zhou;
	usRegHoldingBuf[M_TLIAO] = sys_attr.s_intval;
	usRegHoldingBuf[M_CCNUB] = sys_attr.s_cnt;
	usRegHoldingBuf[M_TICHUI] = sys_attr.s_sethighcm;
	usRegHoldingBuf[M_LIHE] = sys_attr.s_hlihe;
	usRegHoldingBuf[M_HPROT] = sys_attr.s_hprot;

	
	/*��Ȩ��*/
	usRegHoldingBuf[M_ACCRE] = 100;
	
	s_record.nub = 0;
	s_record.deepth = 0;
	s_record.high = 0;
	s_record.tim = 0;
	s_record.cnt = 0;
}
extern volatile struct GUI_DATA	g_showdata;
void ModbusData_flash(void)
{
	usRegHoldingBuf[M_PERCNT] = g_showdata.g_num;
	usRegHoldingBuf[M_ZHOU] = g_showdata.g_Zhoucm;
	usRegHoldingBuf[M_TLIAO] = g_showdata.g_ts;
	usRegHoldingBuf[M_CCNUB] = g_showdata.g_hcnt;
	usRegHoldingBuf[M_TIMES] = g_showdata.g_hcnt;
	
	usRegHoldingBuf[M_TICHUI] = g_showdata.g_sethighcm;
	usRegHoldingBuf[M_LIHE] = g_showdata.g_lihe;
	usRegHoldingBuf[M_HPROT] = g_showdata.g_HighOvercm;
}

void ModbusData_Show(void)
{
	usRegHoldingBuf[M_DEEP] = (int16_t)g_showdata.g_nhigh;
	usRegHoldingBuf[M_THIGH] = (int16_t)g_showdata.g_sethighcm;
	usRegHoldingBuf[M_KS] = s_hang.last_downnum;						/*��ʾ׮�����*/
	usRegHoldingBuf[M_ERR] = g_errnum;
}

/*�ⲿ�����趨 ���¼��   ѭ��*/
extern uint32_t savecnt;

void ModbusData_Chk(void)
{
	/*ÿȦ����*/
	if(usRegHoldingBuf[M_PERCNT] != g_showdata.g_num )
	{
		if((usRegHoldingBuf[M_PERCNT] < 300) && (usRegHoldingBuf[M_PERCNT] > 10))
		{
			g_showdata.g_num  = usRegHoldingBuf[M_PERCNT];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_PERCNT] = g_showdata.g_num;
		}
	}
	/*��Ͳ�ܳ�*/
	if(usRegHoldingBuf[M_ZHOU] != g_showdata.g_Zhoucm)
	{
		if((usRegHoldingBuf[M_ZHOU] < 300) && (usRegHoldingBuf[M_ZHOU] > 20))
		{
			g_showdata.g_Zhoucm = usRegHoldingBuf[M_ZHOU];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_ZHOU] = g_showdata.g_Zhoucm;
		}
	}
	/*���ϵ�ʱ��*/
	if(usRegHoldingBuf[M_TLIAO] != g_showdata.g_ts)
	{
		if((usRegHoldingBuf[M_TLIAO] < 16) && (usRegHoldingBuf[M_TLIAO] > 1))
		{
			g_showdata.g_ts = usRegHoldingBuf[M_TLIAO];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TLIAO] = g_showdata.g_ts;
		}
	}
	/*�趨�򴸵Ĵ���*/
	if(usRegHoldingBuf[M_CCNUB] != g_showdata.g_hcnt)
	{
		if((usRegHoldingBuf[M_CCNUB] < 11) && (usRegHoldingBuf[M_CCNUB] > 2))
		{
			g_showdata.g_hcnt = usRegHoldingBuf[M_CCNUB];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_CCNUB] = g_showdata.g_hcnt;
		}
	}
	/*�趨�ᴸ�߶�*/
	if(usRegHoldingBuf[M_TICHUI] != g_showdata.g_sethighcm)
	{
		if((usRegHoldingBuf[M_TICHUI] <= MAXSET_HIGH) && (usRegHoldingBuf[M_TICHUI] > MINSET_HIGH))
		{
			g_showdata.g_sethighcm = usRegHoldingBuf[M_TICHUI];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_TICHUI] = g_showdata.g_sethighcm;
		}
	}
	/*�趨��ϵ��λ��*/
	if(usRegHoldingBuf[M_LIHE] != g_showdata.g_lihe)
	{
		if((usRegHoldingBuf[M_LIHE] <= (g_showdata.g_sethighcm - 20)))
		{
			g_showdata.g_lihe = usRegHoldingBuf[M_LIHE];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_LIHE] = g_showdata.g_lihe;
		}
	}
	
	/*�趨�����߶ȱ���ֵ*/
	if(usRegHoldingBuf[M_HPROT] != g_showdata.g_HighOvercm)
	{
		if(usRegHoldingBuf[M_HPROT] <= 1000)
		{
			g_showdata.g_HighOvercm = usRegHoldingBuf[M_HPROT];
			g_showdata.g_HasChanged = 2;
		}
		else
		{
			usRegHoldingBuf[M_HPROT] = g_showdata.g_HighOvercm;
		}
	}
	
		/*�ֶδ򴸴���*/
	g_showdata.s_high1 = usRegHoldingBuf[M_HIGH1];
	g_showdata.s_cnt1 = usRegHoldingBuf[M_CNT1];
	g_showdata.s_high2 = usRegHoldingBuf[M_HIGH2];
	g_showdata.s_cnt2 = usRegHoldingBuf[M_CNT2];
}

/*Led״ָ̬ʾ��  ����ģʽ��*/
void LedSta_Show(uint8_t ledsta)
{
	led_sta &= ~SIG_SALL;					/*Led������*/
	switch(ledsta)
	{
		case S_IDLE:		/*����*/
			usRegHoldingBuf[M_STATE] = 0;
			break;
		case S_PULSE:
		case S_TICHUI:		/*�ᴸ*/
			led_sta |= SIG_STICHUI;
			usRegHoldingBuf[M_STATE] = 3;
			break;
		case S_CHECK_UPSTOP:
			led_sta |= SIG_SZHUCHUI;
			usRegHoldingBuf[M_STATE] = 4;
			break;
		case S_XIALIAO:		/*����*/
			led_sta |= SIG_STU;
			usRegHoldingBuf[M_STATE] = 5;
			break;
		case S_DELAY2:
		case S_LIUF:		/*���*/
			led_sta |= SIG_SLIUF;
			usRegHoldingBuf[M_STATE] = 1;
			break;
		case S_DACHUI:		/*����*/
			led_sta |= SIG_SHANGTU;
			usRegHoldingBuf[M_STATE] = 2;
			break;
		default:break;
	}
}


/*���Ƶ�������*/
extern uint32_t g_erract;    
SYS_STA services(void)
{
	static	uint8_t step,down;
	
	SYS_STA ret;
	ret = ERR_NONE;
	
	if(g_erract | g_errnum)				// �����Ϻ�ɲ���Ƿ�����
	{
	  G_LIHE(ACT_OFF,0);
	  G_SHACHE(ACT_ON,0);
	  Log_e("err -- g_erract | g_errnum");
	  
	  DELAYMS(ERRDLY);
	  return g_erract;
	}
	
	if(!((sys_fbsta & FB_24VOK) && (sys_fbsta & FB_RUN)))	//������״̬������
	{
		G_LIHE(ACT_OFF,0);
		G_SHACHE(ACT_ON,0);
		
		Log_e("FB_24VOK | FB_RUN");
		DELAYMS(ERRDLY);
		
		

		if(!(sys_fbsta & FB_24VOK))
			return ERR_PW;
		if(!(sys_fbsta & FB_RUN))
			return ERR_HALT;
	}

  switch(sys_attr.s_zidong)
  {
	  case MOD_TT2:                                         /*��̽ͷ��һ������*/
	  {
		  if(step == 0)
		  {
			  liheupdate();
			  led_sta |= SIG_TICHUI;
			  led_sta &= ~SIG_FANGCHUI;
			  ret = starttaking();          				/*������ѧϰ*/
			  step = 1;
		  }
		  else
          {
			  led_sta |= SIG_TICHUI;
			  led_sta &= ~SIG_FANGCHUI;
			  ret = takeup();			  
              Debug("up err %x\r\n",ret);
          }
		  
		  if(ret == ERR_NONE)
          {
			  led_sta &= ~SIG_TICHUI;
			  led_sta |= SIG_FANGCHUI;
              ret = putdown(0);	  
              Debug("down err %d\r\n",ret);
          }
		  
		  if(ret != ERR_NONE)
		  {
			 sys_attr.s_zidong = MOD_FREE; 
			 step = 0;
		  }
		  if(g_halt != 0)
		  {
			  g_halt = 0;
		  }
		  break;
	  }		
	  
	  case MOD_ZTT2:			//��������	  ����ģʽ
			ret = hangtu(&step);        			/*��ѯ�Ӷ���*/
			/* LEDǿ��������ʾ */
			LedSta_Show(step);						/*Led״̬��ʾ*/
		  break;	
	  case MOD_FREE:
		  step = 0;
		  down = 0;
		  G_SHACHE(ACT_ON,0);
          C_DISCTR();					/* �Ĵ��� ȡ��   Terry 2019.07.04*/
		  led_sta &= ~SIG_TICHUI;
		  break;
	  case MOD_MAN:			//�ֶ�ģʽ
		  if(step == 0)
		  {
		      led_sta |= SIG_TICHUI;
			  G_LIHE(ACT_ON,0);
			  G_SHACHE(ACT_OFF,SHACHEDLY);
			  step = 1;
		  }
		  break;
	  case MOD_MANOFF:
		  G_LIHE(ACT_OFF,LIHEDLY);
		  G_SHACHE(ACT_ON,0);
		  DELAYMS(200);					/*�ȴ�ɲ���ȶ���*/
	      sys_attr.s_zidong = MOD_FREE;
		  break;
	  case MOD_DOWN:						//�·�ģʽ  2018.9.7  С����δʹ��
		  if(down == 0)
		  {
			  G_LIHE(ACT_OFF,LIHEDLY);
			  G_SHACHE(ACT_OFF,0);
			  down = 1;
		  }
		  break;
		  
	  case MOD_TST:
			break;																																			//Terry add 2017.11.16
	  default:	
			step = 0;
			sys_attr.s_zidong = MOD_FREE;
		  break;
	}
	
		
	if(sys_attr.s_zidong > MOD_DOWN)
		sys_attr.s_zidong = MOD_FREE;
		
	if((sys_attr.s_zidong == MOD_FREE) || (sys_attr.s_zidong == MOD_TST) ||(sys_attr.s_zidong == MOD_DOWN))
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
			if((LSpeedCm < 20) && ((LSpeedCm > -20)) && (sys_stadata.m_power.Speed < power))
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
	
	cnt = sys_attr.s_cnt;
	
	/*��Ϊ 0 ʱ������*/

	if((g_showdata.s_high1 == 0) && (g_showdata.s_high2 < 0))
	{
		if(pos > g_showdata.s_high2 || (g_showdata.s_cnt2 > 0))
			cnt = g_showdata.s_cnt2;
	}
	else if((g_showdata.s_high1 < 0) && (g_showdata.s_high2 == 0))
	{
		if(pos > g_showdata.s_high1 || (g_showdata.s_cnt1 > 0))
			cnt = g_showdata.s_cnt1;
	}
	else if(g_showdata.s_high1 > g_showdata.s_high2)
	{
		if((pos > g_showdata.s_high1) && (g_showdata.s_cnt1 > 0))
		{
			cnt = g_showdata.s_cnt1;
		}
		else if((pos > g_showdata.s_high2) && (g_showdata.s_cnt2 > 0))
		{
			cnt = g_showdata.s_cnt2;
		}
	}
	else if(g_showdata.s_high2 > g_showdata.s_high1)
	{
		if((pos > g_showdata.s_high2) && (g_showdata.s_cnt2 > 0))
		{
			cnt = g_showdata.s_cnt2;
		}
		else if((pos > g_showdata.s_high1) && (g_showdata.s_cnt1 > 0))
		{
			cnt = g_showdata.s_cnt1;
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
                if(sys_stadata.m_power.Speed > tmp)
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
				if(sys_stadata.m_power.Speed < tmp)
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
			if(GET_MS_DIFF(s_hang.pvtimer) > (sys_attr.s_intval * 600 - 200))	
			{
				s_hang.last_highnum = GetEncoderLen2Cm();	/*���ʱ�ĸ߶�  Terry 2019.7.5*/
                G_LIHE(ACT_OFF,0);
				G_SHACHE(ACT_LIU,0); 
                *pst = S_DELAY2;
                Debug("wait2 1s\r\n");
			}
            break;
        case S_DELAY2:                          				/*��ͨ��ʱ  �տ�ʼʱ����Ҫ��ʱ*/
			if((GET_MS_DIFF(s_hang.pvtimer) > sys_attr.s_intval * 600) || (stlast == S_IDLE))
            {
                C_DISCTR();			/*�ر��Ĵ���*/
				stlast = S_DELAY2;
				
                s_hang.pvtimer = GET_TICK_MS();
                *pst = S_LIUF;
                Debug("finish xia liao  %ds\r\n",sys_attr.s_intval);
				s_hang.liufang_sta = 0;
            }
            break;
        case S_LIUF:
			/*�½����׼��*/
			
            tmp = Checkdown();   
			
            if(tmp == 1)			/*��⵽������*/
            {
                s_hang.dachui_cnt = sys_attr.s_cnt;
				s_record.high = sys_attr.s_sethighcm / 10;		/*�򴸸߶� Terry 2019.6.5*/
				s_record.tim = sys_attr.s_intval;				/*����ʱ�� Terry 2019.6.5*/
				
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
						sys_attr.s_zidong = MOD_FREE;	/*ֱ���˳�  2019.8.2*/
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
		sys_attr.s_zidong = MOD_FREE; 
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

