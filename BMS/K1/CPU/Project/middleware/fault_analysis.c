

/* Includes ------------------------------------------------------------------*/
#include "fault_analysis.h"
#include "rtc.h"

#define LOG_TAG     "fault_analysis"

#define LIMIT_CHARGE_CURRENT    BMSWarningParam.limit_charge_current    /* 200 */
#define LIMIT_DISCHARGE_CURRENT BMSWarningParam.limit_discharge_current /* -200 */

#define MOS_OT1                 BMSWarningParam.mos_ot1                 /* 750   */
#define MOS_OT2                 BMSWarningParam.mos_ot2                 /* 850   */
#define MOS_OT3                 BMSWarningParam.mos_ot3                 /* 950   */

#define CELL_UV1                BMSWarningParam.cell_uv1                /* 2600  */
#define CELL_UV2                BMSWarningParam.cell_uv2                /* 2550  */
#define CELL_UV3                BMSWarningParam.cell_uv3                /* 2500  */
#define CELL_OV1                BMSWarningParam.cell_ov1                /* 3650  */
#define CELL_OV2                BMSWarningParam.cell_ov2                /* 3700  */
#define CELL_OV3                BMSWarningParam.cell_ov3                /* 3750  */
#define PACK_OV1                BMSWarningParam.pack_ov1                /* 2700*串数 */
#define PACK_UV1                BMSWarningParam.pack_uv1                /* 3550*串数 */
#define CELL_V_DIFF1            BMSWarningParam.cell_voltage_diff       /* 350 */

#define DISCHARGE_OVERCURR1     BMSWarningParam.discharge_overcurr1     /* 110   */
#define DISCHARGE_OVERCURR2     BMSWarningParam.discharge_overcurr2     /* 120   */
#define DISCHARGE_OVERCURR3     BMSWarningParam.discharge_overcurr3     /* 130   */
#define CHARGE_OVERCURR1        BMSWarningParam.charge_overcurr1        /* 105   */
#define CHARGE_OVERCURR2        BMSWarningParam.charge_overcurr2        /* 110   */
#define CHARGE_OVERCURR3        BMSWarningParam.charge_overcurr3        /* 115   */

#define CELL_UT1                BMSWarningParam.cell_ut1                /* -100  */
#define CELL_OT1                BMSWarningParam.cell_ot1                /* 550   */
#define CELL_OT2                BMSWarningParam.cell_ot2                /* 600   */
#define CELL_OT3                BMSWarningParam.cell_ot3                /* 650   */
#define NTC_T_DIFF              BMSWarningParam.ntc_temp_diff           /* 100   */
#define NTC_T_DIFF2             BMSWarningParam.ntc_temp_diff2          /* 150   */

#define SOH_UNDER_LVL1          BMSWarningParam.soh_under_lvl1
#define SOH_UNDER_LVL2          BMSWarningParam.soh_under_lvl2
#define SOH_UNDER_LVL3          BMSWarningParam.soh_under_lvl3

#define CELL_UT                 BMSWarningParam.cell_ut                 /*  10  */
#define TSENSOR_UT              BMSWarningParam.tsensor_ut              /* -400 */
#define TSENSOR_OT              BMSWarningParam.tsensor_ot              /* 1200 */
#define ZERODRIFT_CURRENT       BMSWarningParam.zerodrift_current       /* 2000 */


#define PACK_V_DIFF_VAL         (500)      /* 电芯最大允许压差 */


Fault_analysis_flag gFaultAnalysisFlag;
BMSErrorInfo_t  gBmsErrorInfo;
EmsErrorCode_t  gEmsErrorInfo;     //Ems故障
BMSWarningParam_t  BMSWarningParam;

BMSErrorLockInfo_t gBMSLockErrorInfo[2];    /* 锁存故障信息 */

void _MosAdhesionErrorJudge(PackStatus_t *pPackStatus);             /* mos粘连故障判断 */
void _ChargeMosCloseErrorJudge(PackStatus_t *pPackStatus);          /* 充电mos闭合故障判断 */
void _DisChargeMosCloseErrorJudge(PackStatus_t *pPackStatus);       /* 放电mos闭合故障判断 */
void _MosTempureSensorErrorJudge(PackStatus_t *pPackStatus);        /* mos温度传感器故障判断 */
void _MosTempureErrorJudge(PackStatus_t *pPackStatus);              /* mos温度故障判断 */
void _PackVoltageErrorJudge(PackStatus_t *pPackStatus);             /* 单体电压故障判断 */
void _CurrentErrorJudge(PackStatus_t *pPackStatus);                 /* 电流故障判断 */
void _PackTempureErrorJudge(void);                                  /* 电池温度故障判断 */
void _SohUnderErrorJudge(PackStatus_t *pPackStatus);                /* SOH过低故障判断 */
void _VoltageSensorErrorJudge(PackStatus_t *pPackStatus);           /* 电压传感器故障判断 */
void _PackTempureSensorErrorJudge(PackStatus_t *pPackStatus);       /* 电芯温度传感器故障判断 */
void _FlashErrorJudge(void);                                        /* flash故障判断 */
void _EepromErrorJudge(void);                                       /* Eeprom故障判断 */
void _HeatErrorJudge(void);                                         /* 加热膜故障判断 */
void _RtcErrorJudge(void);                                          /* Rtc故障判断 */
void _DsgShortErrorJudge(void);                                     /* 放电短路故障判断 */
void _ChgShortErrorJudge(PackStatus_t *pPackStatus);                /* 充电短路故障判断 */
void _AfeErrorJudge(PackStatus_t *pPackStatus);                     /* Afe通讯故障判断 */
void _AfeOwErrorJudge(void);                                        /* Afe断线故障判断 */

static void _BMSWarningParamUpdate(void)
{
    uint32_t cell_v_diff_delta       ;//电芯压差大增量
    
    BMSWarningParam.limit_charge_current    = 30;
    BMSWarningParam.limit_discharge_current = 30;
    BMSWarningParam.mos_ot1                 = garrModbusWriteReadReg[0].mos_ot_lvl1;
    BMSWarningParam.mos_ot2                 = garrModbusWriteReadReg[0].mos_ot_lvl2;
    BMSWarningParam.mos_ot3                 = garrModbusWriteReadReg[0].mos_ot_lvl3;

    BMSWarningParam.cell_uv1                = garrModbusWriteReadReg[0].cell_uv_lvl1;
    BMSWarningParam.cell_uv2                = garrModbusWriteReadReg[0].cell_uv_lvl2;
    BMSWarningParam.cell_uv3                = garrModbusWriteReadReg[0].cell_uv_lvl3;
    BMSWarningParam.cell_ov1                = garrModbusWriteReadReg[0].cell_ov_lvl1;
    BMSWarningParam.cell_ov2                = garrModbusWriteReadReg[0].cell_ov_lvl2;
    BMSWarningParam.cell_ov3                = garrModbusWriteReadReg[0].cell_ov_lvl3;
    BMSWarningParam.pack_uv1                = ((uint32_t)garrModbusWriteReadReg[0].pack_uv_lvl_high << 16) + garrModbusWriteReadReg[0].pack_uv_lvl_low;
    BMSWarningParam.pack_ov1                = ((uint32_t)garrModbusWriteReadReg[0].pack_ov_lvl_high << 16) + garrModbusWriteReadReg[0].pack_ov_lvl_low;
    
    //SOH每降低1%允许压差增加10mV，最低0，最高700mV;
    cell_v_diff_delta                       = ((100 - (int)gPackStatus.sox_info.soh) * 10);
    cell_v_diff_delta                       = cell_v_diff_delta > 0 ? cell_v_diff_delta : 0;
    cell_v_diff_delta                       = cell_v_diff_delta < 700 ? cell_v_diff_delta : 700;
    BMSWarningParam.cell_voltage_diff       = garrModbusWriteReadReg[0].cell_voltage_diff + cell_v_diff_delta;
    
    //电池欠压,且afe能测到电压的情况下，压差增大到2V
    if((gCell_Info.total_cell_voltage < 48000) && (gCell_Info.cell_min_voltage > 1000))
    {
        BMSWarningParam.cell_voltage_diff = 2000;
    }

    BMSWarningParam.discharge_overcurr1     = garrModbusWriteReadReg[0].dsg_oc_lvl1;
    BMSWarningParam.discharge_overcurr2     = garrModbusWriteReadReg[0].dsg_oc_lvl2;
    BMSWarningParam.discharge_overcurr3     = garrModbusWriteReadReg[0].dsg_oc_lvl3;
    BMSWarningParam.charge_overcurr1        = garrModbusWriteReadReg[0].chg_oc_lvl1;
    BMSWarningParam.charge_overcurr2        = garrModbusWriteReadReg[0].chg_oc_lvl2;
    BMSWarningParam.charge_overcurr3        = garrModbusWriteReadReg[0].chg_oc_lvl3;

    BMSWarningParam.cell_ut1                = (int16_t)garrModbusWriteReadReg[0].cell_ut_lvl;
    BMSWarningParam.cell_ut2                = (int16_t)garrModbusWriteReadReg[0].cell_ut_lvl;
    BMSWarningParam.cell_ut3                = (int16_t)garrModbusWriteReadReg[0].cell_ut_lvl;
    BMSWarningParam.cell_ot1                = garrModbusWriteReadReg[0].cell_ot_lvl1;
    BMSWarningParam.cell_ot2                = garrModbusWriteReadReg[0].cell_ot_lvl2;
    BMSWarningParam.cell_ot3                = garrModbusWriteReadReg[0].cell_ot_lvl3;
    BMSWarningParam.ntc_temp_diff           = garrModbusWriteReadReg[0].cell_temp_diff;
    BMSWarningParam.ntc_temp_diff2          = garrModbusWriteReadReg[0].cell_temp_diff + 50;

    BMSWarningParam.soh_under_lvl1          = garrModbusWriteReadReg[0].soh_under_lvl1;
    BMSWarningParam.soh_under_lvl2          = garrModbusWriteReadReg[0].soh_under_lvl2;
    BMSWarningParam.soh_under_lvl3          = garrModbusWriteReadReg[0].soh_under_lvl3;
    BMSWarningParam.tsensor_ut              = -400       ;
    BMSWarningParam.tsensor_ot              = 1200       ;
    BMSWarningParam.zerodrift_current       = 2000       ;

}


void FaultRegInit()
{
    uint16_t flash_error_flag = gFaultAnalysisFlag.flash_error_flag;
    uint16_t eeprom_error_flag = gFaultAnalysisFlag.e2prom_error_flag;
    uint8_t cc_open_flag       = gFaultAnalysisFlag.cc_enable_flag;
    
    memset(&gFaultAnalysisFlag, 0, sizeof(Fault_analysis_flag));
    
    gFaultAnalysisFlag.flash_error_flag = flash_error_flag;
    gFaultAnalysisFlag.e2prom_error_flag = eeprom_error_flag;
    gFaultAnalysisFlag.cc_enable_flag = cc_open_flag;
}

/****************bms故障故障判断函数***********************/
void FaultAnalysisProcess(void)
{
    //工装模式，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 1)
    {
        return;
    }
    
    PackStatus_t *pPackStatus = &gPackStatus;

    /* debug Temp */
    _BMSWarningParamUpdate();//更新各种故障类型的阈值参数


    _MosTempureErrorJudge(pPackStatus);             /* MOS温度故障判断 */
    _PackVoltageErrorJudge(pPackStatus);            /* 电池电压故障判断 */
    _PackTempureErrorJudge();                       /* 电池温度故障判断 */
    _DsgShortErrorJudge();                          /* 放电短路故障判断 */
    //工装模式，以下故障判断不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 2)
    {
        return;
    }
    
    _MosAdhesionErrorJudge(pPackStatus);            /* MOS粘连故障判断 */
    _ChargeMosCloseErrorJudge(pPackStatus);         /* 充电MOS闭合失败故障判断 */
    _DisChargeMosCloseErrorJudge(pPackStatus);      /* 放电MOS闭合失败故障判断 */
    _MosTempureSensorErrorJudge(pPackStatus);       /* MOS温度传感器故障判断 */
    _CurrentErrorJudge(pPackStatus);                /* 电流故障判断 */
    _SohUnderErrorJudge(pPackStatus);               /* SOH过低故障判断 */
    _VoltageSensorErrorJudge(pPackStatus);          /* 电压传感器故障判断 */
    _PackTempureSensorErrorJudge(pPackStatus);      /* 电池温度传感器故障判断 */
    _FlashErrorJudge();                             /* Flash故障判断 */
    _EepromErrorJudge();                            /* Eeprom故障判断 */
    _HeatErrorJudge();                              /* 加热膜故障判断 */
    _RtcErrorJudge();                               /* RTC故障判断 */
    
    _ChgShortErrorJudge(pPackStatus);               /* 充电短路故障判断 */
    _AfeErrorJudge(pPackStatus);                    /* afe通讯故障判断 */
    _AfeOwErrorJudge();                             /* afe断线故障判断 */
    
}

/**
 * @brief  : p_mos闭合故障判断
 * @para   : pre_vol : 电池电压
 * @note   : None
 * @return : None
 */
bool PreMosCloseErrorJudge(int32_t pre_vol)
{
    //预充mos打开后，P端电压过低，判断为预充mos闭合失败
    static uint32_t pre_v_error_count = 0;
    static int32_t  last_pre_vol;                   //预充电压记录
    static int32_t  pre_vol_fail_count;             //预充失败计数
    
    //记录预充电压
    last_pre_vol = pre_vol;
    //充电电压没有增长
    if(pre_vol - last_pre_vol < 10)
    {
        pre_vol_fail_count++;
    }
    //预充电压一直很低
    if(pre_vol < gCell_Info.bat_init_vol * 0.3)
    {
        pre_v_error_count++;
    }
    
    if((pre_v_error_count > 20)  || (pre_vol_fail_count > 20))
    {
        return false;
    }
    return true;
}

/**
 * @brief  : mos粘连故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _MosAdhesionErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t mos_adhesion_time           = 0;

    /* 放电 或预充 或充电MOS粘连故障 */ /* 加热MOS粘连故障 */
    if(tim_difftime_from_now(mos_adhesion_time) > 200)
    {
        mos_adhesion_time = tim_get_systime();
        /* 充放电MOs都没打开且放电电流大于2000mA || 充电Mos没有打开且充电电流大于2000mA */
        if(((/*!gFaultAnalysisFlag.premos_enable_flag && */!gFaultAnalysisFlag.dischargemos_enable_flag) && pPackStatus->pack_current_calibration > 2000) || \
           ((!gFaultAnalysisFlag.chargemos_enable_flag && !gFaultAnalysisFlag.cc_enable_flag) && pPackStatus->pack_current_calibration < -2000))
        {
            gFaultAnalysisFlag.mos_adhesion_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_adhesion_count = 0;
        }
    }

    if(gFaultAnalysisFlag.mos_adhesion_count > 50)
    {
//        LOG_DEBUG(LOG_TAG, "mos_adhesion===========================================================================\r\n");
        gBmsErrorInfo.mos_error_code.b.mos_adhesion = 1;
    }    
}

/**
 * @brief  : c_mos闭合故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _ChargeMosCloseErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t cmos_fail_close_time    = 0;  
    static bool cmos_current_normal_flag    = true;
    static bool cc_current_normal_flag      = true;
    float n_pack_charge_v_val               = 0;    
    
    if(tim_difftime_from_now(cmos_fail_close_time) < 1000)
    {
        return;
    }
    cmos_fail_close_time = tim_get_systime();
    
    //1S内限流开启20次以上，故障次数+1
    if(gFaultAnalysisFlag.cc_open_count >= 20)
    {
        gFaultAnalysisFlag.cmos_fail_close_count++;
    }
    else
    {
        gFaultAnalysisFlag.cmos_fail_close_count = 0;
    }
    
    if(gFaultAnalysisFlag.cmos_fail_close_count > 4)
    {
        gBmsErrorInfo.mos_error_code.b.cmos_fail_close = 1;//此处修改
    }
    
    //计数清零
    gFaultAnalysisFlag.cc_open_count = 0;

}


/**
 * @brief  : d_mos闭合故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _DisChargeMosCloseErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t dmos_fail_close_time    = 0;  
    float n_pack_discharge_v_val            = 0;   
    
    if(tim_difftime_from_now(dmos_fail_close_time) < 1000)//
    {
        return;
    }
    
    int32_t charger_vol =  OVCVoltageGet(); //P端电压
    int32_t d_vol       = 0;                //压差
    
    //放电mos开启，且充电mos或限流开启的情况下
    if(gFaultAnalysisFlag.dischargemos_enable_flag && (gFaultAnalysisFlag.cc_enable_flag || gFaultAnalysisFlag.chargemos_enable_flag))
    {
        d_vol = charger_vol - gCell_Info.total_cell_voltage;
        //P端电压与B端电压相差15V以上，判断为放电故障
        if(abs(d_vol) > 15000)
        {
            gFaultAnalysisFlag.dmos_fail_close_count++;
        }
        else
        {
            gFaultAnalysisFlag.dmos_fail_close_count = 0;
        }
    }

    if(gFaultAnalysisFlag.dmos_fail_close_count > 4)
    {
        gBmsErrorInfo.mos_error_code.b.dmos_fail_close = 1;//此处修改
    }
    dmos_fail_close_time = tim_get_systime();
}

/**
 * @brief  : mos温度传感器故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _MosTempureSensorErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t mos_sensor_error_time       = 0;
    /* MOS温度传感器故障 */
    if(tim_difftime_from_now(mos_sensor_error_time) > 1000)
    {
        mos_sensor_error_time = tim_get_systime();
        // 放电mos温度低于-40  或者   放电mos温度高于120
        if(gCell_Info.mos_temperature < TSENSOR_UT || gCell_Info.mos_temperature > TSENSOR_OT)
        {
            gFaultAnalysisFlag.mos_temperature_sensor_error_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_sensor_error_count = 0;
        }

        if(gFaultAnalysisFlag.mos_temperature_sensor_error_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_ntc_error = 1;
        }
    }   
}

/**
 * @brief  : mos温度故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _MosTempureErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t mos_time                    = 0;
    
    if(tim_difftime_from_now(mos_time) > 1000)
    {
        mos_time = tim_get_systime();
        gCell_Info.afe_ntc_temperature      = AfeTemperatureGet() * 10;
        gCell_Info.afe_ambient_temperature  = TaTemperatureGet()  * 10;
        gCell_Info.mos_temperature          = MosTemperatureGet() * 10;
        /* MOS过温1级 */
        if(gCell_Info.mos_temperature > MOS_OT1)
        {
            gFaultAnalysisFlag.mos_temperature_ov1_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov1_count = 0;
        }
        if(gFaultAnalysisFlag.mos_temperature_ov1_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper1 = 0;
        }

        /* MOS过温2级 */
        if(gCell_Info.mos_temperature > MOS_OT2)
        {
            gFaultAnalysisFlag.mos_temperature_ov2_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov2_count = 0;
        }

        if(gFaultAnalysisFlag.mos_temperature_ov2_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper2 = 1;
        }


        /* MOS过温3级 */
        if(gCell_Info.mos_temperature > MOS_OT3)
        {
            gFaultAnalysisFlag.mos_temperature_ov3_count++;
        }
        else
        {
            gFaultAnalysisFlag.mos_temperature_ov3_count = 0;
        }
        if(gFaultAnalysisFlag.mos_temperature_ov3_count > 4)
        {
            gBmsErrorInfo.mos_error_code.b.mos_high_temper3 = 1;
        }
    }    
}


/**
 * @brief  : 电池包电压故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _PackVoltageErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t voltage_time                = 0;
    if(tim_difftime_from_now(voltage_time) > 1000)
    {
        voltage_time = tim_get_systime();
        /* 单体欠压1级 */
        if(gCell_Info.cell_min_voltage < CELL_UV1)
        {
            gFaultAnalysisFlag.cell_under_votage1_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol1 = 0;
        }

        /* 单体欠压2级 */
        if(gCell_Info.cell_min_voltage < CELL_UV2)
        {
            gFaultAnalysisFlag.cell_under_votage2_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage2_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2 = 0;
        }

        /* 单体欠压3级 */
        if(gCell_Info.cell_min_voltage < CELL_UV3)
        {
            gFaultAnalysisFlag.cell_under_votage3_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_under_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_under_votage3_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_under_vol3 = 1;
        }

        /* 单体过压1级 */
        if(gCell_Info.cell_max_voltage > CELL_OV1)
        {
            gFaultAnalysisFlag.cell_over_votage1_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol1 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol1 = 0;
        }

        /* 单体过压2级 */
        if(gCell_Info.cell_max_voltage > CELL_OV2)
        {
            gFaultAnalysisFlag.cell_over_votage2_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage2_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2 = 1;
        }
        else
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2 = 0;
        }


        /* 单体过压3级 */
        if(gCell_Info.cell_max_voltage > CELL_OV3)
        {
            gFaultAnalysisFlag.cell_over_votage3_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_over_votage3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_over_votage3_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_over_vol3 = 1;
        }


        /* 电池包总电压欠压故障 */
        if(gCell_Info.total_cell_voltage < PACK_UV1)
        {
            gFaultAnalysisFlag.pack_under_vol1_count++;
        }
        else
        {
            gFaultAnalysisFlag.pack_under_vol1_count = 0;
        }
        if(gFaultAnalysisFlag.pack_under_vol1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_under_vol1 = 1;
        }


        /* 电池包总电压过压故障 */
        if(gCell_Info.total_cell_voltage > PACK_OV1)
        {
            gFaultAnalysisFlag.pack_over_vol1_count++;
        }
        else
        {
            gFaultAnalysisFlag.pack_over_vol1_count = 0;
        }
        if(gFaultAnalysisFlag.pack_over_vol1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.pack_over_vol1 = 1;
        }

        /* 电芯电压不均衡  单体压差大 */
        if((gCell_Info.cell_max_voltage - gCell_Info.cell_min_voltage) > CELL_V_DIFF1)
        {
            gFaultAnalysisFlag.cell_voltage_diff1_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_voltage_diff1_count = 0;

        }

        if(gFaultAnalysisFlag.cell_voltage_diff1_count > 4)
        {
            gBmsErrorInfo.voltage1_error_code.b.cell_voltage_diff1 = 1;
        }
    }   
}


/**
 * @brief  : 电流故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _CurrentErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t current_time                = 0;
    static uint32_t dsg_oc1_recover_time        = 0;                        //过流一级恢复时间
    int32_t pack_current                        = 0;
    static uint32_t chg_ing_max_current[3]      = {90000, 120000, 150000};  //充电中最大允许电流
    static uint32_t chg_end_max_current[3]      = {55000, 65000,  75000};   //充电末端最大允许电流
    uint32_t* p_max_current                     =  NULL;                    //最大允许电流指针
    //判断电流是正还是负值  负值转换为正值
    if(pPackStatus->pack_current_calibration < 0)
    {
        pack_current = -pPackStatus->pack_current_calibration;
    }
    else
    {
        pack_current = pPackStatus->pack_current_calibration;
    }
    
    if(tim_difftime_from_now(current_time) > 1000)
    {
        current_time = tim_get_systime();

        if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
        {
            /* 放电过流1级 */
            if(pack_current > DISCHARGE_OVERCURR1 * 0.01f * 50000)  //放电电流大于0.5C * 1.2，单位mA
            {
                gFaultAnalysisFlag.dsg_over_current1_count++;
                dsg_oc1_recover_time = tim_get_systime();
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current1_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current1_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 1;
            }
            else if(tim_difftime_from_now(dsg_oc1_recover_time) > 60000)
            {
                //故障消失60s后恢复
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr1 = 0;
            }

            /* 放电过流2级 */
            if(pack_current > DISCHARGE_OVERCURR2 * 0.01 * 50000)   //放电电流大于0.5C * 1.3，单位mA
            {
                gFaultAnalysisFlag.dsg_over_current2_count++;
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current2_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current2_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr2 = 1;
            }


            /* 放电过流3级 */
            if(pack_current > DISCHARGE_OVERCURR3 * 0.01 * 50000)    //放电电流大于0.5C * 1.4，单位mA
            {
                gFaultAnalysisFlag.dsg_over_current3_count++;
            }
            else
            {
                gFaultAnalysisFlag.dsg_over_current3_count = 0;
            }
            if(gFaultAnalysisFlag.dsg_over_current3_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.discharge_overcurr3 = 1;
            }

        }
        else if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
        {
            /*
            充电过流1级： 最高节电芯 < 3.5V，电流0.9C~1.2C，持续超过30S
                          最高节电芯 >= 3.5V，电流0.55C~0.65C，持续超过30S
            
            充电过流2级： 最高节电芯 < 3.5V，电流0.9C~1.2C，持续超过40S
                          最高节电芯 >= 3.5V，电流0.55C~0.65C，持续超过40S
                          最高节电芯 < 3.5V，电流1.2C~1.5C，持续超过3S
                          最高节电芯 >= 3.5V，电流0.65C~0.75C，持续超过3S
            
            充电过流3级： 最高节电芯 < 3.5V，电流0.9C~1.2C，持续超过50S
                          最高节电芯 >= 3.5V，电流0.55C~0.65C，持续超过50S
                          最高节电芯 < 3.5V，电流 > 1.5C，持续超过2S
                          最高节电芯 >= 3.5V，电流 > 0.75C，持续超过2S
            */
            if(pPackStatus->cell_max_voltage >= 3500)
            {
                p_max_current = chg_end_max_current;
            }
            else
            {
                p_max_current = chg_ing_max_current;
            }
                
            /* 充电过流1级计数 */
            if(pack_current > p_max_current[0])
            {   //最大计数为60
                if(gFaultAnalysisFlag.chg_over_current1_count <= 60)
                {
                    gFaultAnalysisFlag.chg_over_current1_count++;
                }
            }
            else
            {   //最小计数为0
                if(gFaultAnalysisFlag.chg_over_current1_count != 0)
                {
                    gFaultAnalysisFlag.chg_over_current1_count--;
                }
            }
            
            /* 充电过流2级计数 */
            if(pack_current > p_max_current[1])
            {   //最大计数为60
                if(gFaultAnalysisFlag.chg_over_current2_count <= 60)
                {
                    gFaultAnalysisFlag.chg_over_current2_count++;
                }
            }
            else
            {   //最小计数为0
                if(gFaultAnalysisFlag.chg_over_current2_count != 0)
                {
                    gFaultAnalysisFlag.chg_over_current2_count--;
                }
            }
            
            /* 充电过流3级计数 */
            if(pack_current > p_max_current[2]) 
            {   //最大计数为60
                if(gFaultAnalysisFlag.chg_over_current3_count <= 60)
                {
                    gFaultAnalysisFlag.chg_over_current3_count++;
                }
            }
            else
            {   //最小计数为0
                if(gFaultAnalysisFlag.chg_over_current3_count != 0)
                {
                    gFaultAnalysisFlag.chg_over_current3_count--;
                }
            }
            
            if(gFaultAnalysisFlag.chg_over_current1_count > 4)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 1;
            }
            else
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr1 = 0;
            }

            
            if((gFaultAnalysisFlag.chg_over_current1_count > 7) ||\
                gFaultAnalysisFlag.chg_over_current2_count > 3)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr2 = 1;
            }


            if((gFaultAnalysisFlag.chg_over_current1_count > 12) ||\
                gFaultAnalysisFlag.chg_over_current3_count > 2)
            {
                gBmsErrorInfo.curr_error_code.b.charge_overcurr3 = 1;
            }
        }
    }
}


/**
 * @brief  : 电池温度故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _PackTempureErrorJudge()
{
    static uint32_t temperature_time            = 0;
    static uint32_t temp_recover_time[2]        = {0, 0};
    
    if(tim_difftime_from_now(temperature_time) > 1000)
    {
        temperature_time = tim_get_systime();
        /* 电池温度过高一级报警 */
        if(gCell_Info.cell_max_temperature > CELL_OT1)
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot1_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper1 = 0;
        }

        /* 电池温度过高二级报警 */
        if(gCell_Info.cell_max_temperature > CELL_OT2)
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot2_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot2_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper2 = 1;
        }

        /* 电池温度过高三级报警 */
        if(gCell_Info.cell_max_temperature > CELL_OT3)
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ot3_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ot3_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_high_temper3 = 1;
        }

        /* 电池温度过低故障  */
        if(gCell_Info.cell_min_temperature < CELL_UT1)
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_ut1_count = 0;
        }
        if(gFaultAnalysisFlag.cell_temperature_ut1_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.cell_low_temper1 = 1;
        }
        else
        {
            gBmsErrorInfo.temper_error_code.b.cell_low_temper1 = 0;
        }
        /* 电池NTC温差大故障  */
        if(gCell_Info.cell_max_temperature - gCell_Info.cell_min_temperature > NTC_T_DIFF)
        {
            gFaultAnalysisFlag.ntc_temperature_ot_count++;
            //刷新恢复起始时间
            temp_recover_time[0] = tim_get_systime();
        }
        else
        {
            //温差过大后若温差恢复规定范围内，则该故障一分钟后恢复
            if((gBmsErrorInfo.temper_error_code.b.ntc_high_diff1 == 1) && (tim_difftime_from_now(temp_recover_time[0]) > 60000))
            {
                gBmsErrorInfo.temper_error_code.b.ntc_high_diff1 = 0;
            }
            gFaultAnalysisFlag.ntc_temperature_ot_count = 0;
        }
        if(gFaultAnalysisFlag.ntc_temperature_ot_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.ntc_high_diff1 = 1;
        }
        
        /* 电池NTC温差大2级故障  */
        if(gCell_Info.cell_max_temperature - gCell_Info.cell_min_temperature > NTC_T_DIFF2)
        {
            gFaultAnalysisFlag.ntc_temperature_ot2_count++;
            //刷新恢复起始时间
            temp_recover_time[1] = tim_get_systime();
        }
        else
        {
            //温差过大后若温差恢复规定范围内，则该故障一分钟后恢复
            if((gBmsErrorInfo.temper_error_code.b.ntc_high_diff2 == 1) && (tim_difftime_from_now(temp_recover_time[1]) > 60000))
            {
                gBmsErrorInfo.temper_error_code.b.ntc_high_diff2 = 0;
            }
            gFaultAnalysisFlag.ntc_temperature_ot2_count = 0;
        }
        
        if(gFaultAnalysisFlag.ntc_temperature_ot2_count > 4)
        {
            gBmsErrorInfo.temper_error_code.b.ntc_high_diff2 = 1;
        }
    }    
}



/**
 * @brief  : SOH过低故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _SohUnderErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t soh_under_time              = 0;
    
    if(tim_difftime_from_now(soh_under_time) > 1000)
    {
        soh_under_time = tim_get_systime();
        /* SOH过低一级 */
        if(pPackStatus->soh_rpt < SOH_UNDER_LVL1)
        {
            gFaultAnalysisFlag.soh_under_lvl1_count++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl1_count = 0;

        }
        /* SOH过低二级 */
        if(pPackStatus->soh_rpt < SOH_UNDER_LVL2)
        {
            gFaultAnalysisFlag.soh_under_lvl2_count++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl2_count = 0;

        }
        /* SOH过低3级 */
        if(pPackStatus->soh_rpt < SOH_UNDER_LVL3)
        {
            gFaultAnalysisFlag.soh_under_lvl3_count++;
        }
        else
        {
            gFaultAnalysisFlag.soh_under_lvl3_count = 0;

        }

        if(gFaultAnalysisFlag.soh_under_lvl1_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error1 = 1;
        }
        if(gFaultAnalysisFlag.soh_under_lvl2_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error2 = 1;
        }
        if(gFaultAnalysisFlag.soh_under_lvl3_count > 4)
        {
            gBmsErrorInfo.state_error_code.b.soh_low_error3 = 1;
        }

    }    
}


/**
 * @brief  : 电压传感器故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _VoltageSensorErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t vsensor_error_time          = 0;
    
    /* 电压传感器故障 */
    if(tim_difftime_from_now(vsensor_error_time) > 1000)
    {
        vsensor_error_time = tim_get_systime();
        if(gCell_Info.cell_max_voltage > 4000 || gCell_Info.cell_min_voltage < 2000)
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_votage_sensor_error_count = 0;

        }

        if(gFaultAnalysisFlag.cell_votage_sensor_error_count > 5)
        {
            gBmsErrorInfo.other1_error_code.b.vol_sensor_error = 1;
        }
    }    
}


/**
 * @brief  : 电芯温度传感器故障
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _PackTempureSensorErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t tsensor_error_time         = 0;
    if(tim_difftime_from_now(tsensor_error_time) >= 2000)
    {
        tsensor_error_time = tim_get_systime();
        /* 电池温度传感器故障 */
        if(gCell_Info.cell_max_temperature > TSENSOR_OT || gCell_Info.cell_min_temperature < TSENSOR_UT)
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count++;
        }
        else
        {
            gFaultAnalysisFlag.cell_temperature_sensor_error_count = 0;

        }
        if(gFaultAnalysisFlag.cell_temperature_sensor_error_count > 10)
        {
            gBmsErrorInfo.other1_error_code.b.cell_ntc_error = 1;
        }
    }    
}

/**
 * @brief  : Flash故障
 * @para   : None
 * @note   : None
 * @return : None
 */
void _FlashErrorJudge()
{
    //flash 故障
    if(gFaultAnalysisFlag.flash_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.spi_flash_error = 1;
    }
}

/**
 * @brief  : Eeprom故障
 * @para   : None
 * @note   : None
 * @return : None
 */
void _EepromErrorJudge()
{
    //e2prom 故障
    if(gFaultAnalysisFlag.e2prom_error_flag)
    {
        gBmsErrorInfo.other1_error_code.b.e2prom_error = 1;
    }
}

/**
 * @brief  : 加热膜故障
 * @para   : None
 * @note   : None
 * @return : None
 */
void _HeatErrorJudge()
{
    static uint32_t heat_time                   = 0;
    /* 加热膜故障  */
    if(tim_difftime_from_now(heat_time) >= 500)
    {
        heat_time = tim_get_systime();
        if(HeatshortCheck() == true)
        {
            gFaultAnalysisFlag.heat_error_count++;
        }
        else
        {
            gFaultAnalysisFlag.heat_error_count = 0;
        }

        if(gFaultAnalysisFlag.heat_error_count > 4)
        {
            gBmsErrorInfo.other1_error_code.b.heat_error = 1;
        }
    }
}


/**
 * @brief  : Rtc故障
 * @para   : None
 * @note   : None
 * @return : None
 */
void _RtcErrorJudge()
{
    static uint32_t rtc_error_time         = 0;
    static uint32_t last_rtc               = 0;
    static uint32_t rtc_error_count        = 0;
    uint32_t next_rtc                      = 0;
    /*RTC 故障*/
    if(tim_difftime_from_now(rtc_error_time) > 10000)
    {
        rtc_error_time = tim_get_systime();
        
        next_rtc = RtcGetTime();
        if(next_rtc < last_rtc)
        {
            rtc_error_count++;
        }
        else
        {
            rtc_error_count = 0;
        }
        
        if(rtc_error_count >= 5)
        {
            gBmsErrorInfo.other1_error_code.b.rtc_error = 1;
        }
        last_rtc = next_rtc;
    }
}


/**
 * @brief  : 放电短路故障判断
 * @para   : None
 * @note   : None
 * @return : None
 */
void _DsgShortErrorJudge()
{
    if(DsgOCCheck() == true)
    {
        gBmsErrorInfo.other1_error_code.b.dsg_short_protection = 1;
    }    
}

/**
 * @brief  : 充电短路故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _ChgShortErrorJudge(PackStatus_t *pPackStatus)
{
    static uint32_t chg_short_time              = 0;
    int32_t pack_current                        = 0;
    
    //判断电流是正还是负值  负值转换为正值
    if(pPackStatus->pack_current_calibration < 0)
    {
        pack_current = -pPackStatus->pack_current_calibration;
    }
    else
    {
        pack_current = pPackStatus->pack_current_calibration;
    }
    
    if(tim_difftime_from_now(chg_short_time) > 500)
    {
        chg_short_time = tim_get_systime();

        if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
        {
            /* 充电电流过大，定为充电短路 */
            if(pack_current > CHARGE_OVERCURR3 * 0.01 * 120000)  //充电电流大于200A
            {
                gFaultAnalysisFlag.chg_short_protection_count++;
            }
            else
            {
                gFaultAnalysisFlag.chg_short_protection_count = 0;
            }
            if(gFaultAnalysisFlag.chg_short_protection_count > 5)
            {
                gBmsErrorInfo.other1_error_code.b.chg_short_protection = 1;
            }
        }
    }
}

/**
 * @brief  : AFE故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _AfeErrorJudge(PackStatus_t *pPackStatus)                    /* afe通讯故障判断 */
{
    static uint32_t error_check_time    = 0;  
    static bool cmos_current_normal_flag    = true;
    static bool cc_current_normal_flag      = true;
    float n_pack_charge_v_val               = 0;    
    
    if(tim_difftime_from_now(error_check_time) < 1000)
    {
        return;
    }
    error_check_time = tim_get_systime();
    
    if(gFaultAnalysisFlag.afe_com_fault_count > 3)
    {
        gFaultAnalysisFlag.afe_com_error_count++;
    }
    else
    {
        if(gFaultAnalysisFlag.afe_com_error_count != 0)
        {
            gFaultAnalysisFlag.afe_com_error_count--;
        }
    }
    
    if(gFaultAnalysisFlag.afe_com_error_count > 3)
    {
        gBmsErrorInfo.other1_error_code.b.afe_com_error = 1;
    }
    
    
    gFaultAnalysisFlag.afe_com_fault_count = 0;
}

/**
 * @brief  : AFE断线检测故障判断
 * @para   : pPackStatus : 电池状态信息
 * @note   : None
 * @return : None
 */
void _AfeOwErrorJudge()
{
    static uint32_t error_check_time    = 0;     
    
    if(tim_difftime_from_now(error_check_time) < 1000)
    {
        return;
    }
    error_check_time = tim_get_systime();
    
    if(GetAfeOwStatus() != 0)
    {
        if(gFaultAnalysisFlag.afe_ow_error_count <= 10)
        {
            gFaultAnalysisFlag.afe_ow_error_count++;
        }
    }
    else
    {
        if(gFaultAnalysisFlag.afe_ow_error_count != 0)
        {
            gFaultAnalysisFlag.afe_ow_error_count--;
        }
    }
    
    if(gFaultAnalysisFlag.afe_ow_error_count > 4)
    {
        gBmsErrorInfo.other1_error_code.b.afe_ow_error = 1;
    }

}

void LatchErrorCheck(void)//此函数的作用：
{
    uint8_t rdata[ERROR11_SAVE_ADDR + 0x08] = {0};
    uint8_t *p = NULL;
//    memset(rdata, 0xAA, 0x40);
//    memset(rdata, 0, ERROR11_SAVE_ADDR + 0x08);
//    EepromWrite(rdata,ERROR1_SAVE_ADDR, 0x50);

    EepromRead(rdata, ERROR1_SAVE_ADDR, ERROR11_SAVE_ADDR + 0x08);
    p = rdata;

    /* 0x05防止指针溢出 */
    for(int i = 0; i < ERROR11_SAVE_ADDR + 0x05; i++)
    {
        if(rdata[i] != 0)
        {
            break;
        }
        else
        {
            p++;
        }
    }

    if(strchr((char*)p, 0xAA) != 0)
    {
//        garrMODBUS_Reg[0].bms_shutdown = 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_under_vol3 = (rdata[0x00] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_over_vol3 = (rdata[0x08] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh1 = (rdata[0x10] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.pack_low_soh2 = (rdata[0x18] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cell1_error_code.b.cell_voltage_diff = (rdata[0x20] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = (rdata[0x28] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.vol_sensor_error = (rdata[0x30] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.cellsample_error_code.b.cell_ntc_error = (rdata[0x38] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.other1_error_code.b.chg_short_protection = (rdata[0x40] ^ 0xAA) ? 0 : 1;
//        gBmsErrorInfo.other1_error_code.b.dsg_short_protection = (rdata[0x48] ^ 0xAA) ? 0 : 1;
    }
}

void ErrorClear(void)
{
    uint8_t rdata[ERROR11_SAVE_ADDR + 0x08] = {0};

    memset(rdata, 0, ERROR11_SAVE_ADDR + 0x08);
    EepromWrite(rdata, ERROR1_SAVE_ADDR, ERROR11_SAVE_ADDR + 0x08);
}

MSH_CMD_EXPORT(ErrorClear, -- -);