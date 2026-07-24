/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
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

#include "dev_system_power.h"
#define LOG_TAG "dev_system_power"

/**
 * @brief  : 开启电源保持
 * @para   : None
 * @note   : None
 * @return : None
 */
void PowerHoldOn()
{
    GpioBitSet((&McuPowerEnHandle));
}

/**
 * @brief  : 关闭电源保持
 * @para   : None
 * @note   : None
 * @return : None
 */
void PowerHoldOff()
{
    GpioBitReset((&McuPowerEnHandle));
}

/**
 * @brief  : 过流模块开启
 * @para   : None
 * @note   : None
 * @return : None
 */
void CCEnOn()
{
    //开启限流电源使能
    GpioBitSet((&CCPowerEnHandle));
    //开启限流
    GpioBitSet((&CCEnHandle));
}

/**
 * @brief  : 过流模块关闭
 * @para   : None
 * @note   : None
 * @return : None
 */
void CCEnOff()
{
    //关闭限流
    GpioBitReset((&CCEnHandle));
    //关闭限流电源使能
    GpioBitReset((&CCPowerEnHandle));
}



/**
 * @brief  : 泄放电路关闭
 * @para   : None
 * @note   : None
 * @return : None
 */
void ReleaseOff(void)
{
    GpioBitReset((&ReleaseHandle));
}

/**
 * @brief  : 泄放电路开启
 * @para   : None
 * @note   : None
 * @return : None
 */
void ReleaseOn(void)
{
    GpioBitSet((&ReleaseHandle));
}


/**
 * @brief  : 清除Dsg过流状态标志
 * @para   : None
 * @note   : None
 * @return : None
 */
void OCStatusClear()
{
    GpioBitSet((&Dsg_OC_Clear_Handle));
    soft_delayms(5);
    GpioBitReset((&Dsg_OC_Clear_Handle));
}


/**
 * @brief  : 过压检测
 * @para   : None
 * @note   : None
 * @return : true： 过压
             false：无过压
 */
bool OVCheck()
{
    return GpioBitGet((&OVCheckHandle));
}    

/**
 * @brief  : 充电器检测
 * @para   : None
 * @note   : None
 * @return : true： 过流
             false：无过流
 */
bool ChargerCheck()
{
	if(GpioBitGet((&ChargerCheckHandle)) == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}  
    
/**
 * @brief  : DSG过流检测
 * @para   : None
 * @note   : None
 * @return : true： 过流
             false：无过流
 */
bool DsgOCCheck()
{
	if(GpioBitGet((&Dsg_OC_Check_Handle)) == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief  : 加热膜短路检测
 * @para   : None
 * @note   : None
 * @return : true： 短路
             false：正常
 */
bool HeatshortCheck()
{
   if ((GpioBitGet((&HeatEnHandle)) == RESET) && (GpioBitGet((&HeatCheckHandle))  == RESET))
	{
		return true;
	}
	else
	{
		return false;
	}
} 


void SystemPowerBspInit(void)
{
//    LedDCPowerGpioBspInit();
//    LedDCPowerOn();
//    if(geBMSType == EN_BMS_HOST)
//    {
//        CanPowerGpioBspInit();
//        CanPowerOn();
//        RS232PowerGpioBspInit();
//        RS232PowerOn();
//    }
//    else if(geBMSType == EN_BMS_EXP)
//    {
//        Ex5VCheck1GpioBspInit();
//        ModeMorsGpioBspInit();
//        ModeMors1GpioBspInit();
//        LinkInGpioBspInit();
//        LinkIn1GpioBspInit();
//    }       
//    MpptPowerGpioBspInit();
//    EVBoardPowerGpioBspInit();
//    BMSBoardPowerGpioBspInit();
////    BMSBoardPowerOff();
//    MainBoardPowerOff();
//    MainBoardPowerGpioBspInit();
//    Ex5VCheckGpioBspInit();
    Dsg_OC_CheckGpioBspInit();
    Dsg_OC_ClearGpioBspInit();
    OCStatusClear();
}
