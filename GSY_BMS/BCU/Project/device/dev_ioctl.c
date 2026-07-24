
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ioctl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_ioctl.h"

static Gpio_HandleType *pOutHandle[] = {&gTwoWayOutputHandle[0], &gTwoWayOutputHandle[1], &gTwoWayOutputHandle[2], &gTwoWayOutputHandle[3],
										&gDryOutputHandle[0], &gDryOutputHandle[1],	
                                        &gLowSideOutputHandle[0], &gLowSideOutputHandle[1], &gLowSideOutputHandle[2], &gLowSideOutputHandle[3],
                                        &gHighSideOutputHandle[0], &gHighSideOutputHandle[1], &gHighSideOutputHandle[2], &gHighSideOutputHandle[3],
                                        &gIsoOutputHandle[0], &gIsoOutputHandle[1], &gIsoOutputHandle[2]};

static Gpio_HandleType *pInHandle[] = {&gTwoWayInputHandle[0], &gTwoWayInputHandle[1], &gTwoWayInputHandle[2], &gTwoWayInputHandle[3],
                                       &gActLowInputHandle[0], &gActLowInputHandle[1], &gActLowInputHandle[2],
                                       &gActHighInputHandle[0], &gActHighInputHandle[1], &gActHighInputHandle[2], &gActHighInputHandle[3],
                                       &gIsoInputHandle[0]};

/** 
* @brief    IO控制初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevIOCtlInit()
{
    IOCtlBspInit();
    /*高边输出默认置低*/
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_1);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_2);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_3);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_4);
    /*低边输出也默认置低(对外接口输出实际为高)*/
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_1);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_2);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_3);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_4);
    /*三路隔离输出默认置低(控制绝缘阻抗检测的继电器和两个光耦)*/
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_1);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_2);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_3);
    /*双向IO编址输出默认设置低*/
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_2WAY_OUT_1);
    DevIOCtlOutReset(EN_DEV_IOCTL_IDX_2WAY_OUT_2);
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




