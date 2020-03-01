

/**
************************************************************
* @file         hal_key.h
* @brief        hal_key.c对应头文件
* @author       Gizwits
* @date         2016-09-05
* @version      V03010101
* @copyright    Gizwits
* 
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值?|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdio.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"

#define G_SET_BIT(a,b)                            (a |= (1 << b))
#define G_CLEAR_BIT(a,b)                          (a &= ~(1 << b))
#define G_IS_BIT_SET(a,b)                         (a & (1 << b))

#define KEY_TIMER_MS                            10
#define KEY_MAX_NUMBER                          20
#define DEBOUNCE_TIME                           40
#define PRESS_LONG_TIME                         1200

#define NO_KEY                                  0x00000000
#define KEY_DOWN                                0x10000000
#define KEY_UP                                  0x20000000
#define KEY_LIAN                                0x40000000
#define KEY_LONG                                0x80000000

typedef void (*gokitKeyFunction)(void);

__packed typedef struct
{
    uint32_t          keyNum;
    uint32_t         keyValidSta;
    GPIO_TypeDef     *keyPort;
    uint32_t         keyGpio;
    gokitKeyFunction shortPress; 
    gokitKeyFunction longPress; 
}keyTypedef_t;

__packed typedef struct
{
    uint32_t      keyTotolNum;
    keyTypedef_t *singleKey; 
}keysTypedef_t; 

void keyGpioInit(void);
void keyHandle(keysTypedef_t *keys);
void keyParaInit(keysTypedef_t *keys);
uint32_t getKey(keysTypedef_t *key);
uint32_t readKeyValue(keysTypedef_t *keys);
keyTypedef_t keyInitOne(uint32_t keyValidSta, GPIO_TypeDef * keyPort, uint32_t keyGpio, gokitKeyFunction shortPress, gokitKeyFunction longPress);

#endif /*_HAL_KEY_H*/


