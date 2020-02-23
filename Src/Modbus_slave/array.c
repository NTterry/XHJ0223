/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "array.h"
#include "string.h"

void flushQueue(QUEUE * q)
{
	q->front = 0;
	q->rear = 0;
	q->act_cnt = 0;
	memset(q->data,0,MAXQSIZE);
}

int inQueue(QUEUE *q,int8_t val)
{
	if((q->rear+1) % MAXQSIZE == q->front)
		return QUEUE_FULL;

	q->data[q->rear] = val;
	q->rear = (q->rear + 1) % MAXQSIZE;
	
	return true;	
}


int outQueue(QUEUE *q,int8_t *pval)
{
	if(q->front == q->rear)
		return QUEUE_EMPTY;
	
	*pval = q->data[q->front];
	q->front = (q->front + 1) % MAXQSIZE;
	
	return true;
}

int getQueueLength(QUEUE *q)
{
	return (q->rear - q->front + MAXQSIZE) % MAXQSIZE;
}


int getQueueBuff(QUEUE *q,int8_t *pval,int len)
{
	int date_num = 0;
	
	if(getQueueLength(q) < len)
		return QUEUE_TOO_SHORT;
	
	while(date_num < len)
	{
		outQueue(q,pval+date_num);
		date_num++;
	}
	
	return date_num;
}

int inQueueBuff(QUEUE *q,int8_t *pval,int len)
{
	int data_num,res;
	
	q->act_cnt++;

	for(data_num = 0;data_num < len;data_num++)
	{
		res = inQueue(q,pval[data_num]);
		if(res < QUEUE_OK)
			return data_num;
	}
	return data_num;
}

int getQueueIdx(QUEUE *q)
{
	if(q->act_cnt > 0)
		return q->act_cnt--;
	else
		return 0;
}

