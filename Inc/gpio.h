/**
  ******************************************************************************
  * File Name          : gpio.h
  * Date               : 06/07/2017 18:55:40
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
// Ö´ÐÐÆ÷
#define PIN_FLH			GPIO_PIN_11							//ÀëºÏ¿ØÖÆ½Å
#define PIN_FSC			GPIO_PIN_12							//É²³µ¿ØÖÆ½Å
						//¼±Í£¿ØÖÆ½Å
#define PIN_AS1         GPIO_PIN_6                          //É²³µ¸¨ÖúÐÅºÅÒý½Å      2018.12.24
#define PIN_AS2         GPIO_PIN_7                          //É²³µ¸¨ÖúÐÅºÅÒý½Å      2018.12.24

#define PORT_CTR		GPIOA

#define PORT_CAS        GPIOC

#define C_LALIHE()		HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_RESET)										//À­ÀëºÏ
#define C_SONGLIHE()	HAL_GPIO_WritePin(PORT_CTR, PIN_FLH,GPIO_PIN_SET)										
/*É²³µ¸¨ÖúÐÅºÅ*/
#define C_AS1_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_RESET)	
#define C_AS1_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS1,GPIO_PIN_SET)	
#define C_AS2_EN()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_RESET)	
#define C_AS2_DS()       HAL_GPIO_WritePin(PORT_CAS, PIN_AS2,GPIO_PIN_SET)


void MX_GPIO_Init(void);
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
