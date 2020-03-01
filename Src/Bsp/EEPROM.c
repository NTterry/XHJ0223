

#include "EEPROM.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "config.h"
/**
  ******************************************************************************
  * 文件名称： EEPROM.c 
  * 作者    : 
  * 版本: V1.0
  * 日期: 2017-7-20
  * 功能: EEPROM(AT24C08)底层程序
  ******************************************************************************
  */
/* ????? ----------------------------------------------------------------*/

/* ?????? --------------------------------------------------------------*/
/* ????? ----------------------------------------------------------------*/
#define EVAL_I2Cx_TIMEOUT_MAX                   3000

/* ???? ------------------------------------------------------------------*/
uint32_t I2cxTimeout = EVAL_I2Cx_TIMEOUT_MAX;

extern I2C_HandleTypeDef hi2c2;
extern void MX_I2C2_Init(void);



/**
  * 函数功能： I2C通信错误处理函数
  * 输入参数： 无
  * 返回值: 无
  * 说明: 一般在I2C 通信超时时调用
  */
static void I2C_EEPROM_Error (void)
{
  /* 反初始化 */
  HAL_I2C_DeInit(&hi2c2);
  
  /*重新初始化*/
  MX_I2C2_Init();
}

/**
  * 函数功能： 通过I2C写入一个值到指定的寄存器内
  * 输入参数: Addr:I2C设备地址
  *           Reg:目标寄存器
  *           Value:值
  * 返回值：无
  * 说明：无
  */
void I2C_EEPROM_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&hi2c2, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2cxTimeout);
  
  /* 检测I2C通信状态*/
  if(status != HAL_OK)
  {
    /* 调用I2C通信错误处理函数 */
    I2C_EEPROM_Error();
  }
}

/**
  * 函数功能：通过I2C写入一段数据到指定的寄存器内
  * 输入参数： Addr:设备地址
  *           Reg:目标寄存器值
  *           RegSize:寄存器尺寸
  *           pBuffer:缓冲区指针
  *           Length:缓冲区长度
  * 返回值: HAL_StatusTypeDef:操作结果
  * 说明: 在循环调用时需加一定的延时时间
  */
HAL_StatusTypeDef I2C_EEPROM_WriteBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&hi2c2, Addr, (uint16_t)Reg, RegSize, pBuffer, Length, I2cxTimeout); 

  /* 检测I2C通信状态 */
  if(status != HAL_OK)
  {
    /* 调用I2C通信错误处理函数 */
    I2C_EEPROM_Error();
  }        
  return status;
}


/**
  * 函数功能： 通过I2C读取一个指定寄存器内容
  * 输入参数： Addr:I2C设备地址
  *           Reg:目标寄存器
  * 返回值: uint8_t:寄存器内容
  * 说明：无
  */
uint8_t I2C_EEPROM_ReadData(uint16_t Addr, uint8_t Reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t value = 0;
  
  status = HAL_I2C_Mem_Read(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2cxTimeout);
 
  /*检测I2C通信状态 */
  if(status != HAL_OK)
  {
    /* 调用I2C通信错误处理函数*/
    I2C_EEPROM_Error();
  
  }
  return value;
}

/**
  * 函数功能 通过I2C读取一段寄存器内容存放到指定的缓冲区内
  * 输入参数： Addr:I2C设备地址
  *           Reg:目标寄存器
  *           RegSize:寄存器尺寸
  *           pBuffer:缓冲区指针
  *           Length:缓冲区长度
  * 返回值: HAL_StatusTypeDef:操作结果
  * 说明: 无
  */
HAL_StatusTypeDef I2C_EEPROM_ReadBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read(&hi2c2, Addr, (uint16_t)Reg, RegSize, pBuffer, Length, I2cxTimeout);
  

  if(status != HAL_OK)
  {
    I2C_EEPROM_Error();
  }        
  return status;
}

/**
  * 函数功能： 检测I2C设备是否处于准备好可以通行的状态
  * 输入参数： DevAddress:I2C设备地址
  *           Trials:尝试测试次数
  * 返回值: HAL_StatusTypeDef:操作结果
  * 说明: 无
  */
HAL_StatusTypeDef I2C_EEPROM_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{ 
  return (HAL_I2C_IsDeviceReady(&hi2c2, DevAddress, Trials, I2cxTimeout));
}


HAL_StatusTypeDef EE_DatasRead(uint16_t Addr,uint8_t *pBuffer, uint16_t Length)
{
	uint8_t i,len;
	uint8_t *p;
	HAL_StatusTypeDef status;
	
	len = Length;
	i = 0;
	p = pBuffer;
	while(len > 0)
	{
		if(len <= 16)
		{
			status = I2C_EEPROM_ReadBuffer(Addr,i,I2C_MEMADD_SIZE_8BIT,p,len);
			if(status) break;
			len = 0;
		}
		else
		{
			status = I2C_EEPROM_ReadBuffer(Addr,i,I2C_MEMADD_SIZE_8BIT,p,16);
			if(status) break;
			len -= 16;
			i += 16;
			p += 16;
		}
		osDelay(1);
	}
	
	return status;
}

HAL_StatusTypeDef EE_DatasWrite(uint16_t Addr,uint8_t *pBuffer, uint16_t Length)
{
	uint8_t i,len;
	uint8_t *p;
	HAL_StatusTypeDef status;
	
	len = Length;
	i = 0;
	p = pBuffer;
	while(len > 0)
	{
		if(len <= 16)
		{
			status = I2C_EEPROM_WriteBuffer(Addr,i,I2C_MEMADD_SIZE_8BIT,p,len);
			if(status) break;
			len = 0;
		}
		else
		{
			status = I2C_EEPROM_WriteBuffer(Addr,i,I2C_MEMADD_SIZE_8BIT,p,16);
			if(status) break;
			len -= 16;
			i += 16;
			p += 16;
		}
		osDelay(2);
	}	
	
	return status;
}

	

uint8_t buffers[100];
uint8_t start = 0;
void EEPROMtest(void)
{
	uint8_t i;
	
	for(i = 0;i < 100; i++)
		buffers[i] = i + start;
	start++;
	
	
//	printf("try %d  ",I2C_EEPROM_IsDeviceReady(0xA0,10));
	
	EE_DatasWrite(0xA0,buffers,100);
//	portENTER_CRITICAL();
//	I2C_EEPROM_WriteBuffer(0XA0,16,I2C_MEMADD_SIZE_8BIT,buffers,16);
//	portEXIT_CRITICAL();
//	for(i = 0;i < 60; i++)
//		buffers[i] = 0;
	osDelay(500);
	
	EE_DatasRead(0xA0,buffers,100);
//	portENTER_CRITICAL();
//	I2C_EEPROM_ReadBuffer(0xA0,16,I2C_MEMADD_SIZE_8BIT,buffers,16);
//	portEXIT_CRITICAL();
	for(i = 0; i < 100;i++)
	{
//		printf("%d ",buffers[i]);
	}
}

/******************* (C) COPYRIGHT 2015-2020 ????????? *****END OF FILE****/
