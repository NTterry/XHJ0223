


#ifndef _PUT_OFF_ACT_H_
#define _PUT_OFF_ACT_H_
#include "stm32f1xx_hal.h"


#define T_ACT_MS			5

/*动作状态*/
#define ACT_DEF	3
#define ACT_LIU	2			/*溜放  */
#define ACT_ON	1           /*拉刹车*/
#define ACT_OFF	0           /*松刹车*/

#define FB_24VOK		0x80	/*电源24V正常*/
#define FB_RUN			0x40	/*非停机模式*/
#define FB_TT2			0x08	/*使用双探头模式*/
#define FB_TTOK			0x04	/*探头电源 OK*/
#define FB_LIHE			0x02	/*离合有信号*/
#define FB_SHACHE		0x01	/*刹车有信号*/


/*信号指示灯 */
#define SIG_CUR         	 0x0001  		/*电流信号指示*/
#define SIG_TICHUI         	(0x0001 << 1)  	/*提锤*/
#define SIG_FANGCHUI        (0x0001 << 2)  	/*放锤*/
#define SIG_SHACHE         	(0x0001 << 3)  	/*刹车指示信号*/
#define SIG_ALL				(SIG_TICHUI|SIG_FANGCHUI|SIG_SHACHE)

#define SIG_SLIUF			(0x0001 << 4)		/*溜放*/
#define SIG_SHANGTU			(0x0001 << 5)		/*夯土*/
#define SIG_STICHUI			(0x0001 << 6)		/*提锤*/
#define SIG_SZHUCHUI		(0x0001 << 7)		/*驻锤*/
#define SIG_STU				(0x0001 << 8)		/*送土*/
#define SIG_SALL			(SIG_SLIUF|SIG_SHANGTU|SIG_STICHUI|SIG_SZHUCHUI|SIG_STU)



/*离合和刹车的动作指令*/
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

