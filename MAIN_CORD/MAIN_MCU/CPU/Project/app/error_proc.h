#ifndef ERROR_HHH
#define ERROR_HHH
#include "slave_bms.h"
#include "slave_pcs.h"
#include "slave_collect.h"
#include "firmware_upgrate.h"

/*******************dsp notice********************/
/*----dsp_grid_notice----*/

/*----dsp_pv_notice----*/
#define  DSP_PV_NOTICE_PV1_NO_CONNECT_MASK    (1UL<<0)     //PV1未接
#define  DSP_PV_NOTICE_PV2_NO_CONNECT_MASK    (1UL<<1)     //PV2未接
#define  DSP_PV_NOTICE_PV3_NO_CONNECT_MASK    (1UL<<2)     //PV3未接
#define  DSP_PV_NOTICE_PV4_NO_CONNECT_MASK    (1UL<<3)     //PV4未接
#define  DSP_PV_NOTICE_PV5_NO_CONNECT_MASK    (1UL<<4)     //PV5未接
#define  DSP_PV_NOTICE_PV6_NO_CONNECT_MASK    (1UL<<5)     //PV6未接
#define  DSP_PV_NOTICE_PV7_NO_CONNECT_MASK    (1UL<<6)     //PV7未接
#define  DSP_PV_NOTICE_PV8_NO_CONNECT_MASK    (1UL<<7)     //PV8未接

/*----dsp_inv_notice----*/
   
/*----dsp_load_notice----*/ 
   
/*----dsp_bus_notice----*/  

/*----dsp_temp_notice----*/
#define  DSP_TEMP_NOTICE_ENVIRONMENT_OT_MASK   (1UL<<0)    //环境过温
#define  DSP_TEMP_NOTICE_ENVIRONMENT_UT_MASK   (1UL<<1)    //环境欠温

/*----dsp_gen_notice----*/ 

/*----dsp_other_notice----*/
#define  DSP_UPG_NOTICE_DSP_FILE_CHECK_FAULT_MASK       (1UL<<0)     //DSP固件包校验失败
#define  DSP_UPG_NOTICE_ARC_FILE_CHECK_FAULT_MASK       (1UL<<1)     //ARC固件包校验失败
#define  DSP_UPG_NOTICE_DSP_COP_FILE_CHECK_FAULT_MASK   (1UL<<2)     //DSP协处理器固件包校验失败

/*******************dsp alarm********************/
/*----dsp_grid_alarm----*/
#define  DSP_GRID_ALARM_VOLT_LOW_MASK               (1UL<<0)
#define  DSP_GRID_ALARM_VOLT_HIGH_MASK              (1UL<<1)
#define  DSP_GRID_ALARM_FREQ_LOW_MASK               (1UL<<2)
#define  DSP_GRID_ALARM_FREQ_HIGH_MASK              (1UL<<3)
#define  DSP_GRID_ALARM_UNBALANCE_MASK              (1UL<<4) //电网电压不平衡
#define  DSP_GRID_ALARM_CURR_HIGH_DC_COMPONENT_MASK (1UL<<5) //直流分量高（电流）

/*----dsp_pv_alarm----*/
#define  DSP_PV0_ALARM_PV1_UV_MASK              (1UL<<0)     //PV欠压
#define  DSP_PV0_ALARM_PV2_UV_MASK              (1UL<<1)     //PV欠压
#define  DSP_PV0_ALARM_PV3_UV_MASK              (1UL<<2)     //PV欠压
#define  DSP_PV0_ALARM_PV4_UV_MASK              (1UL<<3)     //PV欠压
#define  DSP_PV0_ALARM_PV1_OV_MASK              (1UL<<4)     //PV过压
#define  DSP_PV0_ALARM_PV2_OV_MASK              (1UL<<5)     //PV过压
#define  DSP_PV0_ALARM_PV3_OV_MASK              (1UL<<6)     //PV过压
#define  DSP_PV0_ALARM_PV4_OV_MASK              (1UL<<7)     //PV过压
#define  DSP_PV0_ALARM_PV1_OC_MASK              (1UL<<8)     //PV过流
#define  DSP_PV0_ALARM_PV2_OC_MASK              (1UL<<9)     //PV过流
#define  DSP_PV0_ALARM_PV3_OC_MASK              (1UL<<10)    //PV过流
#define  DSP_PV0_ALARM_PV4_OC_MASK              (1UL<<11)    //PV过流
#define  DSP_PV0_ALARM_PV1_OP_MASK              (1UL<<12)    //PV过功率
#define  DSP_PV0_ALARM_PV2_OP_MASK              (1UL<<13)    //PV过功率
#define  DSP_PV0_ALARM_PV3_OP_MASK              (1UL<<14)    //PV过功率
#define  DSP_PV0_ALARM_PV4_OP_MASK              (1UL<<15)    //PV过功率

#define  DSP_PV1_ALARM_OP_MASK                  (1UL<<0)    //PV总功率过高



/*----dsp_inv_alarm----*/
#define  DSP_INV_ALARM_OUTPUT_ERROR_MASK       (1UL<<0)   //逆变输出异常
#define  DSP_INV_ALARM_OUTPUT_VOLT_HIGH_MASK   (1UL<<1)   //逆变输出高
#define  DSP_INV_ALARM_OUTPUT_VOLT_LOW_MASK    (1UL<<2)   //逆变输出低
   
/*----dsp_load_alarm----*/ 
#define  DSP_LOAD_ALARM_OVER_LOAD_MASK         (1UL<<0)   //过载
   
/*----dsp_bus_alarm----*/  
#define  DSP_BUS_ALARM_VOLT_LOW_MASK           (1UL<<0)   //母线电压低
#define  DSP_BUS_ALARM_VOLT_HIGH_MASK          (1UL<<1)   //母线电压高

/*----dsp_temp_alarm----*/
#define  DSP_TEMP_ALARM_INV_OT_MASK                 (1UL<<0)    //逆变过温

// #define  DSP_TEMP_ALARM_BOX_OT_MASK                 (1UL<<0)    //腔体过温
// #define  DSP_TEMP_ALARM_RADIATOR_OT_MASK            (1UL<<1)    //散热器过温
// #define  DSP_TEMP_ALARM_INV_OT_MASK                 (1UL<<2)    //逆变过温
// #define  DSP_TEMP_ALARM_PV_OT_MASK                  (1UL<<3)    //PV过温

/*----dsp_gen_alarm----*/ 
#define  DSP_GEN_ALARM_VOLT_LOW_MASK          (1UL<<0)
#define  DSP_GEN_ALARM_VOLT_HIGH_MASK         (1UL<<1)
#define  DSP_GEN_ALARM_FREQ_LOW_MASK          (1UL<<2)
#define  DSP_GEN_ALARM_FREQ_HIGH_MASK         (1UL<<3)


/*----dsp_other_alarm----*/
#define  DSP_OTHER_ALARM_FAN_MASK                    (1UL<<0)  //风扇异常警告
#define  DSP_OTHER_ALARM_R_CT_OPEN_MASK              (1UL<<1)  //R相CT断路
#define  DSP_OTHER_ALARM_S_CT_OPEN_MASK              (1UL<<2)  //S相CT断路
#define  DSP_OTHER_ALARM_T_CT_OPEN_MASK              (1UL<<3)  //T相CT断路
#define  DSP_OTHER_ALARM_R_CT_INVERSE_MASK           (1UL<<4)  //CT反相
#define  DSP_OTHER_ALARM_S_CT_INVERSE_MASK           (1UL<<5)  //CT反相
#define  DSP_OTHER_ALARM_T_CT_INVERSE_MASK           (1UL<<6)  //CT反相
#define  DSP_OTHER_ALARM_UPGRADE_CHECK_FAULT_MASK    (1UL<<7)  //固件包异常
#define  DSP_OTHER_ALARM_MODEID_MASK                 (1UL<<8)  //MODID异常

/******************dsp_error*******************/
/*----dsp_grid_error----*/
#define  DSP_GRID_ERR_VOLT_HIGH_MASK         (1UL<<0) //电网过压
#define  DSP_GRID_ERR_VOLT_LOW_MASK          (1UL<<1) //电网欠压
#define  DSP_GRID_ERR_FREQ_HIGH_MASK         (1UL<<2) //电网过频
#define  DSP_GRID_ERR_FREQ_LOW_MASK          (1UL<<3) //电网欠频
#define  DSP_GRID_ERR_PHASE_LOSS_MASK        (1UL<<4) //电网缺相
#define  DSP_GRID_ERR_UNBALANCE_MASK         (1UL<<5) //电网电压不平衡
#define  DSP_CURR_HIGH_DC_COMPONENT_MASK     (1UL<<6) //直流分量高（电流）


/*----dsp_pv_error----*/
#define  DSP_PV0_ERR_PV1_SHORT_MASK                (1UL<<0)     //短路故障
#define  DSP_PV0_ERR_PV2_SHORT_MASK                (1UL<<1)     //短路故障
#define  DSP_PV0_ERR_PV3_SHORT_MASK                (1UL<<2)     //短路故障
#define  DSP_PV0_ERR_PV4_SHORT_MASK                (1UL<<3)     //短路故障
#define  DSP_PV0_ERR_PV1_INVERSE_MASK              (1UL<<4)     //PV反接故障
#define  DSP_PV0_ERR_PV2_INVERSE_MASK              (1UL<<5)     //PV反接故障
#define  DSP_PV0_ERR_PV3_INVERSE_MASK              (1UL<<6)     //PV反接故障
#define  DSP_PV0_ERR_PV4_INVERSE_MASK              (1UL<<7)     //PV反接故障
#define  DSP_PV0_ERR_PV1_OUT_CONTROL_MASK          (1UL<<8)     //PV失控故障
#define  DSP_PV0_ERR_PV2_OUT_CONTROL_MASK          (1UL<<9)     //PV失控故障
#define  DSP_PV0_ERR_PV3_OUT_CONTROL_MASK          (1UL<<10)    //PV失控故障
#define  DSP_PV0_ERR_PV4_OUT_CONTROL_MASK          (1UL<<11)    //PV失控故障
#define  DSP_PV0_ERR_PV1_OC_MASK                   (1UL<<12)    //PV过流故障
#define  DSP_PV0_ERR_PV2_OC_MASK                   (1UL<<13)    //PV过流故障
#define  DSP_PV0_ERR_PV3_OC_MASK                   (1UL<<14)    //PV过流故障
#define  DSP_PV0_ERR_PV4_OC_MASK                   (1UL<<15)    //PV过流故障

#define  DSP_PV1_ERR_PV1_INSULATION_LOW_MASK       (1UL<<0)     //绝缘阻抗故障
#define  DSP_PV1_ERR_PV2_INSULATION_LOW_MASK       (1UL<<1)     //绝缘阻抗故障
#define  DSP_PV1_ERR_PV3_INSULATION_LOW_MASK       (1UL<<2)     //绝缘阻抗故障
#define  DSP_PV1_ERR_PV4_INSULATION_LOW_MASK       (1UL<<3)     //绝缘阻抗故障
#define  DSP_PV1_ERR_PV1_ARC_MASK                  (1UL<<4)     //PV拉弧故障
#define  DSP_PV1_ERR_PV2_ARC_MASK                  (1UL<<5)     //PV拉弧故障
#define  DSP_PV1_ERR_PV3_ARC_MASK                  (1UL<<6)     //PV拉弧故障
#define  DSP_PV1_ERR_PV4_ARC_MASK                  (1UL<<7)     //PV拉弧故障
#define  DSP_PV1_ERR_PV1_OV_MASK                   (1UL<<8)     //PV过压
#define  DSP_PV1_ERR_PV2_OV_MASK                   (1UL<<9)     //PV过压
#define  DSP_PV1_ERR_PV3_OV_MASK                   (1UL<<10)    //PV过压
#define  DSP_PV1_ERR_PV4_OV_MASK                   (1UL<<11)    //PV过压


/*----dsp_inv_error----*/
#define  DSP_INV_ERR_OUTPUT_SHORT_MASK            (1UL<<0)   //逆变输出短路故障
#define  DSP_INV_ERR_VOLT_LOW_MASK                (1UL<<1)   //逆变欠压故障
#define  DSP_INV_ERR_VOLT_HIGH_MASK               (1UL<<2)   //逆变过压故障
#define  DSP_INV_ERR_OC_MASK                      (1UL<<3)   //逆变过流故障
#define  DSP_INV_OUTPUT_FAULT_MASK                (1UL<<4)   //逆变输出异常
#define  DSP_VOLT_HIGH_DC_COMPONENT_MASK          (1UL<<5)   //直流分量高（电压）
   
/*----dsp_load_error----*/ 
#define  DSP_LOAD_ERR_OVER_LOAD_1_MASK            (1UL<<0)   //过载保护
#define  DSP_LOAD_ERR_OVER_LOAD_2_MASK            (1UL<<1)   //过载2故障
   
/*----dsp_bus_error----*/  
#define  DSP_BUS_ERR_VOLT_LOW_MASK                (1UL<<0)   //母线电压低故障
#define  DSP_BUS_ERR_VOLT_HIGH_MASK               (1UL<<1)   //母线电压高故障
#define  DSP_BUS_ERR_SHORT_MASK                   (1UL<<2)   //母线短路故障
#define  DSP_BUS_ERR_CONNECT_MASK                 (1UL<<3)   //母线连接故障            
#define  DSP_BUS_ERR_CHG_OC_MASK                  (1UL<<4)   //电池充电过流故障      
#define  DSP_BUS_ERR_DSG_OC_MASK                  (1UL<<5)   //电池放电过流故障      

/*----dsp_temp_error----*/
#define  DSP_TEMP_ERR_INV_OT_MASK                 (1UL<<0)    //逆变器散热过温
#define  DSP_TEMP_ERR_ENVIORONMENT_OT_MASK        (1UL<<1)    //环境过温故障
#define  DSP_TEMP_ERR_ENVIORONMENT_UT_MASK        (1UL<<2)    //环境欠温故障
#define  DSP_TEMP_ERR_PV_OT_MASK                  (1UL<<3)    //PV过温故障

//#define  DSP_TEMP_ERR_BOX_OT_MASK                 (1UL<<0)    //腔体过温故障
//#define  DSP_TEMP_ERR_RADIATOR_OT_MASK            (1UL<<1)    //散热器过温故障
//#define  DSP_TEMP_ERR_INV_OT_MASK                 (1UL<<2)    //逆变过温故障
//#define  DSP_TEMP_ERR_PV_OT_MASK                  (1UL<<3)    //PV过温故障
//#define  DSP_TEMP_ERR_ENVIORONMENT_OT_MASK        (1UL<<4)    //环境过温故障
//#define  DSP_TEMP_ERR_ENVIORONMENT_UT_MASK        (1UL<<5)    //环境欠温故障

/*----dsp_gen_error----*/ 

/*----dsp_other_error----*/
#define  DSP_OTHER_ERR_WAKEUP_OUT_TIME_MASK         (1UL<<0)  //启动超时故障
#define  DSP_OTHER_ERR_CMPSS_MASK                   (1UL<<1)  //硬件过流故障
#define  DSP_OTHER_ERR_LEAK_CURR_HIGH_MASK          (1UL<<2)  //漏电流过高故障
#define  DSP_OTHER_ERR_ARC_ERR_MASK                 (1UL<<3)  //拉弧故障
#define  DSP_OTHER_ERR_INSULATION_ERR_MASK          (1UL<<4)  //绝缘阻抗故障
#define  DSP_OTHER_ERR_RELAY_SHORT_MASK             (1UL<<5)  //继电器故障
#define  DSP_OTHER_ERR_SAMPLING_SENSOR_ERR_MASK     (1UL<<6)  //采样传感器故障
#define  DSP_OTHER_ERR_TEMP_SENSOR_ERR_MASK         (1UL<<7)  //温度传感器故障
#define  DSP_OTHER_ERR_MASTER_TO_SLAVE_COM_ERR_MASK (1UL<<8)  //主从通讯故障
#define  DSP_OTHER_ERR_BYPASS_ERR_MASK              (1UL<<9)  //旁路故障
#define  DSP_OTHER_ERR_PARALLEL_COMM_ERR_MASK       (1UL<<10) //并机通信故障
#define  DSP_OTHER_ERR_PARALLEL_SLAVE_ERR_MASK      (1UL<<11) //并机从机故障
#define  DSP_OTHER_ERR_GROUND_FAULT_ERR_MASK        (1UL<<12) //接地故障
#define  DSP_OTHER_ERR_WITH_ARC_COMM_ERR_MASK       (1UL<<13) //PCS与ARC通信异常

/******************bms_notice*******************/
/*----bms_mos_notice----*/

/*----bms_temp_notice----*/
#define  BMS_TEMP_NOTICE_CHG_MOS_OT_MASK            (1UL<<0)  //充电MOS过温通知
#define  BMS_TEMP_NOTICE_DSG_MOS_OT_MASK            (1UL<<1)  //放电MOS过温通知
#define  BMS_TEMP_NOTICE_PACK_OT_MASK               (1UL<<2)  //PACK过温通知

/*----bms_curr_notice----*/
#define  BMS_CURR_NOTICE_CHG_OC_MASK                (1UL<<0)  //充电MOS过流通知
#define  BMS_CURR_NOTICE_DSG_OC_MASK                (1UL<<1)  //放电MOS过流通知

/*----bms_volt_notice----*/
#define  BMS_VOLT_NOTICE_CELL_OV_MASK               (1UL<<0)  //电芯过压通知
#define  BMS_VOLT_NOTICE_CELL_UV_MASK               (1UL<<1)  //电芯欠压通知

/*----bms_sox_notice----*/
#define  BMS_SOX_NOTICE_SOH_LOW_MASK                (1UL<<0)  //SOH过低通知

/*----bms_other_notice----*/

/******************bms_alarm*******************/
/*----bms_mos_alarm----*/

/*----bms_temp_alarm----*/
#define  BMS_TEMP_ALARM_CELL_OT_MASK               (1UL<<0)  //电芯单体过温告警
#define  BMS_TEMP_ALARM_CELL_UT_MASK               (1UL<<1)  //电芯单体欠温告警
#define  BMS_TEMP_ALARM_POLE_OT_MASK               (1UL<<2)  //极柱过温告警
#define  BMS_TEMP_ALARM_CELL_TEMP_DIFF_MASK        (1UL<<3)  //电芯温差告警
#define  BMS_TEMP_ALARM_CHG_DSG_MOS_OT_MASK        (1UL<<4)  //充放电MOS过温警告
// #define  BMS_TEMP_ALARM_CHG_MOS_OT_MASK            (1UL<<0)  //充电MOS过温警告
// #define  BMS_TEMP_ALARM_DSG_MOS_OT_MASK            (1UL<<1)  //放电MOS过温警告
// #define  BMS_TEMP_ALARM_PACK_OT_MASK               (1UL<<2)  //PACK过温警告

/*----bms_curr_alarm----*/
#define  BMS_CURR_ALARM_DSG_OC_MASK                (1UL<<0)  //放电MOS过流警告
#define  BMS_CURR_ALARM_CHG_OC_MASK                (1UL<<1)  //充电MOS过流警告
 
/*----bms_volt_alarm----*/
#define  BMS_VOLT_ALARM_CELL_OV_MASK               (1UL<<0)  //电芯过压警告
#define  BMS_VOLT_ALARM_CELL_UV_MASK               (1UL<<1)  //电芯欠压警告
#define  BMS_VOLT_ALARM_PACK_OV_MASK               (1UL<<2)  //总压过压警告
#define  BMS_VOLT_ALARM_PACK_UV_MASK               (1UL<<3)  //总压欠压警告
#define  BMS_VOLT_ALARM_CELL_V_DIFF_MASK           (1UL<<4)  //单体压差大警告
/*----bms_sox_alarm----*/
#define  BMS_SOX_ALARM_SOH_LOW_MASK                (1UL<<0)  //SOH过低警告

/*----bms_other_alarm----*/


/******************bms_error*******************/
/*----bms_mos_error----*/
#define  BMS_MOS_ERR_ADHESION_MASK              (1UL<<0)    //MOS粘连
#define  BMS_MOS_ERR_CLOSE_ERR_MASK             (1UL<<1)    //MOS闭合故障
#define  BMS_MOS_ERR_PRE_INVERSE_MASK           (1UL<<2)    //预充反向 预充超时 预充短路
#define  BMS_MOS_ERR_CHG_SHORT_MASK             (1UL<<3)    //充电短路
#define  BMS_MOS_ERR_DSG_SHORT_MASK             (1UL<<4)    //放电短路
// #define  BMS_MOS_ERR_CHG_CLOSE_ERR_MASK         (1UL<<2)    //充电MOS闭合故障
// #define  BMS_MOS_ERR_DSG_CLOSE_ERR_MASK         (1UL<<3)    //放电MOS闭合故障
// #define  BMS_MOS_ERR_PRE_OUT_TIME_MASK          (1UL<<5)    //预充超时


/*----bms_temp_error----*/
#define  BMS_TEMP_ERR_MOS_NTC_ERR_MASK          (1UL<<0)    //MOS NTC故障
#define  BMS_TEMP_ERR_CELL_NTC_ERR_MASK         (1UL<<1)    //PACK NTC故障
#define  BMS_TEMP_ERR_CELL_NTC_2_ERR_MASK       (1UL<<2)    //PACK NTC故障(严重)
#define  BMS_TEMP_ERR_CHG_DSG_MOS_OT2_MASK      (1UL<<3)    //充电MOS过温二级故障 放电MOS过温二级故障
#define  BMS_TEMP_ERR_CHG_DSG_MOS_OT1_MASK      (1UL<<4)    //充电MOS过温一级故障 放电MOS过温一级故障
// #define  BMS_TEMP_ERR_DSG_MOS_OT_MASK          (1UL<<4)    //放电MOS过温故障
// #define  BMS_TEMP_ERR_DSG_MOS_OT_MASK          (1UL<<4)    //放电MOS过温故障
#define  BMS_TEMP_ERR_CELL_OT2_MASK              (1UL<<5)    //CELL过温二级故障
#define  BMS_TEMP_ERR_CELL_OT1_MASK              (1UL<<6)    //CELL过温一级故障
#define  BMS_TEMP_ERR_CELL_UT2_MASK              (1UL<<7)    //CELL欠温二级故障
#define  BMS_TEMP_ERR_CELL_UT1_MASK              (1UL<<8)    //CELL欠温一级故障
#define  BMS_TEMP_ERR_POLE_OT2_MASK              (1UL<<9)    //极柱过温二级
#define  BMS_TEMP_ERR_POLE_OT1_MASK              (1UL<<10)   //极柱过温一级
#define  BMS_TEMP_ERR_CELL_TEMP_DIFF_MASK        (1UL<<11)    //电芯温差大故障

/*----bms_curr_error----*/
#define  BMS_CURR_ERR_SENSOR_MASK                (1UL<<0)  //电流传感器故障
#define  BMS_CURR_ERR_DSG_OC2_MASK               (1UL<<1)  //放电MOS过流二级故障
#define  BMS_CURR_ERR_DSG_OC1_MASK               (1UL<<2)  //充电MOS过流一级故障
#define  BMS_CURR_ERR_CHG_OC2_MASK               (1UL<<3)  //充电MOS过流二级故障
#define  BMS_CURR_ERR_CHG_OC1_MASK               (1UL<<4)  //充电MOS过流一级故障

 
/*----bms_volt_error----*/
#define  BMS_VOLT_ERR_SENSOR_MASK               (1UL<<0)  //电压传感器故障
#define  BMS_VOLT_ERR_CELL_OV2_MASK             (1UL<<1)  //电芯过压二级故障
#define  BMS_VOLT_ERR_CELL_OV1_MASK             (1UL<<2)  //电芯过压一级故障
#define  BMS_VOLT_ERR_CELL_UV2_MASK             (1UL<<3)  //电芯欠压二级故障
#define  BMS_VOLT_ERR_CELL_UV1_MASK             (1UL<<4)  //电芯欠压一级故障
#define  BMS_VOLT_ERR_PACK_OV2_MASK             (1UL<<5)  //总压过压二级故障
#define  BMS_VOLT_ERR_PACK_OV1_MASK             (1UL<<6)  //总压过压一级故障
#define  BMS_VOLT_ERR_PACK_UV2_MASK             (1UL<<7)  //总压欠压二级故障
#define  BMS_VOLT_ERR_PACK_UV1_MASK             (1UL<<8)  //总压欠压一级故障
#define  BMS_VOLT_ERR_CELL_VOLT_DIFF_MASK       (1UL<<9)  //单体压差过大故障
#define  BMS_VOLT_ERR_SAMPLE_ERR_MASK           (1UL<<10) //采样异常故障 断线故障

/*----bms_sox_error----*/
#define  BMS_SOX_ERR_SOH_LOW2_MASK               (1UL<<0)  //SOH过低二级故障
#define  BMS_SOX_ERR_SOH_LOW1_MASK               (1UL<<1)  //SOH过低一级故障

/*----bms_other_error----*/
#define  BMS_OTHER_ERR_FLASH_MASK               (1UL<<0)  //FLASH故障
#define  BMS_OTHER_ERR_EEPROM_MASK              (1UL<<1)  //EEPROM故障
#define  BMS_OTHER_ERR_AFE_MASK                 (1UL<<2)  //AFE故障
#define  BMS_OTHER_ERR_RTC_MASK                 (1UL<<3)  //RTC故障
#define  BMS_OTHER_ERR_BALANCE_MASK             (1UL<<4)  //均衡故障
#define  BMS_OTHER_ERR_HEAT_MASK                (1UL<<5)  //加热膜故障


/*******************dcdc err*****************************/
/*----dcdc_volt_error----*/
#define DCDC_VOLT_ERR_UV_BEFORE_PRE_CHG_RES_MASK                  (1UL<<0)      //输入电阻前欠压
#define DCDC_VOLT_ERR_UV_AFTER_PRE_CHG_RES_MASK                   (1UL<<1)      //输入电阻后欠压
#define DCDC_VOLT_ERR_UV_AFTER_B_MASK                             (1UL<<2)      //B相欠压
#define DCDC_VOLT_ERR_UV_AFTER_A_MASK                             (1UL<<3)      //A相欠压
#define DCDC_VOLT_ERR_OV_BEFORE_PRE_CHG_RES_MASK                  (1UL<<4)      //输入电阻前过压
#define DCDC_VOLT_ERR_OV_AFTER_PRE_CHG_RES_MASK                   (1UL<<5)      //输入电阻后过压
#define DCDC_VOLT_ERR_OV_AFTER_B_MASK                             (1UL<<6)      //B相过压
#define DCDC_VOLT_ERR_OV_AFTER_A_MASK                             (1UL<<7)      //A相过压
#define DCDC_VOLT_ERR_CHG_V_DIFF_PRE_CHG_RES_MASK                 (1UL<<8)      //充电时预充电阻压差过大
#define DCDC_VOLT_ERR_DSG_V_DIFF_PRE_CHG_RES_MASK                 (1UL<<9)      //放电时预充电阻压差过大
#define DCDC_VOLT_ERR_B_LLC_UV_MASK                               (1UL<<10)     //B相LLC欠压
#define DCDC_VOLT_ERR_A_LLC_UV_MASK                               (1UL<<11)     //A相LLC欠压 
#define DCDC_VOLT_ERR_B_LLC_OV_MASK                               (1UL<<12)     //B相LLC过压
#define DCDC_VOLT_ERR_A_LLC_OV_MASK                               (1UL<<13)     //A相LLC过压
#define DCDC_VOLT_ERR_HARD_OV_MASK                                (1UL<<14)     //硬件过压
/*----dcdc_curr_error----*/
#define DCDC_CURR_ERR_B_CURR_MASK                                  (1UL<<0)      //B相总电流故障
#define DCDC_CURR_ERR_A_CURR_MASK                                  (1UL<<1)      //A相总电流故障
#define DCDC_CURR_ERR_B_LLC_CURR_MASK                              (1UL<<2)      //B相LLC电流故障
#define DCDC_CURR_ERR_A_LLC_CURR_MASK                              (1UL<<3)      //A相LLC电流故障
#define DCDC_CURR_ERR_B_RESONANCE_CURR_MASK                        (1UL<<4)      //B相谐振电流故障
#define DCDC_CURR_ERR_A_RESONANCE_CURR_MASK                        (1UL<<5)      //A相谐振电流故障
#define DCDC_CURR_ERR_HARD_CURR_MASK                               (1UL<<6)      //B相硬件电流故障
/*----dcdc_power_other_error----*/
#define DCDC_POWER_OTHER_ERR_INPUT_OP_MASK                         (1UL<<0)      //输入过载 
#define DCDC_POWER_OTHER_ERR_OUTPUT_OP_MASK                        (1UL<<1)      //输出过载 
#define DCDC_POWER_OTHER_ERR_MOS_SWITCH_MASK                       (1UL<<8)      //MOS开关故障 
#define DCDC_POWER_OTHER_ERR_OT_MASK                               (1UL<<9)      //过温故障
#define DCDC_POWER_OTHER_ERR_RELAY_MASK                            (1UL<<10)      //继电器故障 


/***********************************************************************************/

/***********************ID_MASK****************************/
#define BMS_MOS_OT_2AND3_LEVEL_ERR_ID_MASK                      (1UL<<0)                //MOS过温2级 MOS过温3级      
#define BMS_CELL_UV_2AND3_LEVEL_ERR_ID_MASK                     (1UL<<1)                //单体欠压二级 单体欠压三级 
#define BMS_CELL_OV_2AND3_LEVEL_ERR_ID_MASK                     (1UL<<2)                //单体过压二级 单体过压三级   
#define BMS_TOTAL_UV_ERR_ID_MASK                                (1UL<<3)                //总欠压   
#define BMS_TOTAL_OV_ERR_ID_MASK                                (1UL<<4)                //总过压   
#define BMS_CELL_VOLT_DIFF_ERR_ID_MASK                          (1UL<<5)                //单体压差过大   
#define BMS_DSG_OC_2AND3_LEVEL_ERR_ID_MASK                      (1UL<<6)                //放电过流2，3级   
#define BMS_CHG_OC_2AND3_LEVEL_ERR_ID_MASK                      (1UL<<7)                //充电过流2，3级   
#define BMS_CELL_OT_2AND3_LEVEL_ERR_ID_MASK                     (1UL<<9)                //电芯过温2，3级     
#define BMS_SOH_US_1AND2_LEVEL_ID_ERR_MASK                      (1UL<<11)               //SOG过低1 2 3级   
#define BMS_VOLT_SENSOR_ERR_ID_MASK                             (1UL<<12)               //电压传感器故障       
#define BMS_TEMP_SENSOR_ERR_ID_MASK                             (1UL<<13)               //温度传感器故障       
#define BMS_SPI_FLASH_SENSOR_ID_ERR_MASK                        (1UL<<14)               //SPI FLASH           
#define BMS_E2PROM_SENSOR_ERR_ID_MASK                           (1UL<<15)               //E2PROM ERR          
#define BMS_RTC_SENSOR_ERR_ID_MASK                              (1UL<<16)               //RTC ERR               
#define BMS_CHG_SHORT_ERR_ID_MASK                               (1UL<<17)               //CHG SHORT             
#define BMS_DSG_SHORT_ERR_ID_MASK                               (1UL<<18)               //DSG SHORT            
#define BMS_PRE_CHG_OVERTIEM_ID_ERR_MASK                        (1UL<<19)               //预充超时故障        
#define BMS_HEAT_SHORT_ERR_ID_MASK                              (1UL<<20)               //加热短路             
#define BMS_AFE_OPEN_ERR_ID_MASK                                (1UL<<21)               //AFE断线             
#define BMS_MOS_SHORT_ERR_ID_MASK                               (1UL<<22)               //MOS 粘连                 
#define BMS_MOS_PRE_CHG_FAIL_CLOSE_ID_ERR_MASK                  (1UL<<23)               //预充电MOS无法闭合        
#define BMS_MOS_CHG_FAIL_CLOSE_ERR_ID_MASK                      (1UL<<24)               //充电MOS无法闭合          
#define BMS_MOS_DSG_FAIL_CLOSE_ERR_ID_MASK                      (1UL<<25)               //放电MOS无法闭合          
#define BMS_MOS_TEMP_SENSOR_ERR_ID_MASK                         (1UL<<26)               //MOS温度传感器故障        

#define BMS_PRE_CHG_CUR_REVERSE_ERR_ID_MASK                     (1UL<<1)                //预充电流反向 
#define BMS_PRE_SHORT_ERR_ID_MASK                               (1UL<<2)                //预充短路      
#define BMS_CUR_SENSOR_ZERO_DIFF_ID_MASK                        (1UL<<3)                //电流传感器零漂 
#define BMS_PACK_CELL_VOLT_DIFF_ERR_ID_MASK                     (1UL<<4)                //PACK和电芯电压不匹配   
#define BMS_BALANCE_ERR_ID_MASK                                 (1UL<<5)                //均衡回路故障    
//#define BMS_SOC_LOW_1LEVEL_ERR_ID_MASK                          (1UL<<6)                //SOC过低一级    
//#define BMS_SOC_LOW_2LEVEL_ERR_ID_MASK                          (1UL<<7)                //SOC过低二级    
#define BMS_SOC_JUMP_ERR_ID_MASK                                (1UL<<8)                //SOC跳变        
#define BMS_ATE_COMM_ERR_ID_MASK                                (1UL<<9)                //AFE通讯故障    


#define ARC_CHANNEL1_ELE_ARC_ERR_MASK                           (1UL<<0)
#define ARC_CHANNEL2_ELE_ARC_ERR_MASK                           (1UL<<1)
#define ARC_CHANNEL3_ELE_ARC_ERR_MASK                           (1UL<<2)
#define ARC_CHANNEL4_ELE_ARC_ERR_MASK                           (1UL<<3)

#define ARC_CHANNEL1_R_LOW_ERR_MASK                             (1UL<<0)
#define ARC_CHANNEL2_R_LOW_ERR_MASK                             (1UL<<1)
#define ARC_CHANNEL3_R_LOW_ERR_MASK                             (1UL<<2)
#define ARC_CHANNEL4_R_LOW_ERR_MASK                             (1UL<<3)

#define ARC_CHANNEL1_ELE_ARC_ERR_ID_MASK                        (1UL<<0)
#define ARC_CHANNEL2_ELE_ARC_ERR_ID_MASK                        (1UL<<1)
#define ARC_CHANNEL3_ELE_ARC_ERR_ID_MASK                        (1UL<<2)
#define ARC_CHANNEL4_ELE_ARC_ERR_ID_MASK                        (1UL<<3)

#define ARC_CHANNEL1_R_LOW_ERR_ID_MASK                          (1UL<<8)
#define ARC_CHANNEL2_R_LOW_ERR_ID_MASK                          (1UL<<9)
#define ARC_CHANNEL3_R_LOW_ERR_ID_MASK                          (1UL<<10)
#define ARC_CHANNEL4_R_LOW_ERR_ID_MASK                          (1UL<<11)


//----------monitor err--------------------------

#define  MONITOR_BMS_COMM_ERR_MASK                              ((1UL<<0)|(1UL<<1)|(1UL<<2))
#define  MONITOR_PCS_COMM_ERR_MASK                              (1UL<<3)
#define  MONITOR_ARC_COMM_ERR_MASK                              (1UL<<4)

#define  MONITOR_PCS_COMM_ERR_ID_MASK                           (1UL<<0)
#define  MONITOR_BMS_COMM_ERR_ID_MASK                           (1UL<<1)
#define  MONITOR_ARC_COMM_ERR_ID_MASK                           (1UL<<2)


#define  MONITOR_UPGRADE_SEND_PCS_ERR_ID_MASK                   (1UL<<3)
#define  MONITOR_UPGRADE_SEND_ARC_ERR_ID_MASK                   (1UL<<4)
#define  MONITOR_UPGRADE_SEND_BMS_ERR_ID_MASK                   (1UL<<5)
#define  MONITOR_UPGRADE_SEND_ADD_BMS_ERR_ID_MASK               (1UL<<6)
#define  MONITOR_UPGRADE_MAIN_CHECK_ERR_ID_MASK                 (1UL<<7)
#define  MONITOR_UPGRADE_ZIP_ERR_ID_MASK                        (1UL<<8)
#define  MONITOR_UPGRADE_CHECK_ERR_ID_MASK                      (1UL<<9)
#define  MONITOR_UPGRADE_BMS_FLAG_ERR_ID_MASK                   (1UL<<10)
#define  MONITOR_UPGRADE_PCS_FLAG_ERR_ID_MASK                   (1UL<<11)
#define  MONITOR_UPGRADE_ARC_FLAG_ERR_ID_MASK                   (1UL<<12)
#define  MONITOR_UPGRADE_ADRESS_ERR_ID_MASK                     (1UL<<13)


#define DCDC_INPUT_UV_ERR_ID_MASK                               (1UL<<0)      //输入欠压
#define DCDC_INPUT_OV_ERR_ID_MASK                               (1UL<<1)      //输入过压
#define DCDC_INPUT_OC_ERR_ID_MASK                               (1UL<<2)      //输入过流
#define DCDC_INPUT_OP_ERR_ID_MASK                               (1UL<<3)      //输入过载 
#define DCDC_OUTPUT_UV_ERR_ID_MASK                              (1UL<<4)      //输出欠压
#define DCDC_OUTPUT_OV_ERR_ID_MASK                              (1UL<<5)      //输出过压
#define DCDC_OUTPUT_OC_ERR_ID_MASK                              (1UL<<6)      //输出过流
#define DCDC_OUTPUT_OP_ERR_ID_MASK                              (1UL<<7)      //输出过载 
#define DCDC_POWER_PARALLEL_CONN_FAIL_ERR_ID_MASK               (1UL<<8)      //总功率并联未成功
#define DCDC_PRE_CHARGE_FAIL_ERR_ID_MASK                        (1UL<<9)      //预充未成功


/**********故障数量**********/
#define ERR_ALARM_NOTICE_NUM                        (64 * (9 + 3 * BMS_MAX_NUM))      //(监控卡) + (DSP) + (TotalBms) +  (SingleBms * PackNum)

//警告类型
typedef enum
{
    EN_WARING_TYPE_NO,
    EN_WARING_TYPE_NOTICE,
    EN_WARING_TYPE_ALARM,
    EN_WARING_TYPE_ERR
} EN_WARING_TYPE;


typedef enum
{
    EN_DSP_ERR_REPORT = 0,
    EN_BMS_ERR_REPORT,
    EN_ARC_ERR_REPORT,
    EN_MINTOR_ERR_REPORT,
    EN_DSP_ALARM_REPORT,
    EN_BMS_ALARM_REPORT,
    EN_ARC_ALARM_REPORT,
    EN_MINTOR_ALARM_REPORT,
    EN_DCDC_ERR_REPORT,

} EN_ERR_REPORT_ID;


#pragma pack(1)


typedef union _sys_comm_status_union
{
    struct
    {
        uint32_t  bms_comm_time_out: 1;  //0bit
        uint32_t  bms1_comm_time_out: 1;
        uint32_t  bms2_comm_time_out: 1;
        uint32_t  pcs_comm_time_out: 1;
    } bits;
    uint32_t byte_value;

} SysCommErr;

typedef struct 
{
    BmsErrorInfo_t  bms_error[MAX_PACK_NUM];
    PcsErrorInfo_t  pcs_error;
    uint32_t    inpedance_breakdown_info;
    uint32_t    arc_alarm;
    DcdcErrorInfo_t dcdc_error[MAX_PACK_NUM];
    SysCommErr  comm_time_out;
} SysError_t;


/******提示表格*******/
typedef struct
{
    uint16_t*   waring_reg_ptr;
    uint16_t    waring_bit;
    uint8_t     waring_type;
    uint32_t    waring_num;
} SysWaringSheet_t;




//typedef enum
//{
//    EN_WARING_TYPE_NOTICE,
//    EN_WARING_TYPE_ALARM,
//    EN_WARING_TYPE_ERR
//}EN_ERROR_TYPE;

typedef struct _sys_warning_module_disp_map_stru_t
{
    uint16_t            serial_num;//故障编号
    EN_WARING_TYPE      type;//故障告警通知类型
}SysWarningModuleDispMap_t;



typedef struct
{
    
    CollectMonitorError_t       monitor_error;
    CollectMonitorAlarm_t       monitor_alarm;
    CollectMonitorNotice_t      monitor_notice;
    
    CollectPcsError_t           pcs_error;
    CollectPcsAlarm_t           pcs_alarm;
    CollectPcsNotice_t          pcs_notice;
    
    CollectTotalBmsError_t      total_bms_error;
    CollectTotalBmsAlarm_t      total_bms_alarm;
    CollectTotalBmsNotice_t     total_bms_notice;
    
    CollectSingleBmsError_t    single_bms_err[MAX_PACK_NUM]    ;
    CollectSingleBmsAlarm_t    single_bms_alarm[MAX_PACK_NUM]  ;
    CollectSingleBmsNotice_t   single_bms_notice[MAX_PACK_NUM] ;
    
} SysAllError_t;


typedef struct _err_code_stru_t
{
    uint32_t code_num;
    uint16_t code_flag;
    uint8_t  waring_type;
    uint16_t current_idx;
} ErrCode_t;


typedef enum
{
    EN_NO_ERR_ALARM = 0,
    EN_ONLY_ALARM,
    EN_ERR_CLOSE_CHG,
    EN_ERR_CLOSE_DSG,
    EN_ERR_CLOSE_CHG_DSG,
} EN_ERR_WROK_ACTION;

#pragma pack()

extern SysWaringSheet_t     gstWaringSheet[ERR_ALARM_NOTICE_NUM];
extern ErrCode_t            gErrCodeMap[ERR_ALARM_NOTICE_NUM]; 

void ErrorSheetInit(void);
bool GetSysAlarm(void);
EN_ERR_WROK_ACTION GetSysError(void);
void SysErrorProcess(void);
uint8_t  GetPcsCommStatus(void);
uint32_t GetErrId(void);
SysWarningModuleDispMap_t *GetErrorID(void);


bool GetCloseChgDsgFun(void);
bool GetCloseChgFun(void);
bool GetCloseDsgFun(void);
#endif