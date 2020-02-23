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
  * File: $Id: port.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
  */

/* ----------------------- System includes --------------------------------*/
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/

/* ----------------------- Variables ----------------------------------------*/

/*master read 0x03 write single 0x06  write multi 0x10*/
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS]= {0,0,0,0,0,0,0,0,0,0};

/*master read only  code 0x04*/
uint16_t usRegInputBuf[REG_INPUT_NREGS]={0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9};

/*master read  0x01  write coil 0x05 0x0F(multi) */
uint16_t ucRegCoilsBuf[REG_COILS_SIZE]={0x00,0x00,0x00,0x00,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9};

uint16_t ucRegDiscreteBuf[REG_DISCRETE_SIZE]={0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9};
/* ----------------------- Start implementation -----------------------------*/

void
EnterCriticalSection(  )
{
    //�����ٽ���
}

void
ExitCriticalSection(  )
{
    //�˳��ٽ���
}


/*���ּĴ����Ķ���д����*/  
/**/
/**
	* @brief ���ּĴ��������������ּĴ����ɶ���д
	* @param pucRegBuffer ������ʱ--��������ָ�룬д����ʱ--��������ָ��
	* usAddress �Ĵ�����ʼ��ַ
	* usNRegs �Ĵ�������
	* eMode ������ʽ��������д
	* @retval eStatus �Ĵ���״̬
*/
eMBErrorCode 
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
eMBRegisterMode eMode )
{
	//����״̬
	eMBErrorCode eStatus = MB_ENOERR;
	//ƫ����
    int16_t iRegIndex;
	usAddress--;//��Modbus�й� Э���ַ��PLC��ַ������
 	//�жϼĴ����ǲ����ڷ�Χ��
 	if( ( (int16_t)usAddress >= REG_HOLDING_START ) \
 	&& ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
 	{
 	    //����ƫ����
        iRegIndex = ( int16_t )( usAddress - REG_HOLDING_START );

    	switch ( eMode )
     	{
            //�������� 
            case MB_REG_READ:
                while( usNRegs > 0 )
                {
                    *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] >> 8 );
                    *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] & 0xFF );
                    iRegIndex++;
                    usNRegs--;
                }
								
                break;
 	        //д������ 
            case MB_REG_WRITE:
                while( usNRegs > 0 )
                {
                    usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                    usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                    iRegIndex++;
                    usNRegs--;
                }
                break;
        }
    }
    else
    {
        //���ش���״̬
        eStatus = MB_ENOREG;
    }
 
    return eStatus;
}		

/**
	* @brief ����Ĵ���������������Ĵ����ɶ���������д��
	* @param pucRegBuffer ��������ָ��
	* usAddress �Ĵ�����ʼ��ַ
	* usNRegs �Ĵ�������
	* @retval eStatus �Ĵ���״̬
*/
eMBErrorCode 
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode eStatus = MB_ENOERR;
    int16_t iRegIndex;
 	usAddress--;//��Modbus�й� Э���ַ��PLC��ַ������
    //��ѯ�Ƿ��ڼĴ�����Χ��
    //Ϊ�˱��⾯�棬�޸�Ϊ�з�������
    if( ( (int16_t)usAddress >= REG_INPUT_START ) \
    && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        //��ò���ƫ���������β�����ʼ��ַ-����Ĵ����ĳ�ʼ��ַ
        iRegIndex = ( int16_t )( usAddress - REG_INPUT_START );
        //�����ֵ
        while( usNRegs > 0 )
        {
            //��ֵ���ֽ�
            *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] >> 8 );
            //��ֵ���ֽ�
            *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] & 0xFF );
            //ƫ��������
            iRegIndex++;
            //�������Ĵ��������ݼ�
            usNRegs--;
        }
    }
    else
    {
        //���ش���״̬���޼Ĵ��� 
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
 	* @brief ��Ȧ�Ĵ�������������Ȧ�Ĵ����ɶ���д
 	* @param pucRegBuffer ������---��������ָ�룬д����--��������ָ��
 	* usAddress �Ĵ�����ʼ��ַ
 	* usNRegs �Ĵ�������
 	* eMode ������ʽ��������д
 	* @retval eStatus �Ĵ���״̬
 */	 
eMBErrorCode 
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,eMBRegisterMode eMode )
{
    eMBErrorCode eStatus = MB_ENOERR;
    //�Ĵ�������
    int16_t iNCoils = ( int16_t )usNCoils;
    //�Ĵ���ƫ����
    int16_t usBitOffset;
 	usAddress--;//��Modbus�й� Э���ַ��PLC��ַ������
    //���Ĵ����Ƿ���ָ����Χ��
    if( ( (int16_t)usAddress >= REG_COILS_START ) &&
    ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE * 16 ) )  // Terry multiply by 16 2020.2.22
    {
        //����Ĵ���ƫ����
        usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
        switch ( eMode )
        {
            //������
            case MB_REG_READ:
                while( iNCoils > 0 )
                {
                    *pucRegBuffer++ = xMBUtilGetBits( (int16_t *)ucRegCoilsBuf, usBitOffset,
                                     ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
                    iNCoils -= 8;
                    usBitOffset += 8;
                 }
                 break;
             //д����
             case MB_REG_WRITE:
                 while( iNCoils > 0 )
                 {
                     xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
                                   ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),*pucRegBuffer++ );
                     iNCoils -= 8;
                 }
                 break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}
/**
	* @brief ��������Ĵ�������������������Ĵ�����ֻ��
	* @param pucRegBuffer ������---��������ָ�룬д����--��������ָ��
	* usAddress �Ĵ�����ʼ��ַ
	* usNRegs �Ĵ�������
	* eMode ������ʽ��������д
	* @retval eStatus �Ĵ���״̬
*/

eMBErrorCode 
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode eStatus = MB_ENOERR;
    //�����Ĵ�������
    int16_t iNDiscrete = ( int16_t )usNDiscrete;
    //ƫ����
    uint16_t usBitOffset;
 	usAddress--;//��Modbus�й� Э���ַ��PLC��ַ������
    //�жϼĴ���ʱ�����ƶ���Χ��
    if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
      ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE * 16) )  // Terry multiply by 16 2020.2.22
    {
        //���ƫ����
        usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );
 
        while( iNDiscrete > 0 )
        {
            *pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
                             ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );
            iNDiscrete -= 8;
            usBitOffset += 8;
        }
 
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

