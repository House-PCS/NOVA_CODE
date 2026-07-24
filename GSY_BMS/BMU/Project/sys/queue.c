
#include "queue.h"


/*queue_len表示每个队列的大小，queue_depth表示需要设置多少队列深度，需要为队列pData申请queue_depth * queue_len大小的内存空间,*/
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


void QueueDeInit(Queue_t *obj)
{
//	obj->msg_in = 0;
//	obj->msg_out = 0;
//	obj->valid_num = 0;
//	obj->depth = 0;
//	if(obj->pData != NULL)
//	{
//		free(obj->pData);/*和malloc对应*/
//	}
}

/**Function: QueueQuery
 * @brief 查询队列
 * @details 查询队列队尾指针，如果队列满，返回false
 * @param [in]  Queue_t     队列名
 * @param [in]  
 * @return true：队列未满
*/
bool QueueQuery(Queue_t *obj)      
{
	if(obj->pData == NULL)
	{
		return false;
	}
	if(obj->valid_num >= obj->depth)
	{
		return false;
	}
	return true;
}
/**Function: QueueQuery
 * @brief 队尾更新
 * @details 队尾指针指向后一个，如果队列满就覆盖当前队尾
 * @param [in]  Queue_t     队列名
 * @param [in]  p  赋值当前可写入的队尾指针
 * @return true：队列未满
*/
bool QueueRearAdd(Queue_t *obj,void **p)      
{
	if(obj->pData == NULL)
	{
        *p=NULL;
		return false;
	}
    obj->valid_num++;
	if(obj->valid_num >= obj->depth)
	{
        *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_in * obj->len_per_queue]);
		return false;
	}
    obj->msg_in++;
	obj->msg_in %= obj->depth;
	
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_in * obj->len_per_queue]);
	return true;
}
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
    if(obj->valid_num == 0)
    {
        if(obj->msg_out != obj->msg_in)//数据长度和队列输入输出位置不匹配
        {
            //可增加调试代码
            
        }
    }
	memcpy((uint8_t *)(&((uint8_t*)obj->pData)[obj->msg_in * obj->len_per_queue]), (uint8_t *)p, obj->len_per_queue);
	obj->msg_in++;
	obj->msg_in %= obj->depth;
	obj->valid_num++;
	return true;
}

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
    __disable_irq();//各类通信中断均有put队列操作，在此临时关闭中断，防止在get过程中触发中断，出现put和get的嵌套
	*p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
	obj->msg_out++;
	obj->msg_out %= obj->depth;
	obj->valid_num--;
    __enable_irq();
	return true;
}
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
//	obj->msg_out++;
//	obj->msg_out %= obj->depth;
//	obj->valid_num--;
	return true;
}
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
//	*p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
	obj->msg_out++;
	obj->msg_out %= obj->depth;
	obj->valid_num--;
	return true;
}
bool QueueIsFull()
{
    return false;
}
