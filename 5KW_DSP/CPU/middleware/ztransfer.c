/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ztransfer.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-4-10
  * @brief       z变换传递函数的通用处理算法
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-4-10    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "ztransfer.h"

#include <string.h>

/**
 * @brief 初始化z变换传递函数体
 * @param obj 传递函数对象
 */
void ZtransInit(ZtransferObj_t* obj)
{
    memset(obj, 0, sizeof(ZtransferObj_t));
}

/**
 * @brief z变换传递函数的一次处理
 * @param trans 传递函数对象
 * @param input 输入
 * @return 输出
 */
float ZtransHandle(ZtransferObj_t* trans, float input)
{
    trans->un[0] = input;
    trans->yn[0] = trans->un[0] * trans->ku[0];
    for(int i = 1; i <= trans->max_order; ++i)
    {
        trans->yn[0] += trans->un[i] * trans->ku[i] - trans->yn[i] * trans->ky[i];
    }

    float output = trans->yn[0];
    float cache[16];
    memcpy(cache, trans->un, (trans->max_order + 1) * 4);
    memcpy(trans->un + 1, cache, trans->max_order * 4);
    memcpy(cache, trans->yn, (trans->max_order + 1) * 4);
    memcpy(trans->yn + 1, cache, trans->max_order * 4);

    return output;
}
