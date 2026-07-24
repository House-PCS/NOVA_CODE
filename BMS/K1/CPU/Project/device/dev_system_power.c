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
#include "fault_analysis.h"
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
 * @brief  : 限流模块是否开启
 * @para   : None
 * @note   : None
* @return : SET:true   RESET:false
 */
bool _CCIsOpen(void)
{
    if((GpioOutputBitGet((&CCEnHandle)) == SET) && (GpioOutputBitGet((&CCPowerEnHandle)) == true))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief  : 过流模块开启
 * @para   : None
 * @note   : None
 * @return : None
 */
void CCEnOn()
{
    if(true == _CCIsOpen())
    {
        return;
    }

    //开启限流电源使能
    GpioBitSet((&CCPowerEnHandle));
    //开启限流
    GpioBitSet((&CCEnHandle));
    gFaultAnalysisFlag.cc_enable_flag = 1;
    gFaultAnalysisFlag.cc_open_count++;
}

/**
 * @brief  : 过流模块关闭
 * @para   : None
 * @note   : None
 * @return : None
 */
void CCEnOff()
{
    if(false == _CCIsOpen())
    {
        return; 
    }

    //关闭限流
    GpioBitReset((&CCEnHandle));
    //关闭限流电源使能
    GpioBitReset((&CCPowerEnHandle));
    gFaultAnalysisFlag.cc_enable_flag = 0;
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
    soft_delayus(800);
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
 * @brief  : 开启加热膜
 * @para   : None
 * @note   : None
 * @return : 加热膜使能引脚状态
 */
bool HeatOpen()
{
    GpioBitSet((&HeatEnHandle));
    return  GpioBitGet((&HeatEnHandle));
}

/**
 * @brief  : 关闭加热膜
 * @para   : None
 * @note   : None
 * @return : None
 */
bool HeatClose()
{
     GpioBitReset((&HeatEnHandle));
    return  GpioBitGet((&HeatEnHandle));
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
   if (((GpioBitGet((&HeatEnHandle)) == RESET) && (GpioBitGet((&HeatCheckHandle))  == RESET)) ||\
       ((GpioBitGet((&HeatEnHandle)) == SET) && (GpioBitGet((&HeatCheckHandle))  == SET)))
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
