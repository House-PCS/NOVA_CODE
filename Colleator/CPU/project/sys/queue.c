/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        queue.c
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
#include "queue.h"
#include "main.h"

/**
  * @brief  queue队列初始化，创建新的队列
  * @param  obj:队列类型指针
  * @param  queue_depth:表示需要设置多少队列深度
  * @param  queue_len:表示每个队列数据的大小
  * @return NULL
  * @note   需要为队列pData(成员)申请queue_depth * queue_len大小的内存空间
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
	obj->pdata_status = (uint8_t *)malloc(sizeof(uint8_t) * queue_depth);
	if(obj->pdata_status == NULL)
	{
		return false;
	}
	memset(obj->pdata_status, 0, sizeof(uint8_t) * queue_depth);
	
	return true;
}



/**
  * @brief  queue队列反初始化，删除该队列
  * @param  obj:队列类型指针
  * @return NULL
  * @note   none
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
	obj->pData = NULL;
	if (obj->pdata_status)
	{
		free(obj->pdata_status);
	}
	obj->pdata_status = NULL;
}


/**
  * @brief  获取有多少成员入队
  * @param  obj:队列类型指针
  * @return 入队成员数量
  * @note   none
  */
uint16_t QueueValidSize(Queue_t *obj)
{
	return obj->valid_num;
}


/**
  * @brief  成员入队列
  * @param  obj:队列类型指针
  * @param  p:成员指针
  * @return NULL
  * @note   none
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
	obj->pdata_status[obj->msg_in]++;
	obj->msg_in++;
	obj->msg_in %= obj->depth;
	obj->valid_num++;
	return true;
}


/**
  * @brief  成员插队入队列
  * @param  obj:队列类型指针
  * @param  p:成员指针
  * @return NULL
  * @note   none
  */
bool QueueJumpPut(Queue_t *obj, void *p)
{
	if(obj->pData == NULL)
	{
		return false;
	}
	if(obj->valid_num >= obj->depth)
	{
		return false;
	}
    
    if(obj->msg_out == 0)
    {
        obj->msg_out = obj->depth - 1;
    }
    else
    {
        obj->msg_out--;
    }
    
	memcpy((uint8_t *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]), (uint8_t *)p, obj->len_per_queue);
	obj->pdata_status[obj->msg_out]++;
	obj->valid_num++;
	return true;
}


/**
  * @brief  成员出队列
  * @param  obj:队列类型指针
  * @param  p:成员指针的指针
  * @return NULL
  * @note   none
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
    
    __disable_irq();
    while (obj->valid_num && !obj->pdata_status[obj->msg_out])
	{
		obj->msg_out++;
		obj->msg_out %= obj->depth;
		obj->valid_num--;
	}
    if(obj->valid_num == 0)
	{
        __enable_irq();
		return false;
	}
    
	*p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
	obj->pdata_status[obj->msg_out] = 0;
	obj->msg_out++;
	obj->msg_out %= obj->depth;
	obj->valid_num--;
    __enable_irq();
	return true;
}


/**
  * @brief  队列首成员
  * @param  obj:队列类型指针
  * @param  p:成员指针的指针
  * @return 首成员  | NULL
  * @note   none
  */
void* QueueGetHeadPtr(Queue_t *obj)
{
	if(obj->pData == NULL)
	{
		return NULL;
	}
    if(obj->valid_num == 0)
	{
		return NULL;
	}
    __disable_irq();
	while (obj->valid_num && !obj->pdata_status[obj->msg_out])
	{
		obj->msg_out++;
		obj->msg_out %= obj->depth;
		obj->valid_num--;
	}
	if(obj->valid_num == 0)
	{
        __enable_irq();
		return NULL;
	}
    __enable_irq();
	return (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
}



/**
  * @brief  队列清空
  * @param  obj:队列类型指针
  * @return NULL
  * @note   none
  */
void QueueClear(Queue_t *obj)
{
    obj->msg_in = 0;
	obj->msg_out = 0;
	obj->valid_num = 0;
	memset(obj->pdata_status, 0, sizeof(uint8_t) * obj->depth);
}

//bool QueueIsFull()
//{
//    return false;
//}

/**
  * @brief  遍历队列
  * @param  obj:队列类型指针
  * @param  cb:遍历回调函数，用于上层业务做数据处理，返回值大于0则在队列中标记数据为无效值(即假删除)
  * @return 0成功遍历，非0遍历失败
  * @note   cb回调中做数据处理后可以返回大于0的处理逻辑，让队列假删某一条数据，变相实现了数据的中间删除动作
  */
int32_t QueueLoop(Queue_t *obj, int32_t (*cb)(void *userdata, void *data), 
				  void *userdata)
{
	if(obj->pData == NULL)
	{
		return -1;
	}
	if(obj->valid_num == 0)
	{
		return -1;
	}

	int32_t loop_ret = 0;
	int32_t loop_count = obj->valid_num;
	int32_t msg_out = obj->msg_out;
	void *p = NULL;

	while (loop_count >= 0)
	{
		p = (void *)(&((uint8_t*)obj->pData)[msg_out * obj->len_per_queue]);
		if (cb)
		{
			loop_ret = cb(userdata, p);
			if (loop_ret > 0)
			{
				// del this  data and set data is unused
				obj->pdata_status[msg_out] = 0;
			}
		}
		msg_out++;
		msg_out %= obj->depth;
		--loop_count;
	}

	return 0;
}

