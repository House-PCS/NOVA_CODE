
#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
typedef struct _queue_stru_t
{
	void		*pData;
	uint8_t		msg_in;/*入队列时对应的队列标号*/
	uint8_t		msg_out;/*出队列时对应的队列标号*/
	uint16_t	len_per_queue;	
	uint16_t	depth;
	uint16_t	valid_num;
	
}Queue_t;



bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len);
void QueueDeInit(Queue_t *obj);
bool QueuePut(Queue_t *obj, void *p);
bool QueueGet(Queue_t *obj, void **p);
bool QueueIsFull(void);


#endif


































