/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_factory_test.c
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
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "systick.h"
#include "can_tx_rx_process.h"


//#define SLAVE_ADDR_NUM  PACK_NUM
typedef enum
{
    EN_PCS_01_TX = 0,
    EN_PCS_02_TX,
    EN_PCS_03_TX,
    EN_PCS_04_TX,
    EN_PCS_05_TX,
    EN_PCS_06_TX,
    EN_PCS_07_TX,
    EN_PCS_08_TX,
    EN_PCS_09_TX,
    EN_PCS_0A_TX,
    EN_PCS_0B_TX,
    EN_PCS_0C_TX,
    EN_PCS_0D_TX,
    EN_PCS_0E_TX,
    EN_PCS_0F_TX,
    EN_PCS_10_TX,
    EN_PCS_11_TX,
    EN_PCS_12_TX,
    EN_PCS_13_TX,
    EN_PCS_14_TX,
    EN_PCS_15_TX,
    EN_PCS_TX_TOTAL,
} EN_CAN_PCS_TX;
typedef enum
{
    EN_PCS_01_RX = 0,
    EN_PCS_RX_TOTAL,
} EN_CAN_PCS_RX;



void PCS01TxPrep(uint8_t idx);
void PCS02TxPrep(uint8_t idx);
void PCS03TxPrep(uint8_t idx);
void PCS04TxPrep(uint8_t idx);
void PCS05TxPrep(uint8_t idx);
void PCS06TxPrep(uint8_t idx);
void PCS07TxPrep(uint8_t idx);
void PCS08TxPrep(uint8_t idx);
void PCS09TxPrep(uint8_t idx);
void PCS0ATxPrep(uint8_t idx);
void PCS0BTxPrep(uint8_t idx);
void PCS0CTxPrep(uint8_t idx);
void PCS0DTxPrep(uint8_t idx);
void PCS0ETxPrep(uint8_t idx);
void PCS0FTxPrep(uint8_t idx);
void PCS10TxPrep(uint8_t idx);
void PCS11TxPrep(uint8_t idx);
void PCS12TxPrep(uint8_t idx);
void PCS13TxPrep(uint8_t idx);
void PCS14TxPrep(uint8_t idx);
void PCS15TxPrep(uint8_t idx);
void PCS01RxProcess(uint8_t *p_buf);

static void _TxPolling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoPCSTx[] = 
{
    {0x180150F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS01TxPrep, _TxPolling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS02TxPrep, _TxPolling}, 
    {0x180350F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS03TxPrep, _TxPolling}, 
    {0x180450F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS04TxPrep, _TxPolling}, 
    {0x180550F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS05TxPrep, _TxPolling}, 
    {0x180650F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS06TxPrep, _TxPolling}, 
    {0x180750F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS07TxPrep, _TxPolling}, 
    {0x180850F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS08TxPrep, _TxPolling}, 
    {0x180950F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS09TxPrep, _TxPolling}, 
    {0x180A50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS0ATxPrep, _TxPolling}, 
    {0x180B50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS0BTxPrep, _TxPolling}, 
    {0x180C50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200,  false, PCS0CTxPrep, _TxPolling}, 
    {0x180D50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS0DTxPrep, _TxPolling}, 
    {0x180E50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS0ETxPrep, _TxPolling}, 
    {0x180F50F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS0FTxPrep, _TxPolling}, 
    {0x181050F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS10TxPrep, _TxPolling}, 
    {0x181150F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS11TxPrep, _TxPolling}, 
    {0x181250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2000, false, PCS12TxPrep, _TxPolling}, 
    {0x181350F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 500,  false, PCS13TxPrep, _TxPolling}, 
    {0x181450F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 500,  false, PCS14TxPrep, _TxPolling}, 
    {0x181550F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 500,  false, PCS15TxPrep, _TxPolling}, 
};

CanProtoListTxInfo_t gCanProtoListInfoPCSTx = 
{
    .p_proto_info = _gCanProtoInfoPCSTx,
    .num = EN_PCS_TX_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoPCSRx[] = 
{
    {0x1801F150, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, PCS01RxProcess, NULL},  
};

CanProtoListRxInfo_t gCanProtoListInfoPCSRx = 
{
    .p_proto_info = _gCanProtoInfoPCSRx,
    .num = EN_PCS_RX_TOTAL
};



////////////TX//////////////

/** 
* @brief    01报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS01TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gBMSDataInfo.bmu_calc_info.cell_max_volt >> 8) & 0xFF;
    buf[1] = (gBMSDataInfo.bmu_calc_info.cell_max_volt) & 0xFF;
    buf[2] = (gBMSDataInfo.bmu_calc_info.cell_min_volt >> 8) & 0xFF;
    buf[3] = (gBMSDataInfo.bmu_calc_info.cell_min_volt) & 0xFF;
    buf[4] = gPackInfo.sox_info.soc_disp + 0.5;//先上传表显的
    buf[5] = gPackInfo.sox_info.soh + 0.5;
    buf[6] = 0x00;
    buf[7] = (RelayPFeedback() == true) ? 1 : 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_01_TX], buf);
}


/** 
* @brief    02报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS02TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    //总电压暂用BMU侧获得的电芯累加数据，也可使用高压采集的电压值
    buf[0] = ((gBMSDataInfo.bmu_calc_info.cell_total_volt / 100) >> 8) & 0xFF;
    buf[1] = (gBMSDataInfo.bmu_calc_info.cell_total_volt / 100) & 0xFF;
//    buf[0] = ((gHighVoltInfo.v_bp / 100) >> 8) & 0xFF;
//    buf[1] = (gHighVoltInfo.v_bp / 100) & 0xFF;
    buf[2] = ((-gPackInfo.current / 100) >> 8) & 0xFF;//实际的电流方向定义和协议相反
    buf[3] = (-gPackInfo.current / 100) & 0xFF;//实际的电流方向定义和协议相反
    buf[4] = ((gPackInfo.chg_cur_current_limit / 100) >> 8) & 0xFF;
    buf[5] = (gPackInfo.chg_cur_current_limit / 100) & 0xFF;
    buf[6] = ((gPackInfo.dsg_cur_current_limit / 100) >> 8) & 0xFF;
    buf[7] = (gPackInfo.dsg_cur_current_limit / 100) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_02_TX], buf);
}


/** 
* @brief    03报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS03TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = ((gAlarmThreshold.bp_volt_high_threshold[EN_ALARM_LEVEL_1] * 10) >> 8) & 0xFF;
    buf[1] = (gAlarmThreshold.bp_volt_high_threshold[EN_ALARM_LEVEL_1] * 10) & 0xFF;
    buf[2] = ((gAlarmThreshold.bp_volt_low_threshold[EN_ALARM_LEVEL_1] * 10) >> 8) & 0xFF;
    buf[3] = (gAlarmThreshold.bp_volt_low_threshold[EN_ALARM_LEVEL_1] * 10) & 0xFF;
    buf[4] = gBMSDataInfo.bmu_calc_info.cell_max_temp + 40;
    buf[5] = gBMSDataInfo.bmu_calc_info.cell_min_temp + 40;
    buf[6] = gBMSDataInfo.bmu_calc_info.cell_ave_temp + 40;
    buf[7] = 0x00;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_03_TX], buf);
}



/** 
* @brief    04报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS04TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    //PCS协议，编号从0开始
    buf[0] = ((gBMSDataInfo.bmu_calc_info.cell_max_volt_num) >> 8) & 0xFF;
    buf[1] = (gBMSDataInfo.bmu_calc_info.cell_max_volt_num) & 0xFF;
    buf[2] = ((gBMSDataInfo.bmu_calc_info.cell_min_volt_num) >> 8) & 0xFF;
    buf[3] = (gBMSDataInfo.bmu_calc_info.cell_min_volt_num) & 0xFF;
    buf[4] = ((gBMSDataInfo.bmu_calc_info.cell_max_temp_num) >> 8);
    buf[5] = (gBMSDataInfo.bmu_calc_info.cell_max_temp_num) & 0xFF;
    buf[6] = ((gBMSDataInfo.bmu_calc_info.cell_min_temp_num) >> 8);
    buf[7] = (gBMSDataInfo.bmu_calc_info.cell_min_temp_num) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_04_TX], buf);
}


/** 
* @brief    05报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS05TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = ((gBMSDataInfo.bmu_calc_info.cell_ave_volt) >> 8) & 0xFF;
    buf[1] = (gBMSDataInfo.bmu_calc_info.cell_ave_volt) & 0xFF;
    buf[2] = 0x00;
    buf[3] = (gInsResInfo.detect_stat == EN_INS_RES_DETECT_STAT_ING || gInsResInfo.detect_stat == EN_INS_RES_DETECT_STAT_COMPLETE) ? 1 : 0;

    buf[4] = (gInsResInfo.bp_res >> 8) & 0xFF;
    buf[5] = gInsResInfo.bp_res & 0xFF;
	buf[6] = (gInsResInfo.bp_res >> 8) & 0xFF;
    buf[7] = gInsResInfo.bp_res & 0xFF;
//    buf[6] = (gInsResInfo.bn_res >> 8) & 0xFF;
//    buf[7] = gInsResInfo.bn_res & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_05_TX], buf);
}


/** 
* @brief    06报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS06TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    uint8_t temp_u8 = 0;
    /*电池状态*/
    if(gPackInfo.chg_max_current == 0 && gPackInfo.dsg_max_current == 0)
    {
        temp_u8 = 1;
    }
    else if(gPackInfo.chg_max_current == 0)
    {
        temp_u8 = 2;
    }
    else if(gPackInfo.dsg_max_current == 0)
    {
        temp_u8 = 3;
    }
    else
    {
        if(gPackInfo.mode == EN_PACK_CURRENT_CHARGE_MODE)
        {
            temp_u8 = 4;
        }
        else if(gPackInfo.mode == EN_PACK_CURRENT_DISCHARGE_MODE)
        {
            temp_u8 = 5;
        }
        else
        {
            temp_u8 = 0;
        }
    }
    buf[0] = temp_u8;
    
    /*系统状态标识*/
    temp_u8 = 0;
    //系统准备，自检无告警，总正继电器闭合成功
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] == 0 && gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] == 0 && gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] == 0 &&
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] == 0 && gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] == 0 && gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] == 0 &&
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] == 0 && gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] == 0 && gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] == 0 &&
       RelayPFeedback() == true)
    {
        temp_u8 |= (1 << 0);
    }
    else
    {
        temp_u8 &= ~(1 << 0);
    }
    //充电完成
    if(gPackInfo.full_flag == true)
    {
        temp_u8 |= (1 << 1);
    }
    else
    {
        temp_u8 &= ~(1 << 1);
    }
    //放电完成
    if(gPackInfo.empty_flag == true)
    {
        temp_u8 |= (1 << 2);
    }
    else
    {
        temp_u8 &= ~(1 << 2);
    }
    //一级报警
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] != 0)
    {
        temp_u8 |= (1 << 3);
    }
    else
    {
        temp_u8 &= ~(1 << 3);
    }
    //二级报警
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] != 0)
    {
        temp_u8 |= (1 << 4);
    }
    else
    {
        temp_u8 &= ~(1 << 4);
    }
    //三级报警
    if(gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] != 0 || 
       gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] != 0 ||
       gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] != 0)
    {
        temp_u8 |= (1 << 5);
    }
    else
    {
        temp_u8 &= ~(1 << 5);
    }
    buf[1] = temp_u8;
    
    /*一级告警高,测量告警*/
    buf[2] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] >> 24) & 0xFF;
    buf[3] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] >> 16) & 0xFF;
    /*并簇状态*/
    //to do
    temp_u8 = 0;
    if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_RELAY_PRE_ON || gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_RELAY_P_ON)
    {
        temp_u8 = 1;//并簇中
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_NORMAL_RUN)
    {
        temp_u8 = 7;//并簇完成
    }
    else if(gSysRunInfo.sys_stat == EN_SYS_RUN_STAT_FAIL)
    {
        temp_u8 = 255;//并簇失败
    }
    else
    {
        temp_u8 = 0;//未并簇
    }
    buf[4] = temp_u8;
    
    /*一级告警低,测量告警*/
    buf[5] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1] >> 8) & 0xFF;
    buf[6] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_1]) & 0xFF;
    
    buf[7] = 0x00;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_06_TX], buf);
}


/** 
* @brief    07报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS07TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] >> 24) & 0xFF;
    buf[1] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] >> 16) & 0xFF;
    buf[2] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2] >> 8) & 0xFF;
    buf[3] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_2]) & 0xFF;
    buf[4] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] >> 24) & 0xFF;
    buf[5] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] >> 16) & 0xFF;
    buf[6] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3] >> 8) & 0xFF;
    buf[7] = (gAlarmCode.measure_alarm_code[EN_ALARM_LEVEL_3]) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_07_TX], buf);
}


/** 
* @brief    08报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS08TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    uint16_t chargeable_kwh = gPackInfo.q_real * (100 - gPackInfo.sox_info.soc_real / 100) / 1000 * gPackInfo.cell_total_voltage / 1000 / 1000;
    uint16_t dischargeable_kwh = gPackInfo.q_real * (gPackInfo.sox_info.soc_real / 100) / 1000 * gPackInfo.cell_total_voltage / 1000 / 1000;
    
    buf[0] = (chargeable_kwh >> 8) & 0xFF;
    buf[1] = (chargeable_kwh) & 0xFF;
    buf[2] = (dischargeable_kwh >> 8) & 0xFF;
    buf[3] = (dischargeable_kwh) & 0xFF;
    buf[4] = (gPackInfo.sox_info.chg_cycle >> 8) & 0xFF;
    buf[5] = (gPackInfo.sox_info.chg_cycle) & 0xFF;
    buf[6] = (gPackInfo.sox_info.dsg_cycle >> 8) & 0xFF;
    buf[7] = (gPackInfo.sox_info.dsg_cycle) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_08_TX], buf);
}




/** 
* @brief    09报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS09TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 1000) >> 24) & 0xFF;
    buf[1] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 1000) >> 16) & 0xFF;
    buf[2] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 1000) >> 8) & 0xFF;
    buf[3] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_e / 1000)) & 0xFF;
    buf[4] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 1000) >> 24) & 0xFF;
    buf[5] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 1000) >> 16) & 0xFF;
    buf[6] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 1000) >> 8) & 0xFF;
    buf[7] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_e / 1000)) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_09_TX], buf);
}



/** 
* @brief    0A报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS0ATxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 1000) >> 24) & 0xFF;
    buf[1] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 1000) >> 16) & 0xFF;
    buf[2] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 1000) >> 8) & 0xFF;
    buf[3] = ((uint32_t)(gPackInfo.elect_statist.chg_total_accum_q / 1000)) & 0xFF;
    buf[4] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 1000) >> 24) & 0xFF;
    buf[5] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 1000) >> 16) & 0xFF;
    buf[6] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 1000) >> 8) & 0xFF;
    buf[7] = ((uint32_t)(gPackInfo.elect_statist.dsg_total_accum_q / 1000)) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0A_TX], buf);
}


/** 
* @brief    0B报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS0BTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = gTempInfo.temp_relay_p + 40;
    buf[1] = gTempInfo.temp_hvbox + 40;
    //以下预留不用
    buf[2] = 40;
    buf[3] = 40;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0B_TX], buf);
}


/** 
* @brief    0C报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void PCS0CTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gBMSDataInfo.bmu_num >> 8) & 0xFF;
    buf[1] = (gBMSDataInfo.bmu_num) & 0xFF;
    buf[2] = ((gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num) >> 8) & 0xFF;
    buf[3] = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num) & 0xFF;
    buf[4] = ((gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_temp_num) >> 8) & 0xFF;//未包含极柱和正负极
    buf[5] = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_temp_num) & 0xFF;//未包含极柱和正负极
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0C_TX], buf);
}


/** 
* @brief    0D报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS0DTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t start_num;//起始编号，从0开始
    uint16_t total_cell_num = gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num;//电芯总数量
    
    start_num = idx * 3;
    if(start_num >= total_cell_num)//超出总数量
    {
        return;
    }
    buf[0] = (start_num >> 8) & 0xFF; //如果从1开始，则buf[0]和buf[1]的start_num替换成(start_num + 1)，其他地方不变
    buf[1] = start_num & 0xFF;  
    
    for(uint8_t i = 0; i < 3; i++)
    {
        if(start_num + i >= total_cell_num)//超出总数量
        {
            buf[i * 2 + 2] = 0;
            buf[i * 2 + 3] = 0;
        }
        else
        {
            buf[i * 2 + 2] = (gBMSDataInfo.bmu_detail_volt_info[(start_num + i) / gBMSDataInfo.per_bmu_cell_num].cell_volt[(start_num + i) % gBMSDataInfo.per_bmu_cell_num] >> 8) & 0xFF;
            buf[i * 2 + 3] = gBMSDataInfo.bmu_detail_volt_info[(start_num + i) / gBMSDataInfo.per_bmu_cell_num].cell_volt[(start_num + i) % gBMSDataInfo.per_bmu_cell_num] & 0xFF;
        }
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0D_TX], buf);
}


/** 
* @brief    0E报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS0ETxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t start_num;//起始编号，从0开始
    uint16_t total_temp_num = gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_temp_num;//温度总数量
    
    start_num = idx * 6;
    if(start_num >= total_temp_num)//超出总数量
    {
        return;
    }
    buf[0] = (start_num >> 8) & 0xFF; //如果从1开始，则buf[0]和buf[1]的start_num替换成(start_num + 1)，其他地方不变
    buf[1] = start_num & 0xFF; 
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if(start_num + i >= total_temp_num)//超出总数量
        {
            buf[i + 2] = 0;
        }
        else
        {
            buf[i + 2] = gBMSDataInfo.bmu_detail_temp_info[(start_num + i) / gBMSDataInfo.per_bmu_cell_num].cell_temp[(start_num + i) % gBMSDataInfo.per_bmu_cell_temp_num] + 40;
        }
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0E_TX], buf);
}



/** 
* @brief    0F报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS0FTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t soc;
    uint16_t start_num;//起始编号，从0开始
    uint16_t total_cell_num = gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num;//电芯总数量
    
    soc = gPackInfo.sox_info.soc_disp + 0.5;
    start_num = idx * 6;
    if(start_num >= total_cell_num)//超出总数量
    {
        return;
    }
    buf[0] = (start_num >> 8) & 0xFF; //如果从1开始，则buf[0]和buf[1]的start_num替换成(start_num + 1)，其他地方不变
    buf[1] = start_num & 0xFF;  
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if(start_num + i >= total_cell_num)//超出总数量
        {
            buf[i + 2] = 0;
        }
        else
        {
            buf[i + 2] = soc;
        }
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_0F_TX], buf);
}



/** 
* @brief    10报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS10TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t soh;
    uint16_t start_num;//起始编号，从0开始
    uint16_t total_cell_num = gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num;//电芯总数量
    
    soh = gPackInfo.sox_info.soh + 0.5;
    start_num = idx * 6;
    if(start_num >= total_cell_num)//超出总数量
    {
        return;
    }
    buf[0] = (start_num >> 8) & 0xFF; //如果从1开始，则buf[0]和buf[1]的start_num替换成(start_num + 1)，其他地方不变
    buf[1] = start_num & 0xFF;  
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if(start_num + i >= total_cell_num)//超出总数量
        {
            buf[i + 2] = 0;
        }
        else
        {
            buf[i + 2] = soh;
        }
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_10_TX], buf);
}

/** 
* @brief    11报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS11TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t start_num;//起始编号，从0开始
    uint16_t total_cell_num = gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num;//电芯总数量
    
    start_num = idx * 48;
    if(start_num >= total_cell_num)//超出总数量
    {
        return;
    }
    buf[0] = (start_num >> 8) & 0xFF; //如果从1开始，则buf[0]和buf[1]的start_num替换成(start_num + 1)，其他地方不变
    buf[1] = start_num & 0xFF; 
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if(start_num + i * 8 >= total_cell_num)//一帧中有字节超出总数量
        {
            buf[i + 2] = 0;
        }
        else
        {
            for(uint8_t j = 0; j < 8; j++)
            {
                if(start_num + i * 8 + j >= total_cell_num)//一个字节有bit超出总数量
                {
                    buf[i + 2] &= ~(1 << j);
                }
                else
                {
                    if(gBMSDataInfo.bmu_detail_volt_info[(start_num + i * 8 + j) / gBMSDataInfo.per_bmu_cell_num].cell_balance_stat[(start_num + i * 8 + j) % gBMSDataInfo.per_bmu_cell_num] == true)
                    {
                        buf[i + 2] |= (1 << j);
                    }
                    else
                    {
                        buf[i + 2] &= ~(1 << j);
                    }
                }
            }
        }
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_11_TX], buf);
}


/** 
* @brief    12报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS12TxPrep(uint8_t idx)
{
    return;//设备没有MOS
}



/** 
* @brief    13报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS13TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] >> 24) & 0xFF;
    buf[1] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] >> 16) & 0xFF;
    buf[2] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1] >> 8) & 0xFF;
    buf[3] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_1]) & 0xFF;
    buf[4] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] >> 24) & 0xFF;
    buf[5] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] >> 16) & 0xFF;
    buf[6] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2] >> 8) & 0xFF;
    buf[7] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_2]) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_13_TX], buf);
}


/** 
* @brief    14报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS14TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] >> 24) & 0xFF;
    buf[1] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] >> 16) & 0xFF;
    buf[2] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3] >> 8) & 0xFF;
    buf[3] = (gAlarmCode.dev_alarm_code[EN_ALARM_LEVEL_3]) & 0xFF;
    buf[4] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] >> 24) & 0xFF;
    buf[5] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] >> 16) & 0xFF;
    buf[6] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1] >> 8) & 0xFF;
    buf[7] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_1]) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_14_TX], buf);
}


/** 
* @brief    15报文发送数据准备
* @param    idx:下发数据索引,代表第几帧
* @return   NULL
* @note:
*/
void PCS15TxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] >> 24) & 0xFF;
    buf[1] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] >> 16) & 0xFF;
    buf[2] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2] >> 8) & 0xFF;
    buf[3] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_2]) & 0xFF;
    buf[4] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] >> 24) & 0xFF;
    buf[5] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] >> 16) & 0xFF;
    buf[6] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3] >> 8) & 0xFF;
    buf[7] = (gAlarmCode.other_alarm_code[EN_ALARM_LEVEL_3]) & 0xFF;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPCSTx[EN_PCS_15_TX], buf);
}


////////////RX//////////////
/** 
* @brief    01报文接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void PCS01RxProcess(uint8_t *p_buf)
{
    if(p_buf[1] == 0x01)//并簇指令
    {
        gSysRunInfo.cluster_cmd = 1;
    }
    if(p_buf[2] == 0x01)//退簇指令
    {
        gSysRunInfo.cluster_cmd = 2;
    }
    if(p_buf[6] == 0x01)//系统复位
    {
        gBMSDataInfo.bcu_reset_flag = true;
    }
    
    gBMSDataInfo.pcs_comm_last_time = tim_get_systime();
}


/** 
* @brief    数据发送轮询
* @param    idx，对应的协议项索引
* @return   NULL
* @note:
*/
static void _TxPolling(uint8_t idx)
{
    static uint32_t tx_delay_count = 0;
    static uint32_t regular_time_count[EN_PCS_TX_TOTAL];
    
    if(gBMSDataInfo.bmu_calc_info.valid_flag == true)
    {
        if((uint32_t)(tim_get_systime() - tx_delay_count) > 50)
        {
            _gCanProtoInfoPCSTx[EN_PCS_01_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_02_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_03_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_04_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_05_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_06_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_07_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_08_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_09_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_0A_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_0B_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_0C_TX].regular_default_enable = true;
        }
        if((uint32_t)(tim_get_systime() - tx_delay_count) > 100)
        {
            _gCanProtoInfoPCSTx[EN_PCS_13_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_14_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_15_TX].regular_default_enable = true;
        }
        if((uint32_t)(tim_get_systime() - tx_delay_count) > 5000)
        {
            _gCanProtoInfoPCSTx[EN_PCS_0D_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_0E_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_0F_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_10_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_11_TX].regular_default_enable = true;
            _gCanProtoInfoPCSTx[EN_PCS_12_TX].regular_default_enable = true;
        }
    }
    else
    {
        tx_delay_count = tim_get_systime();
    }
    
    if(idx <= EN_PCS_0C_TX || idx >= EN_PCS_13_TX)//一帧就能发送完的数据
    {
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= _gCanProtoInfoPCSTx[idx].tx_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(0);
            }
        } 
    }
    else if(idx == EN_PCS_0D_TX)//单体电压
    {
        static uint8_t frame_count = 0;
        uint8_t frame_num = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num + 2) / 3;
        uint32_t current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if(frame_count == 0)
            {
                current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
            }
            else
            {
                current_regular_period = 6;
            }
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= current_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(frame_count);
                frame_count++;
                if(frame_count >= frame_num)
                {
                    frame_count = 0;
                }
            }
        }
    }
    else if(idx == EN_PCS_0E_TX)//单体温度
    {
        static uint8_t frame_count = 0;
        uint8_t frame_num = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_temp_num + 5) / 6;
        uint32_t current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if(frame_count == 0)
            {
                current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
            }
            else
            {
                current_regular_period = 20;
            }
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= current_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(frame_count);
                frame_count++;
                if(frame_count >= frame_num)
                {
                    frame_count = 0;
                }
            }
        }
    }
    else if(idx == EN_PCS_0F_TX)//单体SOC
    {
        static uint8_t frame_count = 0;
        uint8_t frame_num = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num + 5) / 6;
        uint32_t current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if(frame_count == 0)
            {
                current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
            }
            else
            {
                current_regular_period = 6;
            }
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= current_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(frame_count);
                frame_count++;
                if(frame_count >= frame_num)
                {
                    frame_count = 0;
                }
            }
        }
    }
    else if(idx == EN_PCS_10_TX)//单体SOH
    {
        static uint8_t frame_count = 0;
        uint8_t frame_num = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num + 5) / 6;
        uint32_t current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if(frame_count == 0)
            {
                current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
            }
            else
            {
                current_regular_period = 6;
            }
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= current_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(frame_count);
                frame_count++;
                if(frame_count >= frame_num)
                {
                    frame_count = 0;
                }
            }
        }
    }
    else if(idx == EN_PCS_11_TX)//单体均衡
    {
        static uint8_t frame_count = 0;
        uint8_t frame_num = (gBMSDataInfo.bmu_num * gBMSDataInfo.per_bmu_cell_num + 47) / 48;
        uint32_t current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
        if(_gCanProtoInfoPCSTx[idx].regular_default_enable == true)
        {
            if(frame_count == 0)
            {
                current_regular_period = _gCanProtoInfoPCSTx[idx].tx_regular_period;
            }
            else
            {
                current_regular_period = 6;
            }
            if((uint32_t)(tim_get_systime() - regular_time_count[idx]) >= current_regular_period)
            {
                regular_time_count[idx] = tim_get_systime();
                _gCanProtoInfoPCSTx[idx].data_prep(frame_count);
                frame_count++;
                if(frame_count >= frame_num)
                {
                    frame_count = 0;
                }
            }
        }
    }
//    else if(idx == EN_PCS_12_TX)//单体MOS，预留不用
//    {
//        
//    }
    
    
}







