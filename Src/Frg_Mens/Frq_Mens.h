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

