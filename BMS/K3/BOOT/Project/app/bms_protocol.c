
#include "bms_protocol.h"


/****************************************BMS_ERROR_CODE_LEVER******************************************/
const uint8_t ErrorCodeLevel[11][16] = 
{
    {6, 3, 3, 0, 0, 0, 0, 3, 3, 0, 0, },                /* 通信故障 */
    {7, 7, 6, 6, 6, 6, 0, 0, 0, 6, 5, 6, 5, },          /* MOS故障 */
    {3, 6, 6, 6, 6, 5, },                               /* 电流故障 */
    {0, 6, 7, 6, 7, 7, 7, 6, 3, 6, 7, 3, 0, 0, 0, 0},   /* 电芯故障1 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* 电芯故障2 */
    {7, 0, 8, 6, },                                     /* 预充故障 */
    {6, 6, },                                           /* BMS供电电源故障 */
    {6, 6, 0, 6, 6, },                                  /* 电芯采样故障 */
    {0, },                                              /* 均衡故障 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, },                      /* 其他故障1 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* 其他故障2 */
};

/****************************************BMS_SYSTEM_ERROR_LEVER******************************************/
const uint8_t SystemErrorLevel[11][16] = 
{
    {3, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, },                /* 通信故障 */
    {3, 3, 3, 3, 3, 3, 0, 0, 0, 3, 3, 2, 2, },          /* MOS故障 */
    {1, 3, 3, 3, 3, 2, },                               /* 电流故障 */
    {0, 3, 3, 3, 3, 3, 3, 3, 1, 3, 3, 1, 0, 0, 0, 0},   /* 电芯故障1 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* 电芯故障2 */
    {3, 3, },                                           /* BMS供电电源故障 */
    {3, 3, 0, 3, 3, },                                  /* 电芯采样故障 */
    {0, },                                              /* 均衡故障 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, },                      /* 其他故障1 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* 其他故障2 */
};


static EN_BMS_RUNSTATE _UpLoadStatus = EN_BMS_RUNSTATE_DISABLE;
BMSSysInfo_t   gBMSSys_info;



uint8_t GetSystemErrorLevel(void)
{
    uint8_t bms_sysfailsta;
    if((gBmsErrorInfo.com_error_code.value & 0x0001) || (gBmsErrorInfo.mos_error_code.value & 0x183F) || \
       (gBmsErrorInfo.curr_error_code.value & 0x001E) || (gBmsErrorInfo.cell1_error_code.value & 0x06FE) || \
       (gBmsErrorInfo.cell2_error_code.value & 0x0000) || (gBmsErrorInfo.power_error_code.value & 0x0003) || \
       (gBmsErrorInfo.cellsample_error_code.value & 0x001B) || (gBmsErrorInfo.balance_error_code.value & 0x0000) || \
       (gBmsErrorInfo.other1_error_code.value & 0x0000) || (gBmsErrorInfo.other2_error_code.value & 0x0000))
    {
        bms_sysfailsta = 3;
    }
    /* 参数待修正 */
    else if((gBmsErrorInfo.com_error_code.value & 0x0001) || (gBmsErrorInfo.mos_error_code.value & 0x183F) || \
           (gBmsErrorInfo.curr_error_code.value & 0x001E) || (gBmsErrorInfo.cell1_error_code.value & 0x06FE) || \
           (gBmsErrorInfo.cell2_error_code.value & 0x0000) || (gBmsErrorInfo.power_error_code.value & 0x0003) || \
           (gBmsErrorInfo.cellsample_error_code.value & 0x001B) || (gBmsErrorInfo.balance_error_code.value & 0x0000) || \
           (gBmsErrorInfo.other1_error_code.value & 0x0000) || (gBmsErrorInfo.other2_error_code.value & 0x0000))
    {
        bms_sysfailsta = 2;
    }
    /* 参数待修正 */
    else if((gBmsErrorInfo.com_error_code.value & 0x0001) || (gBmsErrorInfo.mos_error_code.value & 0x183F) || \
           (gBmsErrorInfo.curr_error_code.value & 0x001E) || (gBmsErrorInfo.cell1_error_code.value & 0x06FE) || \
           (gBmsErrorInfo.cell2_error_code.value & 0x0000) || (gBmsErrorInfo.power_error_code.value & 0x0003) || \
           (gBmsErrorInfo.cellsample_error_code.value & 0x001B) || (gBmsErrorInfo.balance_error_code.value & 0x0000) || \
           (gBmsErrorInfo.other1_error_code.value & 0x0000) || (gBmsErrorInfo.other2_error_code.value & 0x0000))
    {
        bms_sysfailsta = 1;
    }
    else
    {
        bms_sysfailsta = 0;
    }
    
    return bms_sysfailsta;
}
void BMSRunStateProcess(void)
{
    switch(_UpLoadStatus)
    {
        case EN_BMS_RUNSTATE_DISABLE:
        {
            break;
        }
        case EN_BMS_RUNSTATE_OFF:
        {

            break;
        }
        case EN_BMS_RUNSTATE_POWER_HOLD:
        {
            if((gBMSSys_info.bms_shutdown == 0) && (gBMSSys_info.bms_bat2_drv_train_enb == 0))
            {
                
            }
            break;
        }
        case EN_BMS_RUNSTATE_NORMAL:
        {
            break;
        }
        case EN_BMS_RUNSTATE_STANDBY:
        {
            if((gBMSSys_info.bms_shutdown == 1) || 0/* (唤醒源无效&&报文丢失) */)
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_OFF;
            }
            
            if(0/* (高压上电完成) */)
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL;
            }
            break;
        }
        default:
            break;
    }
}




















