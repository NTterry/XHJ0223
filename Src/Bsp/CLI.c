
#include "CLI.h"
#include "usart.h"
#include "string.h"
#include "Action.h"
#include "GUI.h"

extern struct RECBUFF Rec_flg;

void getinfo(void);
void Testsh(void);
uint8_t rx_buffer[STRLENTH];
uint8_t str[STRLENTH];


static int stack[STKSIZE];    
static char cstack[CSTKSIZE];  
static int sp;
static int csp;

int numdic;

const table dictionary[] = 
{
	{5,'g','e','t','i','d',0,0,getid},
	{5,'s','e','t','u','p',0,0,setup},
	{7,'g','e','t','i','n','f','o',getinfo},
	{6,'T','e','s','t','s','h',0,Testsh},
};

void CLI_Init()
{
	
	sp = 0;		
	csp = 0;
	numdic = (int)(sizeof(dictionary)/sizeof(dictionary[0]));
	Rec_flg.recv_end_flag = 0;
	Rec_flg.rx_len=0;
}

/*
name:		trypush
description:�����������볢��ѹջ��תΪ���ݻ����ַ���
author:     Terry
time:       2012/4/29
*/
int trypush(char *str)
{
	int i;
	int val = 0;
	if(isalpha(str[0]))  //������ַ����ַ�
	{	
		for(i = strlen(str) -1; i >= 0; i--)  
			pushc(str[i]);
		push((int)(strlen(str)));
	}
	else
	{
		for(i = 0; i< (int)strlen(str); i++)
		{
			if(isdigit(str[i]))   //���������
				val = val * 10 +str[i] - '0';
			else
			{
//				printf("!!%s error???",str);
				return 1;
			}

		}
		push(val);
	}
	return 0;
}
/*
name:		decipher
description:���������
author:     Terry
time:       2012/4/29
*/	
//void decipher(void const * argument)
//{

//	char *tok; 
//	int8_t  err;
//	struct dtable cmds;
//	unsigned char match;
//	unsigned char i;
//	
////	osDelay(1000);
//	printf("welcome use the CLI\r\n");
//	printf("version 1.0\r\n");
//	CLI_Init();
//	while(1)
//	{
//		HAL_UART_Receive_DMA(&huart1,rx_buffer,STRLENTH);				//DMA�ķ�ʽ�������ݣ�һ���п����жϷ�������������ݽ���
//		if(Rec_flg.recv_end_flag ==1)                                  	//recv_end_flag  ���ݳɹ�����
//		{
//			memcpy(str,rx_buffer,Rec_flg.rx_len);
//			Rec_flg.recv_end_flag = 0;
//			Rec_flg.rx_len=0;
//			printf("\r\n$");
//			tok = strtok((char *)str," ");
//			if(tok)
//			{
//				do
//				{
//					cmds.id.bytes.len = (unsigned char)strlen(tok);
//					for(i = 0; i < 7; i++)
//						if(i < cmds.id.bytes.len)
//							cmds.id.bytes.words[i] = (tok[i]);
//						else
//							cmds.id.bytes.words[i] = 0;

//					match = 0;
//					for(i = 0; i < numdic && !match; i++)
//					{
//						if(memcmp(dictionary[i].id.bits ,cmds.id.bits ,8) == 0)
//						{	match = 1;
//							(*dictionary[i].fun)();
//						}
//					}

//				err = 0;
//				if(!match)
//					{
//						if(trypush(tok))
//							err = 1;
//					}
//				tok = strtok(NULL," ");
//				}while(tok && !err);  					//strtok ֻ����һ��������ʹ�ã��漰��ȫ�ֱ���
//			}
//			
//		}
//		osDelay(100);
//	}
//}


/*
		������һ������ֵĴ��󣬲������ݴ���
		ԭ��������ʱ�Ĳ�������д����char�����´���
*/
/*
name:		push
description:������numѹջ
author:     Terry
time:       2012/4/29
*/
void push(int num)
{
	if(sp == STKSIZE)
	{
		Debug("Stack Full!\n");
		sp = 0;
		return;
	}
	stack[sp++] = num;
}
/*
name:		pop
description:���ݵ������ɺ�������ֵ����
author:     Terry
time:       2012/4/29
*/
int pop(void)
{
	if(sp == 0)
	{
		Debug("Stack empty!!!\n");
		return 0;
	}
	return (stack[--sp]);
}
/*
name:		pushc
description:�ַ�cѹջ
author:     Terry
time:       2012/4/29
*/
void pushc(char c)
{
	if(csp == CSTKSIZE)
	{
		Debug("CStack Full!\n");
		csp = 0;
		return;
	}
	cstack[csp++] = c;
}

/*
name:		popc
description:�ַ��������ɷ���ֵ����
author:     Terry
time:       2012/4/29
*/
char popc()
{
	if(csp == 0)
	{
		Debug("CStack empty!!!\r\n");
		return 0;
	}
	return (cstack[--csp]);
}

/*
name:		popcstr
description:����һ���ַ���  x:�ַ������ȣ� *p�����ַ����ĵ�ַ
author:     Terry
time:       2012/4/29
*/
int popcstr(int x , char *p)
{
	int i;
	for(i = 0;i < x; i++)
	{	
		p[i] = popc();
		if(p[i] == 0 )
			break;
	}
	p[i] = 0;
	return i;
}

int getsp()
{
	return sp;
}

int getcsp()
{
	return csp;
}

extern void EEPROMtest(void);
void getid(void)
{
	uint32_t cpuid[3];
	
	cpuid[0] = *(uint32_t *)(0x1ffff7e8);				//STM32 ȫ��ͳһ��ID��
	cpuid[1] = *(uint32_t *)(0x1ffff7ec);
	cpuid[2] = *(uint32_t *)(0x1ffff7f0);
	printf("%x %x %x \r\n",cpuid[0],cpuid[1],cpuid[2]);
//	EEPROMtest();
}

extern struct SYSATTR sys_attr;
void setup(void)
{
	int32_t ret;
	
	ret = pop();
	Debug("zidong = %d\r\n",ret);
	sys_attr.s_zidong = ret;
}

/*����ϵͳ����Ϣ
�趨�Ĳ���
��̬�Ĳ����ȵ�
*/

extern volatile struct GUI_DATA	g_showdata;

void getinfo(void)
{
	printf("Attr:\r\n");
	printf("sethigh %d \r\n",g_showdata.g_sethighcm);
	printf("lihe %d\r\n",g_showdata.g_lihe);
	printf("PNULL %d  PFULL %d\r\n",sys_attr.s_pnull,sys_attr.s_pfull);
	printf("Dir %d \r\n",sys_attr.s_dir);
	
	printf("Pnow %d\r\n",sys_stadata.m_power.Speed);
	printf("spd %d\r\n",sys_stadata.m_high.Speed);
	
	printf("TTCHK %d\r\n",sys_stadata.TTCHK);
}
extern uint32_t g_testflg;

void Testsh(void)
{
	g_testflg = 100;
	printf("Test begin...\r\n");
}


