#ifndef __RBUFFER_H__
#define __RBUFFER_H__

#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif  /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 环形缓冲区管理器，用户不要去直接操作内部成员变量 */
typedef struct
{
     uint8_t* buf;      /* 环形缓冲区地址 */
     uint32_t size;     /* 环形缓冲区大小 */
     uint32_t front;    /* 头指针 */
     uint32_t rear;     /* 尾指针 */
} rbuffer_t;

/**
 * @brief 初始化一个环形缓冲区
 * @param[in] rb 环形缓冲区管理器指针(必须非NULL）
 * @param[in] buf 数据缓冲区(必须非NULL)
 * @param[in] length buf长度(会向下裁剪到2的N次幂)
 */
void rbuffer_init(rbuffer_t* rb, uint8_t* buf, uint32_t length);

/**
 * @brief 清除环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 */
void rbuffer_clear(rbuffer_t* rb);

/**
 * @brief 获取环形缓冲区当前的数据长度
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @return 当前的数据长度
 *
 */
uint32_t rbuffer_length(rbuffer_t* rb);

/**
 * @brief 获取环形缓冲区当前未使用的数据长度
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @return 当前未使用的的数据长度
 */
uint32_t rbuffer_unused_length(rbuffer_t* rb);

/**
 * @brief 删除缓冲区多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] length 需要删除的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_trim_bytes(rbuffer_t* rb, uint32_t length);

/**
 * @brief 删除指定索引以后的缓冲区多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] idx 指定的索引
 * @param[in] length 需要删除的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_trim_idx(rbuffer_t* rb, uint32_t idx, uint32_t length);

/**
 * @brief 写入一字节到环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] dat 数据字节
 * @return 是否写入成功
 */
bool rbuffer_write_byte(rbuffer_t* rb, uint8_t dat);

/**
 * @brief 从环形缓冲区读取一字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] pdat 数据字节指针(必须非NULL)
 * @return 是否读取成功
 */
bool rbuffer_read_byte(rbuffer_t* rb, uint8_t* pdat);

/**
 * @brief 写入多个字节到环形缓冲区
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[in] buf 数据存储(必须非NULL)
 * @param[in] length 数据存储长度
 * @return 写入成功的字节数
 */
uint32_t rbuffer_write_bytes(rbuffer_t* rb, const uint8_t* buf, uint32_t length);

/**
 * @brief 从环形缓冲区读取多个字节
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] buf 数据存储(必须非NULL)
 * @param[in] length 需要读取的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_read_bytes(rbuffer_t* rb, uint8_t* buf, uint32_t length);

/**
 * @brief 从环形缓冲区读取多个字节(只拷贝不删除)
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @param[out] buf 数据存储(必须非NULL)
 * @param[in] length 需要读取的字节数
 * @return 实际读取的字节数
 */
uint32_t rbuffer_get_bytes(rbuffer_t* rb, uint8_t* buf, uint32_t length);


/**
 * @brief 从环形缓冲区获取头指针(只获取指针)
 * @param[in] rb 缓冲区实例指针(必须非NULL)
 * @return 头指针
 */
uint8_t* rbuffer_get_ptr(rbuffer_t* rb);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __RBUFFER_H__ */
