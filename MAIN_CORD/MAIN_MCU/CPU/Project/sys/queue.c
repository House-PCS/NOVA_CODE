
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
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
    obj->depth = 0;
    if(obj->pData != NULL)
    {
        free(obj->pData);/*和malloc对应*/
    }
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
    *p = (void *)(&((uint8_t*)obj->pData)[obj->msg_out * obj->len_per_queue]);
    obj->msg_out++;
    obj->msg_out %= obj->depth;
    obj->valid_num--;
    return true;
}

void QueueClear(Queue_t *obj)
{
    obj->msg_in = 0;
    obj->msg_out = 0;
    obj->valid_num = 0;
}

//bool QueueIsFull()
//{
//    return false;
//}


