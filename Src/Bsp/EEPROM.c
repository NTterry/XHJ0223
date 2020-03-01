

#include "EEPROM.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "config.h"
/**
  ******************************************************************************
  * �ļ����ƣ� EEPROM.c 
  * ����    : 
  * �汾: V1.0
  * ����: 2017-7-20
  * ����: EEPROM(AT24C08)�ײ����
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
  * �������ܣ� I2Cͨ�Ŵ�������
  * ��������� ��
  * ����ֵ: ��
  * ˵��: һ����I2C ͨ�ų�ʱʱ����
  */
static void I2C_EEPROM_Error (void)
{
  /* ����ʼ�� */
  HAL_I2C_DeInit(&hi2c2);
  
  /*���³�ʼ��*/
  MX_I2C2_Init();
}

/**
  * �������ܣ� ͨ��I2Cд��һ��ֵ��ָ���ļĴ�����
  * �������: Addr:I2C�豸��ַ
  *           Reg:Ŀ��Ĵ���
  *           Value:ֵ
  * ����ֵ����
  * ˵������
  */
void I2C_EEPROM_WriteData(uint16_t Addr, uint8_t Reg, uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&hi2c2, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2cxTimeout);
  
  /* ���I2Cͨ��״̬*/
  if(status != HAL_OK)
  {
    /* ����I2Cͨ�Ŵ������� */
    I2C_EEPROM_Error();
  }
}

/**
  * �������ܣ�ͨ��I2Cд��һ�����ݵ�ָ���ļĴ�����
  * ��������� Addr:�豸��ַ
  *           Reg:Ŀ��Ĵ���ֵ
  *           RegSize:�Ĵ����ߴ�
  *           pBuffer:������ָ��
  *           Length:����������
  * ����ֵ: HAL_StatusTypeDef:�������
  * ˵��: ��ѭ������ʱ���һ������ʱʱ��
  */
HAL_StatusTypeDef I2C_EEPROM_WriteBuffer(uint16_t Addr, uint8_t Reg, uint16_t RegSize, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write(&hi2c2, Addr, (uint16_t)Reg, RegSize, pBuffer, Length, I2cxTimeout); 

  /* ���I2Cͨ��״̬ */
  if(status != HAL_OK)
  {
    /* ����I2Cͨ�Ŵ������� */
    I2C_EEPROM_Error();
  }        
  return status;
}


/**
  * �������ܣ� ͨ��I2C��ȡһ��ָ���Ĵ�������
  * ��������� Addr:I2C�豸��ַ
  *           Reg:Ŀ��Ĵ���
  * ����ֵ: uint8_t:�Ĵ�������
  * ˵������
  */
uint8_t I2C_EEPROM_ReadData(uint16_t Addr, uint8_t Reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t value = 0;
  
  status = HAL_I2C_Mem_Read(&hi2c2, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2cxTimeout);
 
  /*���I2Cͨ��״̬ */
  if(status != HAL_OK)
  {
    /* ����I2Cͨ�Ŵ�������*/
    I2C_EEPROM_Error();
  
  }
  return value;
}

/**
  * �������� ͨ��I2C��ȡһ�μĴ������ݴ�ŵ�ָ���Ļ�������
  * ��������� Addr:I2C�豸��ַ
  *           Reg:Ŀ��Ĵ���
  *           RegSize:�Ĵ����ߴ�
  *           pBuffer:������ָ��
  *           Length:����������
  * ����ֵ: HAL_StatusTypeDef:�������
  * ˵��: ��
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
  * �������ܣ� ���I2C�豸�Ƿ���׼���ÿ���ͨ�е�״̬
  * ��������� DevAddress:I2C�豸��ַ
  *           Trials:���Բ��Դ���
  * ����ֵ: HAL_StatusTypeDef:�������
  * ˵��: ��
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
