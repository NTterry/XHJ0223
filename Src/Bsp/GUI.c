


#include "GUI.h"
#include "Action.h"
#include "EEPROM.h"
#include "Hangtu.h"
#include "u_log.h"
#include "keyboard.h"
#include "SingleAct.h"

#define VERSION     19     					/*�汾�� C97  2019.9.17*/

extern struct SYSATTR g_sys_para;				/* ϵͳ����  */
extern void W2Buff(void);
extern void HT1632_Init(void);
extern struct SIG_ACT_DATA g_st_SigData;



uint32_t g_errshow;	/*������*/
volatile struct GUI_DATA	g_GuiData;




//uint8_t g_stopflg = 0;
static uint32_t s_ht1632_test = 0;				//����ģʽ
static int8_t s_liheblink,s_highblink;  	/*�����˸��־  �߶���˸��־*/
static uint32_t s_savecnt = 0;




void GUI_showdata(void);
void savedatas(void);
int16_t get_errnum(uint32_t err);
void clear_err(uint32_t *perr);
void CmdProc(void);


void SetSaveFlag(void)
{
	s_savecnt = 10;
}

void GuiDataUpdate(void)
{
	if(g_sys_para.s_setlihecm != g_GuiData.g_lihe)
	{
		g_sys_para.s_setlihecm = g_GuiData.g_lihe;


//		liheupdate();		//������ϲ���ֵ
		s_savecnt = 20;
	}

	if(g_GuiData.g_sethighcm != g_sys_para.s_sethighcm)		//���¸߶�
	{
		g_sys_para.s_sethighcm = g_GuiData.g_sethighcm;
//		liheupdate();		//������ϲ���ֵ
		s_savecnt = 20;
	}
	/*ÿȦ����*/
	if(g_GuiData.g_num != g_sys_para.s_numchi)
	{
		g_sys_para.s_numchi = g_GuiData.g_num;
//		liheupdate();		//������ϲ���ֵ
		s_savecnt = 20;
	}
	/*��˫��ģʽ*/
	if(g_GuiData.g_mode != g_sys_para.s_mode)
	{
		g_sys_para.s_mode = g_GuiData.g_mode;
		s_savecnt = 20;
	}
	/*˫����ʱ��*/
	if(g_GuiData.g_ts != g_sys_para.s_feedtims)
	{
		g_sys_para.s_feedtims = g_GuiData.g_ts;
		s_savecnt = 20;
	}
	/*��������    Terry 2019.5.21*/
	if(g_GuiData.g_hcnt != g_sys_para.s_rammcnt)
	{
		g_sys_para.s_rammcnt = g_GuiData.g_hcnt;
		s_savecnt = 20;
	}

	/*�ܳ�  ���� */
	if(g_GuiData.g_Zhoucm != g_sys_para.s_pericm)
	{
		g_sys_para.s_pericm = g_GuiData.g_Zhoucm;
		s_savecnt = 20;
	}
	/*Terry add 2019.7.6 ���õ��������޵ĸ߶�*/
	if(g_GuiData.g_HighOvercm != g_sys_para.s_hprot)
	{
		g_sys_para.s_hprot = g_GuiData.g_HighOvercm;
		s_savecnt = 20;
	}
	/*��������*/
	if(g_GuiData.g_index == SHOW_BACKSET)
		s_savecnt = 10;
}


//extern int8_t Get_Fbsignal(uint8_t MASK);
extern struct Locationcm user_encoder;   

void Task_GUI_Function(void) 
{
	switch(g_GuiData.g_index)
	{
		case SHOW_NONE:
			if(s_highblink)
			{
				g_GuiData.g_show = g_GuiData.g_sethighcm / 10;
			}
			else if(s_liheblink)
			{							
				g_GuiData.g_show = g_GuiData.g_lihe/10;
			}
			else	
			{
				if(g_st_SigData.m_Mode == MOD_SIGACT)
				{
					g_GuiData.g_show = g_st_SigData.m_HeightShowCm / 10;
					g_GuiData.g_nhigh = g_GuiData.g_show;							/*����*/
				}
				else
				{
					g_GuiData.g_show = g_st_SigData.m_HeighRammCm / 10;				 //��λ 0.1��
					g_GuiData.g_nhigh = g_GuiData.g_show;							/*����*/
					g_GuiData.g_show = g_GuiData.g_show / 10;		/*����ʱ ������ֻ����ʾ�� Terry 2019.7.5*/
				}
			}
			break;
		case SHOW_TTC:g_GuiData.g_show = g_GuiData.g_num;break;
		case SHOW_ZHOU:g_GuiData.g_show = g_GuiData.g_Zhoucm;break;
		case SHOW_TS:g_GuiData.g_show = g_GuiData.g_ts;break;
        case SHOW_CNT:g_GuiData.g_show = g_GuiData.g_hcnt;break;     /*�򴸵Ĵ���*/
		case SHOW_BACKSET:g_GuiData.g_show = g_GuiData.g_pset;break;
		default:break;
	}
	
	
	if(g_GuiData.g_HasChanged == 0)
		ModbusData_Chk();
	else if(g_GuiData.g_HasChanged == 1)
		ModbusData_flash();
		
	ModbusData_Show();
	/*��ʼд�������*/
	if(g_GuiData.g_HasChanged)
	{
		g_GuiData.g_HasChanged = 0;
		GuiDataUpdate();
	}
	// show normal or show Test
	if(s_ht1632_test == 0)
	{
		GUI_showdata();
		W2Buff();
	}
	else
	{
		s_ht1632_test--;
		if(s_ht1632_test > 60)
			showtest(0);
		else if(s_ht1632_test > 40)
			showtest(1);
		else if(s_ht1632_test > 20)
			showtest(2);
		else
		{
			
			s_ht1632_test = 0;
		}
	}
	
	show_brush();
	savedatas();
	/*�޴����Ҽ�ͣδ����ʱ����ͣOK���Ͽ�*/
	if((g_st_SigData.m_errshow) || g_halt)
	{
		C_STOP();
	}
	else
		C_OK();
}

extern volatile uint8_t sys_fbsta;	
extern void ModbusData_flash(void);
void savedatas(void)
{
//	HAL_StatusTypeDef status;
	
//	status = HAL_OK;
	if(s_savecnt > 0)
		s_savecnt--;
	if(s_savecnt == 1)
	{
		if(g_sys_para.s_dir > 1)
			g_sys_para.s_dir = 1;
//		if((sys_fbsta & FB_24VOK))
//		{
			EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
//			if((status == HAL_OK) && (s_ht1632_test))
//		}
		s_savecnt = 72000;		// 2H�����Զ���������
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
	g_GuiData.g_index = 0;
	g_GuiData.g_HasChanged = 1;
	g_GuiData.g_sethighcm = g_sys_para.s_sethighcm;
	g_GuiData.g_lihe = g_sys_para.s_setlihecm;
	g_GuiData.g_num = g_sys_para.s_numchi;
	g_GuiData.g_ts = g_sys_para.s_feedtims;
	g_GuiData.g_Zhoucm = g_sys_para.s_pericm;
	g_GuiData.g_mode = g_sys_para.s_mode;
    g_GuiData.g_hcnt = g_sys_para.s_rammcnt;
	g_GuiData.g_HighOvercm = g_sys_para.s_hprot;			/*Terry 2019.7.6*/
	
	HT1632_Init();
	s_ht1632_test = 0;										//���Ա�־
	s_liheblink = 0;
	s_highblink = 0;
}

extern const uint8_t LEDCODE[21];
void GUI_showdata(void)
{
	int LPosCm;
	int16_t tmp,tmp1,high;
	
	tmp = (g_st_SigData.m_Power - g_sys_para.s_pnull) * 10 / (g_sys_para.s_pfull - g_sys_para.s_pnull);
	Dsp_PullLight(tmp);
	
	tmp = g_GuiData.g_sethighcm / 50;     						/*0 - 24��Ӧ 0 - 12��  Terry  2019.5.21*/	
	tmp1 = g_GuiData.g_lihe/50;								/*ֱ����ʾ��ϵ�  2019.6.5*/
	LPosCm = g_st_SigData.m_HeightShowCm;
    if(LPosCm > 0)
        high = LPosCm / 50;      /*0-24��Ӧ  0 - 12��  Terry 2019.5.21*/
    else
        high = 0;
	Dsp_HighLight(high,tmp,tmp1,s_liheblink,s_highblink);
    
	if(s_liheblink > 0)  s_liheblink--;
	if(s_highblink > 0)  s_highblink--;
		
	if(g_st_SigData.m_errshow == ERR_NONE)
	{
		/*������ʾ��*/
        if(s_liheblink|s_highblink)
		{
			Dsp_Num(g_GuiData.g_show,1,0);
		}
        else if(g_GuiData.g_index == SHOW_NONE)
		{
            if(g_st_SigData.m_Mode == MOD_SIGACT)
				Dsp_Num(g_GuiData.g_show,1,0);
			else
				Dsp_Num(g_GuiData.g_show,0,0);			/*����ģ#ʽ��������ʾ ��*/
		}
		else
		{
			Dsp_Num(g_GuiData.g_show,0,0);
		}
			
		BUZZER(0);
		g_st_SigData.m_errnum = 0;
	}
	else
	{
		
		g_st_SigData.m_errnum = get_errnum(g_st_SigData.m_errshow);		//��ʾ���ϴ���
		tmp = g_st_SigData.m_errnum;
		if(tmp < 10)
		{
			Dsp_Num(tmp,0,LEDCODE[19]);  // P��E
//			BUZZER(1);
		}
		else
		{
			if(tmp != 16)
			{
				Dsp_Num(tmp,0,LEDCODE[20]);
//				BUZZER(1);							//��ͣ
			}
			else
			{
				Dsp_Num(VERSION,0,LEDCODE[12]);			//�汾��    2019.1.2
				
			}
		}
	}
	/*ѡ��ָʾ�ƺ�ģʽָʾ��*/
	Dsp_setled(g_GuiData.g_index,0,g_GuiData.g_mode);
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
	
//	if(err)
//	{
//		Log_e("err %x,ret %d",err,ret);
//	}
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



void CmdProc(void)
{
	switch(g_GuiData.g_show)
	{
		case 1:						/*����-��������*/
		case 2:
		case 3:
		case 4:
		case 6:	
			g_GuiData.g_HighOvercm = g_GuiData.g_show * 100;
			g_GuiData.g_HasChanged = 1;
			break;
		case 7:						/*��λ*/
			ModbusData_Init();
			GUI_Init();
			break;
		case 8:						/*��У��*/
			g_sys_para.s_pset = 1;
			g_GuiData.g_HasChanged = 1;
			s_savecnt = 10;
			break;
		case 9:						/*�ر�У��*/
			g_sys_para.s_pset = 0;
			g_GuiData.g_HasChanged = 1;
			s_savecnt = 10;
			break;
	}
}


/**
* Key init function
* @param none
* @return none
*/

void Key_HighUp(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_sethighcm < MAXSET_HIGH)   /*���12��  Terry  2019.5.21*/
	{
		g_GuiData.g_sethighcm += PER_HIGH;
		g_GuiData.g_HasChanged = 1;
		s_highblink = HIGHSHOW;
	}
	if(g_GuiData.g_sethighcm > MAXSET_HIGH)
		g_GuiData.g_sethighcm = MAXSET_HIGH;
}

void Key_HighUpL(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_sethighcm < MAXSET_HIGH)   /*���12��  Terry  2019.5.21*/
	{
		g_GuiData.g_sethighcm += PER_HIGH * 10;
		g_GuiData.g_HasChanged = 1;
		s_highblink = HIGHSHOW * 2;
	}
	if(g_GuiData.g_sethighcm > MAXSET_HIGH)
		g_GuiData.g_sethighcm = MAXSET_HIGH;
}
void Key_HighDw(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_sethighcm > MINSET_HIGH)   /* ��С1��  Terry  2019.5,21*/
	{
		g_GuiData.g_sethighcm -= PER_HIGH;		//�߶ȼ�С10cm
		g_GuiData.g_HasChanged = 1;

		if(g_GuiData.g_sethighcm < (g_GuiData.g_lihe + 5))			//��ϱ���С���趨�߶�10cm
			g_GuiData.g_lihe = g_GuiData.g_sethighcm - 5;

		s_highblink = HIGHSHOW;
	}
	if(g_GuiData.g_sethighcm < MINSET_HIGH)
		g_GuiData.g_sethighcm = MINSET_HIGH;
}
void Key_HighDwL(void)
{	
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_sethighcm > MINSET_HIGH)   /* ��С1��  Terry  2019.5,21*/
	{
		g_GuiData.g_sethighcm -= PER_HIGH * 10;		//�߶ȼ�С10cm
		g_GuiData.g_HasChanged = 1;

		if(g_GuiData.g_sethighcm < (g_GuiData.g_lihe + 5))			//��ϱ���С���趨�߶�10cm
			g_GuiData.g_lihe = g_GuiData.g_sethighcm - 5;

		s_highblink = HIGHSHOW * 2;
	}
	if(g_GuiData.g_sethighcm < MINSET_HIGH)
		g_GuiData.g_sethighcm = MINSET_HIGH;
}



void Key_LiheUp(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_lihe < (g_GuiData.g_sethighcm - 6))
	{
		if(g_GuiData.g_lihe < -10)
			g_GuiData.g_lihe +=4;   //Ϊ�����ǣ�������һЩʱ��
		else
			g_GuiData.g_lihe +=2; //ÿ��ֻ�ܼ�2����   2017.11.10
		
		g_GuiData.g_HasChanged = 1;
		s_liheblink = LIHESHOW;			//��˸һ��
		s_highblink = 0;
	}
}

void Key_LiheUpL(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_lihe < (g_GuiData.g_sethighcm - 6))
	{
		if(g_GuiData.g_lihe < -10)
			g_GuiData.g_lihe +=20;   //Ϊ�����ǣ�������һЩʱ��
		else
			g_GuiData.g_lihe +=20; //ÿ��ֻ�ܼ�2����   2017.11.10
		
		g_GuiData.g_HasChanged = 1;
		s_liheblink = LIHESHOW * 2;			//��˸һ��
		s_highblink = 0;
	}
}


void Key_LiheDw(void)
{
	IOT_FUNC_ENTRY;
	if(g_GuiData.g_lihe > -200)		
	{
		if(g_GuiData.g_lihe < -10)
			g_GuiData.g_lihe -=6;
		else
			g_GuiData.g_lihe -=3;				

		g_GuiData.g_HasChanged = 1;						/* ���ݱ����־ */
		s_liheblink = LIHESHOW;
		s_highblink = 0;			
	}
}
void Key_LiheDwL(void)
{
	IOT_FUNC_ENTRY;
	
	if(g_GuiData.g_lihe > -200)		
	{
		if(g_GuiData.g_lihe < -10)
			g_GuiData.g_lihe -=60;
		else
			g_GuiData.g_lihe -=30;				
		
		g_GuiData.g_HasChanged = 1;						/* ���ݱ����־ */
		s_liheblink = LIHESHOW * 2;
		s_highblink = 0;			
	}
	
}
void Key_Set(void)
{
	IOT_FUNC_ENTRY;
	if(g_st_SigData.m_errshow)
		clear_err(&g_st_SigData.m_errshow);
	else if(g_GuiData.g_index == SHOW_BACKSET)
	{
		/*��̨��������*/
		CmdProc();
		g_GuiData.g_index = SHOW_NONE;
	}
	else
	{
		g_GuiData.g_index++;
		if(g_GuiData.g_index > SHOW_CNT)
			g_GuiData.g_index = SHOW_NONE;
		
	}
}
void Key_Add(void)
{
	IOT_FUNC_ENTRY;
	if(g_st_SigData.m_errshow)
		clear_err(&g_st_SigData.m_errshow);
	else
	{
	switch(g_GuiData.g_index)
	{
		case SHOW_NONE:break;
		case SHOW_TTC:
			if(g_GuiData.g_num < 300)								// 2017.12.18
			{
				g_GuiData.g_num++;	
				g_GuiData.g_HasChanged = 1;
				
			}
			break;
		case SHOW_ZHOU:
				g_GuiData.g_Zhoucm++;	
				g_GuiData.g_HasChanged = 1;
				break;
		case SHOW_TS:
			if(g_GuiData.g_ts < 20)
			{
				g_GuiData.g_ts++;	
				g_GuiData.g_HasChanged = 1;
			}break;
		case SHOW_CNT:                             /*Terry add 2019.*/
			if(g_GuiData.g_hcnt < 10)
			{
				g_GuiData.g_hcnt++;
				g_GuiData.g_HasChanged = 1;
			}
			break;
		case SHOW_BACKSET:
			if(g_GuiData.g_pset < 99 )
			{
				g_GuiData.g_pset++;
			}
		break;
		}
	}
}
void Key_Sub(void)
{
	IOT_FUNC_ENTRY;
	if(g_st_SigData.m_errshow)
		clear_err(&g_st_SigData.m_errshow);
	else
	{
		switch(g_GuiData.g_index)
		{
			case SHOW_NONE:break;
			case SHOW_TTC:
				if(g_GuiData.g_num > 15)
				{
					g_GuiData.g_num--;
					g_GuiData.g_HasChanged = 1;
				}
				break;;
			case SHOW_ZHOU:
				if(g_GuiData.g_Zhoucm > 50)
				{
					g_GuiData.g_Zhoucm--;
					g_GuiData.g_HasChanged = 1;
				}
					break;
			case SHOW_TS:
				if(g_GuiData.g_ts > 1)
				{
					g_GuiData.g_ts--;
					g_GuiData.g_HasChanged = 1;
				}
				break;
			case SHOW_CNT:                      /*Terry add*/
				if(g_GuiData.g_hcnt > 3)
				{
					g_GuiData.g_hcnt--;
					g_GuiData.g_HasChanged = 1;
				}
				break;
			case SHOW_BACKSET:
				if(g_GuiData.g_pset > 0 )
				{
					g_GuiData.g_pset--;				/*Terry 2019.7.9  */
				}
			break;
		}
	}
}
void Key_ModLong(void)
{
	IOT_FUNC_ENTRY;
	
	if((g_st_SigData.m_Mode == MOD_FREE) && (g_st_SigData.m_errshow == ERR_NONE))
	{
		if(g_GuiData.g_mode)
		{
			g_GuiData.g_mode = 0;
			g_GuiData.g_HasChanged = 1;
		}
		else
		{
			g_GuiData.g_mode = 1;
			g_GuiData.g_HasChanged = 1;
		}
	}
}

void Key_StartLong(void)
{
	if(g_st_SigData.m_Mode == MOD_FREE)
	{
		if(g_sys_para.s_mode == 0)  		//����ģʽ
			g_st_SigData.m_Mode = MOD_SIGACT;
		else
			g_st_SigData.m_Mode = MOD_AUTOTAMP;
	}

	IOT_FUNC_ENTRY;
}

void Key_Start(void)
{
	if(g_st_SigData.m_Mode != MOD_FREE)
	{
		g_st_SigData.m_Mode = MOD_FREE;
		G_SHACHE(ACT_ON,0);
		G_LIHE(ACT_OFF,200);
	}
	IOT_FUNC_ENTRY;
}








void Key_TBrkLong(void)
{
	IOT_FUNC_ENTRY;
}
void Key_TLhLong(void)
{
	IOT_FUNC_ENTRY;
}
void Key_Dd(void)
{
	IOT_FUNC_ENTRY;
}
void Key_Zd(void)
{
	IOT_FUNC_ENTRY;
}
void Key_Stop(void)
{
	IOT_FUNC_ENTRY;
}
void Key_Liu(void)
{
	IOT_FUNC_ENTRY;
}




#define GPIO_KEY_NUM 16 					///< Defines the total number of key member
static keyTypedef_t DsingleKey[GPIO_KEY_NUM]; 	///< Defines a single key member array pointer
keysTypedef_t keys;  


void keyInit(void)
{
    DsingleKey[0] = keyInitOne(GPIO_PIN_RESET, GPIOA, GPIO_PIN_4, Key_HighUp,   Key_HighUpL);     //KEYUP
    DsingleKey[1] = keyInitOne(GPIO_PIN_RESET, GPIOA, GPIO_PIN_5,  Key_HighDw,  Key_HighDwL);   //KEY_HDW
    DsingleKey[2] = keyInitOne(GPIO_PIN_RESET, GPIOA, GPIO_PIN_6,  Key_LiheUp,  Key_LiheUpL);    //KEY_LUP
    DsingleKey[3] = keyInitOne(GPIO_PIN_RESET, GPIOA, GPIO_PIN_7,  Key_LiheDw,  Key_LiheDwL);    //KEY_LDW
    DsingleKey[4] = keyInitOne(GPIO_PIN_RESET, GPIOC, GPIO_PIN_4,  Key_Set,   0);     //KEY_SET
    DsingleKey[5] = keyInitOne(GPIO_PIN_RESET, GPIOC, GPIO_PIN_5,  Key_Add,    0);    //KEY_SUP
	DsingleKey[6] = keyInitOne(GPIO_PIN_RESET, GPIOB, GPIO_PIN_0,  Key_Sub,   0 );    //KEY_SDW
	DsingleKey[7] = keyInitOne(GPIO_PIN_RESET, GPIOB, GPIO_PIN_1,  0,   Key_ModLong );    //KEY_CHG

	DsingleKey[8] = keyInitOne(GPIO_PIN_RESET, GPIOB, GPIO_PIN_8,  Key_Start,Key_StartLong );   //KEY_START
	
	
//	DsingleKey[9] = keyInitOne(GPIO_PIN_RESET, GPIOB, GPIO_PIN_2,  0,   Key_TBrkLong );    //KEY_TSC
//	DsingleKey[10] = keyInitOne(GPIO_PIN_RESET, GPIOB, GPIO_PIN_9,  0,   Key_TLhLong );    //KEY_TLH
	
//	DsingleKey[11] = keyInitOne(GPIO_PIN_SET, GPIOB, GPIO_PIN_14,  Key_Dd,   0 );   //KEY_DD
//	DsingleKey[12] = keyInitOne(GPIO_PIN_SET, GPIOA, GPIO_PIN_3,  Key_Zd,   0 );    //KEY_ZD
//	DsingleKey[13] = keyInitOne(GPIO_PIN_SET, GPIOB, GPIO_PIN_15,  Key_Liu,   0 );   //KEY_LIU
//	DsingleKey[14] = keyInitOne(NULL, GPIOB, GPIO_PIN_13,  Key_Stop,   0 );   //KEY_STOP
	
	
    keys.singleKey = (keyTypedef_t *)&DsingleKey;
    keyParaInit(&keys); 
}

void KeySta_Poll(void)
{
	keyHandle((keysTypedef_t *)&keys);
}
