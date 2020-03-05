
/**********************************************
Copyright (C),2015-2016,TT Tech.Co.,Ltd.
File name	:main.c
Author		:Terry
Description	:��׮���弶��������������V1.0
Others		:None
Date		:2015.06.01  - 2017.02.11
***********************************************/

#ifndef  _ACTION_H
#define  _ACTION_H
#include "config.h"
#include "tim.h"

#define CALUTICK				5					//40����һ��
#define SHACHEDLY				(800)
#define LIHEDLY         		(4 * 110)    /*0.4��*/
#define ERRDLY					500


#define HANG_TICK       		5      /*��������ʱ�ļ��ʱ��*/
#define T_GAV					(6)
#define LIHESHOW				15
#define HIGHSHOW				15

#define LALIMAXPER		60					//2019.11.15      ԭ�� 70%
#define LALIMIDPER		50
#define LALIMINPER		55					// ʵ���´�ʱ���Ϊ 55% ,�ղ�Ŀ���´�ʱ�����ϵ�����  // 2018.6.1

#define UPTMOUT			20000				//������ʱʱ��
#define VUP				25					// 0.4�׵��ᴸ�ٶ���  ÿ����Ҫ2.5��
#define ERR_NONE		(uint32_t)0x000000
#define ERR_KC			(uint32_t)0x01		// 𤴸 / ����
#define ERR_NC			(uint32_t)(1<<1)	// ���δ�ѿ�
#define ERR_LS			(uint32_t)(1<<2)	//�����Ӧ����
#define ERR_CS			(uint32_t)(1<<3)	//�ᴸʱ�䳬ʱ
#define ERR_TOP			(uint32_t)(1<<4)	//�嶥����
#define ERR_LIU			(uint32_t)(1<<5)	//��Ź���
#define ERR_DOWN		(uint32_t)(1<<6)	//�½���ȹ�С
#define ERR_CHAO		(uint32_t)(1<<7)	//��������



#define ERR_PW			(uint32_t)(1<<8)	//��Դ����
#define ERR_TT			(uint32_t)(1<<9)	//̽ͷ2����
#define ERR_LH			(uint32_t)(1<<10)	//�����·����
#define ERR_SC			(uint32_t)(1<<11)	//ɲ����·����
#define ERR_CT			(uint32_t)(1<<12)	//����δ�����򻥸�������
#define ERR_HALT		(uint32_t)(1<<30)	//�ֶ�ֹͣ
#define ERR_ACE			(uint32_t)(1<<13)   //��Ȩ����




/*����״̬*/

enum emWORKMODE
{
    MOD_FREE = 0,
    MOD_SIGACT,
    MOD_AUTOTAMP,       /*�Զ���������*/
    MOD_MANUAL,
    MOD_MANOFF,
    MOD_TST,
    MOD_DOWN,       /*�ֶ����ģʽ  Terry 2019.5.21*/
};

enum emHANGTU
{
    S_IDLE	 = 0,
    S_PULSE,        /*1���ж� */
    S_TICHUI,       /*�ᴸ ���϶���*/
    S_CHECK_UPSTOP,	/*ȷ������ֹͣ*/
    S_XIALIAO,      /*����*/
    S_DELAY2,
    S_LIUF,         /*���*/
    S_DACHUI,       /*��*/
};

struct HANGTU
{
    uint32_t pvtimer;       /*�ϴδ�ʱ��*/
    int32_t last_highnum;   /*����ʱ�ĸ߶�*/
    int32_t overpow;        /*�����ۻ�����*/
	int32_t lowpow;			/*�޵�������*/
    int32_t speederr;       /*ʧ���ۻ�����*/
    int32_t dachui_cnt;     /*�򴸵Ĵ���*/
    int32_t liufang_sta;    /*���״̬���*/
    int32_t top_sta;        /*����״̬���*/
	int32_t stop_sta;		/*��������ͣ��״̬���*/
    int32_t surecnt;		
	int16_t last_downnum;   /*�ϴ���ŵĳ���*/
	int16_t last_sta;
};    







#define SIG_HAN         (0x01 <<1)   /*�嶥�ź�*/

#define KG_CHUI			6		/*���贸���������ٶ�Ϊ 6m/s2*/
#define SP_CHUI			0.35	/*���贸�������ٶ�Ϊ0.35m/s*/

#define INIT_CHUI		50		/*�ֶ���Ԥ��50�����ı��趨ֵ*/

/*̽ͷ�ᴸʱ���ᴸ����ж�*/
#define PULL_IDLE		0		/*����Ч�ᴸ*/
#define PULL_OBS		1		/*���۲�*/
#define PULL_EFFECT		2		/*��Ч�ᴸ*/




/**********************��¼�����ݽṹ********************************************/

/*ϵͳ���ԣ���������*/
__packed struct SYSATTR
{
	uint32_t s_chickid;					/*У����  0xA5A6A7A8*/
	int32_t s_sethighcm;				//����
	int32_t s_setlihecm;				//����			�趨����ϸ߶�
	int32_t s_pericm;					//��Ͳ�ܳ�  ����
	int32_t s_numchi;					//ÿ�ܳ���  ����
	int32_t s_pnull;					/*����ʱ��ƽ������*/
	int32_t s_pfull;					/*����ʱ��ƽ������*/
	int32_t s_rammcnt;                      /*��������  3-10�� Terry 2019.5.21  */
	uint32_t s_dir;						/*�����������־�ź�*/
	int32_t s_hprot;					/*�߶ȱ������� Ĭ�� 300cm*/
	int16_t s_pset;						/*������Ȩģʽ*/
	uint8_t s_feedtims;					/*˫����ʱ�䣬��λ 0.1��   �ĳ� ����ʱ�� Terry 2019.5.21  ��λ ��  2-15��*/
	int8_t  s_mode;						/*0 ����  1 ��ʾ˫��*/
};

/*��̽ͷ����̽ͷ���õ���ϵ�������*/
__packed struct LIHEDATA			/*������ɳڶȵ��м����ֵ*/
{
	int32_t relaxsum;				/*�ɳڶȵ��ۼ�ֵ*/
	int32_t powersum;				/*�ᴸ��Ч����ƽ��ֵ*/
	int32_t cnt;					/*�ɳڶ��ۼӴ���*/
	int32_t lihemax;				/*��ϵ���������*/
	int32_t lihemim;				/*��ϵ���������*/
	int32_t lihe;					/*����м���*/
	int32_t relaxon;				/*��׼���ɳڶȲο����ֶ�ģʽ��ǰN�����ɳڶ�ƽ��ֵ*/
	int32_t powerave;				/*����ʱ��ƽ������*/
	uint32_t uptime;
	int16_t	songchi;				/*�ɳڶȣ�Ϊ0 ʱ��ʾ������Ӧ��Ϊ1ʱ��ʾ���Զ�������*/
};


__packed struct PRPTECT_HANGTU
{
	int16_t flg;			/*�����ı�־*/
	int16_t tmp;
	int32_t p_high;			/*ʵ�ʵ��ᴸ�߶� ����λ����*/
	int32_t p_lihe;			/*��ϵ�λ�� ����*/
};

__packed struct RECORD
{
	uint16_t nub;			/*���*/
	uint16_t cnt;			/*����*/
	uint16_t high;			/*��ǰ���  0.1��*/
	uint16_t tim;			/*����ʱ��*/
	int16_t deepth;			/*��ǰ���  0.1��*/
};

/*�ٶȲ����ṹ��*/
typedef __packed struct SPEED
{
    uint16_t sta;
    uint16_t lastcount;
    uint32_t lasttim;
    int32_t pertim;
    int32_t pertick;
    int32_t speed;         //  100 * tick /s
    int32_t acce;          //���ٶ� 100 * tick /(s*s)
    int16_t readyflg;
}SSPEED;


#define HALT_BREAK	{			\
						if(g_halt)	\
						{status |= ERR_HALT; break;}  \
                        if(g_st_SigData.m_Mode == MOD_FREE)\
                            break;\
					}

#define ERR_BREAK   if(status)  break

typedef  uint32_t SYS_STA;

extern struct SYSATTR g_sys_para;
extern struct STADATA sys_stadata;
extern volatile uint8_t sys_fbsta;						// �ⲿ�����ź�

/*��ñ�����������ȷ����׼����*/
//void GetEncode(uint32_t dir, struct EncoderCnt *p);
/*��õ�ǰ����*/
void GetPower(struct EtrCnt *p);

/********��̽ͷ�ᴸ����***********/
SYS_STA ntakeup(void);
/*һ������*/
SYS_STA nstarttaking(void);
/*��*/
SYS_STA nputdown(int32_t delay);

void sysattr_init(uint16_t flg);


/*���ɲ����������ִ��*/
void g_action(void);
void G_LIHE(uint32_t sta, uint32_t delay);
void G_SHACHE(uint32_t sta, uint32_t delay);
void G_SHACHE_SG(uint32_t sta, uint32_t delay);     //���������ź�
void liheupdate(void);//������ϲ���ֵ

uint16_t Shache_Proc(void);
//void EncoderClr(struct EncoderCnt *p);

int GetEncoderSpeedCm(void);
int GetEncoderAcceCm(void);
int GetEncoderLen1Cm(void);
int GetEncoderLen2Cm(void);

#endif

