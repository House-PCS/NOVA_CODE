/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "F28x_Project.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
typedef struct _queue_stru_t
{
    void        *pData;
    uint8_t     msg_in;/*入队列时对应的队列标号*/
    uint8_t     msg_out;/*出队列时对应的队列标号*/
    uint16_t    len_per_queue;
    uint16_t    depth;
    uint16_t    valid_num;

} Queue_t;



bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len);
bool QueueInit_1(Queue_t *obj, void* buff,uint16_t queue_depth, uint32_t queue_len);
void QueueDeInit(Queue_t *obj);
bool QueueQuery(Queue_t *obj);
bool QueuePut(Queue_t *obj, void *p);
bool QueueGet(Queue_t *obj, void **p);
bool QueueRead(Queue_t *obj, void **p);
bool QueueIsFull(Queue_t *obj);
bool QueueClear(Queue_t *obj);

extern Queue_t  gDevCanRevQueue[];

#endif
