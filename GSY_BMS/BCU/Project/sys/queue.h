/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-07-23
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-08-07    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2022-07-23    <td>V1.0.1    <td>           <td>增加入队列插队模式
  * </table>
  ******************************************************************************
  */
#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
typedef struct _queue_stru_t
{
	void		*pData;
	uint8_t		*pdata_status; /*1为有效，0为无效但未Pop出队列的数据*/ 
	uint8_t		msg_in;/*入队列时对应的队列标号*/
	uint8_t		msg_out;/*出队列时对应的队列标号*/
	uint16_t	len_per_queue;	
	uint16_t	depth;
	uint16_t	valid_num;
	
}Queue_t;



bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len);
void QueueDeInit(Queue_t *obj);
uint16_t QueueValidSize(Queue_t *obj);
bool QueuePut(Queue_t *obj, void *p);
bool QueueJumpPut(Queue_t *obj, void *p);
bool QueueGet(Queue_t *obj, void **p);
void* QueueGetHeadPtr(Queue_t *obj);
void QueueClear(Queue_t *obj);
int32_t QueueLoop(Queue_t *obj, int32_t (*cb)(void *userdata, void *data), 
				  void *userdata);


#endif

