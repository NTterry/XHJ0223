
#include "PutOffAct.h"
#include "config.h"
#include "u_log.h"
#include "gpio.h"

//�����

	

/*���ģ��ʹ��*/
//#define C_SONGSHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)
//#define C_LASHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET)

/*ʵ��ʹ��ʱ��ʹ����������*/
/* ɲ���ƶ��� */
#define C_SZHIDONG()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET) //���24V  Ϊɲ���ƶ�״̬
#define C_SNZ()			HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)	//ȡ���ƶ�


#ifdef USE_LIHE_PWM
	int gLiheRatio = 0;		//��ֵ 1 - 10
	int LiheRLY	 = ACT_OFF;
#endif

/*��϶�����ɲ���������õı�׼����*/
struct ACT_STA
{
	uint32_t	sta;
	int32_t		delay;				//��λ ����
    uint32_t    pact;
};


/*    useless          */
extern volatile uint8_t sys_fbsta;						// �ⲿ�����ź�
static volatile struct ACT_STA g_alihe,g_shache;		//��� ɲ�� ��������  ��Ҫ��ʼ������
static enum SC sact;                					//ɲ��ִ�е�״̬��


/*���㵱ǰ��ʱ����*/
static uint32_t HAL_MS_DIFF(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = HAL_GetTick();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*����һ��Ŀ��*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}





/* ɲ������  ACT_ON  ��ɲ��    ACT_OFF  ��ɲ��*/
void G_SHACHE(uint32_t sta, uint32_t delay)
{
	g_shache.delay = delay / T_ACT_MS;
    g_shache.sta = sta;
	
	if(delay == 0)						/*2019.12.17*/
	{
		if(g_shache.sta == ACT_ON)		/*������ɲ��  ��������ֱ��ִ��*/
		{
#ifdef USE_LIUF
			C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
#else
			 C_SNZ();
#endif
		}
#ifndef USE_LIUF	
		if(g_shache.sta == ACT_OFF)
		{
			C_SZHIDONG();
		}
#endif

	}
}


/* */
/********************************************************
Function	: G_LIHE
Description	: �����ʱ����   �����ʱΪ0ʱ������ִ��
Input		: sta   ����ָ��
            ��delay ��ʱʱ��
Return		: None
Others		: 1ms �ж��е���
*********************************************************/
void G_LIHE(uint32_t sta, uint32_t delay)
{
	g_alihe.delay = delay / T_ACT_MS;
	g_alihe.sta = sta;
	if((delay == 0) && (sta == ACT_ON))
	{
#ifndef USE_LIHE_PWM
		C_LALIHE();
#else
		LiheRLY = sta;
#endif
	}
	if((delay == 0) && (sta == ACT_OFF))
	{
#ifndef USE_LIHE_PWM
		C_SONGLIHE();
#else
		LiheRLY = sta;
#endif
	}	
}

/********************************************************
Function	: Lihe_Generate_PWM  
Description	: PWM�����Ͽ����ź�,���� gLiheRatio = 10  �� С�� gLiheRatio = 5
Input		: None
Return		: None
Others		: 1ms �ж��е���
*********************************************************/
void Lihe_Generate_PWM(void)
{
#ifdef USE_LIHE_PWM
	static int scnt;
	if(LiheRLY == ACT_ON)
	{
		if(scnt <  gLiheRatio)
			C_LALIHE();
		else
			C_SONGLIHE();
			
		scnt++;
		scnt = scnt % 10;
	}
	else
		C_SONGLIHE();
#endif
}

/********************************************************
Function	: Shache_Proc  
Description	: ���ݿ���ָ�ִ��ɲ�������ź��������ɲ�� ��ɲ�� ��ŵ�
Input		: None
Return		: LEDָʾ����Ϣ
Others		: None
*********************************************************/
static uint16_t Shache_Proc(void)
{
    static uint32_t sctime;
	int16_t ledsta = 0;
    switch(sact)
    {
        case S_NOACT:		
            C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
			ledsta = SIG_SHACHE;					/*ɲ��ָʾ�ź�*/
			if(g_shache.pact == ACT_OFF)
            {
				sact = S_SONG1;
                sctime = HAL_GetTick();
            }
			if(g_shache.pact == ACT_LIU)
			{
				sact = S_LIUFANG;
			}
            break;
        case S_LIUFANG:
            C_SNZ();                        		/*ȡ���ƶ�*/
            C_AS1_EN();
            C_AS2_DS();
			if(g_shache.pact == ACT_ON)
				sact = S_NOACT;
			if(g_shache.pact == ACT_OFF)
			{
				sact = S_SONG1;
				sctime = HAL_GetTick();
			}
			ledsta = SIG_SLIUF;
			
            break;
        case S_SONG1:
            C_SNZ();
            C_AS1_EN();
            C_AS2_EN();
            if(HAL_MS_DIFF(sctime) > 6000)				/*ɲ����ɲ�ƶ�ʱ��*/
				sact = S_SONG2;
			
			if(g_shache.pact == ACT_ON)
				sact = S_NOACT;
			if(g_shache.pact == ACT_LIU)   /*�Ƚ���ɲ��ģʽ���ٽ������*/
			{
				sact = S_NOACT;
			}
			ledsta = 0;
            break;
        case S_SONG2:
            C_SZHIDONG();   /*�л�Ϊ�ƶ�������*/
            C_AS1_DS();
            C_AS2_DS();
		
			if(g_shache.pact == ACT_ON)		// ��ɲ��
				sact = S_NOACT;
			if(g_shache.pact == ACT_LIU)						/*��ɲ���������*/
			{
				sact = S_NOACT;
			}
			ledsta = 0;
			break;
        default:
            sact = S_NOACT;
            break;
    }
	
	return ledsta;
}


/**
  * �������� �ⲿִ����� ��ɲ�� ��ʱִ�в���
  * ��������� ��

  * ����ֵ: ��
  * ˵��: �������� StartDefaultTask  ÿ 10 ����ִ��һ��
  * �����趨ʱ���ִ��
  */
static void g_action(void)
{	 
    if(g_alihe.delay > 0)
    {
        g_alihe.delay--;
    }
    else
    {
        g_alihe.delay = 0;
        if(g_alihe.sta == ACT_OFF)
        {
           				//ÿ10msȷ��һ��
#ifndef USE_LIHE_PWM
			C_SONGLIHE();
#else
			LiheRLY = ACT_OFF;
#endif
        }
        else
        {
#ifndef USE_LIHE_PWM
			C_LALIHE();
#else
			LiheRLY = ACT_ON;
#endif
        }
    }
#ifdef USE_LIUF
    /*ɲ����ʱ�ź�*/
	if(g_shache.delay == 0)
    {
        g_shache.pact = g_shache.sta;
    } 
    if(g_shache.delay >= 0)
    {
        g_shache.delay--;
    }
#else
    if(g_shache.delay > 0)
    {
        g_shache.delay--;
    }
    else
    {
        g_shache.delay = 0;
        if(g_shache.sta == ACT_OFF)
        {
            C_SZHIDONG();				//ÿ10msȷ��һ��
        }
        else
        {
            C_SNZ();		// ɲס
        }
    }
#endif
}

/********************************************************
Function	: G_ActPoll_T  
Description	: ��ʱ���жϵ��ã���Ϻ�ɲ���źŵ�˳��ִ��  ִ������ CALUTICK����
Input		: None
Return		: None
Others		: None
*********************************************************/
void G_ActPoll_T(void)
{
	g_action();
#ifdef USE_LIUF
	Shache_Proc();
#endif
}

/********************************************************
Function	: Pf_Shache_Init
Description	: ɲ���ź��ϵ��ʼ��
Input		: None
Return		: None
Others		: None
*********************************************************/
void Pf_Shache_Init(void)
{
	g_shache.delay = 10;
	g_shache.sta = ACT_OFF;
	
	C_SONGLIHE();
	C_SNZ();
	C_AS1_DS();
	C_AS2_DS();
}

