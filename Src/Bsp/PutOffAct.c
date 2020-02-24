
#include "PutOffAct.h"
#include "config.h"


// ִ����
#define PIN_FLH			GPIO_PIN_11							//��Ͽ��ƽ�
#define PIN_FSC			GPIO_PIN_12							//ɲ�����ƽ�
						//��ͣ���ƽ�
#define PIN_AS1         GPIO_PIN_6                          //ɲ�������ź�����      2018.12.24
#define PIN_AS2         GPIO_PIN_7                          //ɲ�������ź�����      2018.12.24

#define PORT_CTR		GPIOA

#define PORT_CAS        GPIOC

#define C_LALIHE()		HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_RESET)										//�����
#define C_SONGLIHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_SET)										//�����

/*ɲ�������ź�*/
#define C_AS1_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_RESET)	
#define C_AS1_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_SET)	
#define C_AS2_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_RESET)	
#define C_AS2_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_SET)	

/*����ģ��ʹ��*/
//#define C_SONGSHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)
//#define C_LASHACHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET)

/*ʵ��ʹ��ʱ��ʹ����������*/
/* ɲ���ƶ��� */
#define C_SZHIDONG()	HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_RESET) //���24V  Ϊɲ���ƶ�״̬
#define C_SNZ()			HAL_GPIO_WritePin(PORT_CTR, PIN_FSC,GPIO_PIN_SET)	//ȡ���ƶ�


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





/*���ط����ź�*/
int8_t Get_Fbsignal(uint8_t MASK)
{
	if(sys_fbsta & MASK)
		return 1;
	else
		return 0;
}

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
			C_SNZ();
            C_AS1_DS();
            C_AS2_DS();
		}
	}
}


/* ��϶���   �����ʱΪ0ʱ������ִ��*/
void G_LIHE(uint32_t sta, uint32_t delay)
{
//	portENTER_CRITICAL();
	g_alihe.delay = delay / T_ACT_MS;
	g_alihe.sta = sta;
	if((delay == 0) && (sta == ACT_ON))
	{
		C_LALIHE();
	}
	if((delay == 0) && (sta == ACT_OFF))
	{
		C_SONGLIHE();
	}
		
//	portEXIT_CRITICAL();
}

uint16_t Shache_Proc(void)
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
            if(HAL_MS_DIFF(sctime) > 5000)				/*ɲ����ɲ�ƶ�ʱ��*/
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
  * �������� �ⲿִ����� ��ɲ�� ����
  * ��������� ��

  * ����ֵ: ��
  * ˵��: �������� StartDefaultTask  ÿ CALUTICK ����ִ��һ��
  */
//static uint16_t s_lihecnt = 0;
void g_action(void)
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
            C_SONGLIHE();				//ÿ10msȷ��һ��
        }
        else
        {
            C_LALIHE();
        }
    }
    /*ɲ����ʱ�ź�*/
	if(g_shache.delay == 0)
    {
        g_shache.pact = g_shache.sta;
    } 
	
    if(g_shache.delay >= 0)
    {
        g_shache.delay--;
    }

/*Fb_Err_check()*/
//	if(g_alihe.delay == 0)
//	{
//		if((g_alihe.sta == ACT_OFF) && (Get_Fbsignal(FB_LIHE) == 1))	// �����
//		{
//			s_lihecnt++;
//		}
//		else if((g_alihe.sta == ACT_ON) && (Get_Fbsignal(FB_LIHE) == 0))	// �����
//		{
//			s_lihecnt++;
//		}
//		else
//		{
//			s_lihecnt = 0;
//		}
//	}
//	if(s_lihecnt > 120)			/*Terry 2019.7.9*/
//	{
//		g_erract |= ERR_LH;
//	}
}

/*Called by Timer as 10ms*/
void G_ActPoll_10ms(void)
{
	g_action();
	Shache_Proc();
}

void Pf_Lihe_Init(void)
{
	g_alihe.delay = 10;
	g_alihe.sta = ACT_OFF;
}

void Pf_Shache_Init(void)
{
	g_shache.delay = 10;
	g_shache.sta = ACT_OFF;
	
	C_SONGLIHE();
	C_SNZ();
	C_AS1_DS();
	C_AS2_DS();
}
