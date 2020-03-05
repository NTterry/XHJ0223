


#ifndef _PUT_OFF_ACT_H_
#define _PUT_OFF_ACT_H_
#include "stm32f1xx_hal.h"


#define T_ACT_MS			5

/*����״̬*/
#define ACT_DEF	3
#define ACT_LIU	2			/*���  */
#define ACT_ON	1           /*��ɲ��*/
#define ACT_OFF	0           /*��ɲ��*/

#define FB_24VOK		0x80	/*��Դ24V����*/
#define FB_RUN			0x40	/*��ͣ��ģʽ*/
#define FB_TT2			0x08	/*ʹ��˫̽ͷģʽ*/
#define FB_TTOK			0x04	/*̽ͷ��Դ OK*/
#define FB_LIHE			0x02	/*������ź�*/
#define FB_SHACHE		0x01	/*ɲ�����ź�*/


/*�ź�ָʾ�� */
#define SIG_CUR         	 0x0001  		/*�����ź�ָʾ*/
#define SIG_TICHUI         	(0x0001 << 1)  	/*�ᴸ*/
#define SIG_FANGCHUI        (0x0001 << 2)  	/*�Ŵ�*/
#define SIG_SHACHE         	(0x0001 << 3)  	/*ɲ��ָʾ�ź�*/
#define SIG_ALL				(SIG_TICHUI|SIG_FANGCHUI|SIG_SHACHE)

#define SIG_SLIUF			(0x0001 << 4)		/*���*/
#define SIG_SHANGTU			(0x0001 << 5)		/*����*/
#define SIG_STICHUI			(0x0001 << 6)		/*�ᴸ*/
#define SIG_SZHUCHUI		(0x0001 << 7)		/*פ��*/
#define SIG_STU				(0x0001 << 8)		/*����*/
#define SIG_SALL			(SIG_SLIUF|SIG_SHANGTU|SIG_STICHUI|SIG_SZHUCHUI|SIG_STU)



/*��Ϻ�ɲ���Ķ���ָ��*/
enum SC
{
	S_NOACT = 0,
	S_LIUFANG,
	S_SONG1,
	S_SONG2,
};


void G_LIHE(uint32_t sta, uint32_t delay);
void G_SHACHE(uint32_t sta, uint32_t delay);
void G_ActPoll_10ms(void);
void Lihe_Generate_PWM(void);
#endif

