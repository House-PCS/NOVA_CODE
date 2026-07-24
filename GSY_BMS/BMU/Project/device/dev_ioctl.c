/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ioctl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-26
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-26      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_ioctl.h"


//static Gpio_HandleType *pOutHandle[] = {&gJackIOHandle[0], &gJackIOHandle[2], &gMcuLedHandle};
static Gpio_HandleType *pOutHandle[] = {&gJackIOHandle[0], &gJackIOHandle[2]};
static Gpio_HandleType *pInHandle[] = {&gJackIOHandle[1], &gJackIOHandle[3]};



/** 
* @brief    IO控制初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevIOCtlInit()
{
    JackIOBspInit();
//    McuLedBspInit();
}





/** 
* @brief    IO输出置高
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevIOCtlOutSet(EN_DEV_IOCTL_IDX_OUT idx)
{
	GpioBitSet(pOutHandle[idx]);
}




/** 
* @brief    IO输出置低
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevIOCtlOutReset(EN_DEV_IOCTL_IDX_OUT idx)
{
	GpioBitReset(pOutHandle[idx]);
}


/** 
* @brief    IO输入回读
* @param    none
* @param    none
* @return   0为低电平，1为高电平
* @note:
*/
uint8_t DevIOCtlInGet(EN_DEV_IOCTL_IDX_IN idx)
{
    if(GpioBitGet(pInHandle[idx]) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


/** 
* @brief    IO输入回读取反
* @param    none
* @param    none
* @return   0为高电平，1为低电平
* @note:
*/
uint8_t DevIOCtlInGetInvert(EN_DEV_IOCTL_IDX_IN idx)
{
    if(GpioBitGet(pInHandle[idx]) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}