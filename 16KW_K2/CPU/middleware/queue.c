/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.c
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

#include "queue.h"



/**
* @brief: QueueInit
* @param: obj                         
* @param: queue_depth                 
* @param: queue_len                   
* @return: 
* @note: queue_len表示每个队列的大小，queue_depth表示需要设置多少队列深度，需要为队列pData申请queue_depth * queue_len大小的内存空间
*
*/
bool QueueInit(Queue_t *obj, uint16_t queue_depth, uint32_t queue_len)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = queue_depth;
    obj->len_per_queue = queue_len;

    obj->pData = NULL;

    obj->pData = (uint8_t *)malloc(queue_len * queue_depth);/*使用堆空间，需要把.s文件的heap_size改成合适值，也可以不用malloc，采用大数组的方式*/

    if(obj->pData == NULL)
    {
        return false;
    }
    return true;
}

/**
* @brief: QueueInit_1
* @param: obj                         
* @param: buff                        
* @param: queue_depth                 
* @param: queue_len                   
* @return: 
* @note: 
*
*/
bool QueueInit_1(Queue_t *obj, void* buff, uint16_t queue_depth, uint32_t queue_len)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = queue_depth;
    obj->len_per_queue = queue_len;

    obj->pData = NULL;

//  obj->pData = (uint8_t *)malloc(queue_len * queue_depth);/*使用堆空间，需要把.s文件的heap_size改成合适值，也可以不用malloc，采用大数组的方式*/
    obj->pData = (uint8_t *)buff;
    if(obj->pData == NULL)
    {
        return false;
    }
    return true;
}

/**
* @brief: QueueDeInit
* @param: obj                         
* @note: 
*
*/
void QueueDeInit(Queue_t *obj)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = 0;
    if(obj->pData != NULL)
    {
        free(obj->pData);/*和malloc对应*/
    }
}

/**
* @brief: QueueQuery
* @param: obj                         
* @return: true：队列已满
* @note: 
*
*/
bool QueueQuery(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return true;
    }
    if(obj->valid_num >= obj->depth)
    {
        return true;
    }
    return false;
}

/**
* @brief: QueuePut
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueuePut(Queue_t *obj, void *p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num >= obj->depth)
    {
        return false;
    }
    memcpy((uint8_t *)(&((uint8_t*)obj->pData)[obj->msg_in * obj->len_per_queue]), (uint8_t *)p, obj->len_per_queue);
    obj->msg_in++;
    obj->msg_in %= obj->depth;
    obj->valid_num++;
    return true;
}

/**
* @brief: QueueGet
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueueGet(Queue_t *obj, void **p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
    obj->msg_out++;
    obj->msg_out %= obj->depth;
    obj->valid_num--;
    return true;
}

/**
* @brief: QueueRead
* @param: obj                         
* @param: p                           
* @return: 
* @note: 
*
*/
bool QueueRead(Queue_t *obj, void **p)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
//  obj->msg_out++;
//  obj->msg_out %= obj->depth;
//  obj->valid_num--;
    return true;
}

/**
* @brief: QueueClear
* @param: obj                         
* @return: 
* @note: 
*
*/
bool QueueClear(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return false;
    }
    if(obj->valid_num == 0)
    {
        return false;
    }
//  *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
    obj->msg_out++;
    obj->msg_out %= obj->depth;
    obj->valid_num--;
    return true;
}

/**
* @brief: QueueIsFull
* @param: obj                         
* @return: 
* @note: 
*
*/
bool QueueIsFull(Queue_t *obj)
{
    if(obj->pData == NULL)
    {
        return true;
    }
    if(obj->valid_num >= obj->depth)
    {
        return true;
    }
    return false;
}
