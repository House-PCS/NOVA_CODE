
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        activation.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-9-24
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "activation.h"
#include "energy_management.h"
#include "mppt_process.h"
#include "dev_sensor.h"
#include "llc_dcdc_cntl.h"
#include "sys_info.h"
#include "selftest.h"
static EN_ACTIVATION_STATE_DEF geActivationFsm = EN_ACTIVATION_STATE_DEF_SELFTEST;
static uint16_t activationrunstate_flag = 0;
static uint16_t activationreset_flag = 0; //复位PV激活运行状态标置位
/**
* @brief: ActivationFsmRun
* @param: 
* @return:
* @note: 激活整机状态机运行
*
*/

void ActivationFsmRun()
{
    static uint16_t dsp_wake_en = 0;
    if((gSysSetInfo.dsp_wake_en == 1) && (dsp_wake_en != 1))  //监控卡下发激活命令
    {
        dsp_wake_en = 1;
        ActivationRunStateSet(PVACTIVATION_PVSTART);
    }
    else if((gSysSetInfo.dsp_wake_en == 2) && (dsp_wake_en != 2))  //监控卡下发PV激活OK命令
    {
        dsp_wake_en = 2;
        ActivationRunStateSet(PVACTIVATION_PVSTARTOK);
    }
    else if((gSysSetInfo.dsp_wake_en == 3) && (dsp_wake_en != 3))  //监控卡下发正常运行命令
    {
        dsp_wake_en = 3;
        ActivationRunStateSet(PVACTIVATION_BATSTART);
    }

    /*************************/
    //状态切换
    switch(geActivationFsm)
    {
        case EN_ACTIVATION_STATE_DEF_SELFTEST: //自检状态
        {
            SelfTest(); 

            if(ReadSelfTestState())
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;//复位转至就绪状态
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);  //PV停止Boost
                LlcRunStateSet(LLC_READY);  //Llc停止发波
                /***************************/
            }
            
            break;
        }
        case EN_ACTIVATION_STATE_DEF_READY:  //整机就绪状态
        {
            /***************************/
            //func
            /***************************/
            activationreset_flag = 0;
            /***************************/
            //condition
            /***************************/
            if(activationrunstate_flag == PVACTIVATION_PVSTART) //切换至PV启机逻辑
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_ACTIVATIONBOOST;
                /***************************/
                /***************************/
                //action
                activationrunstate_flag = 0;  //复位状态切换命令
                PvRunStateSet(PV_BOOST);
                /***************************/
            }
            else if(activationrunstate_flag == PVACTIVATION_BATSTART) //切换至正常启机逻辑
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_NORMAL;
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_NORMAL);  //PV转至正常运行模式
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            
            break;
        }
        case EN_ACTIVATION_STATE_DEF_ACTIVATIONBOOST:  //PV激活Boost状态，母线预充阶段
        {
            /***************************/
            //func
            /***************************/
            /***************************/
            //condition
            /***************************/
            if(gstSensorData.bus_voltage > PVACTIVAT_BUS_THRESHOLD) //母线预充结束
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_ACTIVATIONLLCRUN; //母线预充结束，转至PV激活LLC运行状态
                /***************************/
                /***************************/
                //action
                LlcRunStateSet(LLC_SOFTSTART);  // LLC软起
                // LLC_Set_EN();
                /***************************/
            }
            /***************************/
            //condition
            /***************************/
            else if(activationrunstate_flag == PVACTIVATION_PVSTARTERR || activationrunstate_flag == PVACTIVATION_PVSTART) //激活失败，转至Ready状态
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);
                LlcRunStateSet(LLC_READY); 
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            else if(activationrunstate_flag == PVACTIVATION_BATSTART) //切换至正常启机逻辑
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_NORMAL;
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_NORMAL);  //PV转至正常运行模式
                LlcRunStateSet(LLC_READY); 
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            break;
        }
        case EN_ACTIVATION_STATE_DEF_ACTIVATIONLLCRUN:  //PV激活LLC软起阶段
        {
            /***************************/
            //func
            /***************************/
            /***************************/
            //condition
            /***************************/
            if(LlcRunStateRead() == EN_LLC_STATE_DEF_SOFTSTARTOK_PV) // LLC软起成功
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_ACTIVATIONREADY;  // LLC软起成功转至激活就绪状态
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);  //PV停止Boost
                LlcRunStateSet(LLC_READY);  //Llc停止发波
                /***************************/
            }
            /***************************/
            //condition
            /***************************/
            else if(activationrunstate_flag == PVACTIVATION_PVSTARTERR || activationrunstate_flag == PVACTIVATION_PVSTART)
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;//激活失败，转至Ready状态
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);  //PV停止Boost
                LlcRunStateSet(LLC_READY);  //Llc停止发波
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            else if(activationrunstate_flag == PVACTIVATION_BATSTART) //切换至正常启机逻辑
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_NORMAL;
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_NORMAL);  //PV转至正常运行模式
                LlcRunStateSet(LLC_READY); 
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            break;
        }
        case EN_ACTIVATION_STATE_DEF_ACTIVATIONREADY:  //PV激活就绪，等待电池开放电MOS状态
        {
            /***************************/
            //func
            /***************************/
            /***************************/
            //condition
            /***************************/
            if(activationrunstate_flag == PVACTIVATION_PVSTARTOK) //
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_NORMAL;//激活成功，转至正常运行模式
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_NORMAL);  //PV转至正常运行模式
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            /***************************/
            //condition
            /***************************/
            else if(activationrunstate_flag == PVACTIVATION_PVSTARTERR || activationreset_flag == PVACTIVATION_PVSTART)
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;//激活失败，转至Ready状态
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);  //PV停止Boost
                LlcRunStateSet(LLC_READY);  //Llc停止发波
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            else if(activationrunstate_flag == PVACTIVATION_BATSTART) //切换至正常启机逻辑
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_NORMAL;
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_NORMAL);  //PV转至正常运行模式
                LlcRunStateSet(LLC_READY); 
                activationrunstate_flag = 0;  //复位状态切换命令
                /***************************/
            }
            break;
        }
        case EN_ACTIVATION_STATE_DEF_NORMAL:
        {
            /***************************/
            //func
            /***************************/
            MainPollingProcess();
            /***************************/
            //condition
            /***************************/
            if(activationreset_flag == 1) // 复位信号
            {
                /***************************/
                //nextstate
                geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;//复位转至就绪状态
                /***************************/
                /***************************/
                //action
                PvRunStateSet(PV_READY);  //PV停止Boost
                LlcRunStateSet(LLC_READY);  //Llc停止发波
                /***************************/
            }
            break;
        }
        case EN_ACTIVATION_STATE_DEF_ERR:
        {
            break;
        }
        default:
        {
            /***************************/
            //func
            /***************************/
            /***************************/
            //nextstate
            geActivationFsm = EN_ACTIVATION_STATE_DEF_READY;
            /***************************/
            /***************************/
            //action
            PvRunStateSet(PV_READY);  //PV停止Boost
            LlcRunStateSet(LLC_READY);  //Llc停止发波
            activationreset_flag = 0;
            /***************************/
            break;
        }
    }
}
/**
* @brief: ActivationRunStateSet
* @param: activationrunstate 
* @return:
* @note: PV激活运行状态设置
*
*/
void ActivationRunStateSet(uint16_t activationrunstate)
{
    if(activationrunstate == PVACTIVATION_PVSTART)
    {
        activationrunstate_flag = PVACTIVATION_PVSTART;  // PV启机标志
    }
    else if(activationrunstate == PVACTIVATION_BATSTART)
    {
        activationrunstate_flag = PVACTIVATION_BATSTART;  // 电池正常启机标志
    }
    else if(activationrunstate == PVACTIVATION_PVSTARTERR)  // PV启机失败
    {
        activationrunstate_flag = PVACTIVATION_PVSTARTERR;
    }
    else if(activationrunstate == PVACTIVATION_PVSTARTOK)  // PV激活成功
    {
        activationrunstate_flag = PVACTIVATION_PVSTARTOK;
    }
}

/**
* @brief: ActivationRunStateReset
* @param:  
* @return:
* @note: 复位激活运行状态设置
*
*/
void ActivationRunStateReset()
{
    activationreset_flag = 1;
}
/**
* @brief: ActivationRunStateRead
* @param:  
* @return:
* @note: 激活运行状态设置读取
*
*/
uint16_t ActivationRunStateRead()
{
    if(geActivationFsm == EN_ACTIVATION_STATE_DEF_READY)
    {
        return ACTIVATION_STATE_READY;
    }
    else if(geActivationFsm == EN_ACTIVATION_STATE_DEF_ACTIVATIONBOOST || geActivationFsm == EN_ACTIVATION_STATE_DEF_ACTIVATIONLLCRUN)
    {
        return ACTIVATION_STATE_ACTIVATING;
    }
    else if(geActivationFsm == EN_ACTIVATION_STATE_DEF_ACTIVATIONREADY)
    {
        return ACTIVATION_STATE_ACTIVATIONREADY;
    }
    else if(geActivationFsm == EN_ACTIVATION_STATE_DEF_NORMAL)
    {
        return ACTIVATION_STATE_ACTIVATIONOK;
    }
    else
    {
        return ACTIVATION_STATE_ERR;
    }
    
    
    
}

/**
* @brief: ActivationFsmStateRead
* @param:  
* @return:
* @note: 激活状态机状态设置读取
*
*/
EN_ACTIVATION_STATE_DEF ActivationFsmStateRead()
{
    return geActivationFsm;
}
