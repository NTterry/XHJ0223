

#include "stm32f1xx_hal.h"



#ifndef __ARRAY_H_
#define __ARRAY_H_

#define MAXQSIZE		128

#ifndef true
	#define true			1
#endif
	
#ifndef false
	#define false 		0
#endif

#define QUEUE_OK            0    /* No error, everything OK. */
#define QUEUE_ERR          -1    /* Out of memory error.     */
#define QUEUE_EMPTY        -3    /* Timeout.                	    */
#define QUEUE_FULL         -4    /* Routing problem.          */
#define QUEUE_TOO_SHORT    -5


typedef struct queueCDT{
	int8_t data[MAXQSIZE];
	int16_t front;
	int16_t rear;
	int32_t act_cnt;
}QUEUE;


void flushQueue(QUEUE * q);

int inQueue(QUEUE *q,int8_t val);
int outQueue(QUEUE *q,int8_t *pval);

int getQueueLength(QUEUE *q);
int getQueueBuff(QUEUE *q,int8_t *pval,int len);
int inQueueBuff(QUEUE *q,int8_t *pval,int len);
int getQueueIdx(QUEUE *q);

#endif
