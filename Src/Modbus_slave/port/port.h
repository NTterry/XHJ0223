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
 * File: $Id: port.h,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

#ifndef _PORT_H
#define _PORT_H

#include <assert.h>
#include <inttypes.h>

#define	INLINE
#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION( )			EnterCriticalSection( )//
#define EXIT_CRITICAL_SECTION( )        	ExitCriticalSection( )//

#define SystemcoreClock	72000000L


extern void EnterCriticalSection( void );
extern void ExitCriticalSection( void );

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

//master read_only(0x04)
#define REG_INPUT_START	 1000
#define REG_INPUT_NREGS	 20
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];

//master read_write 0x01  0x05  0x0F
#define REG_COILS_START	 0000
#define REG_COILS_SIZE	 20
extern uint16_t ucRegCoilsBuf[REG_COILS_SIZE];

//master read_write (0x03)  write sigle (0x06) write multi (0x10)   big fist
#define REG_HOLDING_START 2000  		//���ּĴ�����ʼ��ַ
#define REG_HOLDING_NREGS 50	    //���ּĴ�������
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];

/*master read_only coil (0x02)  litte first*/
#define REG_DISCRETE_START	 2000
#define REG_DISCRETE_SIZE	 10
extern uint16_t ucRegDiscreteBuf[REG_DISCRETE_SIZE];

#endif
