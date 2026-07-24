/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "ht7017.h"

unsigned char HT_Read(unsigned char add,unsigned char *pdata)
{
    unsigned char cnt = 0;
    
    pdata[cnt++] = HT_HEAD;
    pdata[cnt++] = add | (HT_RO << 7);
    
    return cnt;
}
unsigned char HT_Write(unsigned char add,unsigned short data,unsigned char *pdata)
{
    unsigned char cnt = 0;
    unsigned char sum = 0;
    
    pdata[cnt++] = HT_HEAD;
    pdata[cnt++] = add | (HT_WR << 7);
    pdata[cnt++] = data>>8;
    pdata[cnt++] = data;
    sum = pdata[0] + pdata[1] + pdata[2] + pdata[3] ;
    sum = ~sum;
    pdata[cnt++] = sum;
    
    return cnt;
}
