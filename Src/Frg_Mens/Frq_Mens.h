/*
 * Terry is pleased to support the open source community by making IC Capture Fqz available.
 * Copyright (C) 2020 Terry Limited, a Tencent company. All rights reserved.
   捕获模式测量频率
   1. 开启定时器捕获模式（Channel）
   2. 开启IC_Capture IRQ and Timer OverLoad IRQ
   3. 中断中，添加以上两个中断的回调函数
   4. 调用ICOverLoadIRQ() 得到当前频率  multipy 100
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef _FRQ_MENS_H_
#define _FRQ_MENS_H_
#include "stm32f1xx_hal.h"

#define TIM_PREIOD	50000
#define TIM_TICK	10		//10us

#define MAX_TIMUS			40000			// max 0.3s


void IC_Mens_Init(void);
void ICaptureIRQ(uint32_t ic_val);  	// In Capture IRQHandler
void ICOverLoadIRQ(void);				// In Timer over load IRQHandler
int Get_FRQE2(void);

#endif



//定时器的捕获模式配置如下，例子
//void MX_TIM2_Init(void)
//{
// 
//  TIM_MasterConfigTypeDef sMasterConfig = {0};
//  TIM_IC_InitTypeDef sConfigIC = {0};

//  htim2.Instance = TIM2;
//  htim2.Init.Prescaler = 719;
//  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
//  htim2.Init.Period = TIM_PREIOD - 1;
//  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
//  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
//  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
//  sConfigIC.ICFilter = 8;
//  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
//  {
//    //Error_Handler();
//  }
//  __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
//  
//  HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);
//}

//void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
//{

//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(tim_icHandle->Instance==TIM2)
//  {
//  /* USER CODE BEGIN TIM2_MspInit 0 */

//  /* USER CODE END TIM2_MspInit 0 */
//    /* TIM2 clock enable */
//    __HAL_RCC_TIM2_CLK_ENABLE();
//  
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    /**TIM2 GPIO Configuration    
//    PA0-WKUP     ------> TIM2_CH1 
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_0;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//    /* TIM2 interrupt Init */
//    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
//    HAL_NVIC_EnableIRQ(TIM2_IRQn);
//  /* USER CODE BEGIN TIM2_MspInit 1 */
//	
//  /* USER CODE END TIM2_MspInit 1 */
//  }
//}

