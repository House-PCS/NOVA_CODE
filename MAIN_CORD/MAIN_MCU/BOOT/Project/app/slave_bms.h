
#ifndef __SLAVE_BMS_H_
#define __SLAVE_BMS_H_

#include "dev_modbus.h"
#include "bsp.h"

#define BMS_MAX_NUM                                     4
#define SLAVE_BMS_ADDR                                  0x20
#define SLAVE_BMS1_ADDR                                 0x21 
#define SLAVE_BMS2_ADDR                                 0x22  
#define SLAVE_BMS3_ADDR                                 0x23  

#define READ_INPUT_TYPE                                 0X04
#define READ_HOLD_TYPE                                  0X03
#define BAT_NUM_MAX                                     3     //
#define SLAVE_BMS_WRITE_QUEUE_DEPTH                     10
#define BMS_REG_ADDR_MODE_ID                            0X7000
#define BMS_REG_ADDR_PACK_CYCLE                         0X707C
#define BMS_REG_ADDR_HOLD_REG_ADDR                      0X70C0 
#define BMS_INPUT_FRONT_REG_MAX_NUM                     (BMS_REG_ADDR_PACK_CYCLE -BMS_REG_ADDR_MODE_ID)
#define BMS_INPUT_BACK_REG_MAX_NUM                      (BMS_REG_ADDR_HOLD_REG_ADDR - BMS_REG_ADDR_PACK_CYCLE) //(BMS_REG_ADDR_HOLD_REG_ADDR - BMS_REG_ADDR_MODE_ID)
#define BMS_HOLD_REG_MAX_NUM                            (0X7106 - BMS_REG_ADDR_HOLD_REG_ADDR + 1)


              
#define BMS_REG_ADDR_DSG_MOS_CTL                        0X70C0                                                                       
#define BMS_REG_ADDR_CHG_MOS_CTL                        0X70C1
#define BMS_REG_ADDR_PRE_MOS_CTL                        0X70C2
#define BMS_REG_ADDR_LIMIT_CURR_CTRL                    0X70C3
#define BMS_REG_ADDR_SHUTDOWN                           0X70C4
#define BMS_REG_ADDR_RTC_TIME                           0X70C5
#define BMS_REG_ADDR_HEAT_CTRL                          0X70C7


//  ----------------------预留16 

#define BMS_REG_ADDR_CELL_UV_LV1                        0X70D9
#define BMS_REG_ADDR_CELL_UV_LV2                        0X70DA                    
#define BMS_REG_ADDR_CELL_UV_LV3                        0X70DB
#define BMS_REG_ADDR_CELL_OV_LV1                        0X70DC
#define BMS_REG_ADDR_CELL_OV_LV2                        0X70DD
#define BMS_REG_ADDR_CELL_OV_LV3                        0X70DE             
#define BMS_REG_ADDR_PACK_UV_LV1                        0X70DF   //H_BYTE
                                                                 //L_BYTE
#define BMS_REG_ADDR_PACK_OV_LV1                        0X70E1   //H_BYTE
                                                                 //L_BYTE    
#define BMS_REG_ADDR_CELL_OT_LV1                        0X70E3      
#define BMS_REG_ADDR_CELL_OT_LV2                        0X70E4              
#define BMS_REG_ADDR_CELL_OT_LV3                        0X70E5   
#define BMS_REG_ADDR_CELL_UT_LV1                        0X70E6               

#define BMS_REG_ADDR_MOS_OT_LV1                         0X70E7         	               
#define BMS_REG_ADDR_MOS_OT_LV2       	                0X70E8
#define BMS_REG_ADDR_MOS_OT_LV3                         0X70E9
#define BMS_REG_ADDR_DSG_OC_LV1       	                0X70EA
#define BMS_REG_ADDR_DSG_OC_LV2       	                0X70EB
#define BMS_REG_ADDR_DSG_OC_LV3       	                0X70EC
#define BMS_REG_ADDR_CHG_OC_LV1       	                0X70ED
#define BMS_REG_ADDR_CHG_OC_LV2       	                0X70EE
#define BMS_REG_ADDR_CHG_OC_LV3       	                0X70EF
#define BMS_REG_ADDR_CELL_TEMP_DIFF                     0X70F0
#define BMS_REG_ADDR_CELL_VOLT_DIFF                     0X70F1
#define BMS_REG_ADDR_SOH_LOW_LV1       	                0X70F2         
#define BMS_REG_ADDR_SOH_LOW_LV2       	                0X70F3      
#define BMS_REG_ADDR_CLEAN_HISTORY_ERR       	        0X70F4    
#define BMS_REG_ADDR_FORCE_OCV             	            0X70F5  



typedef enum
{
    EN_SLAVE_POLLING_STATUS_IDLE=0,
    EN_SLAVE_POLLING_STATUS_WAITING_REV,
    EN_SLAVE_POLLING_STATUS_REV_OVERTIME,
    EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT,
}EN_BMS_POLLING_STATUS;

#pragma pack(2)


#pragma pack(2)
typedef struct _bms_front_para_stru_t
{
    uint16_t        mode_id;
    uint16_t        sn[10];
    uint16_t        hw_version;
    uint16_t        sw_version;
    uint16_t        reserved[17];
    uint16_t        cell_voltage[20];
    uint16_t        cell_max_voltage;
    uint16_t        cell_min_voltage;
    uint16_t        cell_average_voltage;
    uint16_t        cell_total_voltage[2];
    uint16_t        cell_current[2];
    uint16_t        cell_power;
    uint16_t        reserved1[17];//



    uint16_t        soc_real;
    uint16_t        soc_act;
    uint16_t        soc_rpt;
    uint16_t        rated_capacity[2];
    uint16_t        current_max_capa[2];
    uint16_t        pack_soh;
    uint16_t        pack_cycle;     //0x7053


    uint16_t        total_chg_capacity[2];
    uint16_t        total_dsg_capacity[2];
    uint16_t        current_max_chg_current[2];
    uint16_t        current_max_dsg_current_10s[2];
    uint16_t        current_max_dsg_current_30s[2];
    uint16_t        current_max_dsg_current_60s[2];
    uint16_t        cell_sleep_time[2];
    uint16_t        total_chg_capacity_wh[2];
    uint16_t        total_dsg_capacity_wh[2];  
    uint16_t        pack_status;      //
    uint16_t        recharge_time[2];   
    uint16_t        mos_status;         //7069  
    uint16_t        exp_num;            //
    uint16_t        reserve2[17];
}BMSFrontPara_t;

typedef struct _bms_back_para_stru_t
{
    int16_t         cell_temperature[20]; //0x707C
    int16_t         cell_max_temperature;
    int16_t         cell_min_temperature;
    int16_t         cell_average_temperature;
    int16_t         afe_temperature;
    int16_t         mos_temperature;
    int16_t         ambient_temperature;
    uint16_t        balance_status;
    uint16_t        reserved3[17];

    uint16_t        mos_error_code;//2180
    uint16_t        current_error_code;
    uint16_t        cell_error_code1;
    uint16_t        cell_error_code2;
    uint16_t        pre_error_code;
    uint16_t        cell_sampling_error_code;
    uint16_t        other_error_code;
    uint16_t        reserved4[17];   //0x70af --0x70bf
}BMSBackPara_t;

typedef struct _bms_hold_para_stru_t
{
    uint16_t        dsg_mos_ctrl;       //70C0
    uint16_t        chg_mos_ctrl;
    uint16_t        pre_dsg_mos_ctrl;
    uint16_t        limit_curr_ctrl;
    uint16_t        bms_shutdown;
    uint16_t        rtc_time[2];        //70c5-70c6
    uint16_t        heat_ctrl;
    uint16_t        reserved5[17];

    uint16_t        cell_uv_lv1;//70d9
    uint16_t        cell_uv_lv2;
    uint16_t        cell_uv_lv3;
    uint16_t        cell_ov_lv1;
    uint16_t        cell_ov_lv2;
    uint16_t        cell_ov_lv3;
    uint16_t        pack_total_uv[2];
    uint16_t        pack_total_ov[2];
    uint16_t        cell_ot_lv1;
    uint16_t        cell_ot_lv2;
    uint16_t        cell_ot_lv3;
    uint16_t        cell_ut_lv1;
		
    uint16_t        mos_ot_lv1;	        //70E7
    uint16_t        mos_ot_lv2;
    uint16_t        mos_ot_lv3;
    uint16_t        dsg_oc_lv1;		    //放电过流一级
    uint16_t        dsg_oc_lv2;
    uint16_t        dsg_oc_lv3;
    uint16_t        chg_oc_lv1;	    	//充电过流一级
    uint16_t        chg_oc_lv2;
    uint16_t        chg_oc_lv3;
    uint16_t        cell_temp_diff;     //0X70F0
    uint16_t        cell_volt_diff;
    uint16_t        soh_low_lv1;
    uint16_t        soh_low_lv2;
    uint16_t        error_clean;        //70f4
    uint16_t        force_ocv;          //70f5
    uint16_t        reserved6[17];      //
}BMSHoldPara_t;



typedef struct _bms_para_stru_t
{
    BMSFrontPara_t  bms_front_para;
    BMSBackPara_t   bms_back_para;
    BMSHoldPara_t   bms_hold_para;   
}BMSPara_t;
#pragma pack()

typedef enum
{
    EN_MODBUS_SLAVE_COMM_STATUS_OK,
    EN_MODBUS_SLAVE_COMM_STATUS_NOACK,
    EN_MODBUS_SLAVE_COMM_STATUS_ACK_ERROR,
    EN_MODBUS_SLAVE_COMM_STATUS_DISORDER
}EN_MODBUS_SLAVE_COMM_STATUS;

typedef enum
{
    EN_SLAVE_NO_ERROR = 0,
    EN_SLAVE_ERROR_ILLEGAL_FUNC_CODE = 1,
    EN_SLAVE_ERROR_ILLEGAL_REG_ADDR = 2,
    EN_SLAVE_ERROR_ILLEGAL_WRITE_DATA = 3,
    EN_SLAVE_ERROR_HITCH = 4
}EN_SLAVE_ERROR_CODE;




typedef struct _bms_modbus_slave_para_stru_t
{
    BMSPara_t                       bms_para[BAT_NUM_MAX];
    uint8_t                         bms_comm_err_cnt[BAT_NUM_MAX];
    uint8_t                         bms_com_time_out[BAT_NUM_MAX];
}BmsModBusSlavePara_t;

typedef enum
{
    EN_BMS_READ_EVENT_FRONT,
    EN_BMS_READ_EVENT_BACK,
    EN_BMS_HOLD_EVENT,

    EN_BMS_READ_EVENT_ADD1_FRONT,
    EN_BMS_READ_EVENT_ADD1_BACK,
    EN_BMS_HOLD_EVENT_ADD1,

    EN_BMS_READ_EVENT_ADD2_FRONT,
    EN_BMS_READ_EVENT_ADD2_BACK,
    EN_BMS_HOLD_EVENT_ADD2,

    EN_BMS_READ_EVENT_ADD3_FRONT,
    EN_BMS_READ_EVENT_ADD3_BACK,
    EN_BMS_HOLD_EVENT_ADD3,

    EN_BMS_READ_EVENT_NUM
}EN_BMS_READ_EVENT;




#pragma pack(2)


extern BmsModBusSlavePara_t gBmsSlavePara;

void SlaveBmsCommInit(void);
void SlaveBmsModbusCommProcess(void);
bool BMSDsgMosCtl(uint8_t mode);
bool BMSChgMosCtl(uint8_t mode);
bool BMSPreDsgMosCtl(uint8_t mode);
bool BMSLimitCurrCtrl(uint8_t mode);
bool BMSSetShutDown(uint8_t status);
bool BMS_RtcTimeSet(uint32_t rtc_time);
bool BMSHeatCtrl(uint8_t mode);
bool BMSSetCellUvLevel1(uint16_t val);
bool BMSSetCellUvLevel2(uint16_t val);
bool BMSSetCellUvLevel3(uint16_t val);
bool BMSSetCellOvLevel1(uint16_t val);
bool BMSSetCellOvLevel2(uint16_t val);
bool BMSSetCellOvLevel3(uint16_t val);
bool BMSSetPackUvLevel1(uint32_t val);
bool BMSSetPackOvLevel1(uint32_t val);
bool BMSSetCellOtLevel1(int16_t val);
bool BMSSetCellOtLevel2(int16_t val);
bool BMSSetCellOtLevel3(int16_t val);
bool BMSSetCellUtLevel1(int16_t val);
bool BMSMosOtLevel1(int16_t val);
bool BMSMosOtLevel2(int16_t val);
bool BMSMosOtLevel3(int16_t val);
bool BMSDsgOcLevel1(uint16_t val);
bool BMSDsgOcLevel2(uint16_t val);
bool BMSDsgOcLevel3(uint16_t val);
bool BMSChgOcLevel1(uint16_t val);
bool BMSChgOcLevel2(uint16_t val);
bool BMSChgOcLevel3(uint16_t val);
bool BMSCellTempDiff(uint16_t val);
bool BMSCellBatVoltDiff(uint16_t val);
bool BMSSohLowLevel1(uint16_t val);
bool BMSSohLowLevel2(uint16_t val);
bool BMS_CleanHistoryErr(uint16_t val);
bool BMSForceOcv(uint16_t val);
bool CollectWriteBmsFun(uint16_t reg_addr,uint16_t reg_num,uint8_t *pdata,uint16_t slave_addr);
void GetBmsInputFrontData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num);
void GetDataFromBms(uint8_t *pdata,uint16_t data_num,uint8_t batt_num);
void GetBmsInputBackData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num);
void GetBmsHoldData(uint8_t *pdata,uint16_t data_num,uint8_t batt_num);
#endif





















