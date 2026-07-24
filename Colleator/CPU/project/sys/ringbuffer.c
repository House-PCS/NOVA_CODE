#include <stddef.h>
#include <string.h>
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 计算最小值 */
#define RBUFFER_GET_MIN(a, b) ((a) < (b)) ? (a) : (b)

/**
 * @brief 向下裁剪到2的N次幂
 * @param[in] x 数值
 * @return 向下裁剪到2的N次幂后的值
 * @example 输入17，输出16，输入4，输出4
 */
static uint32_t rbuffer_trim_to_2_pow_n(uint32_t x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return (x + 1) >> 1;
}

/**
 * @brief 初始化一个环形缓冲区
 * @param[in] rb 环形缓冲区管理器指针(必须非NULL）
 * @param[in] buf 数据缓冲区(必须非NULL)
 * @param[in] length buf长度(会向下裁剪到2的N次幂)
 */
void rbuffer_init(rbuffer_t* rb, uint8_t* buf, uint32_t length)
{
    if (rb == NULL)
    {
        return;
    }

    if (buf == NULL)
    {
        rb->size = 0;
        rb->front = rb->rear = 0;
        return;
    }

    rb->buf = buf;
    rb->size = rbuffer_trim_to_2_pow_n(length);
    rb->front = rb->rear = 0;
}

/**
 * @brief 清除环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 */
void rbuffer_clear(rbuffer_t* rb)
{
    rb->front = rb->rear = 0;
}

/**
 * @brief 获取环形缓冲区当前的数据长度
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @return 当前的数据长度
 */
uint32_t rbuffer_length(rbuffer_t* rb)
{
    return (uint32_t)(rb->rear - rb->front);
}

uint32_t rbuffer_unused_length(rbuffer_t* rb)
{
    return (uint32_t)(rb->size - rbuffer_length(rb));
}

/**
 * @brief 删除缓冲区多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] length 需要删除的字节数
 * @return 实际删除的字节数
 */
uint32_t rbuffer_trim_bytes(rbuffer_t* rb, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->rear - rb->front;
    length = RBUFFER_GET_MIN(length, left);
    rb->front += length;
    if (rb->front == rb->rear) {
        rb->front = rb->rear = 0;
    }
    return length;
}

/**
 * @brief 删除指定索引以后的缓冲区多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] idx 指定的索引
 * @param[in] length 需要删除的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_trim_idx(rbuffer_t* rb, uint32_t idx, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->rear - rb->front;
    i = RBUFFER_GET_MIN(length, left);
    memmove(rb->buf + ((rb->front+i) & (rb->size - 1)), 
		   rb->buf + ((rb->front) & (rb->size - 1)), idx);
    rb->front += i;
    if (rb->front == rb->rear) {
        rb->front = rb->rear = 0;
    }
    return i;
}

uint32_t rbuffer_trim_idx_old(rbuffer_t* rb, uint32_t idx, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->rear - rb->front;
    i = RBUFFER_GET_MIN(length, left);
    memmove(rb->buf + ((rb->front+i) & (rb->size - 1)), 
		   rb->buf + ((rb->front) & (rb->size - 1)), idx);
    rb->front += i;
    if (rb->front == rb->rear) {
        rb->front = rb->rear = 0;
    }
    return i;
}

/**
 * @brief 写入一字节到环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] dat 数据字节
 * @return 是否写入成功
 */
bool rbuffer_write_byte(rbuffer_t* rb, uint8_t dat)
{
    uint32_t left;
    left = rb->size + rb->front - rb->rear;
    if (left)
    {
        *(uint8_t*)(rb->buf + (rb->rear & (rb->size - 1))) = dat;
        rb->rear++;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief 从环形缓冲区读取一字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] pdat 数据字节指针(必须非NULL)
 * @return 是否读取成功
 */
bool rbuffer_read_byte(rbuffer_t* rb, uint8_t* pdat)
{
    uint32_t left;
    left = rb->rear - rb->front;
    if (left)
    {
        *pdat = *(uint8_t*)(rb->buf + (rb->front & (rb->size - 1)));
        rb->front++;
        if (rb->front == rb->rear) {
            rb->front = rb->rear = 0;
        }
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief 写入多个字节到环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] buf 数据存储(必须非NULL)
 * @param[in] length 数据存储长度
 * @return 写入成功的字节数
 */
uint32_t rbuffer_write_bytes(rbuffer_t* rb, const uint8_t* buf, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->size + rb->front - rb->rear;
    length = RBUFFER_GET_MIN(length, left);
    i = RBUFFER_GET_MIN(length, (rb->size - (rb->rear & (rb->size - 1))));
    memcpy(rb->buf + (rb->rear & (rb->size - 1)), buf, i);
    if (length - i > 0) {
    memcpy(rb->buf, buf + i, length - i);
    }
    rb->rear += length;
    return length;
}

/**
 * @brief 从环形缓冲区读取多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] buf 数据存储(必须非NULL)
 * @param[in] length 需要读取的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_read_bytes(rbuffer_t* rb, uint8_t* buf, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->rear - rb->front;
    length = RBUFFER_GET_MIN(length, left);
    i = RBUFFER_GET_MIN(length, rb->size - (rb->front & (rb->size - 1)));
    memcpy(buf, rb->buf + (rb->front & (rb->size - 1)), i);
    if (length - i > 0) {
    memcpy(buf + i, rb->buf, length - i);
    }
    rb->front += length;
    if (rb->front == rb->rear) {
        rb->front = rb->rear = 0;
    }
    return length;
}

/**
 * @brief 从环形缓冲区读取多个字节(只拷贝不删除)
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] buf 数据存储(必须非NULL)
 * @param[in] length 需要读取的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_get_bytes(rbuffer_t* rb, uint8_t* buf, uint32_t length)
{
    uint32_t i;
    uint32_t left;
    left = rb->rear - rb->front;
    length = RBUFFER_GET_MIN(length, left);
    i = RBUFFER_GET_MIN(length, rb->size - (rb->front & (rb->size - 1)));
    memcpy(buf, rb->buf + (rb->front & (rb->size - 1)), i);
    if (length - i > 0) {
    memcpy(buf + i, rb->buf, length - i);
    }
    return length;
}

/**
 * @brief 从环形缓冲区获取头指针(只获取指针)
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @return 头指针
 */
uint8_t* rbuffer_get_ptr(rbuffer_t* rb)
{
	return rb->buf + (rb->front & (rb->size - 1));
}

#ifdef __cplusplus
}
#endif  /* __cplusplus */
