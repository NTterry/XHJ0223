



#ifndef _SINGLEACT_H_
#define _SINGLEACT_H_
#include "stdint.h"

#define VALID_MIN_CM		10				// Mininum Speed 0.1m/s    �����ֵ
#define BRAKE_DLY400		400
#define VALID_POWH			70
#define VALID_POWM			50
#define VALID_POWL			30
#define VALID_POWOVER		150


enum ACT_SINGLE_UP
{
	SIG_IDLE,
	SIG_PULL_CLUTCH,
	SIG_PULL_HOLD,
	SIG_WAIT_VALID_UP,
	SIG_WORKUP,
	SIG_REACH_TOP,
	SIG_BLOCKED,
};


enum ACT_SINGLE_DW
{
	DIG_IDLE,
	DIG_CHKDW,
	DIG_WAIT_LIHE,
	DIG_BLOCKED,
};


struct SIG_ACT_DATA
{
	int32_t m_HeightShowCm;
	int32_t m_HeighRammCm;			/*����ʱ�����ļ����߶�*/
	int32_t m_SpeedCm;
	int32_t m_Power;
	int32_t m_Mode;
	int32_t m_errnum;				/*������*/
	int32_t m_manualflg;
	uint32_t m_errshow;				/*��ִ�г���Ĵ��󷵻���*/
	int32_t  m_Lihenew;				/*���º���ϵ�ĸ߶�*/
};


typedef enum ERR_SIGACT
{
	ERR_SIG_OK,
	ERR_SIG_REACHUP,		//�����ᴸ���ﶥ��
	ERR_SIG_REACHDW,  		//�����·Ŵ򴸽���
	ERR_SIG_PULLUP,			//�ᴸ�쳣
	ERR_SIG_ENCODER,		//�������쳣
	ERR_SIG_CUR,			//��������쳣
	ERR_SIG_CLING,			//����
	ERR_SIG_BRAKE,			//ɲ���쳣
	ERR_SIG_TIMOUT,			//�ᴸ�쳣
	ERR_SIG_SOFT,			//�����������
	
}ERR_SIG;

int32_t Per2Power(int16_t  percent);
void Sig_ResetSta(void);
ERR_SIG Sig_TakeUp(void);
ERR_SIG Sig_StudyUp(void);
ERR_SIG Sig_LandDw(void);

void GetLiveData(void);

#endif

