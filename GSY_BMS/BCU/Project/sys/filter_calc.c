/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        filter_calc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "filter_calc.h"



/** 
* @brief    函数用于比较两个整数
* @param    要比较的两个数的指针
* @return   none
* @note: 
*/
static int _int_cmp(const void *a, const void *b)  
{
    return (*(uint16_t*)a - *(uint16_t*)b);
}


/** 
* @brief    冒泡排序-升序
* @param    p_data:数据起始指针，count:数组元素个数，num:待排序数组单元素大小
* @return   none
* @note: 
*/
static void _bubbleAscending(void *p_data, uint16_t count, uint16_t num)
{
    uint8_t *p_start_addr_8 = (uint8_t *)p_data;
    uint16_t *p_start_addr_16 = (uint16_t *)p_data;
    uint32_t *p_start_addr_32 = (uint32_t *)p_data;
    
    for(uint16_t i = 0; i < count - 1; i++)
    {
        for(uint16_t j = 0; j < count - 1 - i; j++)
        {
            if(num == 1)
            {
                uint8_t temp;
                if(p_start_addr_8[j] > p_start_addr_8[j + 1])
                {
                    temp = p_start_addr_8[j + 1];
                    p_start_addr_8[j + 1] = p_start_addr_8[j];
                    p_start_addr_8[j] = temp;
                }
            }
            else if(num == 2)
            {
                uint16_t temp;
                if(p_start_addr_16[j] > p_start_addr_16[j + 1])
                {
                    temp = p_start_addr_16[j + 1];
                    p_start_addr_16[j + 1] = p_start_addr_16[j];
                    p_start_addr_16[j] = temp;
                }
            }
            else if(num == 4)
            {
                uint32_t temp;
                if(p_start_addr_32[j] > p_start_addr_32[j + 1])
                {
                    temp = p_start_addr_32[j + 1];
                    p_start_addr_32[j + 1] = p_start_addr_32[j];
                    p_start_addr_32[j] = temp;
                }
            }
        }
    }
}



/** 
* @brief    计算数组中元素的中值
* @param    p_data:对应数组，count：数组元素个数
* @return   none
* @note: 
*/
uint16_t FilterCalcMedian(uint16_t *p_data, uint16_t count)
{
//    qsort(p_data, count, sizeof(uint16_t), _int_cmp);
    _bubbleAscending(p_data, count, sizeof(uint16_t));
    if(count % 2 != 0)
    {
        return p_data[count / 2];
    }
    else
    {
        return ((p_data[count / 2 - 1] + p_data[count / 2]) / 2);
    }
}