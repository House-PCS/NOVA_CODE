
#include "current_control.h"
#include "sys_info.h"
#include "dev_led.h"
#include "dev_mos.h"
#include "bms_protocol.h"
#include "afe_control.h"

#define LOG_TAG "CURRENT"

/******* 默认地址关闭限流强制开启标志处理 ******/
void _DefaultModbusAddrCcProcess(PackStatus_t *pPackStatus);

/**
 * @brief  : 电池内部限流控制
 * @para   :
 * @note   : None
 * @return : None
 */
void PackCurrentControl(void)
{
    uint8_t error_level = GetSystemErrorLevel();
    PackStatus_t *pPackStatus = NULL;
    static EN_PACK_CURRENT_MODE last_mode[2] = {EN_PACK_CURRENT_NONE, EN_PACK_CURRENT_NONE};
    
    //工装测试模式下，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl != 0)
    {
        return;
    }

    //三级故障下不运行，特定故障状态下不运行
    if(error_level == 3 ||\
       gBmsErrorInfo.mos_error_code.b.mos_ntc_error         || gBmsErrorInfo.mos_error_code.b.mos_high_temper2      ||\
       /*gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2  || */gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2   ||\
       gBmsErrorInfo.curr_error_code.b.charge_overcurr2     || gBmsErrorInfo.curr_error_code.b.discharge_overcurr2  ||\
       gBmsErrorInfo.temper_error_code.b.cell_high_temper2  || gBmsErrorInfo.other1_error_code.b.vol_sensor_error   ||\
       gUartPackInfo.com_out_flag                           || _gBMSSysInfo.start_out_flag
    )
    {
        return;
    }

    //BMS正常运行时才可以对MOS进行控制
    if(_gBMSSysInfo.bms_run_status != EN_BMS_RUNSTATE_NORMAL)
    {
        return;
    }
    
    pPackStatus = &gPackStatus;
    
    //限流空闲模式下允许自身控制CC
    if(pPackStatus->cc_mode == EN_CC_MODE_IDLE)
    {
        //状态有更新时才会进行一次控制，防止一直开mos
        if(last_mode[1] == pPackStatus->mode)
        {
            return;
        }
        
        if(pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
        {
            //空闲模式开启限流
            CloseChargeMos();//关闭充电MOS
            CCEnOn();
            pPackStatus->cc_status = EN_CC_STATUS_OPEN;
        }
        else if((pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE) || (pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE))
        {
            //充放电模式，关闭限流
            OpenChargeMos();
            CCEnOff();
            pPackStatus->cc_status = EN_CC_STATUS_CLOSE;
        }
        
        last_mode[0] = pPackStatus->mode;
        last_mode[1] = last_mode[0];
    }

}


/**
 * @brief  : 限流命令接收处理
 * @para   :
 * @note   : None
 * @return : None
 */
void CcCmdRevProcess(EN_CC_MODE rev_cc_mode)
{
    //工装测试模式下，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 1)
    {
        return;
    }
    
    uint8_t error_level = GetSystemErrorLevel();
    //三级故障下不运行，特定故障状态下不运行
    if(error_level == 3 ||\
       gBmsErrorInfo.mos_error_code.b.mos_ntc_error         || gBmsErrorInfo.mos_error_code.b.mos_high_temper2      ||\
       /*gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2  || */gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2   ||\
       gBmsErrorInfo.curr_error_code.b.charge_overcurr2     || gBmsErrorInfo.curr_error_code.b.discharge_overcurr2  ||\
       gBmsErrorInfo.temper_error_code.b.cell_high_temper2  || gBmsErrorInfo.other1_error_code.b.vol_sensor_error
    )
    {
        return;
    }
    
    PackStatus_t *pPackStatus = &gPackStatus;

    //限流空闲模式下允许自身控制CC
    switch(rev_cc_mode)
    {
        case EN_CC_MODE_IDLE:
        {
            break;
        }
        //开启限流模块
        case EN_CC_MODE_OPEN_CC:
        {
            CCEnOn();           //打开限流模块
            CloseChargeMos();   //关闭充电MOS
            pPackStatus->cc_status = EN_CC_STATUS_OPEN;
            break;
        }
        //开启充电MOS
        case EN_CC_MODE_OPEN_CHG_MOS:
        {
            OpenChargeMos();    //开启充电MOS
            CCEnOff();          //关闭限流模块
            pPackStatus->cc_status = EN_CC_STATUS_CLOSE;
            pPackStatus->cc_mode = EN_CC_MODE_IDLE;
            break;
        }
        //关闭充电MOS与限流
        case EN_CC_MODE_CLOSE_CHG_MOS_AND_CC:
        {
            CloseChargeMos();       //关闭充电MOS
            CCEnOff();              //关闭限流
            pPackStatus->cc_status = EN_CC_STATUS_MOS_CLOSE;
            break;
        }
        //打开放电MOS
        case EN_CC_MODE_OPEN_DSG_MOS:
        {
            OpenDisChargeMos();
            pPackStatus->cc_mode = EN_CC_MODE_IDLE;
            break;
        }
        //关闭放电MOS
        case EN_CC_MODE_CLOSE_DSG_MOS:
        {
            //强充状态下，不关放电MOS
            if((_gBMSSysInfo.force_chg_allow_flag == 2) && 
                (_gBMSSysInfo.uv_hold_flag == 2))
            {
                CloseDisChargeMos();
            }
            pPackStatus->cc_mode = EN_CC_MODE_IDLE;
            break;
        }
        default :
            break;
    }
}




/**
 * @brief  : 一段时间后，若modbus地址还是默认地址，则关闭强制限流
 * @para   :
 * @note   : None
 * @return : None
 */
void _DefaultModbusAddrCcProcess(PackStatus_t *pPackStatus)
{
    static uint32_t cc_close_time           = 0     ;  /*关闭强制限流时间*/
    static bool first_flag                  = false ;  /*首次进入标志*/
    
    //默认地址
    if(gUartPackInfo.ModbusAddr == MODBUS_ADDR_DEFAULT_VALUE)
    {
        if(first_flag == false)
        {
            cc_close_time  = tim_get_systime();
            first_flag = true;
        }
        //开机1s之后，关闭cc强开标志
        if(tim_difftime_from_now(cc_close_time) > 1000)
        {
            pPackStatus->cc_mode = EN_CC_MODE_IDLE;
        }
    }
}
