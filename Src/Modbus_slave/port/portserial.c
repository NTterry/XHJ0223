/*
  * FreeModbus Libary: STM32F103X Port
  * Copyright (C) 2013 Liuweifeng  <lwfchat@qq.com,lwfchat@163.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

#include "stm32f1xx_hal.h"
#include "array.h"
#include "port.h"
#include "gpio.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "usart.h"


/* ----------------------- static functions ---------------------------------*/

static QUEUE u1_rx;



/* ----------------------- Start implementation -----------------------------*/
void  vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
	if( xTxEnable )//enable tx
    {
//		SEND485EN1(1);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);   // led show send begin to complete
	}
	if(xRxEnable)
	{
		// use send data callback function to control  down #HAL_UART_TxCpltCallback
	}
}

void vMBPortClose( void )
{

} 


BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity,eMBStopbit eStopbit )
{
    BOOL  bInitialized = TRUE;

    flushQueue(&u1_rx);
    MX_USART1_UART_Init(ulBaudRate);
    return bInitialized;
}

/*tool for modbus,need add action
 * we can use the DMA function to send a line with no wait
 * for mention,if send complete,we should change the dirtion for receive
 * this mcu ,we use send complete interrupt to change the dirction */
void MBSendData(UCHAR *buff,ULONG len)
{
	HAL_UART_Transmit_DMA(&huart1, buff, len);
}

/*send complete callback*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
//		SEND485EN1(0);  /*自动转为接收模式*/
//		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
	}
}
/*receive a line datas  call by user
 * if the mcu has received a line data,then call this function to
 * put the line_data to buff for parse;
 */
void Usart_received_line(UCHAR *buff,ULONG len)
{
	inQueueBuff(&u1_rx,(int8_t *)buff,len);
}

QUEUE * MB_GetRxQueue(void)
{
	return &u1_rx;
}
