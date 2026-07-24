/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        common.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-27
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-27      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __COMMON_H_
#define __COMMON_H_


//用于各类需要等待响应的IC间操作结果
typedef enum
{
    EN_OP_RES_NONE,
    EN_OP_RES_ING,
    EN_OP_RES_COMPLETE,
    EN_OP_RES_FAIL
}EN_OP_RES;




#endif
