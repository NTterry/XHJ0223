
#include "ModbusRec.h"
#include "usart.h"
#include "Gpio.h"
#include "mbport.h"
#include "stdio.h"
#include "mb.h"
#include "u_log.h"

RecvModBus LineGui;

extern QUEUE u1_rx;

int err_count = 0;
void checkoff(void);


/*crc check*/
uint16_t mb_crc(uint8_t *snd,uint8_t num)
{
	int i,j;
	uint16_t c,crc=0xFFFF;
	uint16_t tmp;
	
	for (i=0;i<num;i++)
	{ 
		c = snd[i] & 0x00FF;
		crc^=c;
		for(j=0;j<8;j++)
		{ 
			if (crc & 0x0001)
			{
				crc>>=1;
				crc^=0xA001;
			}
			else 
				crc>>=1;
		}
	}
	tmp = crc & 0x00ff;

	tmp <<= 8;
	tmp|= (crc >> 8);
	return(tmp);
}

uint16_t mb_BIG(uint8_t *pdata)
{
	uint16_t tmp;
	tmp = *pdata;
	tmp <<= 8;
	tmp |= *(pdata + 1);
	return tmp;
}

void mb2BIG(uint8_t *pdata,uint16_t data)
{
	*pdata = (uint8_t)(data >> 8);
	*(pdata+1) = (uint8_t) (data & 0xff);
}


void vRxProtocolClient(RecvModBus *pRec, QUEUE * pQ)
{
	uint16_t scrc = 0;
	
	if(getQueueIdx(pQ))
	{
		pRec->Index = getQueueLength(pQ);
		
		if(pRec->Index >= 4)
		{
			getQueueBuff(pQ,(int8_t *)&pRec->Data[0],pRec->Index);
				
			pRec->Addr = pRec->Data[0];
			scrc = pRec->Data[pRec->Index - 2];                  
			scrc = scrc << 8;
			scrc |= pRec->Data[pRec->Index - 1];
			
//			Log_e("addr %x len %d",pRec->Addr,pRec->Index);
//			Log_e("finish");
			pRec->Crc = mb_crc(pRec->Data,pRec->Index - 2);
			
			if(pRec->Crc == scrc)
			{
				pRec->ModEnable = true;
			}
		}
		else
			getQueueBuff(pQ,(int8_t *)&pRec->Data[0],pRec->Index);
	}
}


void MBSlavePoll(void)
{

	vRxProtocolClient(&LineGui, MB_GetRxQueue());
	
	if(LineGui.ModEnable && (LineGui.Addr == 0x01))
	{
		LineGui.ModEnable = false;
		xMBPortEventPost( EV_FRAME_RECEIVED );
	}
	eMBPoll();
}



/*
 * How to use This SDK
 * 与设备串口相关的  portserial.c
 * 与寄存器相关的     port.h
 * 主机调用函数
 * #include "ModbusRec.h"
 *
 * 一 初始化
 * eMBInit(MB_RTU, 1, 3, 9600, MB_PAR_NONE,1);
 * eMBEnable();
 * 二 轮询调用
 * MBSlavePoll();  //轮询调用时间 不大于10ms
 * */







