
#ifndef __SLAVE_BMS_H_
#define __SLAVE_BMS_H_

#include "dev_modbus.h"
#include "bsp.h"
#include "dev_udp_comm.h"


#define MAX_PACK_NUM                                    (BMS_MAX_NUM)

#define SET_ENTER_UPGRADE_MODE                          (0x0001)
#define SET_EXIT_UPGRADE_MODE                           (0x0000)

#define STOP_PRE_DSG                                    (2)
#define RELEASE_PRE_DSG                                 (0)
#define DCDC_DOWN_SWITCH_ON                             (1)

#define UDP_SLAVE_TYPE_BMS                              (0x0003      )
#define UDP_READ_BMS_BASE_ADDR                          (0X30010000ul)
#define UDP_READ_DCDC_BASE_ADDR                         (0X30020000ul)


/********************** 寄存器起始/终止地址 *******************************/
#define BMS_REG_INPUT_START_ADDR                        (0X0000)
#define BMS_REG_INPUT_END_ADDR                          (0X00DF)
#define BMS_REG_HOLD_START_ADDR                         (0X00E0)
#define BMS_REG_HOLD_END_ADDR                           (0x00FF)

#define DCDC_REG_INPUT_START_ADDR                       (0X0000)
#define DCDC_REG_INPUT_END_ADDR                         (0X008F)
#define DCDC_REG_HOLD_START_ADDR                        (0X0090)
#define DCDC_REG_HOLD_END_ADDR                          (0x00FF)

#define BMS_DATA_NUM                                   (BMS_REG_HOLD_END_ADDR - BMS_REG_INPUT_START_ADDR + 1)//1024   //?????  地址要改
#define DCDC_DATA_NUM                                  (DCDC_REG_HOLD_END_ADDR - DCDC_REG_INPUT_START_ADDR + 1)

/************************** 寄存器地址 ***********************************/

#define BMS_REG_ADDR_RTC_TIME                           (0X00ED)
#define BMS_REG_ADDR_OCV_CALI                           (UDP_READ_BMS_BASE_ADDR + 0X00EF)

#define UDP_BMS_SHUTDOWN_ADDR                           (UDP_READ_BMS_BASE_ADDR + 0X00EC)   
#define UDP_BMS_PRE_STOP_CTRL_ADDR                      (UDP_READ_BMS_BASE_ADDR + 0X00E8)   //?????没有用
#define BMS_REG_ADDR_DSP_WAKEUP_STATUS                  (UDP_READ_BMS_BASE_ADDR + 0X00E9)   //?????没有用
#define BMS_REG_ADDR_ALL_PACK_AVE_VOLT_STATUS           (UDP_READ_BMS_BASE_ADDR + 0x00F0)
#define UDP_DOWN_SWITCH_ADDR                            (UDP_READ_DCDC_BASE_ADDR + 0X00A1)
#define UDP_DCDC_IP_ADDR_ADDR                           (UDP_READ_DCDC_BASE_ADDR + 0X0090)
#define UDP_DCDC_PCS_TYPE_ADDR                          (UDP_READ_DCDC_BASE_ADDR + 0X0092)
#define UPD_DCDC_BANLANCE_EN                            (UDP_READ_DCDC_BASE_ADDR + 0X00A3)
#define BMS_REG_ADDR_SYS_UPGRADE_STATUS                 (UDP_READ_DCDC_BASE_ADDR + 0x00A4)
#define UDP_MOOD_LED_CLOUR_ADDR                         (UDP_READ_DCDC_BASE_ADDR + 0X0093)



typedef enum
{
    EN_SLAVE_POLLING_STATUS_IDLE = 0,
    EN_SLAVE_POLLING_STATUS_WAITING_REV,
    EN_SLAVE_POLLING_STATUS_REV_OVERTIME,
    EN_SLAVE_POLLING_STATUS_REV_INTERVAL_WAIT,
} EN_BMS_POLLING_STATUS;


/*******************************************************************************/
/*****************************PACK结构体**************************************/
#pragma pack(1)
/*********************BMS信息************************/
/*----BMS基础信息---*/
typedef struct
{
    uint16_t    model_id;               //7000
    char        sn[20];                 //7001~700A
    uint16_t    hw_ver;                 //700B
    uint16_t    sw_ver[2];              //700C~700D
    char        sw_ver_ext[24];         //700E~7019
    uint16_t    reserve[6];             //701D~701F
} BmsBaseInfo_t;

/*----BMS状态信息---*/
typedef struct
{
    uint16_t    addr;               //7020
    uint16_t    work_status;        //7021
    uint16_t    total_vol;          //7022
    int16_t     total_curr;         //7023
    uint16_t    total_power;        //7024
    uint16_t    soc_real;           //7025
    uint16_t    soc_disp;           //7026
    uint16_t    soh;                //7027
    int16_t     max_chg_allow_curr; //7028
    int16_t     max_dsg_allow_curr; //7029
    uint16_t    reserve;            //702A
    uint16_t    design_cap;         //702B  设计容量(额定容量)
    uint16_t    total_cap;          //702C  当前容量
    uint16_t    residual_cap;       //702D     剩余容量
    uint16_t    cycle_count;        //702E
    uint16_t    accu_chg_energy[2]; //702F~7030
    uint16_t    accu_dsg_energy[2]; //7031~7032
    uint16_t    heat_status;        //7033
    uint16_t    mos_status;         //7034
    uint16_t    cell_max_vol;       //7035
    uint16_t    cell_min_vol;       //7036
    uint16_t    cell_ave_vol;       //7037
    int16_t     cell_max_temp;      //7038
    int16_t     cell_min_temp;      //7039
    int16_t     cell_ave_temp;      //703A
    int16_t     mos_max_temp;       //703B
    int16_t     mos_min_temp;       //703C
    int16_t     mos_ave_temp;       //703D
    int16_t     pack_max_temp;      //703E
    int16_t     pack_min_temp;      //703F
    int16_t     pack_ave_temp;      //7040
    uint16_t    balance_status[2];  //7041~7042
    uint16_t    flt_level;          //7043          故障等级
    uint16_t    reserve3[12];       //7044~704F 
} BmsStatusInfo_t;

/*----BMS阈值信息---*/
typedef struct
{
    uint16_t    pack_ov_limit[2];      //7050~7051
    uint16_t    pack_uv_limit[2];      //7052~7053
    uint16_t    cell_ov_limit_1;       //7054
    uint16_t    cell_ov_limit_2;       //7055
    uint16_t    cell_ov_limit_3;       //7056
    uint16_t    cell_uv_limit_1;       //7057
    uint16_t    cell_uv_limit_2;       //7058
    uint16_t    cell_uv_limit_3;       //7059
    int16_t     cell_ot_limit_1;       //705A
    int16_t     cell_ot_limit_2;       //705B
    int16_t     cell_ot_limit_3;       //705C
    int16_t     cell_ut_limit1;        //705D 
    int16_t     cell_ut_limit2;        //705e
    int16_t     cell_ut_limit3;        //705f
    int16_t     mos_ot_limit_1;        //7060
    int16_t     mos_ot_limit_2;        //7061 
    int16_t     mos_ot_limit_3;        //7062
    uint16_t    reserve;               //7063
    uint16_t    dsg_oc_limit_1;        //7064
    uint16_t    dsg_oc_limit_2;        //7065
    uint16_t    dsg_oc_limit_3;        //7066
    uint16_t    chg_oc_limit_1;        //7067
    uint16_t    chg_oc_limit_2;        //7068
    uint16_t    chg_oc_limit_3;        //7069
    int16_t     reserve2[3];           //706A~706C
    int16_t     cell_vol_diff_limit;   //706D
    int16_t     cell_temp_diff_limit;  //706E
    int16_t     soh_under_limit_1;     //706F
    int16_t     soh_under_limit_2;     //7070
    int16_t     soh_under_limit_3;     //7071
    uint16_t    reserve3[14];          //7072~707F 
} BmsLimitInfo_t;

/*----BMS单体电芯信息---*/
typedef struct
{
    uint16_t    cell_vol[16];       //7080~708F
    uint16_t    cell_temp[16];      //7090~709F
    uint16_t    reserve[16];        //70A0~70AF 
} BmsCellInfo_t;

/*----BMS故障信息---*/
typedef struct
{
    uint16_t    mos_error;             //70B0
    uint16_t    temp_error;            //70B1
    uint16_t    curr_error;            //70B2
    uint16_t    vol_error;             //70B3
    uint16_t    sox_error;             //70B4
    uint16_t    other_error;           //70B5
    uint16_t    reserve_error[3];      //70B6~61B7
                                         
    uint16_t    mos_alarm;             //70B9
    uint16_t    temp_alarm;            //70BA
    uint16_t    curr_alarm;            //70BB
    uint16_t    vol_alarm;             //70BC
    uint16_t    sox_alarm;             //70BD
    uint16_t    other_alarm;           //70BE
    uint16_t    reserve_alarm[3];      //70BF~61C1
                                         
    uint16_t    mos_notice;            //70C2
    uint16_t    temp_notice;           //70C3
    uint16_t    curr_notice;           //70C4
    uint16_t    vol_notice;            //70C5
    uint16_t    sox_notice;            //70C6
    uint16_t    other_notice;          //70C7
    uint16_t    reserve_notice[3];     //70C8~61CA

} BmsErrorInfo_t;

/*----BMS预留信息---*/
typedef struct
{
    uint16_t    reserve[21];        //70CB~70DF 
} BmsReserveInfo_t;

/*----BMS控制信息---*/
typedef struct
{
    uint16_t    heat_ctrl;          //70E0
    uint16_t    dsg_mos_ctrl;       //70E1
    uint16_t    chg_mos_ctrl;       //70E2
    uint16_t    pre_dsg_mos_ctrl;   //70E3
    uint16_t    curr_limit_ctrl;    //70E4
    uint16_t    reserve[4];         //70E5~70E8
    uint16_t    upgrade_status;     //70E9
    uint16_t    reset_ctrl;         //70EA
    uint16_t    inv_factory_reset;  //70EB
    uint16_t    shutdown;           //70EC
    uint16_t    rtc_set[2];         //70ED~70EE
    uint16_t    ocv_cali;           //70EF 
    uint16_t    all_pack_ave_volt;  //70F0
    uint16_t    reserve2[15];       //70F1~70FF
} BmsControlInfo_t;

/*----BMS信息----*/
typedef struct
{
    BmsBaseInfo_t       base_info;
    BmsStatusInfo_t     status_info;
    BmsLimitInfo_t      limit_info;
    BmsCellInfo_t       cell_info;
    BmsErrorInfo_t      error_info;
    BmsReserveInfo_t    reserve_info;
    BmsControlInfo_t    ctrl_info;
} BmsInfo_t;

/*********************DCDC信息************************/
/*----DCDC基础信息----*/
typedef struct
{
    uint16_t    model_id;               //7000
    char        sn[20];                 //7001~700A
    uint16_t    hw_ver;                 //700B
    uint16_t    sw_ver[2];              //700C~700D
    char        sw_ver_ext[24];         //700E~7019
    uint16_t    reserve[6];             //701D~701F
} DcdcBaseInfo_t;

/*----DCDC功率信息----*/
typedef struct
{
    uint16_t    output_vol;
    uint16_t    output_curr;
    uint16_t    output_power;
    uint16_t    llc_1_output_vol;
    uint16_t    llc_1_output_curr;
    uint16_t    llc_2_output_vol;
    uint16_t    llc_2_output_curr;
    uint16_t    buck_boost_1_output_vol;
    uint16_t    buck_boost_1_output_curr;
    uint16_t    buck_boost_2_output_vol;
    uint16_t    buck_boost_2_output_curr;
    uint16_t    reserve2[21];
} DcdcPowerInfo_t;

/*----DCDC状态信息----*/
typedef struct
{
    uint16_t    fan_1_speed;
    uint16_t    fan_2_speed; 
    uint16_t    reserve[2];    
    uint16_t    heat_sink_temp_1;
    uint16_t    heat_sink_temp_2;
    uint16_t    reserve2[2];
    uint16_t    dcdc_work_status;
    uint16_t    pcs_type;
    uint16_t    reserve3[22];
} DcdcStatusInfo_t;

/*----DCDC故障信息----*/
typedef struct
{
    uint16_t    volt_error;
    uint16_t    curr_error;
    uint16_t    power_other_error;
    uint16_t    reserve_error[2];
    uint16_t    power_alarm;
    uint16_t    other_alarm;
    uint16_t    reserve_alarm[3];
    uint16_t    power_notice;
    uint16_t    other_notice;
    uint16_t    reserve_notice[4];
} DcdcErrorInfo_t;

/*----DCDC预留信息段----*/
typedef struct
{
    uint16_t    reserve[32];
} DcdcReverseInfo_t;

/*----DCDC控制信息----*/
typedef struct
{
    uint16_t    ip_addr[2];
    uint16_t    pcs_type_set;
    uint16_t    led_dir_r;
    uint16_t    led_g_b;
    uint16_t    led_mode;
    uint16_t    led_period;
    uint16_t    soft_reboot;
    uint16_t    reserve[8];
    uint16_t    shutdown;
    uint16_t    forward_status;         //                  下行通讯端口状态
    uint16_t    dcdc_output_ctrl;
    uint16_t    pack_balance_ctrl;
    uint16_t    upgrade_status;
    uint16_t    fault_clear;
    uint16_t    reserve2[10];
} DcdcCtrlInfo_t;

/*----DCDC预留信息段2----*/
typedef struct
{
    uint16_t    reserve[88];
} DcdcReverse2Info_t;
/*----DCDC信息----*/
typedef struct
{
    DcdcBaseInfo_t       base_info;
    DcdcPowerInfo_t      power_info;
    DcdcStatusInfo_t     status_info;
    DcdcErrorInfo_t      error_info;
    DcdcReverseInfo_t    reserve_info;
    DcdcCtrlInfo_t       ctrl_info;
    DcdcReverse2Info_t   reserve2_info;
} DcdcInfo_t;

/*********************Pack信息************************/
/*----Pack信息----*/
typedef struct
{
    BmsInfo_t       bms_info;
    DcdcInfo_t      dcdc_info;
} PackInfo_t;

#pragma pack()
/*******************************************************************************/
/*******************************************************************************/

typedef union _bms_ctrl_bit_info_u
{
    struct
    {
        uint16_t f_addring_over: 1;
        uint16_t f_addring_ok: 1;
        uint16_t recv_flag: 1;
        uint16_t mood_led_ctrl: 1;
        uint16_t polling_bms_data:1; //第一次完成数据读取
    } bits;
    uint16_t word_value;
} BmsCtrlBitValue_u;

typedef struct _bms_ctrl_info_t
{
    BmsCtrlBitValue_u bms_ctrl_info_value;
    uint8_t pack_offline_count[BMS_MAX_NUM];  //离线标志计数
    uint8_t pack_offline[BMS_MAX_NUM];        //离线标志
    uint8_t batt_num;

} BmsCtrlInfo_t;

#pragma pack()

typedef enum
{
    EN_MODBUS_SLAVE_COMM_STATUS_OK,
    EN_MODBUS_SLAVE_COMM_STATUS_NOACK,
    EN_MODBUS_SLAVE_COMM_STATUS_ACK_ERROR,
    EN_MODBUS_SLAVE_COMM_STATUS_DISORDER
} EN_MODBUS_SLAVE_COMM_STATUS;

typedef enum
{
    EN_SLAVE_NO_ERROR = 0,
    EN_SLAVE_ERROR_ILLEGAL_FUNC_CODE = 1,
    EN_SLAVE_ERROR_ILLEGAL_REG_ADDR = 2,
    EN_SLAVE_ERROR_ILLEGAL_WRITE_DATA = 3,
    EN_SLAVE_ERROR_HITCH = 4
} EN_SLAVE_ERROR_CODE;




typedef struct _bms_modbus_slave_para_stru_t
{
    PackInfo_t                      pack_para[MAX_PACK_NUM]; 
    uint8_t                         bms_comm_err_cnt[MAX_PACK_NUM];
    uint8_t                         bms_com_time_out[MAX_PACK_NUM];
} PackSlavePara_t;

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
    
    EN_BMS_READ_EVENT_ADD4_FRONT,
    EN_BMS_READ_EVENT_ADD4_BACK,
    EN_BMS_HOLD_EVENT_ADD4,
    
    EN_BMS_READ_EVENT_ADD5_FRONT,
    EN_BMS_READ_EVENT_ADD5_BACK,
    EN_BMS_HOLD_EVENT_ADD5,

    EN_BMS_READ_EVENT_NUM
} EN_BMS_READ_EVENT;




#pragma pack(2)
//extern  PackPara_t      PackInfo[MAX_PACK_NUM];//
//extern  PackInfo_t      PackNowInfo[MAX_PACK_NUM];

void SlaveBmsCommInit(void);
void SlaveBmsModbusCommProcess(void);
bool BMSShutDownSend(void);
bool BMS_RtcTimeSet(uint32_t rtc_time);
bool SetBmsSysUpgradeStatus(uint16_t status);
bool SetFirstPackRest(uint16_t status);
bool SetDspWakeUpStatusToBms(uint8_t status);
bool CollectWriteBmsFun(uint16_t reg_addr, uint16_t reg_num, uint8_t *pdata, uint16_t bms_index, uint8_t using_mode);
bool CollectWriteDcdcFun(uint16_t reg_addr, uint16_t reg_num, uint8_t *pdata, uint16_t bms_index, uint8_t using_mode);
void SetSysEnterTestMode(uint8_t test_mode);

extern PackSlavePara_t gstPackSlavePara;
extern BmsCtrlInfo_t    BmsCtrlInfo;
void BmsUdpParaInit(void);
void BmsCommProc(void);
void BmsAddrFun(void);
void TestSendBmsUdpData(void);
uint8_t GetBmsNum(void);

void BroadCastByteDataToBms(uint32_t data_addr, uint16_t data_len, uint8_t *pdata); //data_len 为寄存器个数
void BroadCastWordDataToBms(uint32_t data_addr, uint16_t data);
bool BmsMoodLedCtrl(uint32_t clour, uint16_t mode, uint16_t period);
bool SetBmsChgBlanceCtrl(uint8_t min_id);
bool ResetBmsChgBlanceCtrl(uint8_t min_id);
bool SetBmsDsgBlanceCtrl(uint8_t max_id);
bool ResetBmsDsgBlanceCtrl(uint8_t max_id);
bool SetBmsBlanceDisable(void);
void SetDcdcPcsType(uint16_t pcs_type);
bool SetBmsAllPackAveVolt(uint16_t total_ave_volt);


#endif





















