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
  
  
#ifndef __MEM_PRIV_H_
#define __MEM_PRIV_H_
    
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct _mem_block_stru_t
{
    uint16_t mem_num;
    uint16_t mem_size;
    void *buff;
    struct _mem_block_stru_t *next;
}MemBlockType;
typedef struct _mem_priv_stru_t
{
    uint32_t mem_hand;
    uint32_t mem_max;
    uint8_t *buff;
}MemPrivType;


void MemPrivInit(uint8_t *buff,uint32_t len);
uint8_t* m_malloc(uint16_t len);
    
#endif
