/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        peri_ctl_stat_api.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-04 
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-04    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "peri_ctl_stat_api.h"

bool test_tool_flag = false;

/** 
* @brief    总正继电器控制(高边输出)
* @param    value:true闭合，false断开
* @return   none
* @note:
*/
void RelayPCtl(bool value)
{
	if(!test_tool_flag)
	{
		if(value == true)
		{
			DevIOCtlOutSet(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_1);
		}
		else
		{
			DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_1);
		}
	}
}

/** 
* @brief    总正继电器反馈(高有效输入)
* @param    none
* @return   true闭合，false断开
* @note:    需要取反
*/
bool RelayPFeedback()
{
    if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_ACT_HIGH_IN_1) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/** 
* @brief    预充继电器控制(高边输出)
* @param    value:true闭合，false断开
* @return   none
* @note:
*/
void RelayPreCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_3);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_3);
    }
}


/** 
* @brief    预充继电器反馈(高有效输入)
* @param    none
* @return   true闭合，false断开
* @note:    需要取反
*/
bool RelayPreFeedback()
{
    if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_ACT_HIGH_IN_3) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/** 
* @brief    断路器控制(低边输出)
* @param    value:true触发脱扣，false释放脱扣控制(要恢复未脱扣状态，需要人为手动操作)
* @return   none
* @note:
*/
void BreakerCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_1);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_1);
    }
}

/** 
* @brief    断路器反馈(低有效输入)
* @param    none
* @return   true触发，false未触发
* @note:    需要取反
*/
bool BreakerFeedback()
{
    if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_ACT_LOW_IN_1) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}



/** 
* @brief    风扇继电器控制(低边输出)
* @param    value:true闭合，false断开
* @return   none
* @note:
*/
void RelayFanCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_2);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_2);
    }
}




/** 
* @brief    从控编址IO输出控制--主通道(双向IO输出)
* @param    value:true对外接口输出高电平(类开漏)，false对外接口输出低电平
* @return   none
* @note:    需要取反
*/
void BMUAddressingMainOutCtl(bool value)
{
    if(value == false)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_2WAY_OUT_1);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_2WAY_OUT_1);
    }
}


/** 
* @brief    编址IO反馈(双向IO输入)
* @param    none
* @return   true接口输入高电平，false接口输入低电平
* @note:    
*/
bool BMUAddressingMainInStat()
{
    if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_2WAY_IN_1) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}



/** 
* @brief    从控编址IO输出控制--备份通道(双向IO输出)
* @param    value:true对外接口输出高电平(类开漏)，false对外接口输出低电平
* @return   none
* @note:    需要取反
*/
void BMUAddressingBackupOutCtl(bool value)
{
    if(value == false)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_2WAY_OUT_2);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_2WAY_OUT_2);
    }
}


/** 
* @brief    从控编址IO输入检测(双向IO输入)
* @param    none
* @return   true接口输入高电平，false接口输入低电平
* @note:    
*/
bool BMUAddressingBackupInStat()
{
    if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_2WAY_IN_2) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}


/** 
* @brief    绝缘阻抗上MOS控制(隔离输出)
* @param    value:true MOS导通，false MOS截止
* @return   none
* @note:
*/
void InsImpUpMosCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_ISO_OUT_1);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_1);
    }
}


/** 
* @brief    绝缘阻抗下MOS控制(隔离输出)
* @param    value:true MOS导通，false MOS截止
* @return   none
* @note:
*/
void InsImpDownMosCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_ISO_OUT_2);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_2);
    }
}


/** 
* @brief    绝缘阻抗继电器控制(隔离输出)
* @param    value:true 继电器闭合，false 继电器断开
* @return   none
* @note:
*/
void InsImpRelayCtl(bool value)
{
    if(value == true)
    {
        DevIOCtlOutSet(EN_DEV_IOCTL_IDX_ISO_OUT_3);
    }
    else
    {
        DevIOCtlOutReset(EN_DEV_IOCTL_IDX_ISO_OUT_3);
    }
}


