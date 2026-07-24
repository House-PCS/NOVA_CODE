/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "mem_priv.h"
#include <assert.h>

/*此模块仅适用于初始化阶段的内存分配,内存来源于数据区的全局变量区，不做内存释放。
*注意：适用于32位MCU平台。
*////////////////////////////
//MemBlockType *MemBlockHead = NULL;
MemPrivType MemPriv = {0};
/** 
* @brief  私有内存管理模块初始化
* @param  buff：内存指针
* @param  len：可管理的内存字节数
* @return  none
* @note:
*
*/
void MemPrivInit(uint8_t *buff,uint32_t len)
{
    uint8_t temp = (uint32_t)buff % 4;
    if(temp == 0)
    {
        MemPriv.buff = buff;
        MemPriv.mem_max = len;
    }
    else
    {
        MemPriv.buff = buff + (4 - temp);
        MemPriv.mem_max = len - (4 - temp);
    }
}
/** 
* @brief  私有内存管理申请内存
* @param  len：申请内存字节数
* @return  uint8_t*：成功返回内存指针，失败返回NULL
* @note:
*
*/
uint8_t* m_malloc(uint16_t len)
{
    uint8_t *p = NULL;
//    _Static_assert((true),"错误！");
    if(MemPriv.buff == NULL)
    {
        while(1);   //请先初始化MemPriv。
    }
    else if(len == 0)
    {
        while(1);   //不允许的操作
    }
    else if((MemPriv.mem_hand + len) >MemPriv.mem_max)
    {
        while(1);   //请将MEM_PRIV_SIZE改为足够大。
    }
    else
    {
        p = &MemPriv.buff[MemPriv.mem_hand];
        uint8_t temp = len % 4;
        MemPriv.mem_hand += len;
        if(temp != 0)
        {
            MemPriv.mem_hand += (4 - temp);
        }
    }

    return p;
}
