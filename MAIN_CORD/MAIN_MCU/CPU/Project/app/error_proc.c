#include "error_proc.h"
#include <string.h>


SysError_t          gstSysErrorDat;
SystemErrAlarmId_t  gstErrAlarmId;
EN_ERR_REPORT_ID    geErrIdReport;
uint8_t gErrIdA = 0;
uint8_t gErrIdB = 0;
static SysWarningModuleDispMap_t _gSysWarningModuleDispMap;

SysWarningModuleDispMap_t *GetErrorID(void)
{
    static uint16_t index = 0;//轮询故障索引
    uint16_t i = 0;
    uint16_t error_classify_index;//故障对应模块索引
    uint8_t error_classify_sub_offset;//0~63 
    uint16_t *p_byte[5 + 2 * MAX_PACK_NUM];//对应模块故障信息指针
    uint16_t  code_region_index[5 + 2 * MAX_PACK_NUM];//故障码起始区域位置，详见协议
    EN_WARING_TYPE error_type[5 + 2 * MAX_PACK_NUM];//各模块故障码类型
    uint16_t offset = 0;

    uint16_t total_num = sizeof(CollectPcsError_t) * 8 + sizeof(CollectPcsAlarm_t) * 8 + sizeof(CollectTotalBmsError_t) * 8 + sizeof(CollectTotalBmsAlarm_t) * 8 + 
                         sizeof(CollectMonitorError_t) * 8 + (sizeof(CollectSingleBmsError_t) * 8 + sizeof(CollectSingleBmsAlarm_t) * 8) * 
                         gstCollectInfo.maincard_data.total_pack_info.pack_num;
    
    if(gstCollectInfo.maincard_data.total_pack_info.pack_num > MAX_PACK_NUM)                     
    {
        _gSysWarningModuleDispMap.serial_num = 0xFFFF;
        _gSysWarningModuleDispMap.type = EN_WARING_TYPE_NO;
        return &_gSysWarningModuleDispMap;
    }
    p_byte[offset] = gstCollectInfo.power_data.model_info.dsp_error.byte;
    error_type[offset] = EN_WARING_TYPE_ERR;
    code_region_index[offset] = 0;
    offset++;
    p_byte[offset] = gstCollectInfo.power_data.model_info.dsp_alarm.byte;   
    error_type[offset] = EN_WARING_TYPE_ALARM;    
    code_region_index[offset] = 0;
    offset++;
    p_byte[offset] = gstCollectInfo.maincard_data.total_pack_info.bms_error.byte;
    error_type[offset] = EN_WARING_TYPE_ERR;  
    code_region_index[offset] = 100; 
    offset++;    
    p_byte[offset] = gstCollectInfo.maincard_data.total_pack_info.bms_alarm.byte; 
    error_type[offset] = EN_WARING_TYPE_ALARM;
    code_region_index[offset] = 100; 
    offset++;
    p_byte[offset] = gstCollectInfo.maincard_data.sys_info.monitor_error.byte;
    error_type[offset] = EN_WARING_TYPE_ERR;
    code_region_index[offset] = 200; 
    offset++;
    for(uint16_t k = 0; k < gstCollectInfo.maincard_data.total_pack_info.pack_num; k++)
    {
        p_byte[offset] = gstCollectInfo.pack_data[k].state_info.pack_error.byte;
        error_type[offset] = EN_WARING_TYPE_ERR;
        code_region_index[offset] = 1000 + k * 100; 
        offset++;
        p_byte[offset] = gstCollectInfo.pack_data[k].state_info.pack_alarm.byte;
        error_type[offset] = EN_WARING_TYPE_ALARM;
        code_region_index[offset] = 1000 + k * 100; 
        offset++;
    }
    /*轮询整个故障信息，如发现，则记录并跳出，下次接着轮询，如一圈都没有发现，则返回无故障信息*/   
    for(i = 0; i < total_num; i++)
    {
        error_classify_index = index >> 6;// index/64  
        error_classify_sub_offset = index % 64;
        if((p_byte[error_classify_index][error_classify_sub_offset / 16] & (1 << (error_classify_sub_offset % 16))) != 0)
        {
            _gSysWarningModuleDispMap.serial_num = code_region_index[error_classify_index] + 20 * (error_classify_sub_offset / 16) + (error_classify_sub_offset % 16);
            _gSysWarningModuleDispMap.type = error_type[error_classify_index];
            index++;
            if(index >= total_num)
            {
                index = 0;
            }
            break;
        }
        else
        {
            index++;
            if(index >= total_num)
            {
                index = 0;
            }
        }
    }
    if(i >= total_num ||                                                         //轮询一圈未发现故障 
      ((gstPackSlavePara.pack_para[0].bms_info.ctrl_info.shutdown & 0X0F )!= 0)) //或者处于关机状态
    {
        _gSysWarningModuleDispMap.serial_num = 0xFFFF;
        _gSysWarningModuleDispMap.type = EN_WARING_TYPE_NO;
    }
    return &_gSysWarningModuleDispMap;
}



/**
  * @brief  DSP故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysDspErrorGet()
{
    //------------DSP故障------------
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_ov_err                    = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_VOLT_HIGH_MASK ) != 0);                                                                              
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_uv_err                    = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_VOLT_LOW_MASK  ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_of_err                    = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_FREQ_HIGH_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_uf_err                    = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_FREQ_LOW_MASK  ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_phase_loss_err            = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_PHASE_LOSS_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.grid_unbalance_err             = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error & DSP_GRID_ERR_UNBALANCE_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.over_load_err                  = ((gstUdpPcsSlavePara.pcs_info.error_info.load_error & DSP_LOAD_ERR_OVER_LOAD_1_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_output_err                 = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error  & DSP_INV_OUTPUT_FAULT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_ov_err                     = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error & DSP_INV_ERR_VOLT_HIGH_MASK   ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_uv_err                     = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error & DSP_INV_ERR_VOLT_LOW_MASK    ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_short_err                  = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error & DSP_INV_ERR_OUTPUT_SHORT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bus_volt_high_err              = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_VOLT_HIGH_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bus_volt_low_err               = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_VOLT_LOW_MASK  ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bus_short_err                  = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_SHORT_MASK     ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.wakeup_out_time_err            = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_WAKEUP_OUT_TIME_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.other_error_err                = 0;

    gstCollectInfo.power_data.model_info.dsp_error.b.bat_ov_err                     = 0;
    gstCollectInfo.power_data.model_info.dsp_error.b.bat_uv_err                     = 0;
    gstCollectInfo.power_data.model_info.dsp_error.b.bat_link_err_err               = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_CONNECT_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bat_chg_oc_err                 = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_CHG_OC_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bat_dsg_oc_err                 = ((gstUdpPcsSlavePara.pcs_info.error_info.bus_error & DSP_BUS_ERR_DSG_OC_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_ov_err                      = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV1_OV_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV2_OV_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV3_OV_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV4_OV_MASK )) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_ins_err                     = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV1_INVERSE_MASK)  || 
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV2_INVERSE_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV3_INVERSE_MASK)  || 
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV4_INVERSE_MASK)) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_short_err                   = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV1_SHORT_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV2_SHORT_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV3_SHORT_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV4_SHORT_MASK)) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_out_ctrl_err                = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV1_OUT_CONTROL_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV2_OUT_CONTROL_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV3_OUT_CONTROL_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV4_OUT_CONTROL_MASK)) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_ot_err                      = ((gstUdpPcsSlavePara.pcs_info.error_info.temp_error & DSP_TEMP_ERR_PV_OT_MASK ) != 0);
                                                                                                                                                                                                  
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_ot_err                     = ((gstUdpPcsSlavePara.pcs_info.error_info.temp_error & DSP_TEMP_ERR_INV_OT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.environment_ot_err             = ((gstUdpPcsSlavePara.pcs_info.error_info.temp_error & DSP_TEMP_ERR_ENVIORONMENT_OT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.environment_ut_err             = ((gstUdpPcsSlavePara.pcs_info.error_info.temp_error & DSP_TEMP_ERR_ENVIORONMENT_UT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.insulation_impedance_low_err   = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV1_INSULATION_LOW_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV2_INSULATION_LOW_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV3_INSULATION_LOW_MASK ) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV4_INSULATION_LOW_MASK )) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.leakage_curr_high_err          = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_LEAK_CURR_HIGH_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.bypass_err                     = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_BYPASS_ERR_MASK) != 0);
   
    gstCollectInfo.power_data.model_info.dsp_error.b.master_to_slave_comm_err       = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_MASTER_TO_SLAVE_COM_ERR_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.parallel_comm_err              = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_PARALLEL_COMM_ERR_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.parallel_slave_err             = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_PARALLEL_SLAVE_ERR_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.high_dc_component_inv_volt     = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error   & DSP_VOLT_HIGH_DC_COMPONENT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.high_dc_component_inv_curr     = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_error  & DSP_CURR_HIGH_DC_COMPONENT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.generator_phase_loss_err       = 0;
    gstCollectInfo.power_data.model_info.dsp_error.b.ground_fault_err               = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_GROUND_FAULT_ERR_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.load_imbalance_err             = 0;
    gstCollectInfo.power_data.model_info.dsp_error.b.pv_overcurrent                 = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV1_OC_MASK)  || 
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV2_OC_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV3_OC_MASK)  ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[0] & DSP_PV0_ERR_PV4_OC_MASK)) != 0);
    gstCollectInfo.power_data.model_info.dsp_error.b.inv_overcurrent                = ((gstUdpPcsSlavePara.pcs_info.error_info.inv_error & DSP_INV_ERR_OC_MASK) != 0);                                                                                   

    gstCollectInfo.power_data.model_info.dsp_error.b.hard_cmpss_oc_err              = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_CMPSS_MASK) != 0); 
    gstCollectInfo.power_data.model_info.dsp_error.b.hard_arc_err                   = (((gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV1_ARC_MASK) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV2_ARC_MASK) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV3_ARC_MASK) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.pv_error[1] & DSP_PV1_ERR_PV4_ARC_MASK)) != 0); 
    gstCollectInfo.power_data.model_info.dsp_error.b.hard_relay_err                 = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_RELAY_SHORT_MASK) != 0); 
    gstCollectInfo.power_data.model_info.dsp_error.b.hard_sample_sensor_err         = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_SAMPLING_SENSOR_ERR_MASK) != 0); 
    gstCollectInfo.power_data.model_info.dsp_error.b.hard_temp_sensor_err           = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_TEMP_SENSOR_ERR_MASK) != 0); 
    
    //------------DSP警告------------                                                                    
    gstCollectInfo.power_data.model_info.dsp_alarm.b.grid_volt_high_alarm           = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_alarm & DSP_GRID_ALARM_VOLT_HIGH_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.grid_volt_low_alarm            = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_alarm & DSP_GRID_ALARM_VOLT_LOW_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.grid_freq_high_alarm           = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_alarm & DSP_GRID_ALARM_FREQ_HIGH_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.grid_freq_low_alarm            = ((gstUdpPcsSlavePara.pcs_info.error_info.grid_alarm & DSP_GRID_ALARM_FREQ_LOW_MASK ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.load_high_alarm                = ((gstUdpPcsSlavePara.pcs_info.error_info.load_alarm & DSP_LOAD_ALARM_OVER_LOAD_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.leakage_curr_high_alarm        = 0;
    gstCollectInfo.power_data.model_info.dsp_alarm.b.bat_volt_high_alarm            = 0;
    gstCollectInfo.power_data.model_info.dsp_alarm.b.bat_volt_low_alarm             = 0;
    gstCollectInfo.power_data.model_info.dsp_alarm.b.inv_temp_high_alarm            = ((gstUdpPcsSlavePara.pcs_info.error_info.temp_alarm & DSP_TEMP_ALARM_INV_OT_MASK)!= 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.inv_temp_low_alarm             = 0;   
    gstCollectInfo.power_data.model_info.dsp_alarm.b.inv_curr_high_alarm            = 0;

    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_fan_alarm                 = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_FAN_MASK         ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_r_open_alarm              = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_R_CT_OPEN_MASK   ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_s_open_alarm              = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_S_CT_OPEN_MASK   ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_t_open_alarm              = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_T_CT_OPEN_MASK   ) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_r_inverse_alarm           = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_R_CT_INVERSE_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_s_inverse_alarm           = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_S_CT_INVERSE_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_alarm.b.hard_t_inverse_alarm           = ((gstUdpPcsSlavePara.pcs_info.error_info.other_alarm & DSP_OTHER_ALARM_T_CT_INVERSE_MASK) != 0);
    
    //------------DSP警通知------------
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_1_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV1_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_2_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV2_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_3_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV3_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_4_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV4_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_5_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV5_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_6_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV6_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_7_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV7_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.pv_8_loss_notice              = ((gstUdpPcsSlavePara.pcs_info.error_info.pv_notice[0] & DSP_PV_NOTICE_PV8_NO_CONNECT_MASK) != 0);
    gstCollectInfo.power_data.model_info.dsp_notice.b.cut_to_of_grid_notice         = 0;
    gstCollectInfo.power_data.model_info.dsp_notice.b.upgrade_pack_notice           = (((gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_SEND_PCS_ERR) ||
                                                                                        (gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_PCS_FLAG_ERR) ||
                                                                                        (gstUdpPcsSlavePara.pcs_info.error_info.other_notice & DSP_UPG_NOTICE_DSP_FILE_CHECK_FAULT_MASK)) != 0);

}


/**
  * @brief  总BMS故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysTotalBmsErrorGet()
{
    uint16_t pack_max_v = 0;
    uint16_t pack_min_v = 3000;  //实际是30.00V
    
    CollectTotalBmsError_t  error_temp;
    CollectTotalBmsAlarm_t  alarm_temp;
    CollectTotalBmsNotice_t notice_temp;
    
    memset(&error_temp,  0, sizeof(CollectTotalBmsError_t));
    memset(&alarm_temp,  0, sizeof(CollectTotalBmsAlarm_t));
    memset(&notice_temp, 0, sizeof(CollectTotalBmsNotice_t));
    
    for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        pack_max_v = (pack_max_v > gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol) ? 
                      pack_max_v : gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol;
        
        pack_min_v = (pack_min_v < gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol) ? 
                      pack_min_v : gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol;
    }
    
    //------------总包故障------------
    for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        error_temp.b.bat_ov_1_err           &= gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ov_1_err;
//        error_temp.b.bat_ov_2_err           &= 
        error_temp.b.bat_uv_1_err           &= gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_uv_1_err;
//        error_temp.b.bat_uv_2_err           &= 
//        error_temp.b.p_ov_1_err             &= 
//        error_temp.b.p_ov_2_err             &= 
//        error_temp.b.p_uv_1_err             &= 
//        error_temp.b.p_uv_2_err             &= 
        error_temp.b.chg_oc_1_err           &= gstCollectInfo.pack_data[i].state_info.pack_error.b.chg_oc_1_err;
//        error_temp.b.chg_oc_2_err           &= 
        error_temp.b.dsg_oc_1_err           &= gstCollectInfo.pack_data[i].state_info.pack_error.b.dsg_oc_1_err;
//        error_temp.b.dsg_oc_2_err           &= 
        error_temp.b.soh_low_1_err          &= gstCollectInfo.pack_data[i].state_info.pack_error.b.soh_low_1_err;
//        error_temp.b.soh_low_2_err          &= 
        
//        error_temp.b.pre_dsg_fault_err      &= 
        error_temp.b.mos_ot_1_err           &= gstCollectInfo.pack_data[i].state_info.pack_error.b.mos_ot_1_err;
//        error_temp.b.mos_ot_2_err           &= 

       error_temp.b.pack_run_err &= (gstCollectInfo.pack_data[i].state_info.work_status != 1);

    }
    
    if((pack_max_v - pack_min_v > 100) && (gstCollectInfo.maincard_data.total_pack_info.pack_num > 1))
    {
        error_temp.b.pack_v_diff_err = 1;
    }
    
    //编址程序已经执行完，并且编址失败，设置故障位
    if((BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok == 0)&&(BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over ==1))
    {
        error_temp.b.addr_set_err = 1;
    }
    
    //------------总包警告------------
    for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        alarm_temp.b.bat_volt_high_alarm       &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_volt_high_alarm;
        alarm_temp.b.bat_volt_low_alarm        &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_volt_low_alarm;
//        alarm_temp.b.p_volt_high_alarm         &= 
//        alarm_temp.b.p_volt_low_alarm          &= 
        alarm_temp.b.chg_curr_high_alarm       &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.chg_curr_high_alarm;
        alarm_temp.b.dsg_curr_high_alarm       &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.dsg_curr_high_alarm;
        alarm_temp.b.soh_alarm                 &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.soh_alarm;
        alarm_temp.b.mos_temp_high_alarm       &= gstCollectInfo.pack_data[i].state_info.pack_alarm.b.mos_temp_high_alarm;
        alarm_temp.b.pack_version_diff_alarm   &= (gstCollectInfo.pack_data[i].base_info.sw_ver != gstCollectInfo.pack_data[0].base_info.sw_ver);
    }
    
    if((pack_max_v - pack_min_v > 80) && (error_temp.b.pack_v_diff_err == 0))
    {
        alarm_temp.b.pack_v_diff_alarm  = 1;
    }
    
    //------------总包通知------------
    notice_temp.b.upgrade_pack_notice = (((gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_SEND_BMS_ERR) || 
                                          (gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_BMS_FLAG_ERR)) != 0);
    
    //赋值
//    memcpy(&gstCollectInfo.maincard_data.total_pack_info.bms_error, &error_temp,  sizeof(CollectTotalBmsError_t));
//    memcpy(&gstCollectInfo.maincard_data.total_pack_info.bms_alarm, &alarm_temp,  sizeof(CollectTotalBmsAlarm_t));
//    memcpy(&gstCollectInfo.maincard_data.total_pack_info.bms_notice, &notice_temp, sizeof(CollectTotalBmsNotice_t));
}

/**
  * @brief  监控卡故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysMonitorErrGet()
{

    gstCollectInfo.maincard_data.sys_info.monitor_error.b.pcs_com_err = gstUdpPcsSlavePara.pcs_comm_time_out;
#if(TEST_MODE != 2)
    gstCollectInfo.maincard_data.sys_info.monitor_error.b.bms_com_err = gstPackSlavePara.bms_com_time_out[0];
#endif
    gstCollectInfo.maincard_data.sys_info.monitor_error.b.arc_com_err = ((gstUdpPcsSlavePara.pcs_info.error_info.other_error & DSP_OTHER_ERR_WITH_ARC_COMM_ERR_MASK) != 0);
    gstCollectInfo.maincard_data.sys_info.monitor_notice.b.upgrade_pack_notice = ((gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_MAIN_CHECK_ERR) != 0);

}

/**
  * @brief  单BMS故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysSingleBmsErrorGet()
{
    for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        //------------Pack故障------------
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ov_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_OV1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ov_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_OV2_MASK) != 0);                                                                
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_uv_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_UV1_MASK) != 0);                                                                
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_uv_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_UV2_MASK) != 0);                                                                       
        gstCollectInfo.pack_data[i].state_info.pack_error.b.pack_ov_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_PACK_OV1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.pack_ov_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_PACK_OV2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.pack_uv_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_PACK_UV1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.pack_uv_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_PACK_UV2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.chg_oc_1_err          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_CHG_OC1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.chg_oc_2_err          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_CHG_OC2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.dsg_oc_1_err          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_DSG_OC1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.dsg_oc_2_err          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_DSG_OC2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.soh_low_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.sox_error & BMS_SOX_ERR_SOH_LOW1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.soh_low_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.sox_error & BMS_SOX_ERR_SOH_LOW2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_volt_diff_err    = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_VOLT_DIFF_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.pre_dsg_fault_err     = 0;
        gstCollectInfo.pack_data[i].state_info.pack_error.b.mos_ot_1_err          = 0;
        gstCollectInfo.pack_data[i].state_info.pack_error.b.mos_ot_2_err          = 0;
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ot_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_OT1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ot_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_OT2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ut_1_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_UT1_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_ut_2_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_UT2_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.polar_column_ot_1_err = 0;
        gstCollectInfo.pack_data[i].state_info.pack_error.b.polar_column_ot_2_err = 0;
        gstCollectInfo.pack_data[i].state_info.pack_error.b.cell_temp_diff_err    = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_TEMP_DIFF_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.other_err             = 0;
        
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_flash_err           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_FLASH_MASK  ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_eeprom_err          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_EEPROM_MASK ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_afe_err             = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_AFE_MASK    ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_rtc_err             = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_RTC_MASK    ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_balance_err         = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_BALANCE_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_heat_err            = ((gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_HEAT_MASK   ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_volt_sensor_err     = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error   & BMS_VOLT_ERR_SENSOR_MASK    ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_curr_sensor_err     = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error  & BMS_CURR_ERR_SENSOR_MASK   ) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_cell_ntc_sensor_err = (((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_NTC_ERR_MASK) || 
                                                                                         (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error & BMS_TEMP_ERR_CELL_NTC_2_ERR_MASK)) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_mos_ntc_sensor_err  = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error  & BMS_TEMP_ERR_MOS_NTC_ERR_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_mos_adhesion_err    = ((gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error   & BMS_MOS_ERR_ADHESION_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_mos_close_err       = ((gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error  & BMS_MOS_ERR_CLOSE_ERR_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_error.b.hard_sample_disconn_err  = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_SAMPLE_ERR_MASK) != 0);

        //------------Pack警告------------
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_volt_high_alarm          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_alarm & BMS_VOLT_ALARM_CELL_OV_MASK) != 0); 
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_volt_low_alarm           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_alarm & BMS_VOLT_ALARM_CELL_UV_MASK) != 0); 
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.pack_volt_high_alarm          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_alarm & BMS_VOLT_ALARM_PACK_OV_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.pack_volt_low_alarm           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_alarm & BMS_VOLT_ALARM_PACK_UV_MASK) != 0); 
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.chg_curr_high_alarm           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_alarm & BMS_CURR_ALARM_CHG_OC_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.dsg_curr_high_alarm           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.curr_alarm & BMS_CURR_ALARM_DSG_OC_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.soh_alarm                     = ((gstPackSlavePara.pack_para[i].bms_info.error_info.sox_alarm  & BMS_SOX_ALARM_SOH_LOW_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_v_diff_alarm             = ((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_alarm & BMS_VOLT_ALARM_CELL_V_DIFF_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.mos_temp_high_alarm           = 0;
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_temp_high_alarm          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_alarm & BMS_TEMP_ALARM_CELL_OT_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_temp_low_alarm           = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_alarm & BMS_TEMP_ALARM_CELL_UT_MASK) != 0);
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.polar_column_temp_high_alarm  = 0;
        gstCollectInfo.pack_data[i].state_info.pack_alarm.b.cell_temp_diff_alarm          = ((gstPackSlavePara.pack_para[i].bms_info.error_info.temp_alarm & BMS_TEMP_ALARM_CELL_TEMP_DIFF_MASK) != 0);
    
        //------------Pack通知------------
        gstCollectInfo.pack_data[i].state_info.pack_notice.b.upgrade_pack_notice = ((gstPackSlavePara.pack_para[i].bms_info.ctrl_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) == 2);
    
    }
}

/**
  * @brief  拉弧故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysArcErrorGet()
{
    
}

/**
  * @brief  DCDC故障获取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SysDCDCErrorGet()
{
    
}





/**
  * @brief  故障获取函数
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SysErrorProcess(void)
{
    _SysDspErrorGet();
    _SysTotalBmsErrorGet();
    _SysMonitorErrGet();
    _SysSingleBmsErrorGet();
    _SysArcErrorGet();
    _SysDCDCErrorGet();
    
}

/************************************----------**********************************************/


bool GetSysAlarm(void)
{
    uint8_t i = 0;
    bool eflag = false;
    if(gstSysErrorDat.pcs_error.pv_alarm[0]    != 0 ||
       gstSysErrorDat.pcs_error.grid_alarm  != 0 ||
       gstSysErrorDat.pcs_error.bus_alarm   != 0 ||
       gstSysErrorDat.pcs_error.inv_alarm   != 0 ||
       gstSysErrorDat.pcs_error.load_alarm  != 0 ||
       gstSysErrorDat.pcs_error.temp_alarm  != 0 ||
       gstSysErrorDat.pcs_error.other_alarm != 0
    )
    {
        eflag = true;

    }
    return eflag;
}

void SetErrAlarmIdBit(uint32_t *ps_data, uint32_t err_alarm_mask, uint32_t *pdata, uint32_t  err_alarm_id_mask)
{

    if(*ps_data & err_alarm_mask)
    {
        *pdata |= err_alarm_id_mask;
    }
}

bool GetCloseChgDsgFun(void)
{
    bool        eflag = false;
    uint8_t     i = 0;
    uint32_t    data_test = 0;
    for(i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        if(
            (gstPackSlavePara.pack_para[i].bms_info.error_info.sox_error & BMS_SOX_ERR_SOH_LOW1_MASK          ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.sox_error & BMS_SOX_ERR_SOH_LOW2_MASK          ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_CELL_VOLT_DIFF_MASK   ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_OT1_MASK       ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_OT2_MASK       ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_UT1_MASK       ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_UT2_MASK       ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_TEMP_DIFF_MASK ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_FLASH_MASK         ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_EEPROM_MASK        ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_AFE_MASK           ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_RTC_MASK           ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_BALANCE_MASK       ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.other_error & BMS_OTHER_ERR_HEAT_MASK          ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error   & BMS_VOLT_ERR_SENSOR_MASK         ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error  & BMS_CURR_ERR_SENSOR_MASK         ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_NTC_ERR_MASK   ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_CELL_NTC_2_ERR_MASK ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.temp_error &  BMS_TEMP_ERR_MOS_NTC_ERR_MASK    ) ||
            (gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error &   BMS_MOS_ERR_ADHESION_MASK        ) || 
            (gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error &   BMS_MOS_ERR_CLOSE_ERR_MASK       ) 
//            (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error & BMS_VOLT_ERR_SAMPLE_ERR_MASK       ) ||
//            (gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error & BMS_MOS_ERR_CHG_SHORT_MASK         ) || 
//            (gstPackSlavePara.pack_para[i].bms_info.error_info.mos_error & BMS_MOS_ERR_DSG_SHORT_MASK         ) || 
           ) 
        {
            eflag = true;
            break;
        }
    }
    
    if(gstCollectInfo.maincard_data.sys_info.monitor_error.b.bms_com_err ||
       gstCollectInfo.maincard_data.sys_info.monitor_error.b.pcs_com_err ||
       gstCollectInfo.maincard_data.sys_info.monitor_error.b.hard_err
      )
    {
            eflag = true;
    }
    //DSP除PV问题外，全部停止充放电
    if(gstCollectInfo.power_data.model_info.dsp_error.byte[0] ||
       (gstCollectInfo.power_data.model_info.dsp_error.byte[1] & 0XFF1F) ||
       (gstCollectInfo.power_data.model_info.dsp_error.byte[2] & 0XBFFF) ||
       gstCollectInfo.power_data.model_info.dsp_error.byte[3]
      )
    {
            eflag = true;
    }
    
    return eflag;
}
bool GetCloseChgFun(void)
{
    bool    eflag = false;
    uint8_t i = 0;
    for(i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        if((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_CELL_OV1_MASK) != 0  ||      
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_CELL_OV2_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_PACK_OV1_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_PACK_OV2_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_CHG_OC1_MASK)  != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_CHG_OC2_MASK)  != 0  
          )
        {
            eflag = true;     //?????充电停止故障
            break;
        }
    }
    return eflag;
}

bool GetCloseDsgFun(void)
{
    bool    eflag = false;
    uint8_t i = 0;
    for(i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        if((gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_CELL_UV1_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_CELL_UV2_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_PACK_UV1_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.vol_error  & BMS_VOLT_ERR_PACK_UV2_MASK) != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_DSG_OC1_MASK)  != 0  ||
           (gstPackSlavePara.pack_para[i].bms_info.error_info.curr_error & BMS_CURR_ERR_DSG_OC2_MASK)  != 0
          )
        {
            eflag = true;
            break;
        }
    }
    return eflag;
}

bool GetAlarmNoCloseFun()
{
    bool    eflag = false;
    uint8_t i = 0;
//    for(i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
//    {
//        if(     ((gstSysErrorDat.bms_error[i].curr_error_code & BMS_CHG_OC_1LEVEL_ALARM_MASK) != 0)                 ||  \       /////?????   警告修改
//                ((gstSysErrorDat.bms_error[i].curr_error_code & BMS_SOC_JUMP_ERR_ID_MASK) != 0)                     ||  \
//                ((gstSysErrorDat.bms_error[i].state_error_code & BMS_OT_1LEVEL_ALARM_MASK) != 0)                        \
//          )
//        {
//            eflag = true;
//            break;
//        }
//    }
//    if(     ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_SOFT_STARTUP_OT_ERR_MASK) != 0)                 ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_OUTPUT_SHORT_ERR_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BUS_VOLT_LOW_ERR_MASK) != 0)                    ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BUS_VOLT_HIGH_ERR_MASK) != 0)                   ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BUS_SHORT_ERR_MASK) != 0)                       ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_HARDWARE_FAULT_ERR_MASK) != 0)                  ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_OC_ERR_MASK) != 0)                          ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_DC_COMPONENT_ERR_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BOX_OTP_ERR_MASK) != 0)                         ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_RADIATOR_OTP_ERR_MASK) != 0)                    ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_CMPSS_OC_ERR_MASK) != 0)                        ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BATT_DSG_OC_ERR_MASK) != 0)                     ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_BATT_CHG_OC_ERR_MASK) != 0)                     ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_OUTPUT_FAULT_ERR_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_OVERLOAD1_ERR_MASK) != 0)                       ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_OVERLOAD2_ERR_MASK) != 0)                       ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_UVP_ERR_MASK) != 0)                         ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_INV_OVP_ERR_MASK) != 0)                         ||  \
//            ((gstSysErrorDat.pcs_error.dsp_error_code & DSP_COMM_ERR_MASK) != 0)                            ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_GRID_VOLT_LOW_ALARM_MASK) != 0)                 ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_GRID_VOLT_HIGH_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_GRID_FREQ_LOW_ALARM_MASK) != 0)                 ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_GRID_FREQ_HIGH_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_PV1_NO_CONNECT_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_PV_OUT_OF_CTRL_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_OVERLOAD_ALARM_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_LEAKAGE_CUR_HIGH_ALARM_MASK) != 0)              ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_FAN_ALARM_MASK) != 0)                           ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_R_CT_REVERSE_ALARM_MASK) != 0)                  ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_S_CT_REVERSE_ALARM_MASK) != 0)                  ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_T_CT_REVERSE_ALARM_MASK) != 0)                  ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_PV2_NO_CONNECT_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_PV3_NO_CONNECT_ALARM_MASK) != 0)                ||  \
//            ((gstSysErrorDat.pcs_error.dsp_alarm_code & DSP_PV4_NO_CONNECT_ALARM_MASK) != 0))
//    {
//        eflag = true;
//    }


//    if(gstFirmwareUpgrateInfo.upgrad_err_code != 0)
//    {
//        eflag = true;
//    }

    return eflag;
}

#define  TEST_ERR_DISPLAY   0

#if(TEST_ERR_DISPLAY)
uint16_t        mos_error_code_test = 0;
uint16_t        mos_error_code_test1 = 0;
uint16_t        volt_error_code_test = 0;
uint16_t        volt_error2_code_test = 0;
uint16_t        curr_error_code_test = 0;
uint16_t        temp_error_code_test = 0;
uint16_t        state_error_code_test = 0;
uint16_t        other_error_code_test = 0;
uint32_t        dsp_error_code_test = 0;
uint32_t        dsp_alarm_code_test = 0;
uint16_t        comm_time_out_test = 0;

uint16_t        arc_alarm_test;
uint16_t        inpedance_breakdown_info_test;

#endif
EN_ERR_WROK_ACTION GetSysError(void)
{
    uint8_t i = 0;
    bool eCloseDsgChgflag ;
    bool eCloseDsgflag ;
    bool eCloseChgflag ;
    bool eNoActionAlarm;
    EN_ERR_WROK_ACTION e_err_work_action;
    e_err_work_action = EN_NO_ERR_ALARM;

#if(TEST_ERR_DISPLAY)
    gstCollectInfo.maincard_data.total_pack_info.pack_num = 2;
    dsp_error_code_test = 0X4040000;

    gstSysErrorDat.bms_error[0].mos_error_code      = mos_error_code_test;
    gstSysErrorDat.bms_error[1].mos_error_code      = mos_error_code_test1;
    gstSysErrorDat.bms_error[0].volt_error_code     = volt_error_code_test;
//    gstSysErrorDat.bms_error[0].volt_error2_code    = volt_error2_code_test;
    gstSysErrorDat.bms_error[0].curr_error_code     = curr_error_code_test;
    gstSysErrorDat.bms_error[0].temp_error_code     =  temp_error_code_test;
    gstSysErrorDat.bms_error[0].state_error_code    = state_error_code_test;
    gstSysErrorDat.bms_error[0].other_error_code    = other_error_code_test;
    gstSysErrorDat.pcs_error.dsp_error_code         = dsp_error_code_test;
    gstSysErrorDat.pcs_error.dsp_alarm_code         = dsp_alarm_code_test;
    gstSysErrorDat.comm_time_out.byte_value         =  comm_time_out_test;
    gstSysErrorDat.arc_alarm = arc_alarm_test;
    gstSysErrorDat.inpedance_breakdown_info = inpedance_breakdown_info_test;


#endif

    eCloseDsgChgflag = GetCloseChgDsgFun();
    eCloseDsgflag = GetCloseDsgFun();
    eCloseChgflag = GetCloseChgFun();
    eNoActionAlarm = GetAlarmNoCloseFun();
    if(eCloseDsgChgflag == true)
    {
        //关闭充放电MOS
        e_err_work_action = EN_ERR_CLOSE_CHG_DSG;
    }
    else
    {
        if((eCloseDsgflag == true) && (eCloseChgflag == true))
        {
            e_err_work_action = EN_ERR_CLOSE_CHG_DSG;
        }
        else
        {
            if(eCloseDsgflag == true)
            {
                e_err_work_action = EN_ERR_CLOSE_DSG;
            }
            else  if(eCloseChgflag == true)
            {
                e_err_work_action = EN_ERR_CLOSE_CHG;
            }
            else
            {
                if(eNoActionAlarm == true)
                {
                    //仅仅警告 无动作
                    e_err_work_action = EN_ONLY_ALARM;
                }
                else
                {
                    gstErrAlarmId.sys_err_alarm_id = 0;
                    //无警告 。
                }

            }

        }

    }
    if(e_err_work_action != EN_NO_ERR_ALARM)
    {
        //说明此时有错误或者警告

        static uint32_t time_out = 0;

        if((uint32_t)(tim_get_systime() - time_out) < 1000)
        {
            return e_err_work_action;
        }
        time_out = tim_get_systime();

//every 1S
        memset((uint8_t *)&gstErrAlarmId, 0, sizeof(SystemErrAlarmId_t));   //?????  故障修改
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_SOFT_STARTUP_OT_ERR_MASK,    &gstErrAlarmId.dsp_err[0], DSP_SOFT_STARTUP_OT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_OUTPUT_SHORT_ERR_MASK,   &gstErrAlarmId.dsp_err[0], DSP_INV_OUTPUT_SHORT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BUS_VOLT_LOW_ERR_MASK,       &gstErrAlarmId.dsp_err[0], DSP_BUS_VOLT_LOW_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BUS_VOLT_HIGH_ERR_MASK,      &gstErrAlarmId.dsp_err[0], DSP_BUS_VOLT_HIGH_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BUS_SHORT_ERR_MASK,          &gstErrAlarmId.dsp_err[0], DSP_BUS_SHORT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_HARDWARE_FAULT_ERR_MASK,     &gstErrAlarmId.dsp_err[0], DSP_HARDWARE_FAULT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_OC_ERR_MASK,             &gstErrAlarmId.dsp_err[0], DSP_INV_OC_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_DC_COMPONENT_ERR_MASK,   &gstErrAlarmId.dsp_err[0], DSP_INV_DC_COMPONENT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BOX_OTP_ERR_MASK,            &gstErrAlarmId.dsp_err[0], DSP_BOX_OTP_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_RADIATOR_OTP_ERR_MASK,       &gstErrAlarmId.dsp_err[0], DSP_RADIATOR_OTP_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_CMPSS_OC_ERR_MASK,           &gstErrAlarmId.dsp_err[0], DSP_CMPSS_OC_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BATT_DSG_OC_ERR_MASK,        &gstErrAlarmId.dsp_err[0], DSP_BATT_DSG_OC_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_BATT_CHG_OC_ERR_MASK,        &gstErrAlarmId.dsp_err[0], DSP_BATT_CHG_OC_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_OUTPUT_FAULT_ERR_MASK,   &gstErrAlarmId.dsp_err[0], DSP_INV_OUTPUT_FAULT_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_OVERLOAD1_ERR_MASK,          &gstErrAlarmId.dsp_err[0], DSP_OVERLOAD1_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_OVERLOAD2_ERR_MASK,          &gstErrAlarmId.dsp_err[0], DSP_OVERLOAD2_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_UVP_ERR_MASK,            &gstErrAlarmId.dsp_err[1], DSP_INV_VOLT_LOW_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INV_OVP_ERR_MASK,            &gstErrAlarmId.dsp_err[1], DSP_INV_OVP_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_R_CT_OPEN_ERR_MASK,            &gstErrAlarmId.dsp_err[1], DSP_R_CT_OPEN_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_S_CT_OPEN_ERR_MASK,            &gstErrAlarmId.dsp_err[1], DSP_S_CT_OPEN_ERR_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_T_CT_OPEN_ERR_MASK,            &gstErrAlarmId.dsp_err[1], DSP_T_CT_OPEN_ERR_ID_MASK);
//        
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_LEAKAGE_CUR_HIGH_MASK,            &gstErrAlarmId.dsp_err[1], DSP_LEAKAGE_CUR_HIGH_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_INSULATION_IMPED_LOW_MASK,        &gstErrAlarmId.dsp_err[1], DSP_INSULATION_IMPED_LOW_ID_MASK);

//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_error_code, DSP_COMM_ERR_MASK,               &gstErrAlarmId.dsp_err[1], DSP_COMM_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.arc_alarm, ARC_CHANNEL1_ELE_ARC_ERR_MASK,       &gstErrAlarmId.arc_err, ARC_CHANNEL1_ELE_ARC_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.arc_alarm, ARC_CHANNEL2_ELE_ARC_ERR_MASK,       &gstErrAlarmId.arc_err, ARC_CHANNEL2_ELE_ARC_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.arc_alarm, ARC_CHANNEL3_ELE_ARC_ERR_MASK,       &gstErrAlarmId.arc_err, ARC_CHANNEL3_ELE_ARC_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.arc_alarm, ARC_CHANNEL4_ELE_ARC_ERR_MASK,       &gstErrAlarmId.arc_err, ARC_CHANNEL4_ELE_ARC_ERR_ID_MASK);

//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.inpedance_breakdown_info, ARC_CHANNEL1_R_LOW_ERR_MASK, &gstErrAlarmId.arc_err, ARC_CHANNEL1_R_LOW_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.inpedance_breakdown_info, ARC_CHANNEL2_R_LOW_ERR_MASK, &gstErrAlarmId.arc_err, ARC_CHANNEL2_R_LOW_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.inpedance_breakdown_info, ARC_CHANNEL3_R_LOW_ERR_MASK, &gstErrAlarmId.arc_err, ARC_CHANNEL3_R_LOW_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.inpedance_breakdown_info, ARC_CHANNEL4_R_LOW_ERR_MASK, &gstErrAlarmId.arc_err, ARC_CHANNEL4_R_LOW_ERR_ID_MASK);


//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_SEND_PCS_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_SEND_PCS_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_SEND_ARC_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_SEND_ARC_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_SEND_BMS_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_SEND_BMS_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_SEND_ADD_BMS_ERR, &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_SEND_ADD_BMS_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_MAIN_CHECK_ERR,   &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_MAIN_CHECK_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_ZIP_ERR,          &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_ZIP_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_CHECK_ERR,        &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_CHECK_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_BMS_FLAG_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_BMS_FLAG_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_PCS_FLAG_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_PCS_FLAG_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_ARC_FLAG_ERR,     &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_ARC_FLAG_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstFirmwareUpgrateInfo.upgrad_err_code, UPGRADE_ADRESS_ERR,       &gstErrAlarmId.monitor_err, MONITOR_UPGRADE_ADRESS_ERR_ID_MASK);

//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.comm_time_out.byte_value, MONITOR_BMS_COMM_ERR_MASK, &gstErrAlarmId.monitor_err, MONITOR_BMS_COMM_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.comm_time_out.byte_value, MONITOR_PCS_COMM_ERR_MASK, &gstErrAlarmId.monitor_err, MONITOR_PCS_COMM_ERR_ID_MASK);
//        SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.comm_time_out.byte_value, MONITOR_ARC_COMM_ERR_MASK, &gstErrAlarmId.monitor_err, MONITOR_ARC_COMM_ERR_ID_MASK);


//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_GRID_VOLT_LOW_ALARM_MASK,    &gstErrAlarmId.dsp_alarm, DSP_GRID_VOLT_LOW_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_GRID_VOLT_HIGH_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_GRID_VOLT_HIGH_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_GRID_FREQ_LOW_ALARM_MASK,    &gstErrAlarmId.dsp_alarm, DSP_GRID_FREQ_LOW_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_GRID_FREQ_HIGH_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_GRID_FREQ_HIGH_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_PV1_NO_CONNECT_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_PV1_NO_CONNECT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_PV_OUT_OF_CTRL_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_PV_OUT_OF_CTRL_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_OVERLOAD_ALARM_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_OVERLOAD_ALARM_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_LEAKAGE_CUR_HIGH_ALARM_MASK, &gstErrAlarmId.dsp_alarm, DSP_LEAKAGE_CUR_HIGH_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_FAN_ALARM_MASK,              &gstErrAlarmId.dsp_alarm, DSP_FAN_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_R_CT_REVERSE_ALARM_MASK,     &gstErrAlarmId.dsp_alarm, DSP_R_CT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_S_CT_REVERSE_ALARM_MASK,     &gstErrAlarmId.dsp_alarm, DSP_S_CT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_T_CT_REVERSE_ALARM_MASK,     &gstErrAlarmId.dsp_alarm, DSP_T_CT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_PV2_NO_CONNECT_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_PV2_NO_CONNECT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_PV3_NO_CONNECT_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_PV3_NO_CONNECT_ALARM_ID_MASK);
//        SetErrAlarmIdBit(&gstSysErrorDat.pcs_error.dsp_alarm_code, DSP_PV4_NO_CONNECT_ALARM_MASK,   &gstErrAlarmId.dsp_alarm, DSP_PV4_NO_CONNECT_ALARM_ID_MASK);



//        for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
//        {
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp_error_code,  BMS_MOS_OT_1LEVEL_ALARM_MASK,       &gstErrAlarmId.bms_alarm, BMS_MOS_OT_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_CELL_UV_1LEVEL_ALARM_MASK,      &gstErrAlarmId.bms_alarm, BMS_CELL_UV_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_CELL_OV_1LEVEL_ALARM_MASK,      &gstErrAlarmId.bms_alarm, BMS_CELL_OV_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].curr_error_code,  BMS_DSG_OC_1LEVEL_ALARM_MASK,       &gstErrAlarmId.bms_alarm, BMS_DSG_OC_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].curr_error_code,  BMS_CHG_OC_1LEVEL_ALARM_MASK,       &gstErrAlarmId.bms_alarm, BMS_CHG_OC_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_UT_1LEVEL_ALARM_MASK,           &gstErrAlarmId.bms_alarm, BMS_UT_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_OT_1LEVEL_ALARM_MASK,           &gstErrAlarmId.bms_alarm, BMS_OT_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_TEMP_DIFF_1LEVEL_ALARM_MASK,    &gstErrAlarmId.bms_alarm, BMS_TEMP_DIFF_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].state_error_code, BMS_SOC_LOW_1LEVEL_ALARM_MASK,      &gstErrAlarmId.bms_alarm, BMS_SOC_LOW_1LEVEL_ALARM_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].state_error_code, BMS_SOC_LOW_2LEVEL_ALARM_MASK,      &gstErrAlarmId.bms_alarm, BMS_SOC_LOW_2LEVEL_ALARM_ID_MASK);
//            
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp_error_code,  BMS_MOS_OT_2AND3_LEVEL_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_MOS_OT_2AND3_LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_CELL_UV_2AND3_LEVEL_ERR_MASK,   &gstErrAlarmId.bms_err[0], BMS_CELL_UV_2AND3_LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_CELL_OV_2AND3_LEVEL_ERR_MASK,   &gstErrAlarmId.bms_err[0], BMS_CELL_OV_2AND3_LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_TOTAL_UV_ERR_MASK,              &gstErrAlarmId.bms_err[0], BMS_TOTAL_UV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_TOTAL_OV_ERR_MASK,              &gstErrAlarmId.bms_err[0], BMS_TOTAL_OV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_CELL_VOLT_DIFF_ERR_MASK,        &gstErrAlarmId.bms_err[0], BMS_CELL_VOLT_DIFF_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].curr_error_code,  BMS_DSG_OC_2AND3_LEVEL_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_DSG_OC_2AND3_LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].curr_error_code,  BMS_CHG_OC_2AND3_LEVEL_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_CHG_OC_2AND3_LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_CELL_OT_2AND3_LEVEL_ERR_MASK,   &gstErrAlarmId.bms_err[0], BMS_CELL_OT_2AND3_LEVEL_ERR_ID_MASK);

//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].health_err_code,  BMS_SOH_US_1AND2_LEVEL_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_SOH_US_1AND2_LEVEL_ID_ERR_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_VOLT_SENSOR_ERR_MASK,           &gstErrAlarmId.bms_err[0], BMS_VOLT_SENSOR_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_TEMP_SENSOR_ERR_MASK,           &gstErrAlarmId.bms_err[0], BMS_TEMP_SENSOR_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].other_error_code, BMS_SPI_FLASH_SENSOR_ERR_MASK,      &gstErrAlarmId.bms_err[0], BMS_SPI_FLASH_SENSOR_ID_ERR_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].other_error_code, BMS_E2PROM_SENSOR_ERR_MASK,         &gstErrAlarmId.bms_err[0], BMS_E2PROM_SENSOR_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].rtc_err_code,     BMS_RTC_SENSOR_ERR_MASK,            &gstErrAlarmId.bms_err[0], BMS_RTC_SENSOR_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_CHG_SHORT_ERR_MASK,             &gstErrAlarmId.bms_err[0], BMS_CHG_SHORT_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_DSG_SHORT_ERR_MASK,             &gstErrAlarmId.bms_err[0], BMS_DSG_SHORT_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_PRE_CHG_OVERTIEM_ERR_MASK,      &gstErrAlarmId.bms_err[0], BMS_PRE_CHG_OVERTIEM_ID_ERR_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp1_error_code, BMS_HEAT_SHORT_ERR_MASK,            &gstErrAlarmId.bms_err[0], BMS_HEAT_SHORT_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_AFE_OPEN_ERR_MASK,              &gstErrAlarmId.bms_err[0], BMS_AFE_OPEN_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_MOS_SHORT_ERR_MASK,             &gstErrAlarmId.bms_err[0], BMS_MOS_SHORT_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_MOS_PRE_CHG_FAIL_CLOSE_ERR_MASK,&gstErrAlarmId.bms_err[0], BMS_MOS_PRE_CHG_FAIL_CLOSE_ID_ERR_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_MOS_CHG_FAIL_CLOSE_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_MOS_CHG_FAIL_CLOSE_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_MOS_DSG_FAIL_CLOSE_ERR_MASK,    &gstErrAlarmId.bms_err[0], BMS_MOS_DSG_FAIL_CLOSE_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].temp_error_code,  BMS_MOS_TEMP_SENSOR_ERR_MASK,       &gstErrAlarmId.bms_err[0], BMS_MOS_TEMP_SENSOR_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_PRE_CHG_CUR_REVERSE_ERR_MASK,   &gstErrAlarmId.bms_err[1], BMS_PRE_CHG_CUR_REVERSE_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].mos_error_code,   BMS_PRE_SHORT_ERR_MASK,             &gstErrAlarmId.bms_err[1], BMS_PRE_SHORT_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].curr_error_code,  BMS_CUR_SENSOR_ZERO_DIFF_MASK,      &gstErrAlarmId.bms_err[1], BMS_CUR_SENSOR_ZERO_DIFF_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].volt_error_code,  BMS_PACK_CELL_VOLT_DIFF_ERR_MASK,   &gstErrAlarmId.bms_err[1], BMS_PACK_CELL_VOLT_DIFF_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].cell_balance_err_code, BMS_BALANCE_ERR_MASK,          &gstErrAlarmId.bms_err[1], BMS_BALANCE_ERR_ID_MASK);
////            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].state_error_code, BMS_SOC_LOW_1LEVEL_ERR_MASK,        &gstErrAlarmId.bms_err[1], BMS_SOC_LOW_1LEVEL_ERR_ID_MASK);
////            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].state_error_code, BMS_SOC_LOW_2LEVEL_ERR_MASK,        &gstErrAlarmId.bms_err[1], BMS_SOC_LOW_2LEVEL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].state_error_code, BMS_SOC_JUMP_ERR_MASK,              &gstErrAlarmId.bms_err[1], BMS_SOC_JUMP_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.bms_error[i].afe_ctrl_err_code, BMS_ATE_COMM_ERR_MASK,             &gstErrAlarmId.bms_err[1], BMS_ATE_COMM_ERR_ID_MASK);
//            
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_INPUT_UV_ERR_MASK,            &gstErrAlarmId.dcdc_err,   DCDC_INPUT_UV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_INPUT_OV_ERR_MASK,            &gstErrAlarmId.dcdc_err,   DCDC_INPUT_OV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_INPUT_OC_ERR_MASK,            &gstErrAlarmId.dcdc_err,   DCDC_INPUT_OC_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_INPUT_OP_ERR_MASK,            &gstErrAlarmId.dcdc_err,   DCDC_INPUT_OP_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_OUTPUT_UV_ERR_MASK,           &gstErrAlarmId.dcdc_err,   DCDC_OUTPUT_UV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_OUTPUT_OV_ERR_MASK,           &gstErrAlarmId.dcdc_err,   DCDC_OUTPUT_OV_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_OUTPUT_OC_ERR_MASK,           &gstErrAlarmId.dcdc_err,   DCDC_OUTPUT_OC_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code1, DCDC_OUTPUT_OP_ERR_MASK,           &gstErrAlarmId.dcdc_err,   DCDC_OUTPUT_OP_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code2, DCDC_POWER_PARALLEL_CONN_FAIL_ERR_MASK, &gstErrAlarmId.dcdc_err, DCDC_POWER_PARALLEL_CONN_FAIL_ERR_ID_MASK);
//            SetErrAlarmIdBit((uint32_t *)&gstSysErrorDat.dcdc_error[i].dcdc_error_code2, DCDC_PRE_CHARGE_FAIL_ERR_MASK,     &gstErrAlarmId.dcdc_err, DCDC_PRE_CHARGE_FAIL_ERR_ID_MASK);
//        }

        switch(geErrIdReport)
        {
            case EN_DSP_ERR_REPORT:
            {
                //DSP ERR ID 有2个32bit 来标识
                for(; gErrIdA < 2; gErrIdA++)
                {
                    for(; gErrIdB < 32; gErrIdB++)
                    {
                        if((gstErrAlarmId.dsp_err[gErrIdA] & (1 << gErrIdB)) != 0)
                        {

                            gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                            gstErrAlarmId.sys_err_alarm_id |= 0x80000000;
                            gstErrAlarmId.sys_err_alarm_id |= 0 << 16;
                            gErrIdB++;
                            return e_err_work_action;
                        }
                    }
                    gErrIdB = 0;

                }

                gErrIdA = 0;
                gErrIdB = 0;



                geErrIdReport =   EN_BMS_ERR_REPORT;
//                break;
            }
            case EN_BMS_ERR_REPORT:
            {

                for(; gErrIdA < 2; gErrIdA++)
                {
                    for(; gErrIdB < 32; gErrIdB++)
                    {
                        if((gstErrAlarmId.bms_err[gErrIdA] & (1 << gErrIdB)) != 0)
                        {

                            gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                            gstErrAlarmId.sys_err_alarm_id |= 0x80000000;
                            gstErrAlarmId.sys_err_alarm_id |= 1 << 16;
                            gErrIdB++;
                            return e_err_work_action;
                        }
                    }
                    gErrIdB = 0;
                }
                gErrIdA = 0;
                gErrIdB = 0;
                geErrIdReport =   EN_ARC_ERR_REPORT;

//                break;
            }
            case EN_ARC_ERR_REPORT:
            {

                for(; gErrIdB < 32; gErrIdB++)
                {
                    if((gstErrAlarmId.arc_err & (1 << gErrIdB)) != 0)
                    {
                        gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                        gstErrAlarmId.sys_err_alarm_id |= 0x80000000;
                        gstErrAlarmId.sys_err_alarm_id |= 2 << 16;
                        gErrIdB++;
                        return e_err_work_action;
                    }
                }
                gErrIdA = 0;
                gErrIdB = 0;
                geErrIdReport =   EN_MINTOR_ERR_REPORT;

//                break;
            }
            case EN_MINTOR_ERR_REPORT:
            {
                for(; gErrIdB < 32; gErrIdB++)
                {

                    if((gstErrAlarmId.monitor_err & (1 << gErrIdB)) != 0)
                    {
                        gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                        gstErrAlarmId.sys_err_alarm_id |= 0x80000000;
                        gstErrAlarmId.sys_err_alarm_id |= 3 << 16;
                        gErrIdB++;
                        return e_err_work_action;
                    }

                }
                gErrIdA = 0;
                gErrIdB = 0;
                geErrIdReport =   EN_DSP_ALARM_REPORT;
//                break;
            }
            case EN_DSP_ALARM_REPORT:
            {
                for(; gErrIdB < 32; gErrIdB++)
                {
                    if((gstErrAlarmId.dsp_alarm & (1 << gErrIdB)) != 0)
                    {
                        //计算出警告码
                        gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                        gstErrAlarmId.sys_err_alarm_id &= 0x3fffffff;
                        gstErrAlarmId.sys_err_alarm_id |= 0x40000000;
                        gstErrAlarmId.sys_err_alarm_id |= 0 << 16;
                        gErrIdB++;
                        return e_err_work_action;
                    }
                }
                gErrIdA = 0;
                gErrIdB = 0;

                geErrIdReport =   EN_BMS_ALARM_REPORT;
//                break;
            }
            case EN_BMS_ALARM_REPORT:
            {

                for(; gErrIdB < 32; gErrIdB++)
                {

                    if((gstErrAlarmId.bms_alarm & (1 << gErrIdB)) != 0)
                    {
                        gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                        gstErrAlarmId.sys_err_alarm_id &= 0x3fffffff;
                        gstErrAlarmId.sys_err_alarm_id |= 0x40000000;
                        gstErrAlarmId.sys_err_alarm_id |= 1 << 16;
                        gErrIdB++;
                        return e_err_work_action;
                    }

                }
                geErrIdReport =   EN_ARC_ALARM_REPORT;
                gErrIdA = 0;
                gErrIdB = 0;
//                break;
            }
            case EN_ARC_ALARM_REPORT:
            {

                geErrIdReport =   EN_MINTOR_ALARM_REPORT;
//                break;
            }
            case EN_MINTOR_ALARM_REPORT:
            {
                geErrIdReport =   EN_DCDC_ERR_REPORT;
//                break;
            }
            case EN_DCDC_ERR_REPORT:
            {//20250609 新增
                for(; gErrIdB < 32; gErrIdB++)
                {
                    if((gstErrAlarmId.dcdc_err & (1 << gErrIdB)) != 0)
                    {
                        gstErrAlarmId.sys_err_alarm_id = gErrIdA * 32 + gErrIdB;
                        gstErrAlarmId.sys_err_alarm_id |= 0x80000000;
                        gstErrAlarmId.sys_err_alarm_id |= 4 << 16;
                        gErrIdB++;
                        return e_err_work_action;
                    }
                }              
                gErrIdA = 0;
                gErrIdB = 0;
                geErrIdReport = EN_DSP_ERR_REPORT;
            }
            default:
                break;
        }
    }


    return e_err_work_action;
}

uint32_t GetErrId(void)
{
    return gstErrAlarmId.sys_err_alarm_id;
}
uint8_t  GetPcsCommStatus(void)
{
    return gstSysErrorDat.comm_time_out.bits.pcs_comm_time_out;
}