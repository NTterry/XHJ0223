
#include "stm32f1xx_hal.h"
#include "array.h"
#include "mb.h"

#ifndef __MODBUSREC_H_
#define __MODBUSREC_H_

#define RX_SUBCOM_LEN		128
#define TX_SUBCOM_LEN 		32

typedef struct
{
	uint8_t 	Addr;
	uint8_t 	CtrlCode;
	uint16_t 	SReg;
	uint16_t 	SNum;
	uint8_t 	Data[RX_SUBCOM_LEN];
	uint8_t 	Index;				//ָʾ��������ֵ��λ��
	uint16_t 	Crc;
	uint16_t 	uData[8];
	uint16_t 	TimeOver;
	uint8_t 	ModEnable;
	
}RecvModBus;


void vRxProtocol(RecvModBus *pRec, QUEUE * pQ);

void MBSlavePoll(void);
void Usart_received_line(UCHAR *buff,ULONG len);

#endif

